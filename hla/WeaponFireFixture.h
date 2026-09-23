// ループバックで往復させる WeaponFire の「送ったはずの値」。
//
// **このゲートウェイで最初のインタラクション。** オブジェクト（RadarBeam ほか）と違い、
// Delivery::Events で流れる — 送り残しを次の周期に持ち越す経路は、このクラスが来るまで
// 一度も実行されていなかった。
//
// 作り方の約束は RadarBeamFixture.h と同じで、**決定的**であること。同じ i なら同じ値に
// なるので、往復した結果を1バイトずつ突き合わせられる。
//
// EventIdentifier.EventCount に通し番号を入れてあるのは、レコード自身に順番を持たせるため。
// VerifyingReceiver は「n 件目の受信」を「n 件目の送信」と比べるので途中の欠落に弱いが、
// この番号があれば、落ちたのか壊れたのかを人が後から読める。

#pragma once

#include <cstddef>
#include <string>

#include "../icd/WeaponFire.h"
#include "ObjectId.h"

namespace hla {

/// i 番目の WeaponFire。RPR FOM の「誰が何を何に向けて撃ったか」を一通り埋める。
inline icdfom::WeaponFire makeWeaponFire(std::size_t i) {
    const float f = static_cast<float>(i);
    icdfom::WeaponFire v{};

    // 通し番号。レコード自身が何件目かを持つ唯一の場所。
    v.EventIdentifier.EventCount = static_cast<icdfom::UnsignedInteger16>(i);
    v.EventIdentifier.IssuingObjectIdentifier = objectId("FED-1");

    v.FireControlSolutionRange = 1500.0f + f;
    v.FireMissionIndex         = static_cast<icdfom::UnsignedInteger32>(i / 4);

    v.FiringLocation.X = 4.0e6 + static_cast<double>(i);
    v.FiringLocation.Y = 2.0e6 - static_cast<double>(i);
    v.FiringLocation.Z = 4.5e6;

    v.FiringObjectIdentifier   = objectId("PLTFRM-" + std::to_string(i % 8));
    v.MunitionObjectIdentifier = objectId("MUN-" + std::to_string(i));
    v.TargetObjectIdentifier   = objectId("TGT-" + std::to_string(i % 3));

    v.FuseType    = icdfom::FuseTypeEnum16::UltraQuick;
    v.WarheadType = icdfom::WarheadTypeEnum16::Other;

    v.InitialVelocityVector.XVelocity = 250.0f + f;
    v.InitialVelocityVector.YVelocity = -12.5f;
    v.InitialVelocityVector.ZVelocity = 3.25f;

    // DIS の entity type: 2 = Munition。残りは意味のある並びであれば何でもよい。
    v.MunitionType.EntityKind  = 2;
    v.MunitionType.Domain      = 1;
    v.MunitionType.CountryCode = 225;
    v.MunitionType.Category    = 1;
    v.MunitionType.Subcategory = 2;
    v.MunitionType.Specific    = static_cast<icdfom::Octet>(i & 0xFF);
    v.MunitionType.Extra       = 0;

    // 斉射。1発のときと連射のときを交互に出して、値が固定にならないようにする。
    v.QuantityFired = static_cast<icdfom::UnsignedInteger16>(1 + (i % 3));
    v.RateOfFire    = static_cast<icdfom::UnsignedInteger16>(v.QuantityFired > 1 ? 600 : 0);

    return v;
}

}  // namespace hla
