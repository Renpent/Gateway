#include "CWeaponFireRti.h"

#include "CDb.h"
#include "CTypeConv.h"

icdfom::WeaponFire CWeaponFireRti::toIcd(const tk::WeaponFire& i) {
    icdfom::WeaponFire r{};
    r.EventIdentifier          = CTypeConv::toIcd(i.getEventIdentifier());
    r.FireControlSolutionRange = i.getFireControlSolutionRange();
    r.FireMissionIndex         = i.getFireMissionIndex();
    r.FiringLocation           = CTypeConv::toIcd(i.getFiringLocation());
    r.FiringObjectIdentifier   = CTypeConv::toIcd(i.getFiringObjectIdentifier());
    r.FuseType                 = static_cast<icdfom::FuseTypeEnum16>(i.getFuseType());
    r.InitialVelocityVector    = CTypeConv::toIcd(i.getInitialVelocityVector());
    r.MunitionObjectIdentifier = CTypeConv::toIcd(i.getMunitionObjectIdentifier());
    r.MunitionType             = CTypeConv::toIcd(i.getMunitionType());
    r.QuantityFired            = i.getQuantityFired();
    r.RateOfFire               = i.getRateOfFire();
    r.TargetObjectIdentifier   = CTypeConv::toIcd(i.getTargetObjectIdentifier());
    r.WarheadType              = static_cast<icdfom::WarheadTypeEnum16>(i.getWarheadType());
    return r;
}

void CWeaponFireRti::fillRti(const icdfom::WeaponFire& r, tk::WeaponFire* i) {
    i->setEventIdentifier(CTypeConv::toRti(r.EventIdentifier));
    i->setFireControlSolutionRange(r.FireControlSolutionRange);
    i->setFireMissionIndex(r.FireMissionIndex);
    i->setFiringLocation(CTypeConv::toRti(r.FiringLocation));
    i->setFiringObjectIdentifier(CTypeConv::toRti(r.FiringObjectIdentifier));
    i->setFuseType(static_cast<std::uint16_t>(r.FuseType));
    i->setInitialVelocityVector(CTypeConv::toRti(r.InitialVelocityVector));
    i->setMunitionObjectIdentifier(CTypeConv::toRti(r.MunitionObjectIdentifier));
    i->setMunitionType(CTypeConv::toRti(r.MunitionType));
    i->setQuantityFired(r.QuantityFired);
    i->setRateOfFire(r.RateOfFire);
    i->setTargetObjectIdentifier(CTypeConv::toRti(r.TargetObjectIdentifier));
    i->setWarheadType(static_cast<std::uint16_t>(r.WarheadType));
}

void CWeaponFireRti::sendWeaponFire(const icdfom::WeaponFire& r) {
    tk::WeaponFire* i = CDb::getInstance().getWorld()->getInteractionManager()->getWeaponFire();
    fillRti(r, i);
    i->sendInteraction();
}

