// RTI の代わりに RadarBeam を作る供給元。
//
// 値は RadarBeamFixture.h の makeRadarBeam が決める。照合側（VerifyingReceiver）も
// 同じ関数を呼ぶので、「送ったはずの値」がずれることがない。

#pragma once

#include <cstddef>
#include <vector>

#include "../icd/RadarBeam.h"
#include "FromHla.h"
#include "RadarBeamFixture.h"

namespace hla {

class StubRadarBeamFeed : public FromHla<icdfom::RadarBeam> {
public:
    explicit StubRadarBeamFeed(std::size_t perDrain = 4) : m_perDrain(perDrain) {}

    std::size_t drain(std::vector<icdfom::RadarBeam>& out) override {
        for (std::size_t n = 0; n < m_perDrain; ++n) out.push_back(makeRadarBeam(m_produced++));
        return m_perDrain;
    }

    [[nodiscard]] std::size_t produced() const noexcept { return m_produced; }

private:
    std::size_t m_perDrain;         ///< 1回の drain で作る件数
    std::size_t m_produced = 0;     ///< これまでに作った累計件数（makeRadarBeam の添字）
};

}  // namespace hla
