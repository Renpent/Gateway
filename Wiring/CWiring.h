// 配線の基底クラス。スタブの配線（CStubWiring）と本番の配線（CRtiWiring）が継承し、
// build() の中で addClass / addRaw を使う。
//
//   送受信     addClass<T>(g, &fromHla, &toHla);
//   送信のみ   addClass<T>(g, &fromHla, nullptr);
//   受信のみ   addClass<T>(g, nullptr,  &toHla);
//   独自データ addRaw<T>(g, &handler);
//
// **T は必ず明示すること。** nullptr からは型が決まらない。

#pragma once

#include <memory>

#include "../Core/CGateway.h"
#include "../Core/CTClassChannel.h"
#include "../Core/CTMessageHandler.h"
#include "../Core/CTRawChannel.h"
#include "../Core/TClassBinding.h"

class CWiring {
protected:
    /// ICD のクラスを1つ足す。ID・ポートは T の生成された定数から決まる。
    /// fromHla が nullptr なら送信しない、toHla が nullptr なら受けた分を捨てる。
    /// どちらも借りるだけなので、CGateway より長く生きていること。
    template <class T>
    static void addClass(CGateway& g, CTFromHla<T>* fromHla, CTToHla<T>* toHla) {
        g.add(std::unique_ptr<CChannel>(
            new CTClassChannel<T>(bindingOf<T>(), fromHla, toHla)));
    }

    /// FOM に無い独自データの受信口を足し、ハンドラの onTickEnd() も周期末処理に登録する
    /// （別の行にすると書き忘れて、積んだものが処理されなくなる）。ポートは T::kPort。
    template <class T>
    static void addRaw(CGateway& g, CTMessageHandler<T>* handler) {
        g.add(std::unique_ptr<CChannel>(new CTRawChannel<T>(handler)));
        if (handler != nullptr) g.addTickEnd([handler] { handler->onTickEnd(); });
    }
};

