// WeaponFire（インタラクションクラス）の本番の変換。**インタラクションを1つ足すときの雛形。**
//
// オブジェクトと違い、HLA からは RTI のコールバックで押し込まれてくる：
//
//   HLA → UDP  CWeaponFireCallback（ツールキットのコールバックを継承）が toIcd してキューに push する
//   UDP → HLA  hla::CTRtiInteractionToHla{&sendWeaponFire}

#pragma once

#include "../icd/interaction/WeaponFire.h"
#include "Toolkit.h"

namespace rti {

/// 受信したインタラクション -> 1レコード。**RTI のスレッドから呼ばれる**（コールバックの中）。
icdfom::WeaponFire toIcd(const tk::WeaponFire& i);

/// 1レコード -> 送信するインタラクションのパラメータ。
void fillRti(const icdfom::WeaponFire& r, tk::WeaponFire* i);

/// Send：パラメータを詰めて sendInteraction する。
void sendWeaponFire(const icdfom::WeaponFire& r);

}  // namespace rti
