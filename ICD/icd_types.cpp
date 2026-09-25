// 自動生成 — 編集しないこと。
// FOMのデータ型
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#include "icd_types.h"

namespace icdfom {

icd::Result decode(icd::Reader& r, EventIdentifierStruct& v) {
    if (const icd::Result rc = decode(r, v.EventCount); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.IssuingObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const EventIdentifierStruct& v) {
    encode(w, v.EventCount);
    icd::encodeBounded(w, v.IssuingObjectIdentifier, 16);
}

icd::Result decode(icd::Reader& r, FederateIdentifierStruct& v) {
    if (const icd::Result rc = decode(r, v.SiteID); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.ApplicationID); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const FederateIdentifierStruct& v) {
    encode(w, v.SiteID);
    encode(w, v.ApplicationID);
}

icd::Result decode(icd::Reader& r, EntityIdentifierStruct& v) {
    if (const icd::Result rc = decode(r, v.FederateIdentifier); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.EntityNumber); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const EntityIdentifierStruct& v) {
    encode(w, v.FederateIdentifier);
    encode(w, v.EntityNumber);
}

icd::Result decode(icd::Reader& r, RelativePositionStruct& v) {
    if (const icd::Result rc = decode(r, v.BodyXDistance); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BodyYDistance); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BodyZDistance); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const RelativePositionStruct& v) {
    encode(w, v.BodyXDistance);
    encode(w, v.BodyYDistance);
    encode(w, v.BodyZDistance);
}

icd::Result decode(icd::Reader& r, MineFusingStruct& v) {
    if (const icd::Result rc = decode(r, v.Primary); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Secondary); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.AntiHandlingDevice); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Padding); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const MineFusingStruct& v) {
    encode(w, v.Primary);
    encode(w, v.Secondary);
    encode(w, v.AntiHandlingDevice);
    encode(w, v.Padding);
}

icd::Result decode(icd::Reader& r, ClockTimeStruct& v) {
    if (const icd::Result rc = decode(r, v.Hours); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.TimePastTheHour); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const ClockTimeStruct& v) {
    encode(w, v.Hours);
    encode(w, v.TimePastTheHour);
}

icd::Result decode(icd::Reader& r, WorldLocationStruct& v) {
    if (const icd::Result rc = decode(r, v.X); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Y); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Z); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const WorldLocationStruct& v) {
    encode(w, v.X);
    encode(w, v.Y);
    encode(w, v.Z);
}

icd::Result decode(icd::Reader& r, OrientationStruct& v) {
    if (const icd::Result rc = decode(r, v.Psi); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Theta); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Phi); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const OrientationStruct& v) {
    encode(w, v.Psi);
    encode(w, v.Theta);
    encode(w, v.Phi);
}

icd::Result decode(icd::Reader& r, EntityTypeStruct& v) {
    if (const icd::Result rc = decode(r, v.EntityKind); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Domain); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.CountryCode); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Category); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Subcategory); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Specific); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.Extra); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const EntityTypeStruct& v) {
    encode(w, v.EntityKind);
    encode(w, v.Domain);
    encode(w, v.CountryCode);
    encode(w, v.Category);
    encode(w, v.Subcategory);
    encode(w, v.Specific);
    encode(w, v.Extra);
}

icd::Result decode(icd::Reader& r, VelocityVectorStruct& v) {
    if (const icd::Result rc = decode(r, v.XVelocity); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.YVelocity); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.ZVelocity); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const VelocityVectorStruct& v) {
    encode(w, v.XVelocity);
    encode(w, v.YVelocity);
    encode(w, v.ZVelocity);
}

icd::Result decode(icd::Reader& r, AccelerationVectorStruct& v) {
    if (const icd::Result rc = decode(r, v.XAcceleration); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.YAcceleration); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.ZAcceleration); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const AccelerationVectorStruct& v) {
    encode(w, v.XAcceleration);
    encode(w, v.YAcceleration);
    encode(w, v.ZAcceleration);
}

}  // namespace icdfom
