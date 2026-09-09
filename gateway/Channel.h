// 1クラス = 1ポートぶんの送受信。
//
// クラスごとにレコード型が違うので、周期ループから見えるのは Channel という型を持たない
// 抽象だけにしてある。型が要るのは ClassChannel<T> の内側 — つまり生成コーデックを呼ぶ
// 場所 — だけで、そこから外には出ない。

#ifndef HLAGW_CHANNEL_H
#define HLAGW_CHANNEL_H

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
        : bind_(bind), source_(source), sink_(sink),
          pub_(bind.classId, bind.payload), sub_(bind.classId, bind.payload) {}

    [[nodiscard]] const ClassBinding& binding() const noexcept override { return bind_; }
    [[nodiscard]] UdpSocket& socket() noexcept override { return sock_; }

    [[nodiscard]] bool open(const std::string& peerHost) override {
        return sock_.open(bind_.port, peerHost, bind_.port);
    }

    void setLoopRate(unsigned loopHz) noexcept override {
        // ICD の Rate 列がこのクラスの送信周期。ループより遅いクラスは間引く。
        // rate が 0（変化時のみ）やループより速い指定は、毎周期に落とす。
        const unsigned rate = bind_.rateHz;
        divisor_ = (rate == 0 || rate >= loopHz) ? 1u : (loopHz / rate);
        phase_ = 0;
    }

    std::size_t pumpOut() override {
        if (source_ == nullptr || !sock_.canSend()) return 0;
        if (++phase_ < divisor_) return 0;
        phase_ = 0;

        // スナップショットは前回の残りが既に古い。撮り直す前に捨てる。
        // イベントは1件ずつ意味があるので、残っているぶんの後ろに足す。
        if (bind_.delivery == Delivery::Snapshot) outbox_.clear();
        source_->drain(outbox_);
        if (outbox_.empty()) return 0;

        // 1周期に出すデータグラム数の上限。受信側の maxPerDrain と対になる制限で、
        // **これが無いと周期が守れない。** イベントが束で来た周期に全部出そうとすると、
        // その1周が何十 ms にもなって全クラスが遅れる。
        const std::size_t cap = maxDatagramsPerTick_ * pub_.capacityInRecords();
        const std::size_t want = outbox_.size() < cap ? outbox_.size() : cap;

        std::size_t sent = 0;
        while (sent < want && pub_.publish(outbox_[sent], sock_)) ++sent;

        // 周期の終わりに必ず出し切る。次の周期まで抱えると、その1周期ぶん遅れる。
        (void)pub_.flush(sock_);

        outbox_.erase(outbox_.begin(), outbox_.begin() + static_cast<std::ptrdiff_t>(sent));
        if (!outbox_.empty()) {
            ++deferrals_;
            backlog_ = outbox_.size();
        } else {
            backlog_ = 0;
        }
        return sent;
    }

    std::size_t pumpIn() override {
        const long got = sub_.drain(sock_, [this](const T& rec) {
            if (sink_ != nullptr) sink_->accept(rec);
        });
        return got < 0 ? 0 : static_cast<std::size_t>(got);
    }

    [[nodiscard]] std::uint64_t sentTotal() const noexcept override {
        return pub_.recordsSent();
    }
    [[nodiscard]] std::size_t backlog() const noexcept override { return backlog_; }
    [[nodiscard]] std::uint64_t deferrals() const noexcept override { return deferrals_; }
    [[nodiscard]] const SubscriberStats& inStats() const noexcept override {
        return sub_.stats();
    }
    [[nodiscard]] const std::string& lastError() const noexcept override {
        return sock_.lastError();
    }

    [[nodiscard]] std::size_t recordSize() const noexcept override {
        return icd::fixedSize<T>;
    }
    [[nodiscard]] std::size_t capacityInRecords() const noexcept override {
        return pub_.capacityInRecords();
    }

private:
    ClassBinding bind_;
    hla::Source<T>* source_;
    hla::Sink<T>* sink_;
    UdpSocket sock_;
    Publisher<T> pub_;
    Subscriber<T> sub_;
    std::vector<T> outbox_;
    unsigned divisor_ = 1;
    unsigned phase_ = 0;
    std::size_t maxDatagramsPerTick_ = 8;
    std::size_t backlog_ = 0;
    std::uint64_t deferrals_ = 0;
};

}  // namespace gw

#endif  // HLAGW_CHANNEL_H
