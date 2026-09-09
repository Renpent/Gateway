// RTI の代わりにレコードを作るスタブと、受け取ったものを見るためのシンク。
//
// 本番では RTI が reflectAttributeValues で運んでくるものを、ここでは決まった手順で作る。
// **決定的**（同じ i なら同じ値）にしてあるのは、ループバックで往復させた結果を
// 元と1バイトずつ突き合わせるため。乱数だと「壊れたのか元から違うのか」が分からなくなる。

#ifndef HLAGW_STUB_SOURCE_H
#define HLAGW_STUB_SOURCE_H

#include <cstdint>
#include <string>
#include <vector>

#include "../icd/RadarBeam.h"
#include "Federate.h"

namespace hla {

/// FOM の RTIobjectId は null 終端の文字配列。std::string から詰め替える。
inline icdfom::RTIobjectId objectId(const std::string& s) {
    return icdfom::RTIobjectId(s.begin(), s.end());
}

/// i 番目の RadarBeam。ループバックの照合側も同じ関数を呼ぶので、
/// 「送ったはずの値」の定義がここ1箇所に閉じる。
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

class StubRadarBeamSource : public Source<icdfom::RadarBeam> {
public:
    explicit StubRadarBeamSource(std::size_t perDrain = 4) : perDrain_(perDrain) {}

    std::size_t drain(std::vector<icdfom::RadarBeam>& out) override {
        for (std::size_t n = 0; n < perDrain_; ++n) out.push_back(makeRadarBeam(produced_++));
        return perDrain_;
    }

    [[nodiscard]] std::size_t produced() const noexcept { return produced_; }

private:
    std::size_t perDrain_;
    std::size_t produced_ = 0;
};

/// 中身に意味を持たせない供給元。ポートが複数あって poll が正しく振り分けているかを
/// 見るためだけのもので、値は既定構築のまま（可変長配列は 0 要素）。
template <class T>
class ConstantSource : public Source<T> {
public:
    explicit ConstantSource(std::size_t perDrain = 1) : perDrain_(perDrain) {}

    std::size_t drain(std::vector<T>& out) override {
        for (std::size_t n = 0; n < perDrain_; ++n) out.push_back(T{});
        produced_ += perDrain_;
        return perDrain_;
    }

    [[nodiscard]] std::size_t produced() const noexcept { return produced_; }

private:
    std::size_t perDrain_;
    std::size_t produced_ = 0;
};

/// 受け取った件数だけ数えるシンク。
template <class T>
class CountingSink : public Sink<T> {
public:
    void accept(const T&) override { ++count_; }
    [[nodiscard]] std::size_t count() const noexcept { return count_; }

private:
    std::size_t count_ = 0;
};

}  // namespace hla

#endif  // HLAGW_STUB_SOURCE_H
