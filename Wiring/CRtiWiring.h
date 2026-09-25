// 本番の配線。**実際の FOM クラスを足すときは、ここにメンバと build() の1行を足す。**
//
// CWiring（スタブの配線）と同じ形で、stub:: の代わりに hla::CTRti... の器と HLAConversion/ の変換を繋ぐ。
// 変換関数は world を掴まず、呼ばれたときに hlaconv::CDb から読むので、配線は join より前に作ってよい。
//
// 使う順番（main の形）：
//
//   CRtiWiring wiring;  core::CGateway gateway;  wiring.build(gateway);
//   join → hlaconv::CDb::getInstance().setWorld(world) → wiring.subscribe()
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
#include "../HLAConversion/CDb.h"
#include "../HLAConversion/CInteractionCallback.h"
#include "../HLAConversion/DesignatorRti.h"
#include "../HLAConversion/WeaponFireRti.h"
#include "../ICD/icd_classes.h"
#include "../NonFOM/CCommandHandler.h"
#include "../NonFOM/CControlHandler.h"
#include "../NonFOM/TCommand.h"
#include "../NonFOM/TControl.h"
#include "AddChannel.h"

namespace wiring {

class CRtiWiring {
public:
    // ── オブジェクト：Designator ─────────────────────────────────────
    /// HLA → UDP。毎周期 getRemoteDesignator で全インスタンスを取り、toIcd で変換する
    hla::CTRtiObjectFromHla<icdfom::Designator, tk::DesignatorPtr> designatorFromHla{
        &hlaconv::getRemoteDesignator, &hlaconv::toIcd};
    /// UDP → HLA。keyOf でインスタンスを決め、初見なら登録し、updateDesignator で書いて update
    hla::CTRtiObjectToHla<icdfom::Designator, tk::DesignatorPtr> designatorToHla{
        &hlaconv::keyOf, &hlaconv::registerDesignator, &hlaconv::updateDesignator};

    // ── インタラクション ─────────────────────────────────────────────
    /// HLA → UDP。全インタラクション共通の受信コールバックで、クラスごとのキューを持つ
    /// （interactions.weaponFireFromHla など）。RTI のスレッドから push され、周期ループが drain する
    hlaconv::CInteractionCallback interactions;

    /// UDP → HLA（WeaponFire）。sendWeaponFire でパラメータを詰めて sendInteraction
    hla::CTRtiInteractionToHla<icdfom::WeaponFire> fireToHla{&hlaconv::sendWeaponFire};

    // ── FOM に無い独自データ ──────────────────────────────────────────
    nonfom::CCommandHandler commandHandler;   ///< コマンド文字列の受け口
    nonfom::CControlHandler controlHandler;   ///< 制御文字列の受け口

    void build(core::CGateway& g) {
        addClass<icdfom::Designator>(g, &designatorFromHla, &designatorToHla);
        addClass<icdfom::WeaponFire>(g, &interactions.weaponFireFromHla, &fireToHla);

        addRaw<nonfom::TCommand>(g, &commandHandler);
        addRaw<nonfom::TControl>(g, &controlHandler);
    }

    /// インタラクションの受信コールバックをツールキットに登録する。インタラクションが増えても1行のまま。
    /// **CDb に world を置いたあとに1回呼ぶ。** 登録の仕方はツールキット次第（ここは仮）。
    void subscribe() {
        hlaconv::CDb::getInstance().getWorld()->getInteractionManager()->setInteractionCallback(&interactions);
    }
};

}  // namespace wiring
