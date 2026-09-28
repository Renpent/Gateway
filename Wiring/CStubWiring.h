// スタブの配線。RTI が無いこの環境で動かすためのもので、main.cpp が使う。
// **本番の配線は Wiring/CRtiWiring.h**（同じ形で、Stub/ の代用品の代わりに CTRti... と Wiring/FOM/ の変換を繋ぐ）。
//
// ID・ポート・ペイロードは書かない。addClass<T> が生成物の定数（bindingOf<T>()）から決める。
//
// 送受信の相手（継ぎ目の実体）はこのクラスが値で持ち、チャネルは借りるだけ。
// そのため CStubWiring は CGateway より長く生きる必要がある（main.cpp の宣言順）。
//
//   送受信     addClass<T>(g, &fromHla, &toHla);
//   送信のみ   addClass<T>(g, &fromHla, nullptr);
//   受信のみ   addClass<T>(g, nullptr,  &toHla);
//   独自データ addRaw<T>(g, &handler);
//
// 送信側は Stub/ の代用品。受信側は Designator（サンプル）だけ表示するモックを繋ぎ、
// ほかは繋いでいない（受けた分は捨てる）。

#pragma once

#include "../Core/CGateway.h"
#include "../ICD/icd_classes.h"
#include "../Stub/CDesignatorToHla.h"
#include "../Stub/CTConstantFromHla.h"
#include "../Stub/CTFixtureFromHla.h"
#include "../Stub/DesignatorFixture.h"
#include "../Stub/RadarBeamFixture.h"
#include "../Stub/WeaponFireFixture.h"
#include "CWiring.h"
#include "NonFOM/CCommandHandler.h"
#include "NonFOM/CControlHandler.h"
#include "NonFOM/TCommand.h"
#include "NonFOM/TControl.h"

class CStubWiring : private CWiring {
public:
    // HLA → UDP
    CTFixtureFromHla<icdfom::RadarBeam>      beamFromHla{makeRadarBeam, 4};   ///< RadarBeam の供給元
    CTConstantFromHla<icdfom::RadioReceiver> radioFromHla{1};                      ///< RadioReceiver の供給元
    CTConstantFromHla<icdfom::MinefieldData> minefieldFromHla{1};                  ///< MinefieldData の供給元
    CTFixtureFromHla<icdfom::WeaponFire>     fireFromHla{makeWeaponFire, 3};  ///< WeaponFire の供給元
    CTFixtureFromHla<icdfom::Designator>     designatorFromHla{makeDesignator, 1};  ///< Designator の供給元（サンプル）

    // UDP → HLA
    CDesignatorToHla designatorToHla;   ///< Designator の受け口（サンプル。受けたものを表示する）

    // UDP → アプリ（FOM に無い独自データ）
    CCommandHandler commandHandler;   ///< コマンド文字列の受け口
    CControlHandler controlHandler;   ///< 制御文字列の受け口

    void build(CGateway& g) {
        addClass<icdfom::RadarBeam>    (g, &beamFromHla,       nullptr);
        addClass<icdfom::RadioReceiver>(g, &radioFromHla,      nullptr);
        addClass<icdfom::MinefieldData>(g, &minefieldFromHla,  nullptr);
        addClass<icdfom::WeaponFire>   (g, &fireFromHla,       nullptr);
        addClass<icdfom::Designator>   (g, &designatorFromHla, &designatorToHla);   // サンプル：送受信

        addRaw<TCommand>(g, &commandHandler);
        addRaw<TControl>(g, &controlHandler);
    }
};

