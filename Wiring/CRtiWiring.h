// 本番の配線。**実際の FOM クラスを足すときは、ここにメンバと build() の1行を足す。**
//
// CStubWiring（スタブの配線）と同じ形で、Stub/ の代用品の代わりに CTRti... の器と Wiring/FOM/ の変換を繋ぐ。
// 変換関数は world を掴まず、呼ばれたときに CDb から読むので、配線は join より前に作ってよい。
//
// 使う順番（main の形）：
//
//   CRtiWiring wiring;  CGateway gateway;  wiring.build(gateway);
//   join → CDb::getInstance().setWorld(world) → wiring.subscribe()
//   → gateway.openAll(peer) → gateway.run(hz, 0)
//   終了: run が戻る → resign → setWorld(nullptr)
//
// **wiring は resign より後まで生きていること。** コールバックのオブジェクト（interactions）を
// ツールキットに貸しているため。

#pragma once

#include "../Core/CGateway.h"
#include "../Core/HLA/CTRtiInteractionFromHla.h"
#include "../Core/HLA/CTRtiInteractionToHla.h"
#include "../Core/HLA/CTRtiObjectFromHla.h"
#include "../Core/HLA/CTRtiObjectToHla.h"
#include "../ICD/icd_classes.h"
#include "CWiring.h"
#include "FOM/CDb.h"
#include "FOM/CInteractionCallback.h"
#include "FOM/CDesignatorRti.h"
#include "FOM/CWeaponFireRti.h"
#include "NonFOM/CCommandHandler.h"
#include "NonFOM/CControlHandler.h"
#include "NonFOM/TCommand.h"
#include "NonFOM/TControl.h"

class CRtiWiring : private CWiring {
public:
    // ── オブジェクト：Designator ─────────────────────────────────────
    /// HLA → UDP。毎周期 getRemoteDesignator で全インスタンスを取り、toIcd で変換する
    CTRtiObjectFromHla<icdfom::Designator, tk::DesignatorPtr> designatorFromHla{
        &CDesignatorRti::getRemoteDesignator, &CDesignatorRti::toIcd};
    /// UDP → HLA。keyOf でインスタンスを決め、初見なら登録し、updateDesignator で書いて update
    CTRtiObjectToHla<icdfom::Designator, tk::DesignatorPtr> designatorToHla{
        &CDesignatorRti::keyOf, &CDesignatorRti::registerDesignator, &CDesignatorRti::updateDesignator};

    // ── インタラクション ─────────────────────────────────────────────
    /// HLA → UDP。全インタラクション共通の受信コールバックで、クラスごとのキューを持つ
    /// （interactions.weaponFireFromHla など）。RTI のスレッドから push され、周期ループが drain する
    CInteractionCallback interactions;

    /// UDP → HLA（WeaponFire）。sendWeaponFire でパラメータを詰めて sendInteraction
    CTRtiInteractionToHla<icdfom::WeaponFire> fireToHla{&CWeaponFireRti::sendWeaponFire};

    // ── FOM に無い独自データ ──────────────────────────────────────────
    CCommandHandler commandHandler;   ///< コマンド文字列の受け口
    CControlHandler controlHandler;   ///< 制御文字列の受け口

    void build(CGateway& g) {
        addClass<icdfom::Designator>(g, &designatorFromHla, &designatorToHla);
        addClass<icdfom::WeaponFire>(g, &interactions.weaponFireFromHla, &fireToHla);

        addRaw<TCommand>(g, &commandHandler);
        addRaw<TControl>(g, &controlHandler);
    }

    /// インタラクションの受信コールバックをツールキットに登録する。インタラクションが増えても1行のまま。
    /// **CDb に world を置いたあとに1回呼ぶ。** 登録の仕方はツールキット次第（ここは仮）。
    void subscribe() {
        CDb::getInstance().getWorld()->getInteractionManager()->setInteractionCallback(&interactions);
    }
};

