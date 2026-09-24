// WeaponFire の受信コールバック。**インタラクションを1つ足すときの雛形。**
//
// ツールキットが生成した仮想関数のクラスを継承し、受け取ったパラメータを ICD のレコードに
// 変換して、そのクラスのキュー（hla::CTRtiInteractionFromHla）に push する。
// キューは周期ループが毎周期 drain して UDP に送る。
//
// **RTI のスレッドから呼ばれる。** ここでやるのは変換と push だけにすること：
//   - パラメータはコールバックの間しか有効でないのが普通なので、この場で値に写す
//   - push はロック付きなので、RTI のスレッドから呼んでよい
//   - ゲートウェイのほかのもの（チャネルやソケット）には触らない
//
// 登録（setWeaponFireCallback）は app/CRtiWiring.h の subscribe() で行う。
// このオブジェクトは借りられるだけなので、resign するまで生きていること。

#pragma once

#include "../gateway/hla/CTRtiInteractionFromHla.h"
#include "../icd/interaction/WeaponFire.h"
#include "Toolkit.h"
#include "WeaponFireRti.h"

namespace rti {

class CWeaponFireCallback : public tk::WeaponFireCallback {
public:
    explicit CWeaponFireCallback(hla::CTRtiInteractionFromHla<icdfom::WeaponFire>& queue)
        : m_queue(queue) {}

    void onWeaponFire(const tk::WeaponFire& interaction) override {
        m_queue.push(toIcd(interaction));
    }

private:
    hla::CTRtiInteractionFromHla<icdfom::WeaponFire>& m_queue;   ///< WeaponFire のキュー。配線が持つ
};

}  // namespace rti
