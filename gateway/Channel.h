// 1クラス = 1ポートぶんの送受信。
//
// クラスごとにレコード型が違うので、周期ループから見えるのは Channel という型を持たない
// 抽象だけにしてある。型が要るのは ClassChannel<T> の内側 — つまり生成コーデックを呼ぶ
// 場所 — だけで、そこから外には出ない。

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../hla/Federate.h"
#include "../net/UdpSocket.h"
#include "ClassIds.h"
#include "Publisher.h"
#include "Subscriber.h"

namespace gw {

/// 1周期ぶんの内訳。型に依らないので Channel の外に出せる。
struct ChannelTick {
    std::size_t sentRecords = 0;
    std::size_t receivedRecords = 0;
    bool error = false;
};

class Channel {
public:
    virtual ~Channel() = default;

    [[nodiscard]] virtual const ClassBinding& binding() const noexcept = 0;
    [[nodiscard]] virtual UdpSocket& socket() noexcept = 0;

    /// 受信ポートを bind し、送信先を設定する。peerHost が空なら受信専用。
    [[nodiscard]] virtual bool open(const std::string& peerHost) = 0;

    /// HLA 側から出てきたぶんを送る。ブロックしない。
    virtual std::size_t pumpOut() = 0;

    /// 周期ループの周波数を伝える。ICD の Rate 列（クラスごとの更新レート）を
    /// 何周に1回送るかに変換するために要る。
    virtual void setLoopRate(unsigned loopHz) noexcept = 0;

    /// 来ているぶんを読み切って HLA 側へ渡す。ブロックしない。
    /// **poll が「読める」と言ったときだけ呼ぶこと。** 呼んでも害はないが、
    /// 空振りの recvfrom がポート数ぶん積み上がる。
    virtual std::size_t pumpIn() = 0;

    /// 1レコードのバイト数と、1データグラムに入る件数。どちらも固定長だから開く前に分かる。
    /// 件数が 0 なら **1件がペイロードに収まらない** — ジャンボフレームか、ICD の上限見直し。
    [[nodiscard]] virtual std::size_t recordSize() const noexcept = 0;
    [[nodiscard]] virtual std::size_t capacityInRecords() const noexcept = 0;

    [[nodiscard]] virtual std::uint64_t sentTotal() const noexcept = 0;

    /// 送り切れずに次の周期へ回した件数と、そうなった周期の回数。
    /// **Events で backlog が減らないなら、そのクラスは供給に追いついていない。**
    /// 上げるのは Rate か payload（＝1発の件数）で、放っておくとメモリが伸び続ける。
    [[nodiscard]] virtual std::size_t backlog() const noexcept = 0;
    [[nodiscard]] virtual std::uint64_t deferrals() const noexcept = 0;

    [[nodiscard]] virtual const SubscriberStats& inStats() const noexcept = 0;
    [[nodiscard]] virtual const std::string& lastError() const noexcept = 0;
};

template <class T>
class ClassChannel final : public Channel {
public:
    /// source が null なら送信しない、sink が null なら受けたものを捨てる。
    /// 実際のフェデレーションでも publish だけ / subscribe だけのクラスはあるので、
    /// 両方揃っていることを前提にしない。
    ClassChannel(const ClassBinding& bind,
                 hla::Source<T>* source,
                 hla::Sink<T>* sink)
        : m_bind(bind), m_source(source), m_sink(sink),
          m_pub(bind.classId, bind.payload), m_sub(bind.classId, bind.payload) {}

    [[nodiscard]] const ClassBinding& binding() const noexcept override { return m_bind; }
    [[nodiscard]] UdpSocket& socket() noexcept override { return m_sock; }

    [[nodiscard]] bool open(const std::string& peerHost) override {
        return m_sock.open(m_bind.port, peerHost, m_bind.port);
    }

    void setLoopRate(unsigned loopHz) noexcept override {
        // ICD の Rate 列がこのクラスの送信周期。ループより遅いクラスは間引く。
        // rate が 0（変化時のみ）やループより速い指定は、毎周期に落とす。
        const unsigned rate = m_bind.rateHz;
        m_divisor = (rate == 0 || rate >= loopHz) ? 1u : (loopHz / rate);
        m_phase = 0;
    }

    std::size_t pumpOut() override {
        if (m_source == nullptr || !m_sock.canSend()) return 0;
        if (++m_phase < m_divisor) return 0;
        m_phase = 0;

        // スナップショットは前回の残りが既に古い。撮り直す前に捨てる。
        // イベントは1件ずつ意味があるので、残っているぶんの後ろに足す。
        if (m_bind.delivery == Delivery::Snapshot) m_outbox.clear();
        m_source->drain(m_outbox);
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
            if (m_sink != nullptr) m_sink->accept(rec);
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
    ClassBinding m_bind;
    hla::Source<T>* m_source;
    hla::Sink<T>* m_sink;
    UdpSocket m_sock;
    Publisher<T> m_pub;
    Subscriber<T> m_sub;
    std::vector<T> m_outbox;
    unsigned m_divisor = 1;
    unsigned m_phase = 0;
    std::size_t m_backlog = 0;
    std::uint64_t m_deferrals = 0;
};

}  // namespace gw
