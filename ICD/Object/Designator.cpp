// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmbeddedSystem.Designator
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#include "Designator.h"

namespace icdfom {

icd::Result decode(icd::Reader& r, Designator& v) {
    if (const icd::Result rc = decode(r, v.EntityIdentifier); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.HostObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.RelativePosition); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.CodeName); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.DesignatedObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.DesignatorCode); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.DesignatorEmissionWavelength); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.DesignatorOutputPower); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.DesignatorSpotLocation); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.DeadReckoningAlgorithm); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.RelativeSpotLocation); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.SpotLinearAccelerationVector); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const Designator& v) {
    encode(w, v.EntityIdentifier);
    icd::encodeBounded(w, v.HostObjectIdentifier, 16);
    encode(w, v.RelativePosition);
    encode(w, v.CodeName);
    icd::encodeBounded(w, v.DesignatedObjectIdentifier, 16);
    encode(w, v.DesignatorCode);
    encode(w, v.DesignatorEmissionWavelength);
    encode(w, v.DesignatorOutputPower);
    encode(w, v.DesignatorSpotLocation);
    encode(w, v.DeadReckoningAlgorithm);
    encode(w, v.RelativeSpotLocation);
    encode(w, v.SpotLinearAccelerationVector);
}

}  // namespace icdfom
