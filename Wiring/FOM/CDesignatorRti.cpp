#include "CDesignatorRti.h"

#include <string>

#include "CDb.h"
#include "CTypeConv.h"

std::vector<tk::DesignatorPtr> CDesignatorRti::getRemoteDesignator() {
    return CDb::getInstance().getWorld()->getObjectManager()->getRemoteDesignator();
}

icdfom::Designator CDesignatorRti::toIcd(const tk::DesignatorPtr& p) {
    icdfom::Designator r{};
    r.EntityIdentifier             = CTypeConv::toIcd(p->getEntityIdentifier());
    r.HostObjectIdentifier         = CTypeConv::toIcd(p->getHostObjectIdentifier());
    r.RelativePosition             = CTypeConv::toIcd(p->getRelativePosition());
    r.CodeName                     = static_cast<icdfom::DesignatorCodeNameEnum16>(p->getCodeName());
    r.DesignatedObjectIdentifier   = CTypeConv::toIcd(p->getDesignatedObjectIdentifier());
    r.DesignatorCode               = static_cast<icdfom::DesignatorCodeEnum16>(p->getDesignatorCode());
    r.DesignatorEmissionWavelength = p->getDesignatorEmissionWavelength();
    r.DesignatorOutputPower        = p->getDesignatorOutputPower();
    r.DesignatorSpotLocation       = CTypeConv::toIcd(p->getDesignatorSpotLocation());
    r.DeadReckoningAlgorithm       = static_cast<icdfom::DeadReckoningAlgorithmEnum8>(p->getDeadReckoningAlgorithm());
    r.RelativeSpotLocation         = CTypeConv::toIcd(p->getRelativeSpotLocation());
    r.SpotLinearAccelerationVector = CTypeConv::toIcd(p->getSpotLinearAccelerationVector());
    return r;
}

int CDesignatorRti::keyOf(const icdfom::Designator& r) { return r.EntityIdentifier.EntityNumber; }

tk::DesignatorPtr CDesignatorRti::registerDesignator(int key) {
    // インスタンス名はフェデレーション全体で一意である必要がある。名前を任せられる API なら任せてよい
    return CDb::getInstance().getWorld()->getObjectManager()->registerDesignator(
        "Designator-" + std::to_string(key));
}

void CDesignatorRti::updateDesignator(const icdfom::Designator& r, const tk::DesignatorPtr& p) {
    // ICD は常に全属性を送ってくるので、全部書く。
    // （送信側が持っていなかった属性はゼロで届く。前回値で埋めるかどうかは未決）
    p->setEntityIdentifier(CTypeConv::toRti(r.EntityIdentifier));
    p->setHostObjectIdentifier(CTypeConv::toRti(r.HostObjectIdentifier));
    p->setRelativePosition(CTypeConv::toRti(r.RelativePosition));
    p->setCodeName(static_cast<std::uint16_t>(r.CodeName));
    p->setDesignatedObjectIdentifier(CTypeConv::toRti(r.DesignatedObjectIdentifier));
    p->setDesignatorCode(static_cast<std::uint16_t>(r.DesignatorCode));
    p->setDesignatorEmissionWavelength(r.DesignatorEmissionWavelength);
    p->setDesignatorOutputPower(r.DesignatorOutputPower);
    p->setDesignatorSpotLocation(CTypeConv::toRti(r.DesignatorSpotLocation));
    p->setDeadReckoningAlgorithm(static_cast<std::uint8_t>(r.DeadReckoningAlgorithm));
    p->setRelativeSpotLocation(CTypeConv::toRti(r.RelativeSpotLocation));
    p->setSpotLinearAccelerationVector(CTypeConv::toRti(r.SpotLinearAccelerationVector));
    p->update();
}

