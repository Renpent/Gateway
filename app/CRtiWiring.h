// 本番の配線。**実際の FOM クラスを足すときは、ここにメンバと build() の1行を足す。**
//
// CWiring（スタブの配線）と同じ形で、stub:: の代わりに hla::CTRti... の器と rti/ の変換を繋ぐ。
// 変換関数は world を掴まず、呼ばれたときに rti::CDb から読むので、配線は join より前に作ってよい。
//
// 使う順番（main の形）：
//
//   CRtiWiring wiring;  gw::CGateway gateway;  wiring.build(gateway);
//   join → rti::CDb::getInstance().setWorld(world) → wiring.subscribe()
//   → gateway.openAll(peer) → gateway.run(hz, 0)
//   終了: run が戻る → resign → setWorld(nullptr)
//
// **wiring は resign より後まで生きていること。** コールバックのオブジェクト（interactions）を
// ツールキットに貸しているため。

#pragma once

#include "../gateway/CGateway.h"
#include "../gateway/hla/CTRtiInteractionFromHla.h"
#include "../gateway/hla/CTRtiInteractionToHla.h"
#include "../gateway/hla/CTRtiObjectFromHla.h"
#include "../gateway/hla/CTRtiObjectToHla.h"
#include "../icd/icd_classes.h"
#include "../rti/CDb.h"
#include "../rti/CInteractionCallback.h"
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
    /// HLA → UDP。毎周期 getRemoteDesignator で全インスタンスを取り、toIcd で変換する
    hla::CTRtiObjectFromHla<icdfom::Designator, tk::DesignatorPtr> designatorFromHla{
        &rti::getRemoteDesignator, &rti::toIcd};
    /// UDP → HLA。keyOf でインスタンスを決め、初見なら登録し、updateDesignator で書いて update
    hla::CTRtiObjectToHla<icdfom::Designator, tk::DesignatorPtr> designatorToHla{
        &rti::keyOf, &rti::registerDesignator, &rti::updateDesignator};

    // ── インタラクション ─────────────────────────────────────────────
    /// HLA → UDP。全インタラクション共通の受信コールバックで、クラスごとのキューを持つ
    /// （interactions.weaponFireFromHla など）。RTI のスレッドから push され、周期ループが drain する
    rti::CInteractionCallback interactions;

    /// UDP → HLA（WeaponFire）。sendWeaponFire でパラメータを詰めて sendInteraction
    hla::CTRtiInteractionToHla<icdfom::WeaponFire> fireToHla{&rti::sendWeaponFire};

    // ── FOM に無い独自データ ──────────────────────────────────────────
    CCommandHandler commandHandler;   ///< コマンド文字列の受け口
    CControlHandler controlHandler;   ///< 制御文字列の受け口

    void build(gw::CGateway& g) {
        addClass<icdfom::Designator>(g, &designatorFromHla, &designatorToHla);
        addClass<icdfom::WeaponFire>(g, &interactions.weaponFireFromHla, &fireToHla);

        addRaw<app::TCommand>(g, &commandHandler);
        addRaw<app::TControl>(g, &controlHandler);
    }

    /// インタラクションの受信コールバックをツールキットに登録する。インタラクションが増えても1行のまま。
    /// **CDb に world を置いたあとに1回呼ぶ。** 登録の仕方はツールキット次第（ここは仮）。
    void subscribe() {
        rti::CDb::getInstance().getWorld()->getInteractionManager()->setInteractionCallback(&interactions);
    }
};

}  // namespace app
