// 「どのクラスをどちら向きに流すか」を決める唯一の場所。
//
// **クラスが増えたときに伸びるのはこのファイルだけ。** main.cpp から分けてあるのはそのためで、
// 実運用のクラス数（数十）になっても最上位のファイルは起動処理のままでいられる。
//
// ID・ポート・ペイロードは書かない。add<T> が T の生成された定数から bindingOf<T>() で組み立てる
// ので、別クラスの値を渡し間違える余地が無く、ICD を更新して再生成すればここは変わらない。
// **オブジェクトかインタラクションかもここには書かない** — kIsInteraction から決まる。
//
// メンバは向きで命名してある（xxxFromHla / xxxToHla）。クラス名のほうに Receiver のような
// 語が入ることがあるので、役割を型名から取ると receiverReceiver のような名前が出てしまう。
//
// 継ぎ目の実体を**値で持っている**のは、ClassChannel が生ポインタで借りるだけだから。
// ここが所有者で、Gateway より長生きする必要がある（main.cpp の宣言順を参照）。
//
// 片方向のクラスは、要らないほうのメンバを作らず nullptr を渡す：
//   送受信  add<T>(g, &fromHla, &toHla);
//   送信のみ add<T>(g, &fromHla, nullptr);
//   受信のみ add<T>(g, nullptr,  &toHla);
// T は必ず明示すること。nullptr からは型が決まらない。

#pragma once

#include <cstddef>
#include <memory>

#include "../gateway/Channel.h"
#include "../gateway/ClassBinding.h"
#include "../gateway/ClassChannel.h"
#include "../gateway/Gateway.h"
#include "../hla/ConstantFeed.h"
#include "../hla/CountingReceiver.h"
#include "../hla/FixtureFeed.h"
#include "../hla/RadarBeamFixture.h"
#include "../hla/VerifyingReceiver.h"
#include "../hla/WeaponFireFixture.h"
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
    hla::FixtureFeed<icdfom::RadarBeam>       beamFromHla{hla::makeRadarBeam, 4};   ///< RadarBeam の供給元
    hla::ConstantFeed<icdfom::RadioReceiver>  radioFromHla{1};                      ///< RadioReceiver の供給元
    hla::ConstantFeed<icdfom::MinefieldData>  minefieldFromHla{1};                  ///< MinefieldData の供給元
    hla::FixtureFeed<icdfom::WeaponFire>      fireFromHla{hla::makeWeaponFire, 3};  ///< WeaponFire の供給元

    // UDP → HLA（受信側の受け口）
    hla::VerifyingReceiver<icdfom::RadarBeam>    beamToHla{hla::makeRadarBeam};   ///< RadarBeam の受け口。往復照合もする
    hla::CountingReceiver<icdfom::RadioReceiver> radioToHla;                      ///< RadioReceiver の受け口。数えるだけ
    hla::CountingReceiver<icdfom::MinefieldData> minefieldToHla;                  ///< MinefieldData の受け口。数えるだけ
    hla::VerifyingReceiver<icdfom::WeaponFire>   fireToHla{hla::makeWeaponFire};  ///< WeaponFire の受け口。往復照合もする

    /// verify が false なら照合するクラスの受信は捨てる（照合はループバックのときだけ）。
    void build(gw::Gateway& g, bool verify) {
        add<icdfom::RadarBeam>    (g, &beamFromHla,      verify ? &beamToHla : nullptr);
        add<icdfom::RadioReceiver>(g, &radioFromHla,     &radioToHla);
        add<icdfom::MinefieldData>(g, &minefieldFromHla, &minefieldToHla);
        // **唯一のインタラクション。** Delivery::Events になるのは kIsInteraction=true だから
        // で、ここには何も書いていない。送り残しを次の周期へ持ち越すのはこのクラスだけ。
        add<icdfom::WeaponFire>   (g, &fireFromHla,      verify ? &fireToHla : nullptr);
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
    static VerifyResult tally(const hla::VerifyingReceiver<T>& v) {
        return VerifyResult{v.received(), v.mismatched()};
    }

    /// 1行1クラスで並ぶようにするための包み。binding は T から決まり、new と unique_ptr は
    /// ここに1度だけ現れる。
    template <class T>
    static void add(gw::Gateway& g, hla::FromHla<T>* fromHla, hla::ToHla<T>* toHla) {
        g.add(std::unique_ptr<gw::Channel>(
            new gw::ClassChannel<T>(gw::bindingOf<T>(), fromHla, toHla)));
    }
};

}  // namespace app
