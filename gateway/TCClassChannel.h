// ICD のクラス1つぶんの CChannel。**型が現れるのはここから内側だけ。**
//
// 生成コーデックを呼ぶのも、HLA 側の継ぎ目に触るのもこのクラスで、外へは CChannel の抽象しか
// 出ない。クラスを増やしても増えるのは実体化の数であって、周期ループ側のコードではない。
//
// FOM に無い独自データ（相手が決めた形式）は TCRawChannel が受け持つ。こちらは ICD の枠 —
// 12バイトヘッダ + 固定長レコード — に乗るものだけ。

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "../hla/TCFromHla.h"
#include "../hla/TCToHla.h"
#include "../icd/icd_codec.h"
#include "../net/CUdpSocket.h"
#include "CChannel.h"
#include "TClassBinding.h"
#include "TClassKind.h"
#include "TCPublisher.h"
#include "TCSubscriber.h"

namespace gw {

template <class T>
class TCClassChannel final : public CChannel {
public:
    /// fromHla が null なら送信しない、toHla が null なら受けたものを捨てる。
    /// 実際のフェデレーションでも publish だけ / subscribe だけのクラスはあるので、
    /// 両方揃っていることを前提にしない。
    ///
    /// **どちらも借りているだけ**で、実体の寿命はこのチャネルより長くなければならない。
    /// 配線側（app/CWiring.h）が値で持っているのはそのため。
    TCClassChannel(const TClassBinding& bind,
                 hla::TCFromHla<T>* fromHla,
                 hla::TCToHla<T>* toHla)
        : m_bind(bind), m_name(withoutRoot(bind.fomName)), m_fromHla(fromHla), m_toHla(toHla),
          m_pub(bind.classId, bind.payload), m_sub(bind.classId, bind.payload) {}

    [[nodiscard]] const char* name() const noexcept override { return m_name; }
    [[nodiscard]] std::uint16_t port() const noexcept override { return m_bind.port; }
    [[nodiscard]] std::size_t payload() const noexcept override { return m_bind.payload; }
    [[nodiscard]] const char* kindLabel() const noexcept override {
        return m_bind.kind == TClassKind::Object ? "オブジェクト" : "インタラクション";
    }
    [[nodiscard]] CUdpSocket& socket() noexcept override { return m_sock; }

    [[nodiscard]] bool open(const std::string& peerHost) override {
        return m_sock.open(m_bind.port, peerHost, m_bind.port);
    }

    /// 毎周期、渡されたものを全部出す。クラスごとの間引きは無い — 周期は tick() を叩く側が
    /// 決めていて、それが全クラスの送信レート。
    std::size_t pumpOut() override {
        if (m_fromHla == nullptr || !m_sock.canSend()) return 0;

        // オブジェクトは前回の残りが既に古い。撮り直す前に捨てる。
        // インタラクションは1件ずつ意味があるので、残っているぶんの後ろに足す。
        if (m_bind.kind == TClassKind::Object) m_outbox.clear();
        m_fromHla->drain(m_outbox);
        // ここで m_backlog を戻しておくこと。オブジェクトが上の clear() で残りを捨てた周期は
        // ここを通って抜けるので、書き直さないと**捨てたはずの件数を積み残しとして
        // 報告し続ける**。実際に消えているのに「まだ手元にある」と読める表示になる。
        if (m_outbox.empty()) { m_backlog = 0; return 0; }

        // 件数の上限は設けない。**1周期に渡されたものはその周期で出し切る。**
        // 止まるのはソケットが受け付けなかったときだけで、そのとき残った分が持ち越しになる。
        //
        // 以前は「1周期あたり8データグラム」で切っていた。それはイベント（インタラクション）が
        // 束で来る場合を想定した制限だったが、**オブジェクトには有害だった**: 次の周期の頭で
        // 残りを捨てる設計なので、上限を超えた末尾が毎周期おなじように落ち続け、
        // インスタンス数が上限を超えたフェデレーションでは末尾が永久に送られなかった。
        //
        // **出せた件数は TCPublisher に数えさせること。publish が true を返した回数ではない。**
        // publish はレコードをデータグラムに積んだ時点で true を返すが、積まれたぶんが
        // 実際に出るのは flush のときで、そこで断られると writer ごと捨てられる。
        // true の回数を「送った件数」として outbox から消すと、**持ち越すはずのイベントが
        // 毎回きっかり1データグラムぶん静かに消える**（500件のバーストで 434件しか届かない）。
        // recordsSent() は送信が成功したときしか増えないので、これが唯一の正しい件数になる。
        const std::uint64_t before = m_pub.recordsSent();
        for (const T& record : m_outbox) {
            if (!m_pub.publish(record, m_sock)) break;
        }

        // 周期の終わりに必ず出し切る。次の周期まで抱えると、その1周期ぶん遅れる。
        (void)m_pub.flush(m_sock);

        // 出たのは outbox の先頭から連続したぶんなので、その件数だけ削れば順序は保たれる。
        const std::size_t sent = static_cast<std::size_t>(m_pub.recordsSent() - before);
        m_outbox.erase(m_outbox.begin(), m_outbox.begin() + static_cast<std::ptrdiff_t>(sent));
        if (!m_outbox.empty()) {
            ++m_deferrals;
            m_backlog = m_outbox.size();
        } else {
            m_backlog = 0;
        }
        return sent;
    }

    std::size_t pumpIn() override {
        const long got = m_sub.drain(m_sock, [this](const T& rec) {
            if (m_toHla != nullptr) m_toHla->accept(rec);
        });
        return got < 0 ? 0 : static_cast<std::size_t>(got);
    }

    [[nodiscard]] std::uint64_t sentTotal() const noexcept override {
        return m_pub.recordsSent();
    }
    [[nodiscard]] std::size_t backlog() const noexcept override { return m_backlog; }
    [[nodiscard]] std::uint64_t deferrals() const noexcept override { return m_deferrals; }
    [[nodiscard]] const TSubscriberStats& inStats() const noexcept override {
        return m_sub.stats();
    }
    [[nodiscard]] const std::string& lastError() const noexcept override {
        return m_sock.lastError();
    }

    [[nodiscard]] std::size_t recordSize() const noexcept override {
        return icd::fixedSize<T>;
    }
    [[nodiscard]] std::size_t capacityInRecords() const noexcept override {
        return m_pub.capacityInRecords();
    }

private:
    /// "HLAobjectRoot.EmitterBeam.RadarBeam" -> "EmitterBeam.RadarBeam"。
    /// 根の名前はどのクラスにも付いていて、表示では何も区別しない。以前は CGateway が表示のたびに
    /// やっていたが、FOM 名であることを知っているのはこのクラスだけなので、ここで1度だけ行う。
    static const char* withoutRoot(const char* fomName) noexcept {
        const char* dot = std::strchr(fomName, '.');
        return dot ? dot + 1 : fomName;
    }

    TClassBinding m_bind;            ///< このクラスの ID / ポート / 上限 / 種別（生成物の定数から）
    const char* m_name;             ///< 表示名。m_bind.fomName の根を除いた部分を指す
    hla::TCFromHla<T>* m_fromHla;     ///< HLA 側の供給元。借り物で、null なら送信しない
    hla::TCToHla<T>* m_toHla;         ///< HLA 側の受け口。借り物で、null なら受信を捨てる
    CUdpSocket m_sock;               ///< このクラス専用のソケット（送受信とも1本）
    TCPublisher<T> m_pub;             ///< レコード → データグラム
    TCSubscriber<T> m_sub;            ///< データグラム → レコード
    std::vector<T> m_outbox;        ///< 送信待ちのレコード。インタラクションは次の周期へ持ち越す
    std::size_t m_backlog = 0;      ///< 出し切れず残った件数
    std::uint64_t m_deferrals = 0;  ///< 出し切れなかった周期の回数
};

}  // namespace gw
