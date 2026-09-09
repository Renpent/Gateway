// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmbeddedSystem.MinefieldData
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#ifndef ICDFOM_MINEFIELDDATA_H
#define ICDFOM_MINEFIELDDATA_H

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
};

[[nodiscard]] icd::Result decode(icd::Reader& r, MinefieldData& v);
void encode(icd::Writer& w, const MinefieldData& v);
[[nodiscard]] std::size_t encodedSize(const MinefieldData& v);

/// Reads the records batched into one datagram. The class id is a runtime argument:
/// the ICD's ID column is filled in by hand, so it is not known at generation time.
typedef icd::DatagramReader<MinefieldData> MinefieldDataReader;

/// Packs records into one datagram until the next one will not fit.
typedef icd::DatagramWriter<MinefieldData> MinefieldDataWriter;

}  // namespace icdfom

#endif  // ICDFOM_MINEFIELDDATA_H
