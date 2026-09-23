// 自動生成 — 編集しないこと。
// HLAinteractionRoot.WeaponFire
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#include "WeaponFire.h"

namespace icdfom {

icd::Result decode(icd::Reader& r, WeaponFire& v) {
    if (const icd::Result rc = decode(r, v.EventIdentifier); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.FireControlSolutionRange); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.FireMissionIndex); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.FiringLocation); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.FiringObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.FuseType); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.InitialVelocityVector); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.MunitionObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.MunitionType); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.QuantityFired); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.RateOfFire); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.TargetObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.WarheadType); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const WeaponFire& v) {
    encode(w, v.EventIdentifier);
    encode(w, v.FireControlSolutionRange);
    encode(w, v.FireMissionIndex);
    encode(w, v.FiringLocation);
    icd::encodeBounded(w, v.FiringObjectIdentifier, 16);
    encode(w, v.FuseType);
    encode(w, v.InitialVelocityVector);
    icd::encodeBounded(w, v.MunitionObjectIdentifier, 16);
    encode(w, v.MunitionType);
    encode(w, v.QuantityFired);
    encode(w, v.RateOfFire);
    icd::encodeBounded(w, v.TargetObjectIdentifier, 16);
    encode(w, v.WarheadType);
}

}  // namespace icdfom
