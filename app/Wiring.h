// 「どのクラスをどちら向きに流すか」を決める唯一の場所。
//
// **クラスが増えたときに伸びるのはこのファイルだけ。** main.cpp から分けてあるのはそのためで、
// 実運用のクラス数（数十）になっても最上位のファイルは起動処理のままでいられる。
//
// ID・ポート・ペイロードは書かない。add<T> が T の生成された定数から bindingOf<T>() で組み立てる
// ので、別クラスの値を渡し間違える余地が無く、ICD を更新して再生成すればここは変わらない。
// **オブジェクトかインタラクションかもここには書かない** — kIsInteraction から決まる。
//
// **`stub::` が付いているものは本番には無い。** いま繋いでいる供給元と受け口はすべて
// `stub/` の代用品で、実 RTI に繋ぐときは `hla::RtiObjectFromHla` などに差し替える。
// `stub/` を見ているファイルは他に無いので、**書き換え対象はこのファイルだけ**になる。
//
// メンバは向きで命名してある（xxxFromHla / xxxToHla）。型の側も同じ規則で、`FromHla<T>` の
// 実装はすべて `〜FromHla`、`ToHla<T>` の実装はすべて `〜ToHla`。派生の名前から基底が読めるので、
// Feed や Receiver のような「どちら向きか分からない語」を覚える必要がない。
//
// 継ぎ目の実体を**値で持っている**のは、ClassChannel が生ポインタで借りるだけだから。
// ここが所有者で、Gateway より長生きする必要がある（main.cpp の宣言順を参照）。
//
// 片方向のクラスは、要らないほうのメンバを作らず nullptr を渡す：
//   送受信  add<T>(g, &fromHla, &toHla);
//   送信のみ add<T>(g, &fromHla, nullptr);
//   受信のみ add<T>(g, nullptr,  &toHla);
// T は必ず明示すること。nullptr からは型が決まらない。
//
// FOM に無い独自データ（相手が決めた形式）は addRaw<T> で足す。T は手書きで raw/ に置き、
// 名前・ポート・parse を持つ（gateway/RawChannel.h）。受け口は hla::ToHla ではなく app::ToApp：
//   受信のみ addRaw<T>(g, &commandToApp);
//
// **コマンドの処理は tick() の最後。** 受け口は受信中にキューへ積むだけにして、Gateway の
// setTickEnd で登録した処理がまとめて実行する（app/CommandToApp.h）。
//
// setTickEnd に渡すラムダは this（Wiring）を掴む。Wiring が Gateway より長生きするのが前提で、
// main.cpp の宣言順がそれを保証している。

#pragma once

#include <cstddef>
#include <memory>

#include "../gateway/Channel.h"
#include "../gateway/ClassBinding.h"
#include "../gateway/ClassChannel.h"
#include "../gateway/Gateway.h"
#include "../gateway/RawChannel.h"
#include "../raw/Command.h"
#include "CommandToApp.h"
#include "ToApp.h"
#include "../stub/ConstantFromHla.h"
#include "../stub/CountingToHla.h"
#include "../stub/FixtureFromHla.h"
#include "../stub/RadarBeamFixture.h"
#include "../stub/VerifyingToHla.h"
#include "../stub/WeaponFireFixture.h"
// ここだけが全クラスを名指しする。生成物なので、ICD にクラスを足せば自動で追随する。
#include "../icd/icd_classes.h"

namespace app {

struct Wiring {
    /// 往復照合の集計。**main.cpp にクラス名を持ち出さないための器。**
    struct VerifyResult {
        std::size_t received = 0;    ///< 照合した受信件数の合計
        std::size_t mismatched = 0;  ///< うちバイト列が食い違った件数
    };

    // HLA → UDP（送信側の供給元）
    stub::FixtureFromHla<icdfom::RadarBeam>       beamFromHla{stub::makeRadarBeam, 4};   ///< RadarBeam の供給元
    stub::ConstantFromHla<icdfom::RadioReceiver>  radioFromHla{1};                      ///< RadioReceiver の供給元
    stub::ConstantFromHla<icdfom::MinefieldData>  minefieldFromHla{1};                  ///< MinefieldData の供給元
    stub::FixtureFromHla<icdfom::WeaponFire>      fireFromHla{stub::makeWeaponFire, 3};  ///< WeaponFire の供給元

    // UDP → HLA（受信側の受け口）
    stub::VerifyingToHla<icdfom::RadarBeam>    beamToHla{stub::makeRadarBeam};   ///< RadarBeam の受け口。往復照合もする
    stub::CountingToHla<icdfom::RadioReceiver> radioToHla;                      ///< RadioReceiver の受け口。数えるだけ
    stub::CountingToHla<icdfom::MinefieldData> minefieldToHla;                  ///< MinefieldData の受け口。数えるだけ
    stub::VerifyingToHla<icdfom::WeaponFire>   fireToHla{stub::makeWeaponFire};  ///< WeaponFire の受け口。往復照合もする

    // UDP → アプリ（FOM に無い独自データ）。**stub ではない** — 本番でもこのまま使う
    CommandToApp commandToApp;   ///< コマンド文字列の受け口。処理は CommandToApp::handle

    /// verify が false なら照合するクラスの受信は捨てる（照合はループバックのときだけ）。
    void build(gw::Gateway& g, bool verify) {
        add<icdfom::RadarBeam>    (g, &beamFromHla,      verify ? &beamToHla : nullptr);
        add<icdfom::RadioReceiver>(g, &radioFromHla,     &radioToHla);
        add<icdfom::MinefieldData>(g, &minefieldFromHla, &minefieldToHla);
        // **唯一のインタラクション。** ClassKind::Interaction になるのは kIsInteraction=true だから
        // で、ここには何も書いていない。送り残しを次の周期へ持ち越すのはこのクラスだけ。
        add<icdfom::WeaponFire>   (g, &fireFromHla,      verify ? &fireToHla : nullptr);

        // FOM に無い独自データ。受信のみ。
        addRaw<raw::Command>(g, &commandToApp);

        // 周期の終わりにコマンドを処理する。受信中は積むだけ、ここでまとめて実行。
        g.setTickEnd([this] { commandToApp.applyPending(); });
    }

    /// 照合する受け口ぜんぶの合計。クラスが増えたらここに1行足す。
    [[nodiscard]] VerifyResult verifyResult() const {
        VerifyResult r;
        for (const VerifyResult& one : {tally(beamToHla), tally(fireToHla)}) {
            r.received += one.received;
            r.mismatched += one.mismatched;
        }
        return r;
    }

private:
    template <class T>
    static VerifyResult tally(const stub::VerifyingToHla<T>& v) {
        return VerifyResult{v.received(), v.mismatched()};
    }

    /// 1行1クラスで並ぶようにするための包み。binding は T から決まり、new と unique_ptr は
    /// ここに1度だけ現れる。
    template <class T>
    static void add(gw::Gateway& g, hla::FromHla<T>* fromHla, hla::ToHla<T>* toHla) {
        g.add(std::unique_ptr<gw::Channel>(
            new gw::ClassChannel<T>(gw::bindingOf<T>(), fromHla, toHla)));
    }

    /// FOM に無い独自データの受信口。ポートと名前は T の定数から決まる。
    template <class T>
    static void addRaw(gw::Gateway& g, ToApp<T>* toApp) {
        g.add(std::unique_ptr<gw::Channel>(new gw::RawChannel<T>(toApp)));
    }
};

}  // namespace app
