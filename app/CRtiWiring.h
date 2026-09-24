// 本番の配線。**実際の FOM クラスを足すときは、ここに2本のメンバと1行を足す。**
//
// CWiring（スタブの配線）と同じ形で、stub:: の代わりに hla::CTRti... の器と rti/ の変換関数を繋ぐ。
// 変換関数は world を掴まず、呼ばれたときに rti::CDb から読むので、配線は join より前に作ってよい。
//
// 使う順番（main の形）：
//
//   CRtiWiring wiring;  gw::CGateway gateway;  wiring.build(gateway);
//   join → rti::CDb::getInstance().setWorld(world) → wiring.subscribe()
//   → gateway.openAll(peer) → gateway.run(hz, 0)
//   終了: run が戻る → resign → setWorld(nullptr)

#pragma once

#include "../gateway/CGateway.h"
#include "../gateway/hla/CTRtiInteractionFromHla.h"
#include "../gateway/hla/CTRtiInteractionToHla.h"
#include "../gateway/hla/CTRtiObjectFromHla.h"
#include "../gateway/hla/CTRtiObjectToHla.h"
#include "../icd/icd_classes.h"
#include "../rti/DesignatorRti.h"
#include "../rti/WeaponFireRti.h"
#include "AddChannel.h"
#include "CCommandHandler.h"
#include "CControlHandler.h"
#include "TCommand.h"
#include "TControl.h"

namespace app {

class CRtiWiring {
public:
    // ── オブジェクト：Designator ─────────────────────────────────────
    hla::CTRtiObjectFromHla<icdfom::Designator, tk::DesignatorPtr> designatorFromHla{
        &rti::getRemoteDesignator, &rti::toIcd};                                  ///< HLA → UDP
    hla::CTRtiObjectToHla<icdfom::Designator, tk::DesignatorPtr> designatorToHla{
        &rti::keyOf, &rti::registerDesignator, &rti::writeBack};                   ///< UDP → HLA

    // ── インタラクション：WeaponFire ─────────────────────────────────
    hla::CTRtiInteractionFromHla<icdfom::WeaponFire> fireFromHla;                  ///< HLA → UDP（subscribe で繋ぐ）
    hla::CTRtiInteractionToHla<icdfom::WeaponFire> fireToHla{&rti::sendWeaponFire}; ///< UDP → HLA

    // ── FOM に無い独自データ ──────────────────────────────────────────
    CCommandHandler commandHandler;   ///< コマンド文字列の受け口
    CControlHandler controlHandler;   ///< 制御文字列の受け口

    void build(gw::CGateway& g) {
        addClass<icdfom::Designator>(g, &designatorFromHla, &designatorToHla);
        addClass<icdfom::WeaponFire>(g, &fireFromHla, &fireToHla);

        addRaw<app::TCommand>(g, &commandHandler);
        addRaw<app::TControl>(g, &controlHandler);
    }

    /// インタラクションの受信コールバックを登録する。**CDb に world を置いたあとに1回呼ぶ。**
    void subscribe() {
        rti::subscribeWeaponFire(fireFromHla);
    }
};

}  // namespace app
