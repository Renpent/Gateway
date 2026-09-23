// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmitterBeam.RadarBeam
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#include "RadarBeam.h"

namespace icdfom {

icd::Result decode(icd::Reader& r, RadarBeam& v) {
    if (const icd::Result rc = decode(r, v.BeamAzimuthCenter); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BeamAzimuthSweep); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BeamElevationCenter); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BeamElevationSweep); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BeamFunctionCode); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BeamIdentifier); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.BeamParameterIndex); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.EffectiveRadiatedPower); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.EmissionFrequency); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.EmitterSystemIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.EventIdentifier); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.FrequencyRange); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.PulseRepetitionFrequency); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.PulseWidth); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.SweepSynch); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.HighDensityTrack); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.TrackObjectIdentifiers, 3, 16); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const RadarBeam& v) {
    encode(w, v.BeamAzimuthCenter);
    encode(w, v.BeamAzimuthSweep);
    encode(w, v.BeamElevationCenter);
    encode(w, v.BeamElevationSweep);
    encode(w, v.BeamFunctionCode);
    encode(w, v.BeamIdentifier);
    encode(w, v.BeamParameterIndex);
    encode(w, v.EffectiveRadiatedPower);
    encode(w, v.EmissionFrequency);
    icd::encodeBounded(w, v.EmitterSystemIdentifier, 16);
    encode(w, v.EventIdentifier);
    encode(w, v.FrequencyRange);
    encode(w, v.PulseRepetitionFrequency);
    encode(w, v.PulseWidth);
    encode(w, v.SweepSynch);
    encode(w, v.HighDensityTrack);
    icd::encodeBounded(w, v.TrackObjectIdentifiers, 3, 16);
}

}  // namespace icdfom
