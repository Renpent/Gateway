// 1ポートぶんの送受信を、型を持たない形で見せる抽象。
//
// ポートごとにデータの型が違うので、周期ループから見えるのはこの Channel だけにしてある。
// 型が要るのは各実装の内側だけで、そこから外には出ない。実装は2つ：
//
//   ClassChannel<T>  ICD のクラス。12バイトヘッダ + 固定長レコード、相手は HLA
//   RawChannel<T>    FOM に無い独自データ。相手が決めた形式のまま、相手はアプリ
//
// 以前はこれが「FOM のクラス1つ」を表していて、binding() で ClassBinding（classId・
// ClassKind・FOM 名）をそのまま見せていた。独自データにはそのどれも当てはまらないので、
// 周期ループが本当に使う4つ — 名前・ポート・上限・種別の表示 — だけを出すようにした。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "../net/UdpSocket.h"
#include "SubscriberStats.h"

namespace gw {

class Channel {
public:
    virtual ~Channel() = default;

    /// ログと統計表に出す名前。ICD のクラスなら FOM 名（先頭の HLAobjectRoot などを除いたもの）、
    /// 独自データなら型に付けた名前。
    [[nodiscard]] virtual const char* name() const noexcept = 0;

    /// このチャネル専用の UDP ポート（送受信とも同じ番号）。**全チャネルで1つの番号空間。**
    /// 重複は Gateway::openAll が開く前に断る。
    [[nodiscard]] virtual std::uint16_t port() const noexcept = 0;

    /// 1データグラムの上限。受信バッファの長さでもある。
    [[nodiscard]] virtual std::size_t payload() const noexcept = 0;

    /// 計画表の「種別」列に出す文字列。
    [[nodiscard]] virtual const char* kindLabel() const noexcept = 0;

    [[nodiscard]] virtual UdpSocket& socket() noexcept = 0;

    /// 受信ポートを bind し、送信先を設定する。peerHost が空なら受信専用。
    [[nodiscard]] virtual bool open(const std::string& peerHost) = 0;

    /// 手元（HLA やアプリ）から出てきたぶんを送る。ブロックしない。受信専用なら何もしない。
    virtual std::size_t pumpOut() = 0;

    /// 来ているぶんを読み切って手元へ渡す。ブロックしない。
    /// **poll が「読める」と言ったときだけ呼ぶこと。** 呼んでも害はないが、
    /// 空振りの recvfrom がポート数ぶん積み上がる。
    virtual std::size_t pumpIn() = 0;

    /// 1レコードのバイト数と、1データグラムに入る件数。
    ///
    /// ICD のクラスは固定長なので開く前に分かる。件数が 0 なら **1件がペイロードに収まらない**
    /// — ジャンボフレームか、ICD の上限見直し。
    ///
    /// **recordSize() が 0 なら可変長**で、1データグラム = 1メッセージ（件数は 1）。
    /// 相手が決めた形式のまま受ける RawChannel がこれにあたる。
    [[nodiscard]] virtual std::size_t recordSize() const noexcept = 0;
    [[nodiscard]] virtual std::size_t capacityInRecords() const noexcept = 0;

    [[nodiscard]] virtual std::uint64_t sentTotal() const noexcept = 0;

    /// 送り切れずに次の周期へ回した件数と、そうなった周期の回数。
    /// **インタラクションで backlog が減らないなら、そのクラスは供給に追いついていない。**
    /// 上げるのは周期そのものか payload（＝1発の件数）で、放っておくとメモリが伸び続ける。
    [[nodiscard]] virtual std::size_t backlog() const noexcept = 0;
    [[nodiscard]] virtual std::uint64_t deferrals() const noexcept = 0;

    [[nodiscard]] virtual const SubscriberStats& inStats() const noexcept = 0;
    [[nodiscard]] virtual const std::string& lastError() const noexcept = 0;
};

}  // namespace gw
