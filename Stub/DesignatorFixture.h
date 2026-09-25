// **本番には持っていかないファイル。** Stub/ は RTI が無いこの環境で送信側を動かすための
// 代用品で、実 RTI に繋ぐときはフォルダごと消せる（直すのは Wiring/CWiring.h だけ）。
//
// Designator の i 件目の値。同じ i なら必ず同じ値になる。
// 指示器は "DSG-0" と "DSG-1" の2台で交互に出し、照射点（SpotLocation）が少しずつ動く。

#pragma once

#include <cstddef>
#include <string>

#include "../ICD/Object/Designator.h"
#include "ObjectId.h"

namespace stub {

/// i 番目の Designator。
inline icdfom::Designator makeDesignator(std::size_t i) {
    const std::size_t unit = i % 2;              // どちらの指示器か
    const double step = static_cast<double>(i / 2);   // その指示器の何回目か
    icdfom::Designator d{};

    d.EntityIdentifier.FederateIdentifier.SiteID = 1;
    d.EntityIdentifier.FederateIdentifier.ApplicationID = 1;
    d.EntityIdentifier.EntityNumber = static_cast<icdfom::UnsignedInteger16>(100 + unit);
    d.HostObjectIdentifier = objectId("DSG-" + std::to_string(unit));

    d.RelativePosition.BodyXDistance = 1.5f;
    d.RelativePosition.BodyYDistance = 0.0f;
    d.RelativePosition.BodyZDistance = -0.5f;

    d.CodeName = icdfom::DesignatorCodeNameEnum16::Other;
    d.DesignatedObjectIdentifier = objectId("TGT-" + std::to_string(unit));
    d.DesignatorCode = icdfom::DesignatorCodeEnum16::Other;
    d.DesignatorEmissionWavelength = 1.064f;     // Nd:YAG
    d.DesignatorOutputPower = 10.0f + static_cast<float>(unit);

    d.DesignatorSpotLocation.X = 4.0e6 + 10.0 * step;
    d.DesignatorSpotLocation.Y = 2.0e6 + 1000.0 * static_cast<double>(unit);
    d.DesignatorSpotLocation.Z = 4.5e6;
    d.DeadReckoningAlgorithm = icdfom::DeadReckoningAlgorithmEnum8::Static;
    return d;
}

}  // namespace stub
