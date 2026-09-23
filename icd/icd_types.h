// 自動生成 — 編集しないこと。
// FOMのデータ型
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#pragma once

#include "icd_codec.h"

namespace icdfom {

// The runtime's primitive and container codecs. Generated types are found through
// argument-dependent lookup; these are not, so they are named explicitly.
using icd::decode;
using icd::encode;
using icd::fixedSize;
using icd::wireSize;

/// FOM: AngleRadianFloat32  [radian (rad)]
typedef float AngleRadianFloat32;

/// FOM: BeamFunctionCodeEnum8
/// 値は BeamFunctionCodeEnum8::<列挙子名>。
enum class BeamFunctionCodeEnum8 : uint8_t {
    Other = 0,
    Search = 1,
    HeightFinder = 2,
    Acquisition = 3,
    Tracking = 4,
    AcquisitionAndTracking = 5,
    CommandGuidance = 6,
    Illumination = 7,
    RangeOnlyRadar = 8,
    MissileBeacon = 9,
    MissileFuze = 10,
    ActiveRadarMissileSeeker = 11,
    Jammer = 12,
    IFF = 13,
    NavigationalOrWeather = 14,
    Meteorological = 15,
    DataTransmission = 16,
    NavigationalDirectionalBeacon = 17,
    Time_SharedSearch = 20,  ///< FOM: "Time-SharedSearch"
    Time_SharedAcquisition = 21,  ///< FOM: "Time-SharedAcquisition"
    Time_SharedTrack = 22,  ///< FOM: "Time-SharedTrack"
    Time_SharedCommandGuidance = 23,  ///< FOM: "Time-SharedCommandGuidance"
    Time_SharedIllumination = 24,  ///< FOM: "Time-SharedIllumination"
    Time_SharedJamming = 25,  ///< FOM: "Time-SharedJamming"
};

[[nodiscard]] inline icd::Result decode(icd::Reader& r, BeamFunctionCodeEnum8& v) {
    uint8_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<BeamFunctionCodeEnum8>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, BeamFunctionCodeEnum8 v) {
    icd::encode(w, static_cast<uint8_t>(v));
}

}  // namespace icdfom
namespace icd {
template <> struct FixedSize<icdfom::BeamFunctionCodeEnum8> { static constexpr std::size_t value = 1; };
}
namespace icdfom {

/// FOM: Octet
typedef uint8_t Octet;

/// FOM: UnsignedInteger16
typedef uint16_t UnsignedInteger16;

/// FOM: PowerRatioDecibelMilliwattFloat32  [decibel milliwatt (dBm)]
typedef float PowerRatioDecibelMilliwattFloat32;

/// FOM: FrequencyHertzFloat32  [hertz (Hz)]
typedef float FrequencyHertzFloat32;

/// FOM: HLAASCIIchar
typedef uint8_t HLAASCIIchar;

/// FOM: RTIobjectId  cardinality=Dynamic encoding=RPRnullTerminatedArray
typedef std::vector<HLAASCIIchar> RTIobjectId;

/// FOM: EventIdentifierStruct
struct EventIdentifierStruct {
    UnsignedInteger16 EventCount;  ///< FOM: EventCount : UnsignedInteger16
    RTIobjectId IssuingObjectIdentifier;  ///< FOM: IssuingObjectIdentifier : RTIobjectId
    static constexpr std::size_t kEncodedSize = 20;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, EventIdentifierStruct& v);
void encode(icd::Writer& w, const EventIdentifierStruct& v);

/// FOM: TimeMicrosecondFloat32  [microsecond]
typedef float TimeMicrosecondFloat32;

/// FOM: PercentFloat32  [percent (%)]
typedef float PercentFloat32;

/// FOM: RPRboolean
/// 値は RPRboolean::<列挙子名>。
enum class RPRboolean : uint8_t {
    False = 0,
    True = 1,
};

[[nodiscard]] inline icd::Result decode(icd::Reader& r, RPRboolean& v) {
    uint8_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<RPRboolean>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, RPRboolean v) {
    icd::encode(w, static_cast<uint8_t>(v));
}

}  // namespace icdfom
namespace icd {
template <> struct FixedSize<icdfom::RPRboolean> { static constexpr std::size_t value = 1; };
}
namespace icdfom {

/// FOM: RTIobjectIdArray  cardinality=Dynamic encoding=HLAvariableArray
typedef std::vector<RTIobjectId> RTIobjectIdArray;

/// FOM: FederateIdentifierStruct
struct FederateIdentifierStruct {
    UnsignedInteger16 SiteID;  ///< FOM: SiteID : UnsignedInteger16
    UnsignedInteger16 ApplicationID;  ///< FOM: ApplicationID : UnsignedInteger16
    static constexpr std::size_t kEncodedSize = 4;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, FederateIdentifierStruct& v);
void encode(icd::Writer& w, const FederateIdentifierStruct& v);

/// FOM: EntityIdentifierStruct
struct EntityIdentifierStruct {
    FederateIdentifierStruct FederateIdentifier;  ///< FOM: FederateIdentifier : FederateIdentifierStruct
    UnsignedInteger16 EntityNumber;  ///< FOM: EntityNumber : UnsignedInteger16
    static constexpr std::size_t kEncodedSize = 6;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, EntityIdentifierStruct& v);
void encode(icd::Writer& w, const EntityIdentifierStruct& v);

/// FOM: MeterFloat32  [meter (m)]
typedef float MeterFloat32;

/// FOM: RelativePositionStruct
struct RelativePositionStruct {
    MeterFloat32 BodyXDistance;  ///< FOM: BodyXDistance : MeterFloat32
    MeterFloat32 BodyYDistance;  ///< FOM: BodyYDistance : MeterFloat32
    MeterFloat32 BodyZDistance;  ///< FOM: BodyZDistance : MeterFloat32
    static constexpr std::size_t kEncodedSize = 12;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, RelativePositionStruct& v);
void encode(icd::Writer& w, const RelativePositionStruct& v);

/// FOM: ReceiverOperationalStatusEnum16
/// 値は ReceiverOperationalStatusEnum16::<列挙子名>。
enum class ReceiverOperationalStatusEnum16 : uint16_t {
    Off = 0,
    OnButNotReceiving = 1,
    OnAndReceiving = 2,
};

[[nodiscard]] inline icd::Result decode(icd::Reader& r, ReceiverOperationalStatusEnum16& v) {
    uint16_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<ReceiverOperationalStatusEnum16>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, ReceiverOperationalStatusEnum16 v) {
    icd::encode(w, static_cast<uint16_t>(v));
}

}  // namespace icdfom
namespace icd {
template <> struct FixedSize<icdfom::ReceiverOperationalStatusEnum16> { static constexpr std::size_t value = 2; };
}
namespace icdfom {

/// FOM: DepthMeterFloat32  [meter (m)]
typedef float DepthMeterFloat32;

/// FOM: DepthMeterFloat32LengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<DepthMeterFloat32> DepthMeterFloat32LengthlessArray;

/// FOM: MinefieldFusingEnum32
/// 値は MinefieldFusingEnum32::<列挙子名>。
enum class MinefieldFusingEnum32 : uint32_t {
    NoFuse = 0,
    Other = 1,
    Pressure = 2,
    Magnetic = 3,
    TiltRod = 4,
    Command = 5,
    TripWire = 6,
};

[[nodiscard]] inline icd::Result decode(icd::Reader& r, MinefieldFusingEnum32& v) {
    uint32_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<MinefieldFusingEnum32>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, MinefieldFusingEnum32 v) {
    icd::encode(w, static_cast<uint32_t>(v));
}

}  // namespace icdfom
namespace icd {
template <> struct FixedSize<icdfom::MinefieldFusingEnum32> { static constexpr std::size_t value = 4; };
}
namespace icdfom {

/// FOM: OctetArray3  cardinality=3 encoding=HLAfixedArray
typedef std::array<Octet, 3> OctetArray3;

/// FOM: MineFusingStruct
struct MineFusingStruct {
    MinefieldFusingEnum32 Primary;  ///< FOM: Primary : MinefieldFusingEnum32
    MinefieldFusingEnum32 Secondary;  ///< FOM: Secondary : MinefieldFusingEnum32
    RPRboolean AntiHandlingDevice;  ///< FOM: AntiHandlingDevice : RPRboolean
    OctetArray3 Padding;  ///< FOM: Padding : OctetArray3
    static constexpr std::size_t kEncodedSize = 12;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, MineFusingStruct& v);
void encode(icd::Writer& w, const MineFusingStruct& v);

/// FOM: MineFusingStructLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<MineFusingStruct> MineFusingStructLengthlessArray;

/// FOM: ClockTimeHourInteger32  [hour]
typedef int32_t ClockTimeHourInteger32;

/// FOM: TimestampUnsignedInteger32  [3600/(2^31) second]
typedef uint32_t TimestampUnsignedInteger32;

/// FOM: ClockTimeStruct
struct ClockTimeStruct {
    ClockTimeHourInteger32 Hours;  ///< FOM: Hours : ClockTimeHourInteger32
    TimestampUnsignedInteger32 TimePastTheHour;  ///< FOM: TimePastTheHour : TimestampUnsignedInteger32
    static constexpr std::size_t kEncodedSize = 8;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, ClockTimeStruct& v);
void encode(icd::Writer& w, const ClockTimeStruct& v);

/// FOM: ClockTimeStructLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<ClockTimeStruct> ClockTimeStructLengthlessArray;

/// FOM: MineIdentifier
typedef uint16_t MineIdentifier;

/// FOM: MineIdentifierLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<MineIdentifier> MineIdentifierLengthlessArray;

/// FOM: MeterFloat64  [meter (m)]
typedef double MeterFloat64;

/// FOM: WorldLocationStruct
struct WorldLocationStruct {
    MeterFloat64 X;  ///< FOM: X : MeterFloat64
    MeterFloat64 Y;  ///< FOM: Y : MeterFloat64
    MeterFloat64 Z;  ///< FOM: Z : MeterFloat64
    static constexpr std::size_t kEncodedSize = 24;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, WorldLocationStruct& v);
void encode(icd::Writer& w, const WorldLocationStruct& v);

/// FOM: WorldLocationStructLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<WorldLocationStruct> WorldLocationStructLengthlessArray;

/// FOM: OrientationStruct
struct OrientationStruct {
    AngleRadianFloat32 Psi;  ///< FOM: Psi : AngleRadianFloat32
    AngleRadianFloat32 Theta;  ///< FOM: Theta : AngleRadianFloat32
    AngleRadianFloat32 Phi;  ///< FOM: Phi : AngleRadianFloat32
    static constexpr std::size_t kEncodedSize = 12;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, OrientationStruct& v);
void encode(icd::Writer& w, const OrientationStruct& v);

/// FOM: OrientationStructLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<OrientationStruct> OrientationStructLengthlessArray;

/// FOM: EntityTypeStruct
struct EntityTypeStruct {
    Octet EntityKind;  ///< FOM: EntityKind : Octet
    Octet Domain;  ///< FOM: Domain : Octet
    UnsignedInteger16 CountryCode;  ///< FOM: CountryCode : UnsignedInteger16
    Octet Category;  ///< FOM: Category : Octet
    Octet Subcategory;  ///< FOM: Subcategory : Octet
    Octet Specific;  ///< FOM: Specific : Octet
    Octet Extra;  ///< FOM: Extra : Octet
    static constexpr std::size_t kEncodedSize = 8;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, EntityTypeStruct& v);
void encode(icd::Writer& w, const EntityTypeStruct& v);

/// FOM: UnsignedInteger8
typedef uint8_t UnsignedInteger8;

/// FOM: UnsignedInteger8LengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<UnsignedInteger8> UnsignedInteger8LengthlessArray;

/// FOM: MinefieldPaintSchemeEnum32
/// 値は MinefieldPaintSchemeEnum32::<列挙子名>。
enum class MinefieldPaintSchemeEnum32 : uint32_t {
    Other = 0,
    Standard = 1,
    CamouflageDesert = 2,
    CamouflageJungle = 3,
    CamouflageSnow = 4,
    CamouflageGravel = 5,
    CamouflagePavement = 6,
    CamouflageSand = 7,
    NaturalWood = 8,
    Clear = 9,
    Red = 10,
    Blue = 11,
    Green = 12,
    Olive = 13,
    White = 14,
    Tan = 15,
    Black = 16,
    Yellow = 17,
    Brown = 18,
};

[[nodiscard]] inline icd::Result decode(icd::Reader& r, MinefieldPaintSchemeEnum32& v) {
    uint32_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<MinefieldPaintSchemeEnum32>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, MinefieldPaintSchemeEnum32 v) {
    icd::encode(w, static_cast<uint32_t>(v));
}

}  // namespace icdfom
namespace icd {
template <> struct FixedSize<icdfom::MinefieldPaintSchemeEnum32> { static constexpr std::size_t value = 4; };
}
namespace icdfom {

/// FOM: MinefieldPaintSchemeLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<MinefieldPaintSchemeEnum32> MinefieldPaintSchemeLengthlessArray;

/// FOM: MineDielectricDifference
typedef float MineDielectricDifference;

/// FOM: MineDielectricDifferenceLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<MineDielectricDifference> MineDielectricDifferenceLengthlessArray;

/// FOM: MinefieldSensorTypeEnum32
/// 値は MinefieldSensorTypeEnum32::<列挙子名>。
enum class MinefieldSensorTypeEnum32 : uint32_t {
    Other = 0,
    UnaidedEyeActivelySearching = 4096,
    UnaidedEyeNotActivelySearching = 4097,
    Binoculars = 4098,
    ImageIntensifier = 4099,
    HMMWVOccupantActivelySearching = 4100,
    HMMWVOccupantNotActivelySearching = 4101,
    TruckOccupantActivelySearching = 4102,
    TruckOccupantNotActivelySearching = 4103,
    TrackedVehicleOccupantClosedHatchActivelySearching = 4104,
    TrackedVehicleOccupantClosedHatchNotActivelySearching = 4105,
    TrackedVehicleOccupantOpenHatchActivelySearching = 4106,
    TrackedVehicleOccupantOpenHatchNotActivelySearching = 4107,
    FLIR_Generic3_5 = 8192,
    FLIR_Generic8_12 = 8193,
    FLIR_ASTAMIDS_I = 8194,
    FLIR_ASTAMIDS_II = 8195,
    FLIR_GSTAMIDS3_5 = 8196,
    FLIR_GSTAMIDS8_12 = 8197,
    FLIR_HSTAMIDS3_5 = 8198,
    FLIR_HSTAMIDS8_12 = 8199,
    FLIR_COBRA3_5 = 8200,
    FLIR_COBRA8_12 = 8201,
    RADAR_Generic = 12288,
    RADAR_Generic_GPR = 12289,
    RADAR_GSTAMIDS_I = 12290,
    RADAR_GSTAMIDS_II = 12291,
    RADAR_HSTAMIDS_I = 12292,
    RADAR_HSTAMIDS_II = 12293,
    Magnetic_Generic = 16384,
    Magnetic_ANPSS_11 = 16385,
    Magnetic_ANPSS_12 = 16386,
    Magnetic_GSTAMIDS = 16389,
    Laser_Generic = 20480,
    Laser_ASTAMIDS = 20481,
    SONAR_Generic = 24576,
    Physical_GenericProbe = 28672,
    Physical_ProbeMetalContent = 28673,
    Physical_ProbeNoMetalContent = 28674,
    Multispectral_Generic = 32768,
};

[[nodiscard]] inline icd::Result decode(icd::Reader& r, MinefieldSensorTypeEnum32& v) {
    uint32_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<MinefieldSensorTypeEnum32>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, MinefieldSensorTypeEnum32 v) {
    icd::encode(w, static_cast<uint32_t>(v));
}

}  // namespace icdfom
namespace icd {
template <> struct FixedSize<icdfom::MinefieldSensorTypeEnum32> { static constexpr std::size_t value = 4; };
}
namespace icdfom {

/// FOM: MinefieldSensorTypeLengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<MinefieldSensorTypeEnum32> MinefieldSensorTypeLengthlessArray;

/// FOM: TemperatureDegreeCelsiusFloat32  [degree Celsius (C)]
typedef float TemperatureDegreeCelsiusFloat32;

/// FOM: TemperatureDegreeCelsiusFloat32LengthlessArray  cardinality=Dynamic encoding=RPRlengthlessArray
typedef std::vector<TemperatureDegreeCelsiusFloat32> TemperatureDegreeCelsiusFloat32LengthlessArray;

/// FOM: LengthMeterFloat32  [meter (m)]
typedef float LengthMeterFloat32;

/// FOM: UnsignedInteger32
typedef uint32_t UnsignedInteger32;

/// FOM: FuseTypeEnum16
/// 値は FuseTypeEnum16::<列挙子名>。
enum class FuseTypeEnum16 : uint16_t {
    Other = 0,
    IntelligentInfluence = 10,
    Sensor = 20,
    SelfDestruct = 30,
    UltraQuick = 40,
    Body = 50,
    DeepIntrusion = 60,
    Multifunction = 100,
    PointDetonation_PD = 200,
    BaseDetonation_BD = 300,
    Contact = 1000,
    ContactInstantImpact = 1100,
    ContactDelayed = 1200,
    Contact10msDelay = 1201,
    Contact20msDelay = 1202,
    Contact50msDelay = 1205,
    Contact60msDelay = 1206,
    Contact100msDelay = 1210,
    Contact125msDelay = 1212,
    Contact250msDelay = 1225,
    ContactElectronicObliqueContact = 1300,
    ContactGraze = 1400,
    ContactCrush = 1500,
    ContactHydrostatic = 1600,
    ContactMechanical = 1700,
    ContactChemical = 1800,
    ContactPiezoelectric = 1900,
    ContactPointInitiating = 1910,
    ContactPointInitiatingBaseDetonating = 1920,
    ContactBaseDetonating = 1930,
    ContactBallisticCapAndBase = 1940,
    ContactBase = 1950,
    ContactNose = 1960,
    ContactFittedInStandoffProbe = 1970,
    ContactNonAligned = 1980,
    Timed = 2000,
    TimedProgrammable = 2100,
    TimedBurnout = 2200,
    TimedPyrotechnic = 2300,
    TimedElectronic = 2400,
    TimedBaseDelay = 2500,
    TimedReinforcedNoseImpactDelay = 2600,
    TimedShortDelayImpact = 2700,
    Timed10msDelay = 2701,
    Timed20msDelay = 2702,
    Timed50msDelay = 2705,
    Timed60msDelay = 2706,
    Timed100msDelay = 2710,
    Timed125msDelay = 2712,
    Timed250msDelay = 2725,
    TimedNoseMountedVariableDelay = 2800,
    TimedLongDelaySide = 2900,
    TimedSelectableDelay = 2910,
    TimedImpact = 2920,
    TimedSequence = 2930,
    Proximity = 3000,
    ProximityActiveLaser = 3100,
    ProximityMagneticMagpolarity = 3200,
    ProximityActiveDopplerRadar = 3300,
    ProximityRadioFrequencyRF = 3400,
    ProximityProgrammable = 3500,
    ProximityProgrammablePrefragmented = 3600,
    ProximityInfrared = 3700,
    Command = 4000,
    CommandElectronicRemotelySet = 4100,
    Altitude = 5000,
    AltitudeRadioAltimeter = 5100,
    AltitudeAirBurst = 5200,
    Depth = 6000,
    Acoustic = 7000,
    Pressure = 8000,
    PressureDelay = 8010,
    Inert = 8100,
    Dummy = 8110,
    Practice = 8120,
    PlugRepresenting = 8130,
    Training = 8150,
    Pyrotechnic = 9000,
    PyrotechnicDelay = 9010,
    ElectroOptical = 9100,
    ElectroMechanical = 9110,
    ElectroMechanicalNose = 9120,
    Strikerless = 9200,
    StrikerlessNoseImpact = 9210,
    StrikerlessCompressionIgnition = 9220,
    CompressionIgnition = 9300,
    CompressionIgnitionStrikerlessNoseImpact = 9310,
    Percussion = 9400,
    PercussionInstantaneous = 9410,
    Electronic = 9500,
    ElectronicInternallyMounted = 9510,
    ElectronicRangeSetting = 9520,
    ElectronicProgrammed = 9530,
    Mechanical = 9600,
    MechanicalNose = 9610,
    MechanicalTail = 9620,
};

[[nodiscard]] inline icd::Result decode(icd::Reader& r, FuseTypeEnum16& v) {
    uint16_t raw = 0;
    if (const icd::Result rc = icd::decode(r, raw); rc != icd::Result::Ok) return rc;
    v = static_cast<FuseTypeEnum16>(raw);
    return icd::Result::Ok;
}

inline void encode(icd::Writer& w, FuseTypeEnum16 v) {
    icd::encode(w, static_cast<uint16_t>(v));
}

}  // namespace icdfom
namespace icd {
template <> struct FixedSize<icdfom::FuseTypeEnum16> { static constexpr std::size_t value = 2; };
}
namespace icdfom {

/// FOM: VelocityMeterPerSecondFloat32  [meter per second (m/s)]
typedef float VelocityMeterPerSecondFloat32;

/// FOM: VelocityVectorStruct
struct VelocityVectorStruct {
    VelocityMeterPerSecondFloat32 XVelocity;  ///< FOM: XVelocity : VelocityMeterPerSecondFloat32
    VelocityMeterPerSecondFloat32 YVelocity;  ///< FOM: YVelocity : VelocityMeterPerSecondFloat32
    VelocityMeterPerSecondFloat32 ZVelocity;  ///< FOM: ZVelocity : VelocityMeterPerSecondFloat32
    static constexpr std::size_t kEncodedSize = 12;
};

[[nodiscard]] icd::Result decode(icd::Reader& r, VelocityVectorStruct& v);
void encode(icd::Writer& w, const VelocityVectorStruct& v);

/// FOM: WarheadTypeEnum16
/// 値は WarheadTypeEnum16::<列挙子名>。
enum class WarheadTypeEnum16 : uint16_t {
    Other = 0,
    CargoVariableSubmunitions = 10,
    FuelAirExplosive = 20,
    GlassBeads = 30,
    Warhead_1um = 31,
    Warhead_5um = 32,
    Warhead_10um = 33,
    HighExplosive = 1000,
    HE_Plastic = 1100,
    HE_Incendiary = 1200,
    HE_Fragmentation = 1300,
    HE_Antitank = 1400,
    HE_Bomblets = 1500,
    HE_ShapedCharge = 1600,
    HE_ContinuousRod = 1610,
    HE_TungstenBall = 1615,
    HE_BlastFragmentation = 1620,
    HE_SteerableDartswithHE = 1625,
    HE_Darts = 1630,
    HE_Flechettes = 1635,
    HE_DirectedFragmentation = 1640,
    HE_SemiArmorPiercing = 1645,
    HE_ShapedChargeFragmentation = 1650,
    HE_SemiArmorPiercingFragmentation = 1655,
    HE_HollowCharge = 1660,
    HE_DoubleHollowCharge = 1665,
    HE_GeneralPurpose = 1670,
    HE_BlastPenetrator = 1675,
    HE_RodPenetrator = 1680,
    HE_Antipersonnel = 1685,
    Smoke = 2000,
    Illumination = 3000,
    Practice = 4000,
    Kinetic = 5000,
    Mines = 6000,
    Nuclear = 7000,
    NuclearIMT = 7010,
    ChemicalGeneral = 8000,
    ChemicalBlisterAgent = 8100,
    HD_Mustard = 8110,
    ThickenedHD_Mustard = 8115,
    DustyHD_Mustard = 8120,
    ChemicalBloodAgent = 8200,
    AC_HCN = 8210,
    CK_CNCI = 8215,
    CG_Phosgene = 8220,
    ChemicalNerveAgent = 8300,
    VX = 8310,
    ThickenedVX = 8315,
    DustyVX = 8320,
    GA_Tabun = 8325,
    ThickenedGA_Tabun = 8330,
    DustyGA_Tabun = 8335,
    GB_Sarin = 8340,
    ThickenedGB_Sarin = 8345,
    DustyGB_Sarin = 8350,
    GD_Soman = 8355,
    ThickenedGD_Soman = 8360,
    DustyGD_Soman = 8365,
    GF = 8370,
    ThickenedGF = 8375,
    DustyGF = 8380,
    Biological = 9000,
    BiologicalVirus = 9100,
    BiologicalBacteria = 9200,
    BiologicalRickettsia = 9300,
    BiologicalGeneticallyModifiedMicroOrganisms = 9400,
    BiologicalToxin = 9500,
};

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
namespace icd {
template <> struct FixedSize<icdfom::WarheadTypeEnum16> { static constexpr std::size_t value = 2; };
}
namespace icdfom {

}  // namespace icdfom

