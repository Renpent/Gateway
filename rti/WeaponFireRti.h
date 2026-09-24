// WeaponFire（インタラクションクラス）の本番の変換。**インタラクションを1つ足すときの雛形。**
//
// オブジェクトと違い、HLA からは RTI のコールバックで押し込まれてくる：
//
//   HLA → UDP  subscribeWeaponFire(fireFromHla)   join 後に1回。コールバックで toIcd して push する
//   UDP → HLA  hla::CTRtiInteractionToHla{&sendWeaponFire}

#pragma once

#include "../gateway/hla/CTRtiInteractionFromHla.h"
#include "../icd/interaction/WeaponFire.h"
#include "Toolkit.h"

namespace rti {

/// 受信したインタラクション -> 1レコード。**RTI のスレッドから呼ばれる**（コールバックの中）。
icdfom::WeaponFire toIcd(const tk::WeaponFire& i);

/// 1レコード -> 送信するインタラクションのパラメータ。
void fillRti(const icdfom::WeaponFire& r, tk::WeaponFire* i);

/// Send：パラメータを詰めて sendInteraction する。
void sendWeaponFire(const icdfom::WeaponFire& r);

/// 受信のコールバックを登録する。**join して CDb に world を置いたあとに1回呼ぶ。**
/// コールバックは RTI のスレッドで、変換してから queue に push する
/// （RTI のパラメータはコールバックの間しか有効でないため、先に値へ写す）。
void subscribeWeaponFire(hla::CTRtiInteractionFromHla<icdfom::WeaponFire>& queue);

}  // namespace rti
