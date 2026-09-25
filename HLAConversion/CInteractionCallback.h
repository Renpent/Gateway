// インタラクションの受信コールバック。**インタラクションを足すときは、このファイルに2つ足す。**
//
// ツールキットが生成する InteractionCallback には、FOM の全インタラクションの仮想関数が並んでいる。
// それを1回だけ継承し、**ゲートウェイで流すものだけ**実装する（実装しないものは何もしない）。
//
// 流すインタラクション1つにつき、次の2つを足す：
//   1. そのクラスのキュー（hla::CTRtiInteractionFromHla<T>）を public メンバに
//   2. 仮想関数の override。受け取ったパラメータを toIcd で変換して、1 のキューに push する
//
// キューは配線（Wiring/CRtiWiring.h）が addClass に渡し、周期ループが毎周期 drain して UDP に送る。
//
// **RTI のスレッドから呼ばれる。** override の中でやるのは変換と push だけにすること：
//   - パラメータはコールバックの間しか有効でないのが普通なので、この場で値に写す
//   - push はロック付きなので、RTI のスレッドから呼んでよい
//   - ゲートウェイのほかのもの（チャネルやソケット）には触らない
//
// 登録は Wiring/CRtiWiring.h の subscribe() で1回。ツールキットに貸すので、resign するまで生きていること。

#pragma once

#include "../Core/HLA/CTRtiInteractionFromHla.h"
#include "../ICD/Interaction/WeaponFire.h"
#include "Toolkit.h"
#include "WeaponFireRti.h"

namespace hlaconv {

class CInteractionCallback : public tk::InteractionCallback {
public:
    // ── キュー（流すインタラクション1つにつき1つ） ─────────────────────
    hla::CTRtiInteractionFromHla<icdfom::WeaponFire> weaponFireFromHla;   ///< WeaponFire のキュー

    // ── 実装する仮想関数（流すものだけ） ──────────────────────────────
    void onWeaponFire(const tk::WeaponFire& interaction) override {
        weaponFireFromHla.push(toIcd(interaction));
    }
};

}  // namespace hlaconv
