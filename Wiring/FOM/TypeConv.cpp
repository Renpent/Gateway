#include "TypeConv.h"

namespace fom {

icdfom::RTIobjectId toIcd(const std::string& s) { return icdfom::RTIobjectId(s.begin(), s.end()); }
std::string toRti(const icdfom::RTIobjectId& v) { return std::string(v.begin(), v.end()); }

icdfom::EntityIdentifierStruct toIcd(const tk::EntityIdentifierStruct& v) {
    icdfom::EntityIdentifierStruct r{};
    r.FederateIdentifier.SiteID = v.FederateIdentifier.SiteID;
    r.FederateIdentifier.ApplicationID = v.FederateIdentifier.ApplicationID;
    r.EntityNumber = v.EntityNumber;
    return r;
}
tk::EntityIdentifierStruct toRti(const icdfom::EntityIdentifierStruct& v) {
    tk::EntityIdentifierStruct r;
    r.FederateIdentifier.SiteID = v.FederateIdentifier.SiteID;
    r.FederateIdentifier.ApplicationID = v.FederateIdentifier.ApplicationID;
    r.EntityNumber = v.EntityNumber;
    return r;
}

icdfom::RelativePositionStruct toIcd(const tk::RelativePositionStruct& v) {
    return icdfom::RelativePositionStruct{v.BodyXDistance, v.BodyYDistance, v.BodyZDistance};
}
tk::RelativePositionStruct toRti(const icdfom::RelativePositionStruct& v) {
    return tk::RelativePositionStruct{v.BodyXDistance, v.BodyYDistance, v.BodyZDistance};
}

icdfom::WorldLocationStruct toIcd(const tk::WorldLocationStruct& v) {
    return icdfom::WorldLocationStruct{v.X, v.Y, v.Z};
}
tk::WorldLocationStruct toRti(const icdfom::WorldLocationStruct& v) {
    return tk::WorldLocationStruct{v.X, v.Y, v.Z};
}

icdfom::AccelerationVectorStruct toIcd(const tk::AccelerationVectorStruct& v) {
    return icdfom::AccelerationVectorStruct{v.XAcceleration, v.YAcceleration, v.ZAcceleration};
}
tk::AccelerationVectorStruct toRti(const icdfom::AccelerationVectorStruct& v) {
    return tk::AccelerationVectorStruct{v.XAcceleration, v.YAcceleration, v.ZAcceleration};
}

icdfom::VelocityVectorStruct toIcd(const tk::VelocityVectorStruct& v) {
    return icdfom::VelocityVectorStruct{v.XVelocity, v.YVelocity, v.ZVelocity};
}
tk::VelocityVectorStruct toRti(const icdfom::VelocityVectorStruct& v) {
    return tk::VelocityVectorStruct{v.XVelocity, v.YVelocity, v.ZVelocity};
}

icdfom::EventIdentifierStruct toIcd(const tk::EventIdentifierStruct& v) {
    icdfom::EventIdentifierStruct r{};
    r.EventCount = v.EventCount;
    r.IssuingObjectIdentifier = toIcd(v.IssuingObjectIdentifier);
    return r;
}
tk::EventIdentifierStruct toRti(const icdfom::EventIdentifierStruct& v) {
    tk::EventIdentifierStruct r;
    r.EventCount = v.EventCount;
    r.IssuingObjectIdentifier = toRti(v.IssuingObjectIdentifier);
    return r;
}

icdfom::EntityTypeStruct toIcd(const tk::EntityTypeStruct& v) {
    icdfom::EntityTypeStruct r{};
    r.EntityKind = v.EntityKind;
    r.Domain = v.Domain;
    r.CountryCode = v.CountryCode;
    r.Category = v.Category;
    r.Subcategory = v.Subcategory;
    r.Specific = v.Specific;
    r.Extra = v.Extra;
    return r;
}
tk::EntityTypeStruct toRti(const icdfom::EntityTypeStruct& v) {
    tk::EntityTypeStruct r;
    r.EntityKind = v.EntityKind;
    r.Domain = v.Domain;
    r.CountryCode = v.CountryCode;
    r.Category = v.Category;
    r.Subcategory = v.Subcategory;
    r.Specific = v.Specific;
    r.Extra = v.Extra;
    return r;
}

}  // namespace fom
