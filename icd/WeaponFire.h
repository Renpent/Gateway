// 自動生成 — 編集しないこと。
// HLAinteractionRoot.WeaponFire
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#pragma once

#include "icd_types.h"

namespace icdfom {

/// FOM: HLAinteractionRoot.WeaponFire
///
/// Members are in the ICD's row order, which is the order they occupy on the wire.
struct WeaponFire {
    EventIdentifierStruct EventIdentifier;  ///< FOM: EventIdentifier : EventIdentifierStruct
    LengthMeterFloat32 FireControlSolutionRange;  ///< FOM: FireControlSolutionRange : LengthMeterFloat32
    UnsignedInteger32 FireMissionIndex;  ///< FOM: FireMissionIndex : UnsignedInteger32
    WorldLocationStruct FiringLocation;  ///< FOM: FiringLocation : WorldLocationStruct
    RTIobjectId FiringObjectIdentifier;  ///< FOM: FiringObjectIdentifier : RTIobjectId
    FuseTypeEnum16 FuseType;  ///< FOM: FuseType : FuseTypeEnum16
    VelocityVectorStruct InitialVelocityVector;  ///< FOM: InitialVelocityVector : VelocityVectorStruct
    RTIobjectId MunitionObjectIdentifier;  ///< FOM: MunitionObjectIdentifier : RTIobjectId
    EntityTypeStruct MunitionType;  ///< FOM: MunitionType : EntityTypeStruct
    UnsignedInteger16 QuantityFired;  ///< FOM: QuantityFired : UnsignedInteger16
    UnsignedInteger16 RateOfFire;  ///< FOM: RateOfFire : UnsignedInteger16
    RTIobjectId TargetObjectIdentifier;  ///< FOM: TargetObjectIdentifier : RTIobjectId
    WarheadTypeEnum16 WarheadType;  ///< FOM: WarheadType : WarheadTypeEnum16
    static constexpr std::size_t kEncodedSize = 134;
    static constexpr std::size_t kPayload = icd::kJumboPayload;  // 1データグラムの上限。ICD の MTU 9000 より
    static constexpr bool kIsInteraction = true;
    static constexpr const char* kFomName = "HLAinteractionRoot.WeaponFire";
    static constexpr std::uint32_t kClassId = 4;  // 抽出概要シートの ID 列
    static constexpr std::uint16_t kPort = 24004;  // 抽出概要シートの Port 列
};

static_assert(WeaponFire::kEncodedSize + icd::kHeaderSize <= WeaponFire::kPayload,
              "WeaponFire: 1件がペイロードに収まりません。ICDgenerator の MTU か配列上限を見直してください");

[[nodiscard]] icd::Result decode(icd::Reader& r, WeaponFire& v);
void encode(icd::Writer& w, const WeaponFire& v);

/// Reads the records batched into one datagram.
/// The class id stays a runtime argument so a deployment can override the ICD's number
/// without regenerating; pass kClassId to take it.
typedef icd::DatagramReader<WeaponFire> WeaponFireReader;

/// Packs records into one datagram until the next one will not fit.
typedef icd::DatagramWriter<WeaponFire> WeaponFireWriter;

}  // namespace icdfom

