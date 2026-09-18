// Channel の唯一の実装。**型が現れるのはここから内側だけ。**
//
// 生成コーデックを呼ぶのも、HLA 側の継ぎ目に触るのもこのクラスで、外へは Channel の抽象しか
// 出ない。クラスを増やしても増えるのは実体化の数であって、周期ループ側のコードではない。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../hla/FromHla.h"
#include "../hla/ToHla.h"
#include "../icd/icd_codec.h"
#include "../net/UdpSocket.h"
#include "Channel.h"
#include "ClassBinding.h"
#include "Delivery.h"
#include "Publisher.h"
#include "Subscriber.h"

namespace gw {

template <class T>
class ClassChannel final : public Channel {
public:
    /// fromHla が null なら送信しない、toHla が null なら受けたものを捨てる。
    /// 実際のフェデレーションでも publish だけ / subscribe だけのクラスはあるので、
    /// 両方揃っていることを前提にしない。
    ///
    /// **どちらも借りているだけ**で、実体の寿命はこのチャネルより長くなければならない。
    /// 配線側（app/Wiring.h）が値で持っているのはそのため。
    ClassChannel(const ClassBinding& bind,
                 hla::FromHla<T>* fromHla,
                 hla::ToHla<T>* toHla)
        : m_bind(bind), m_fromHla(fromHla), m_toHla(toHla),
          m_pub(bind.classId, bind.payload), m_sub(bind.classId, bind.payload) {}

    [[nodiscard]] const ClassBinding& binding() const noexcept override { return m_bind; }
    [[nodiscard]] UdpSocket& socket() noexcept override { return m_sock; }

    [[nodiscard]] bool open(const std::string& peerHost) override {
        return m_sock.open(m_bind.port, peerHost, m_bind.port);
    }

    /// 毎周期、渡されたものを全部出す。クラスごとの間引きは無い — 周期は tick() を叩く側が
    /// 決めていて、それが全クラスの送信レート。
    std::size_t pumpOut() override {
        if (m_fromHla == nullptr || !m_sock.canSend()) return 0;

        // スナップショットは前回の残りが既に古い。撮り直す前に捨てる。
        // イベントは1件ずつ意味があるので、残っているぶんの後ろに足す。
        if (m_bind.delivery == Delivery::Snapshot) m_outbox.clear();
        m_fromHla->drain(m_outbox);
        if (m_outbox.empty()) return 0;

        // 件数の上限は設けない。**1周期に渡されたものはその周期で出し切る。**
        // 止まるのはソケットが受け付けなかったときだけで、そのとき残った分が持ち越しになる。
        //
        // 以前は「1周期あたり8データグラム」で切っていた。それはイベント（インタラクション）が
        // 束で来る場合を想定した制限だったが、**Snapshot には有害だった**: 次の周期の頭で
        // 残りを捨てる設計なので、上限を超えた末尾が毎周期おなじように落ち続け、
        // インスタンス数が上限を超えたフェデレーションでは末尾が永久に送られなかった。
        std::size_t sent = 0;
        while (sent < m_outbox.size() && m_pub.publish(m_outbox[sent], m_sock)) ++sent;

        // 周期の終わりに必ず出し切る。次の周期まで抱えると、その1周期ぶん遅れる。
        (void)m_pub.flush(m_sock);

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
    [[nodiscard]] const SubscriberStats& inStats() const noexcept override {
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
    ClassBinding m_bind;            ///< このクラスの ID / ポート / レート（ICD から写した値）
    hla::FromHla<T>* m_fromHla;     ///< HLA 側の供給元。借り物で、null なら送信しない
    hla::ToHla<T>* m_toHla;         ///< HLA 側の受け口。借り物で、null なら受信を捨てる
    UdpSocket m_sock;               ///< このクラス専用のソケット（送受信とも1本）
    Publisher<T> m_pub;             ///< レコード → データグラム
    Subscriber<T> m_sub;            ///< データグラム → レコード
    std::vector<T> m_outbox;        ///< 送信待ちのレコード。Events では次の周期へ持ち越す
    std::size_t m_backlog = 0;      ///< 出し切れず残った件数
    std::uint64_t m_deferrals = 0;  ///< 出し切れなかった周期の回数
};

}  // namespace gw
