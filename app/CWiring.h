// どのクラスをどちら向きに流すかを決める場所。**クラスを足すときに触るのはこのファイルだけ。**
//
// ID・ポート・ペイロードは書かない。add<T> が生成物の定数（bindingOf<T>()）から決める。
//
// 送受信の相手（継ぎ目の実体）はこのクラスが値で持ち、チャネルは借りるだけ。
// そのため CWiring は CGateway より長く生きる必要がある（main.cpp の宣言順）。
//
//   送受信    add<T>(g, &fromHla, &toHla);
//   送信のみ  add<T>(g, &fromHla, nullptr);
//   受信のみ  add<T>(g, nullptr,  &toHla);
//   独自データ addRaw<T>(g, &handler);
//
// いまは RTI が無いので、送信側は stub/ の代用品。受信側は Designator（サンプル）だけ表示する
// モックを繋ぎ、ほかは繋いでいない（受けた分は捨てる）。
// 本番では hla::CTRtiObjectFromHla などに差し替える（README の「本番（RTI）での形」）。

#pragma once

#include <memory>

#include "../gateway/CGateway.h"
#include "../gateway/CTClassChannel.h"
#include "../gateway/CTMessageHandler.h"
#include "../gateway/CTRawChannel.h"
#include "../gateway/TClassBinding.h"
#include "../icd/icd_classes.h"
#include "../stub/CTConstantFromHla.h"
#include "../stub/CDesignatorToHla.h"
#include "../stub/CTFixtureFromHla.h"
#include "../stub/DesignatorFixture.h"
#include "../stub/RadarBeamFixture.h"
#include "../stub/WeaponFireFixture.h"
#include "CCommandHandler.h"
#include "CControlHandler.h"
#include "TCommand.h"
#include "TControl.h"

namespace app {

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
    CCommandHandler commandHandler;   ///< コマンド文字列の受け口
    CControlHandler controlHandler;   ///< 制御文字列の受け口

    void build(gw::CGateway& g) {
        add<icdfom::RadarBeam>    (g, &beamFromHla,      nullptr);
        add<icdfom::RadioReceiver>(g, &radioFromHla,     nullptr);
        add<icdfom::MinefieldData>(g, &minefieldFromHla, nullptr);
        add<icdfom::WeaponFire>   (g, &fireFromHla,      nullptr);
        add<icdfom::Designator>   (g, &designatorFromHla, &designatorToHla);   // サンプル：送受信

        addRaw<app::TCommand>(g, &commandHandler);
        addRaw<app::TControl>(g, &controlHandler);
    }

private:
    template <class T>
    static void add(gw::CGateway& g, hla::CTFromHla<T>* fromHla, hla::CTToHla<T>* toHla) {
        g.add(std::unique_ptr<gw::CChannel>(
            new gw::CTClassChannel<T>(gw::bindingOf<T>(), fromHla, toHla)));
    }

    /// 独自データの受信口を足し、ハンドラの onTickEnd() も周期末処理に登録する
    /// （別の行にすると書き忘れて、積んだものが処理されなくなる）。
    template <class T>
    static void addRaw(gw::CGateway& g, gw::CTMessageHandler<T>* handler) {
        g.add(std::unique_ptr<gw::CChannel>(new gw::CTRawChannel<T>(handler)));
        if (handler != nullptr) g.addTickEnd([handler] { handler->onTickEnd(); });
    }
};

}  // namespace app
