// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmbeddedSystem.MinefieldData
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#include "MinefieldData.h"

namespace icdfom {

icd::Result decode(icd::Reader& r, MinefieldData& v) {
    if (const icd::Result rc = decode(r, v.EntityIdentifier); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.HostObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.RelativePosition); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.GroundBurialDepthOffset, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.Fusing, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.MineEmplacementTime, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.MineEntityIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.MinefieldIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.MineLocation, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.MineOrientation, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.MineType); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.NumberTripDetonationWires, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.NumberWireVertices, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.PaintScheme, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.Reflectance, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.ScalarDetectionCoefficient, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.SensorTypes, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.SnowBurialDepthOffset, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.ThermalContrast, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.WaterBurialDepthOffset, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.WireVertices, 16); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const MinefieldData& v) {
    encode(w, v.EntityIdentifier);
    icd::encodeBounded(w, v.HostObjectIdentifier, 16);
    encode(w, v.RelativePosition);
    icd::encodeBounded(w, v.GroundBurialDepthOffset, 16);
    icd::encodeBounded(w, v.Fusing, 16);
    icd::encodeBounded(w, v.MineEmplacementTime, 16);
    icd::encodeBounded(w, v.MineEntityIdentifier, 16);
    icd::encodeBounded(w, v.MinefieldIdentifier, 16);
    icd::encodeBounded(w, v.MineLocation, 16);
    icd::encodeBounded(w, v.MineOrientation, 16);
    encode(w, v.MineType);
    icd::encodeBounded(w, v.NumberTripDetonationWires, 16);
    icd::encodeBounded(w, v.NumberWireVertices, 16);
    icd::encodeBounded(w, v.PaintScheme, 16);
    icd::encodeBounded(w, v.Reflectance, 16);
    icd::encodeBounded(w, v.ScalarDetectionCoefficient, 16);
    icd::encodeBounded(w, v.SensorTypes, 16);
    icd::encodeBounded(w, v.SnowBurialDepthOffset, 16);
    icd::encodeBounded(w, v.ThermalContrast, 16);
    icd::encodeBounded(w, v.WaterBurialDepthOffset, 16);
    icd::encodeBounded(w, v.WireVertices, 16);
}

std::size_t encodedSize(const MinefieldData& v) {
    (void)v;
    return 1902;
}

}  // namespace icdfom
