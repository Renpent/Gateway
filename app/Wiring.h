// 「どのクラスをどちら向きに流すか」を決める唯一の場所。
//
// **クラスが増えたときに伸びるのはこのファイルだけ。** main.cpp から分けてあるのはそのためで、
// 実運用のクラス数（数十）になっても最上位のファイルは起動処理のままでいられる。
//
// メンバは向きで命名してある（xxxFromHla / xxxToHla）。クラス名のほうに Receiver のような
// 語が入ることがあるので、役割を型名から取ると receiverReceiver のような名前が出てしまう。
//
// 継ぎ目の実体を**値で持っている**のは、ClassChannel が生ポインタで借りるだけだから。
// ここが所有者で、Gateway より長生きする必要がある（main.cpp の宣言順を参照）。
//
// 片方向のクラスは、要らないほうのメンバを作らず nullptr を渡す：
//   送受信  add<T>(g, bind, &fromHla, &toHla);
//   送信のみ add<T>(g, bind, &fromHla, nullptr);
//   受信のみ add<T>(g, bind, nullptr,  &toHla);
// T は必ず明示すること。nullptr からは型が決まらない。

#pragma once

#include <memory>

#include "../gateway/Channel.h"
#include "../gateway/ClassChannel.h"
#include "../gateway/ClassIds.h"
#include "../gateway/Gateway.h"
#include "../hla/ConstantFeed.h"
#include "../hla/CountingReceiver.h"
#include "../hla/StubRadarBeamFeed.h"
#include "../hla/VerifyingReceiver.h"
// ここだけが全クラスを名指しする。生成物なので、ICD にクラスを足せば自動で追随する。
#include "../icd/icd_classes.h"

namespace app {

struct Wiring {
    // HLA → UDP（送信側の供給元）
    hla::StubRadarBeamFeed                    beamFromHla{4};       ///< RadarBeam の供給元
    hla::ConstantFeed<icdfom::RadioReceiver>  radioFromHla{1};      ///< RadioReceiver の供給元
    hla::ConstantFeed<icdfom::MinefieldData>  minefieldFromHla{1};  ///< MinefieldData の供給元

    // UDP → HLA（受信側の受け口）
    hla::VerifyingReceiver                       beamToHla;       ///< RadarBeam の受け口。往復照合もする
    hla::CountingReceiver<icdfom::RadioReceiver> radioToHla;      ///< RadioReceiver の受け口。数えるだけ
    hla::CountingReceiver<icdfom::MinefieldData> minefieldToHla;  ///< MinefieldData の受け口。数えるだけ

    /// verify が false なら RadarBeam の受信は捨てる（照合はループバックのときだけ）。
    void build(gw::Gateway& g, bool verify) {
        add<icdfom::RadarBeam>    (g, gw::kRadarBeam,     &beamFromHla,
                                   verify ? &beamToHla : nullptr);
        add<icdfom::RadioReceiver>(g, gw::kRadioReceiver, &radioFromHla,     &radioToHla);
        add<icdfom::MinefieldData>(g, gw::kMinefieldData, &minefieldFromHla, &minefieldToHla);
    }

private:
    /// 1行1クラスで並ぶようにするための包み。new と unique_ptr がここに1度だけ現れる。
    template <class T>
    static void add(gw::Gateway& g, const gw::ClassBinding& bind,
                    hla::FromHla<T>* fromHla, hla::ToHla<T>* toHla) {
        g.add(std::unique_ptr<gw::Channel>(new gw::ClassChannel<T>(bind, fromHla, toHla)));
    }
};

}  // namespace app
