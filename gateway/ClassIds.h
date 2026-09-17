// ICD の「抽出概要」シートに手で書き込む3列 — ID / Port / Rate — をコードに写したもの。
//
// **ゲートウェイ内で ID とポートを書くのはこのファイルだけ。** ICD を更新したら、
// まずここを合わせること。型の定義は ClassBinding.h、種別の意味は Delivery.h にある。

#pragma once

#include "../icd/icd_codec.h"
#include "ClassBinding.h"

namespace gw {

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
