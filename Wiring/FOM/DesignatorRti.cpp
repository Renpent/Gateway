#include "DesignatorRti.h"

#include "CDb.h"
#include "TypeConv.h"

namespace fom {

std::vector<tk::DesignatorPtr> getRemoteDesignator() {
    return CDb::getInstance().getWorld()->getObjectManager()->getRemoteDesignator();
}

icdfom::Designator toIcd(const tk::DesignatorPtr& p) {
    icdfom::Designator r{};
    r.EntityIdentifier             = toIcd(p->getEntityIdentifier());
    r.HostObjectIdentifier         = toIcd(p->getHostObjectIdentifier());
    r.RelativePosition             = toIcd(p->getRelativePosition());
    r.CodeName                     = static_cast<icdfom::DesignatorCodeNameEnum16>(p->getCodeName());
    r.DesignatedObjectIdentifier   = toIcd(p->getDesignatedObjectIdentifier());
    r.DesignatorCode               = static_cast<icdfom::DesignatorCodeEnum16>(p->getDesignatorCode());
    r.DesignatorEmissionWavelength = p->getDesignatorEmissionWavelength();
    r.DesignatorOutputPower        = p->getDesignatorOutputPower();
    r.DesignatorSpotLocation       = toIcd(p->getDesignatorSpotLocation());
    r.DeadReckoningAlgorithm       = static_cast<icdfom::DeadReckoningAlgorithmEnum8>(p->getDeadReckoningAlgorithm());
    r.RelativeSpotLocation         = toIcd(p->getRelativeSpotLocation());
    r.SpotLinearAccelerationVector = toIcd(p->getSpotLinearAccelerationVector());
    return r;
}

std::string keyOf(const icdfom::Designator& r) { return toRti(r.HostObjectIdentifier); }

tk::DesignatorPtr registerDesignator(const std::string& key) {
    return CDb::getInstance().getWorld()->getObjectManager()->registerDesignator(key);
}

void updateDesignator(const icdfom::Designator& r, const tk::DesignatorPtr& p) {
    // ICD は常に全属性を送ってくるので、全部書く。
    // （送信側が持っていなかった属性はゼロで届く。前回値で埋めるかどうかは未決）
    p->setEntityIdentifier(toRti(r.EntityIdentifier));
    p->setHostObjectIdentifier(toRti(r.HostObjectIdentifier));
    p->setRelativePosition(toRti(r.RelativePosition));
    p->setCodeName(static_cast<std::uint16_t>(r.CodeName));
    p->setDesignatedObjectIdentifier(toRti(r.DesignatedObjectIdentifier));
    p->setDesignatorCode(static_cast<std::uint16_t>(r.DesignatorCode));
    p->setDesignatorEmissionWavelength(r.DesignatorEmissionWavelength);
    p->setDesignatorOutputPower(r.DesignatorOutputPower);
    p->setDesignatorSpotLocation(toRti(r.DesignatorSpotLocation));
    p->setDeadReckoningAlgorithm(static_cast<std::uint8_t>(r.DeadReckoningAlgorithm));
    p->setRelativeSpotLocation(toRti(r.RelativeSpotLocation));
    p->setSpotLinearAccelerationVector(toRti(r.SpotLinearAccelerationVector));
    p->update();
}

}  // namespace fom
