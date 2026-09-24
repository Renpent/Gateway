// **本番には持っていかないファイル。** stub/ は RTI が無いこの環境でゲートウェイを動かし、
// 往復を検証するための代用品だけが入っている。実 RTI に繋ぐときは stub/ ごと消せて、
// 直す先は app/CWiring.h の1ファイルで済む。
//
// ループバックで往復させる RadarBeam の「送ったはずの値」。
//
// 本番では RTI が reflectAttributeValues で運んでくるものを、ここでは決まった手順で作る。
// **決定的**（同じ i なら同じ値）にしてあるのは、往復した結果を元と1バイトずつ突き合わせる
// ため。乱数だと「壊れたのか元から違うのか」が分からなくなる。
//
// 供給側（TCFixtureFromHla）と照合側（TCVerifyingToHla）の両方がここを呼ぶので、
// 「送ったはずの値」の定義が1箇所に閉じる。

#pragma once

#include <cstddef>
#include <string>

#include "../icd/object/RadarBeam.h"
#include "ObjectId.h"

namespace stub {

/// i 番目の RadarBeam。
inline icdfom::RadarBeam makeRadarBeam(std::size_t i) {
    const float f = static_cast<float>(i);
    icdfom::RadarBeam b{};

    b.BeamAzimuthCenter      = 0.10f * f;
    b.BeamAzimuthSweep       = 0.25f;
    b.BeamElevationCenter    = -0.05f * f;
    b.BeamElevationSweep     = 0.125f;
    b.BeamFunctionCode       = icdfom::BeamFunctionCodeEnum8::Search;
    b.BeamIdentifier         = static_cast<icdfom::Octet>(i & 0xFF);
    b.BeamParameterIndex     = static_cast<icdfom::UnsignedInteger16>(100 + i);
    b.EffectiveRadiatedPower = 42.5f + f;
    b.EmissionFrequency      = 9.3e9f;
    b.EmitterSystemIdentifier = objectId("EMIT-" + std::to_string(i));

    b.EventIdentifier.EventCount = static_cast<icdfom::UnsignedInteger16>(i);
    b.EventIdentifier.IssuingObjectIdentifier = objectId("FED-1");

    b.FrequencyRange           = 1.0e6f;
    b.PulseRepetitionFrequency = 1000.0f + f;
    b.PulseWidth               = 1.5f;
    b.SweepSynch               = 0.0f;
    b.HighDensityTrack = (i % 2 == 0) ? icdfom::RPRboolean::False
                                      : icdfom::RPRboolean::True;

    // 上限3本の可変長配列。件数を回ごとに変えて、詰め物（ゼロ埋め）の側も動かす。
    b.TrackObjectIdentifiers.clear();
    for (std::size_t k = 0; k < (i % 4); ++k) {
        b.TrackObjectIdentifiers.push_back(objectId("TRK-" + std::to_string(k)));
    }
    return b;
}

}  // namespace stub
