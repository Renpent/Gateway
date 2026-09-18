// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmitterBeam.RadarBeam
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#pragma once

#include "icd_types.h"

namespace icdfom {

/// FOM: HLAobjectRoot.EmitterBeam.RadarBeam
///
/// Members are in the ICD's row order, which is the order they occupy on the wire.
struct RadarBeam {
    AngleRadianFloat32 BeamAzimuthCenter;  ///< FOM: BeamAzimuthCenter : AngleRadianFloat32
    AngleRadianFloat32 BeamAzimuthSweep;  ///< FOM: BeamAzimuthSweep : AngleRadianFloat32
    AngleRadianFloat32 BeamElevationCenter;  ///< FOM: BeamElevationCenter : AngleRadianFloat32
    AngleRadianFloat32 BeamElevationSweep;  ///< FOM: BeamElevationSweep : AngleRadianFloat32
    BeamFunctionCodeEnum8 BeamFunctionCode;  ///< FOM: BeamFunctionCode : BeamFunctionCodeEnum8
    Octet BeamIdentifier;  ///< FOM: BeamIdentifier : Octet
    UnsignedInteger16 BeamParameterIndex;  ///< FOM: BeamParameterIndex : UnsignedInteger16
    PowerRatioDecibelMilliwattFloat32 EffectiveRadiatedPower;  ///< FOM: EffectiveRadiatedPower : PowerRatioDecibelMilliwattFloat32
    FrequencyHertzFloat32 EmissionFrequency;  ///< FOM: EmissionFrequency : FrequencyHertzFloat32
    RTIobjectId EmitterSystemIdentifier;  ///< FOM: EmitterSystemIdentifier : RTIobjectId
    EventIdentifierStruct EventIdentifier;  ///< FOM: EventIdentifier : EventIdentifierStruct
    FrequencyHertzFloat32 FrequencyRange;  ///< FOM: FrequencyRange : FrequencyHertzFloat32
    FrequencyHertzFloat32 PulseRepetitionFrequency;  ///< FOM: PulseRepetitionFrequency : FrequencyHertzFloat32
    TimeMicrosecondFloat32 PulseWidth;  ///< FOM: PulseWidth : TimeMicrosecondFloat32
    PercentFloat32 SweepSynch;  ///< FOM: SweepSynch : PercentFloat32
    RPRboolean HighDensityTrack;  ///< FOM: HighDensityTrack : RPRboolean
    RTIobjectIdArray TrackObjectIdentifiers;  ///< FOM: TrackObjectIdentifiers : RTIobjectIdArray
    static constexpr std::size_t kEncodedSize = 139;
    static constexpr std::size_t kPayload = icd::kJumboPayload;  // 1データグラムの上限。ICD の MTU 9000 より
    static constexpr bool kIsInteraction = false;
    static constexpr const char* kFomName = "HLAobjectRoot.EmitterBeam.RadarBeam";
    static constexpr std::uint32_t kClassId = 1;  // 抽出概要シートの ID 列
    static constexpr std::uint16_t kPort = 24001;  // 抽出概要シートの Port 列
};

static_assert(RadarBeam::kEncodedSize + icd::kHeaderSize <= RadarBeam::kPayload,
              "RadarBeam: 1件がペイロードに収まりません。ICDgenerator の MTU か配列上限を見直してください");

[[nodiscard]] icd::Result decode(icd::Reader& r, RadarBeam& v);
void encode(icd::Writer& w, const RadarBeam& v);

/// Reads the records batched into one datagram.
/// The class id stays a runtime argument so a deployment can override the ICD's number
/// without regenerating; pass kClassId to take it.
typedef icd::DatagramReader<RadarBeam> RadarBeamReader;

/// Packs records into one datagram until the next one will not fit.
typedef icd::DatagramWriter<RadarBeam> RadarBeamWriter;

}  // namespace icdfom

