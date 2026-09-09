// ICD の「抽出概要」シートに手で書き込む3列 — ID / Port / Rate — をコードに写したもの。
//
// FOM にはポートも更新レートも存在しないので生成器はここを埋められない。ICD 側で決めた値を
// **手で写す**のがこのファイルの役目で、ゲートウェイ内で ID とポートを書くのはここだけ。
// ICD を更新したら、まずこのファイルを合わせること。
//
// 1クラス = 1ポート。データグラムの classId はポートと情報が重複するが、
// ポートの設定ミスを受信側で捕まえられるので両方持つ。

#ifndef HLAGW_CLASS_IDS_H
#define HLAGW_CLASS_IDS_H

#include <cstdint>

#include "../icd/icd_codec.h"

namespace gw {

/// HLA 側から何が出てくるかの種別。**送り切れなかったぶんの扱いが正反対**なので、
/// これを取り違えると静かに壊れる。
enum class Delivery {
    /// オブジェクト属性。HLA ツールの getRemoteXXX() で「その瞬間の全体像」が取れるので、
    /// 毎周期それを写して送る。1周期ぶん落ちても次の周期が現在値を運ぶので**自己回復する**。
    /// 送り残しは次の周期には古い値になっているので、**捨てて撮り直す**のが正しい。
    Snapshot,

    /// インタラクション。イベントとして飛んでくるので、1件1件が意味を持つ。
    /// 落とすと二度と戻らないし、次の周期が埋め合わせてもくれない。
    /// 送り残しは**持ち越す**。
    Events,
};

/// ICD 1行ぶん。ID・ポート・更新レートが1つの型で並ぶので、写し間違いが目で見つかる。
struct ClassBinding {
    std::uint32_t classId;
    std::uint16_t port;
    unsigned      rateHz;       ///< 0 = 変化時のみ送信
    std::size_t   payload;      ///< 1データグラムの上限。経路の MTU で決まる
    Delivery      delivery;
    const char*   fomName;
};

// 抽出概要シートより。
//
// payload は経路の MTU から決まる。レコードが固定長なので、
// 「1件のバイト数 > payload - 12」なら**そのクラスは1件も送れない**（起動時に弾かれる）。
// MinefieldData は 1902 B あり、1500 MTU では収まらないのでジャンボ側に置いている。
inline constexpr ClassBinding kRadarBeam{
    1, 24001, 10, icd::kDefaultPayload, Delivery::Snapshot,
    "HLAobjectRoot.EmitterBeam.RadarBeam"};
inline constexpr ClassBinding kRadioReceiver{
    2, 24002, 5, icd::kDefaultPayload, Delivery::Snapshot,
    "HLAobjectRoot.EmbeddedSystem.RadioReceiver"};
inline constexpr ClassBinding kMinefieldData{
    3, 24003, 1, icd::kJumboPayload, Delivery::Snapshot,
    "HLAobjectRoot.EmbeddedSystem.MinefieldData"};

}  // namespace gw

#endif  // HLAGW_CLASS_IDS_H
