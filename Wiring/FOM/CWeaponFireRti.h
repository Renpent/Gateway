// WeaponFire（インタラクションクラス）の本番の変換。**インタラクションを1つ足すときの雛形。**
//
// オブジェクトと違い、HLA からは RTI のコールバックで押し込まれてくる：
//
//   HLA → UDP  CInteractionCallback::onWeaponFire が CWeaponFireRti::toIcd して、WeaponFire のキューに push する
//   UDP → HLA  CTRtiInteractionToHla{&CWeaponFireRti::sendWeaponFire}

#pragma once

#include "../../ICD/Interaction/WeaponFire.h"
#include "Toolkit.h"

/// WeaponFire の変換を static 関数として並べるだけのクラス（名前空間の代わり）。
class CWeaponFireRti {
public:
    /// 受信したインタラクション -> 1レコード。**RTI のスレッドから呼ばれる**（コールバックの中）。
    static icdfom::WeaponFire toIcd(const tk::WeaponFire& i);

    /// 1レコード -> 送信するインタラクションのパラメータ。
    static void fillRti(const icdfom::WeaponFire& r, tk::WeaponFire* i);

    /// Send：パラメータを詰めて sendInteraction する。
    static void sendWeaponFire(const icdfom::WeaponFire& r);
};
