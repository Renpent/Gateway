// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmbeddedSystem.MinefieldData
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#pragma once

#include "icd_types.h"

namespace icdfom {

/// FOM: HLAobjectRoot.EmbeddedSystem.MinefieldData
///
/// Members are in the ICD's row order, which is the order they occupy on the wire.
struct MinefieldData {
    EntityIdentifierStruct EntityIdentifier;  ///< FOM: EntityIdentifier : EntityIdentifierStruct
    RTIobjectId HostObjectIdentifier;  ///< FOM: HostObjectIdentifier : RTIobjectId
    RelativePositionStruct RelativePosition;  ///< FOM: RelativePosition : RelativePositionStruct
    DepthMeterFloat32LengthlessArray GroundBurialDepthOffset;  ///< FOM: GroundBurialDepthOffset : DepthMeterFloat32LengthlessArray
    MineFusingStructLengthlessArray Fusing;  ///< FOM: Fusing : MineFusingStructLengthlessArray
    ClockTimeStructLengthlessArray MineEmplacementTime;  ///< FOM: MineEmplacementTime : ClockTimeStructLengthlessArray
    MineIdentifierLengthlessArray MineEntityIdentifier;  ///< FOM: MineEntityIdentifier : MineIdentifierLengthlessArray
    RTIobjectId MinefieldIdentifier;  ///< FOM: MinefieldIdentifier : RTIobjectId
    WorldLocationStructLengthlessArray MineLocation;  ///< FOM: MineLocation : WorldLocationStructLengthlessArray
    OrientationStructLengthlessArray MineOrientation;  ///< FOM: MineOrientation : OrientationStructLengthlessArray
    EntityTypeStruct MineType;  ///< FOM: MineType : EntityTypeStruct
    UnsignedInteger8LengthlessArray NumberTripDetonationWires;  ///< FOM: NumberTripDetonationWires : UnsignedInteger8LengthlessArray
    UnsignedInteger8LengthlessArray NumberWireVertices;  ///< FOM: NumberWireVertices : UnsignedInteger8LengthlessArray
    MinefieldPaintSchemeLengthlessArray PaintScheme;  ///< FOM: PaintScheme : MinefieldPaintSchemeLengthlessArray
    MineDielectricDifferenceLengthlessArray Reflectance;  ///< FOM: Reflectance : MineDielectricDifferenceLengthlessArray
    UnsignedInteger8LengthlessArray ScalarDetectionCoefficient;  ///< FOM: ScalarDetectionCoefficient : UnsignedInteger8LengthlessArray
    MinefieldSensorTypeLengthlessArray SensorTypes;  ///< FOM: SensorTypes : MinefieldSensorTypeLengthlessArray
    DepthMeterFloat32LengthlessArray SnowBurialDepthOffset;  ///< FOM: SnowBurialDepthOffset : DepthMeterFloat32LengthlessArray
    TemperatureDegreeCelsiusFloat32LengthlessArray ThermalContrast;  ///< FOM: ThermalContrast : TemperatureDegreeCelsiusFloat32LengthlessArray
    DepthMeterFloat32LengthlessArray WaterBurialDepthOffset;  ///< FOM: WaterBurialDepthOffset : DepthMeterFloat32LengthlessArray
    WorldLocationStructLengthlessArray WireVertices;  ///< FOM: WireVertices : WorldLocationStructLengthlessArray
    static constexpr std::size_t kEncodedSize = 1902;
    static constexpr std::size_t kPayload = icd::kJumboPayload;  // 1データグラムの上限。ICD の MTU 9000 より
    static constexpr bool kIsInteraction = false;
    static constexpr const char* kFomName = "HLAobjectRoot.EmbeddedSystem.MinefieldData";
    static constexpr std::uint32_t kClassId = 3;  // 抽出概要シートの ID 列
    static constexpr std::uint16_t kPort = 24003;  // 抽出概要シートの Port 列
};

static_assert(MinefieldData::kEncodedSize + icd::kHeaderSize <= MinefieldData::kPayload,
              "MinefieldData: 1件がペイロードに収まりません。ICDgenerator の MTU か配列上限を見直してください");

[[nodiscard]] icd::Result decode(icd::Reader& r, MinefieldData& v);
void encode(icd::Writer& w, const MinefieldData& v);

/// Reads the records batched into one datagram.
/// The class id stays a runtime argument so a deployment can override the ICD's number
/// without regenerating; pass kClassId to take it.
typedef icd::DatagramReader<MinefieldData> MinefieldDataReader;

/// Packs records into one datagram until the next one will not fit.
typedef icd::DatagramWriter<MinefieldData> MinefieldDataWriter;

}  // namespace icdfom

