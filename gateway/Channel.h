// 1クラス = 1ポートぶんの送受信を、型を持たない形で見せる抽象。
//
// クラスごとにレコード型が違うので、周期ループから見えるのはこの Channel だけにしてある。
// 型が要るのは ClassChannel<T> の内側 — つまり生成コーデックを呼ぶ場所 — だけで、
// そこから外には出ない。実装は ClassChannel.h。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "../net/UdpSocket.h"
#include "ClassBinding.h"
#include "SubscriberStats.h"

namespace gw {

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

}  // namespace gw
