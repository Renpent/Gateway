// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmbeddedSystem.Designator
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#pragma once

#include "../icd_types.h"

namespace icdfom {

/// FOM: HLAobjectRoot.EmbeddedSystem.Designator
///
/// Members are in the ICD's row order, which is the order they occupy on the wire.
struct Designator {
    EntityIdentifierStruct EntityIdentifier;  ///< FOM: EntityIdentifier : EntityIdentifierStruct
    RTIobjectId HostObjectIdentifier;  ///< FOM: HostObjectIdentifier : RTIobjectId
    RelativePositionStruct RelativePosition;  ///< FOM: RelativePosition : RelativePositionStruct
    DesignatorCodeNameEnum16 CodeName;  ///< FOM: CodeName : DesignatorCodeNameEnum16
    RTIobjectId DesignatedObjectIdentifier;  ///< FOM: DesignatedObjectIdentifier : RTIobjectId
    DesignatorCodeEnum16 DesignatorCode;  ///< FOM: DesignatorCode : DesignatorCodeEnum16
    WavelengthMicronFloat32 DesignatorEmissionWavelength;  ///< FOM: DesignatorEmissionWavelength : WavelengthMicronFloat32
    PowerWattFloat32 DesignatorOutputPower;  ///< FOM: DesignatorOutputPower : PowerWattFloat32
    WorldLocationStruct DesignatorSpotLocation;  ///< FOM: DesignatorSpotLocation : WorldLocationStruct
    DeadReckoningAlgorithmEnum8 DeadReckoningAlgorithm;  ///< FOM: DeadReckoningAlgorithm : DeadReckoningAlgorithmEnum8
    RelativePositionStruct RelativeSpotLocation;  ///< FOM: RelativeSpotLocation : RelativePositionStruct
    AccelerationVectorStruct SpotLinearAccelerationVector;  ///< FOM: SpotLinearAccelerationVector : AccelerationVectorStruct
    static constexpr std::size_t kEncodedSize = 115;
    static constexpr std::size_t kPayload = icd::kJumboPayload;  // 1データグラムの上限。ICD の MTU 9000 より
    static constexpr bool kIsInteraction = false;
    static constexpr const char* kFomName = "HLAobjectRoot.EmbeddedSystem.Designator";
    static constexpr std::uint32_t kClassId = 5;  // 抽出概要シートの ID 列
    static constexpr std::uint16_t kPort = 24005;  // 抽出概要シートの Port 列
};

static_assert(Designator::kEncodedSize + icd::kHeaderSize <= Designator::kPayload,
              "Designator: 1件がペイロードに収まりません。ICDgenerator の MTU か配列上限を見直してください");

[[nodiscard]] icd::Result decode(icd::Reader& r, Designator& v);
void encode(icd::Writer& w, const Designator& v);

/// Reads the records batched into one datagram.
/// The class id stays a runtime argument so a deployment can override the ICD's number
/// without regenerating; pass kClassId to take it.
typedef icd::DatagramReader<Designator> DesignatorReader;

/// Packs records into one datagram until the next one will not fit.
typedef icd::DatagramWriter<Designator> DesignatorWriter;

}  // namespace icdfom

