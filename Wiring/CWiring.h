// スタブの配線。RTI が無いこの環境で動かすためのもので、main.cpp が使う。
// **本番の配線は Wiring/CRtiWiring.h**（同じ形で、stub:: の代わりに hla::CTRti... と Wiring/FOM/ の変換を繋ぐ）。
//
// ID・ポート・ペイロードは書かない。addClass<T> が生成物の定数（bindingOf<T>()）から決める。
//
// 送受信の相手（継ぎ目の実体）はこのクラスが値で持ち、チャネルは借りるだけ。
// そのため CWiring は CGateway より長く生きる必要がある（main.cpp の宣言順）。
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
#include "AddChannel.h"
#include "NonFOM/CCommandHandler.h"
#include "NonFOM/CControlHandler.h"
#include "NonFOM/TCommand.h"
#include "NonFOM/TControl.h"

namespace wiring {

class CWiring {
public:
    // HLA → UDP
    stub::CTFixtureFromHla<icdfom::RadarBeam>      beamFromHla{stub::makeRadarBeam, 4};   ///< RadarBeam の供給元
    stub::CTConstantFromHla<icdfom::RadioReceiver> radioFromHla{1};                      ///< RadioReceiver の供給元
    stub::CTConstantFromHla<icdfom::MinefieldData> minefieldFromHla{1};                  ///< MinefieldData の供給元
    stub::CTFixtureFromHla<icdfom::WeaponFire>     fireFromHla{stub::makeWeaponFire, 3};  ///< WeaponFire の供給元
    stub::CTFixtureFromHla<icdfom::Designator>     designatorFromHla{stub::makeDesignator, 1};  ///< Designator の供給元（サンプル）

    // UDP → HLA
    stub::CDesignatorToHla designatorToHla;   ///< Designator の受け口（サンプル。受けたものを表示する）

    // UDP → アプリ（FOM に無い独自データ）
    nonfom::CCommandHandler commandHandler;   ///< コマンド文字列の受け口
    nonfom::CControlHandler controlHandler;   ///< 制御文字列の受け口

    void build(core::CGateway& g) {
        addClass<icdfom::RadarBeam>    (g, &beamFromHla,       nullptr);
        addClass<icdfom::RadioReceiver>(g, &radioFromHla,      nullptr);
        addClass<icdfom::MinefieldData>(g, &minefieldFromHla,  nullptr);
        addClass<icdfom::WeaponFire>   (g, &fireFromHla,       nullptr);
        addClass<icdfom::Designator>   (g, &designatorFromHla, &designatorToHla);   // サンプル：送受信

        addRaw<nonfom::TCommand>(g, &commandHandler);
        addRaw<nonfom::TControl>(g, &controlHandler);
    }
};

}  // namespace wiring
