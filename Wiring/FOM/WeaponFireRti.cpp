#include "WeaponFireRti.h"

#include "CDb.h"
#include "TypeConv.h"

namespace fom {

icdfom::WeaponFire toIcd(const tk::WeaponFire& i) {
    icdfom::WeaponFire r{};
    r.EventIdentifier          = toIcd(i.getEventIdentifier());
    r.FireControlSolutionRange = i.getFireControlSolutionRange();
    r.FireMissionIndex         = i.getFireMissionIndex();
    r.FiringLocation           = toIcd(i.getFiringLocation());
    r.FiringObjectIdentifier   = toIcd(i.getFiringObjectIdentifier());
    r.FuseType                 = static_cast<icdfom::FuseTypeEnum16>(i.getFuseType());
    r.InitialVelocityVector    = toIcd(i.getInitialVelocityVector());
    r.MunitionObjectIdentifier = toIcd(i.getMunitionObjectIdentifier());
    r.MunitionType             = toIcd(i.getMunitionType());
    r.QuantityFired            = i.getQuantityFired();
    r.RateOfFire               = i.getRateOfFire();
    r.TargetObjectIdentifier   = toIcd(i.getTargetObjectIdentifier());
    r.WarheadType              = static_cast<icdfom::WarheadTypeEnum16>(i.getWarheadType());
    return r;
}

void fillRti(const icdfom::WeaponFire& r, tk::WeaponFire* i) {
    i->setEventIdentifier(toRti(r.EventIdentifier));
    i->setFireControlSolutionRange(r.FireControlSolutionRange);
    i->setFireMissionIndex(r.FireMissionIndex);
    i->setFiringLocation(toRti(r.FiringLocation));
    i->setFiringObjectIdentifier(toRti(r.FiringObjectIdentifier));
    i->setFuseType(static_cast<std::uint16_t>(r.FuseType));
    i->setInitialVelocityVector(toRti(r.InitialVelocityVector));
    i->setMunitionObjectIdentifier(toRti(r.MunitionObjectIdentifier));
    i->setMunitionType(toRti(r.MunitionType));
    i->setQuantityFired(r.QuantityFired);
    i->setRateOfFire(r.RateOfFire);
    i->setTargetObjectIdentifier(toRti(r.TargetObjectIdentifier));
    i->setWarheadType(static_cast<std::uint16_t>(r.WarheadType));
}

void sendWeaponFire(const icdfom::WeaponFire& r) {
    tk::WeaponFire* i = CDb::getInstance().getWorld()->getInteractionManager()->getWeaponFire();
    fillRti(r, i);
    i->sendInteraction();
}

}  // namespace fom
