// 自動生成 — 編集しないこと。
// FOMのデータ型（電文との変換）
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#pragma once

#include "icd_codec.h"
#include "icd_types.h"

// 電文上のバイト数。構造体は kEncodedSize を持つが、列挙（とツールキットの型）は
// メンバを持てないので、ここで伝える。配列の要素になったときにランタイムが使う。
namespace icd {
template <> struct FixedSize<icdfom::BeamFunctionCodeEnum8> { static constexpr std::size_t value = 1; };
template <> struct FixedSize<icdfom::RPRboolean> { static constexpr std::size_t value = 1; };
template <> struct FixedSize<icdfom::ReceiverOperationalStatusEnum16> { static constexpr std::size_t value = 2; };
template <> struct FixedSize<icdfom::MinefieldFusingEnum32> { static constexpr std::size_t value = 4; };
template <> struct FixedSize<icdfom::MinefieldPaintSchemeEnum32> { static constexpr std::size_t value = 4; };
template <> struct FixedSize<icdfom::MinefieldSensorTypeEnum32> { static constexpr std::size_t value = 4; };
template <> struct FixedSize<icdfom::FuseTypeEnum16> { static constexpr std::size_t value = 2; };
template <> struct FixedSize<icdfom::WarheadTypeEnum16> { static constexpr std::size_t value = 2; };
}  // namespace icd

namespace icdfom {

// The runtime's primitive and container codecs. Generated types are found through
// argument-dependent lookup; these are not, so they are named explicitly.
using icd::decode;
using icd::encode;
using icd::fixedSize;
using icd::wireSize;

/// FOM: BeamFunctionCodeEnum8
[[nodiscard]] inline icd::Result decode(icd::Reader& r, BeamFunctionCodeEnum8& v) {
    uint8_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<BeamFunctionCodeEnum8>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, BeamFunctionCodeEnum8 v) {
    icd::encode(w, static_cast<uint8_t>(v));
}

/// FOM: EventIdentifierStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, EventIdentifierStruct& v);
void encode(icd::Writer& w, const EventIdentifierStruct& v);

/// FOM: RPRboolean
[[nodiscard]] inline icd::Result decode(icd::Reader& r, RPRboolean& v) {
    uint8_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<RPRboolean>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, RPRboolean v) {
    icd::encode(w, static_cast<uint8_t>(v));
}

/// FOM: FederateIdentifierStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, FederateIdentifierStruct& v);
void encode(icd::Writer& w, const FederateIdentifierStruct& v);

/// FOM: EntityIdentifierStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, EntityIdentifierStruct& v);
void encode(icd::Writer& w, const EntityIdentifierStruct& v);

/// FOM: RelativePositionStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, RelativePositionStruct& v);
void encode(icd::Writer& w, const RelativePositionStruct& v);

/// FOM: ReceiverOperationalStatusEnum16
[[nodiscard]] inline icd::Result decode(icd::Reader& r, ReceiverOperationalStatusEnum16& v) {
    uint16_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<ReceiverOperationalStatusEnum16>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, ReceiverOperationalStatusEnum16 v) {
    icd::encode(w, static_cast<uint16_t>(v));
}

/// FOM: MinefieldFusingEnum32
[[nodiscard]] inline icd::Result decode(icd::Reader& r, MinefieldFusingEnum32& v) {
    uint32_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<MinefieldFusingEnum32>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, MinefieldFusingEnum32 v) {
    icd::encode(w, static_cast<uint32_t>(v));
}

/// FOM: MineFusingStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, MineFusingStruct& v);
void encode(icd::Writer& w, const MineFusingStruct& v);

/// FOM: ClockTimeStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, ClockTimeStruct& v);
void encode(icd::Writer& w, const ClockTimeStruct& v);

/// FOM: WorldLocationStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, WorldLocationStruct& v);
void encode(icd::Writer& w, const WorldLocationStruct& v);

/// FOM: OrientationStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, OrientationStruct& v);
void encode(icd::Writer& w, const OrientationStruct& v);

/// FOM: EntityTypeStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, EntityTypeStruct& v);
void encode(icd::Writer& w, const EntityTypeStruct& v);

/// FOM: MinefieldPaintSchemeEnum32
[[nodiscard]] inline icd::Result decode(icd::Reader& r, MinefieldPaintSchemeEnum32& v) {
    uint32_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<MinefieldPaintSchemeEnum32>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, MinefieldPaintSchemeEnum32 v) {
    icd::encode(w, static_cast<uint32_t>(v));
}

/// FOM: MinefieldSensorTypeEnum32
[[nodiscard]] inline icd::Result decode(icd::Reader& r, MinefieldSensorTypeEnum32& v) {
    uint32_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<MinefieldSensorTypeEnum32>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, MinefieldSensorTypeEnum32 v) {
    icd::encode(w, static_cast<uint32_t>(v));
}

/// FOM: FuseTypeEnum16
[[nodiscard]] inline icd::Result decode(icd::Reader& r, FuseTypeEnum16& v) {
    uint16_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<FuseTypeEnum16>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, FuseTypeEnum16 v) {
    icd::encode(w, static_cast<uint16_t>(v));
}

/// FOM: VelocityVectorStruct
[[nodiscard]] icd::Result decode(icd::Reader& r, VelocityVectorStruct& v);
void encode(icd::Writer& w, const VelocityVectorStruct& v);

/// FOM: WarheadTypeEnum16
[[nodiscard]] inline icd::Result decode(icd::Reader& r, WarheadTypeEnum16& v) {
    uint16_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<WarheadTypeEnum16>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, WarheadTypeEnum16 v) {
    icd::encode(w, static_cast<uint16_t>(v));
}

}  // namespace icdfom

