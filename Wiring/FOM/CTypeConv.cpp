#include "CTypeConv.h"

icdfom::RTIobjectId CTypeConv::toIcd(const std::string& s) { return icdfom::RTIobjectId(s.begin(), s.end()); }
std::string CTypeConv::toRti(const icdfom::RTIobjectId& v) { return std::string(v.begin(), v.end()); }

icdfom::EntityIdentifierStruct CTypeConv::toIcd(const tk::EntityIdentifierStruct& v) {
    icdfom::EntityIdentifierStruct r{};
    r.FederateIdentifier.SiteID = v.FederateIdentifier.SiteID;
    r.FederateIdentifier.ApplicationID = v.FederateIdentifier.ApplicationID;
    r.EntityNumber = v.EntityNumber;
    return r;
}
tk::EntityIdentifierStruct CTypeConv::toRti(const icdfom::EntityIdentifierStruct& v) {
    tk::EntityIdentifierStruct r;
    r.FederateIdentifier.SiteID = v.FederateIdentifier.SiteID;
    r.FederateIdentifier.ApplicationID = v.FederateIdentifier.ApplicationID;
    r.EntityNumber = v.EntityNumber;
    return r;
}

icdfom::RelativePositionStruct CTypeConv::toIcd(const tk::RelativePositionStruct& v) {
    return icdfom::RelativePositionStruct{v.BodyXDistance, v.BodyYDistance, v.BodyZDistance};
}
tk::RelativePositionStruct CTypeConv::toRti(const icdfom::RelativePositionStruct& v) {
    return tk::RelativePositionStruct{v.BodyXDistance, v.BodyYDistance, v.BodyZDistance};
}

icdfom::WorldLocationStruct CTypeConv::toIcd(const tk::WorldLocationStruct& v) {
    return icdfom::WorldLocationStruct{v.X, v.Y, v.Z};
}
tk::WorldLocationStruct CTypeConv::toRti(const icdfom::WorldLocationStruct& v) {
    return tk::WorldLocationStruct{v.X, v.Y, v.Z};
}

icdfom::AccelerationVectorStruct CTypeConv::toIcd(const tk::AccelerationVectorStruct& v) {
    return icdfom::AccelerationVectorStruct{v.XAcceleration, v.YAcceleration, v.ZAcceleration};
}
tk::AccelerationVectorStruct CTypeConv::toRti(const icdfom::AccelerationVectorStruct& v) {
    return tk::AccelerationVectorStruct{v.XAcceleration, v.YAcceleration, v.ZAcceleration};
}

icdfom::VelocityVectorStruct CTypeConv::toIcd(const tk::VelocityVectorStruct& v) {
    return icdfom::VelocityVectorStruct{v.XVelocity, v.YVelocity, v.ZVelocity};
}
tk::VelocityVectorStruct CTypeConv::toRti(const icdfom::VelocityVectorStruct& v) {
    return tk::VelocityVectorStruct{v.XVelocity, v.YVelocity, v.ZVelocity};
}

icdfom::EventIdentifierStruct CTypeConv::toIcd(const tk::EventIdentifierStruct& v) {
    icdfom::EventIdentifierStruct r{};
    r.EventCount = v.EventCount;
    r.IssuingObjectIdentifier = toIcd(v.IssuingObjectIdentifier);
    return r;
}
tk::EventIdentifierStruct CTypeConv::toRti(const icdfom::EventIdentifierStruct& v) {
    tk::EventIdentifierStruct r;
    r.EventCount = v.EventCount;
    r.IssuingObjectIdentifier = toRti(v.IssuingObjectIdentifier);
    return r;
}

icdfom::EntityTypeStruct CTypeConv::toIcd(const tk::EntityTypeStruct& v) {
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
tk::EntityTypeStruct CTypeConv::toRti(const icdfom::EntityTypeStruct& v) {
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

