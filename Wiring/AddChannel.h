// 配線用の2つの関数。スタブの配線（CWiring）と本番の配線（CRtiWiring）で共有する。
//
//   addClass<T>(g, &fromHla, &toHla)   ICD のクラス。ID・ポートは T の生成された定数から決まる
//   addRaw<T>(g, &handler)             FOM に無い独自データ。ポートは T::kPort
//
// **T は必ず明示すること。** nullptr からは型が決まらない。

#pragma once

#include <memory>

#include "../Core/CGateway.h"
#include "../Core/CTClassChannel.h"
#include "../Core/CTMessageHandler.h"
#include "../Core/CTRawChannel.h"
#include "../Core/TClassBinding.h"

namespace wiring {

/// fromHla が nullptr なら送信しない、toHla が nullptr なら受けた分を捨てる。
/// どちらも借りるだけなので、CGateway より長く生きていること。
template <class T>
void addClass(core::CGateway& g, hla::CTFromHla<T>* fromHla, hla::CTToHla<T>* toHla) {
    g.add(std::unique_ptr<core::CChannel>(
        new core::CTClassChannel<T>(core::bindingOf<T>(), fromHla, toHla)));
}

/// 独自データの受信口を足し、ハンドラの onTickEnd() も周期末処理に登録する
/// （別の行にすると書き忘れて、積んだものが処理されなくなる）。
template <class T>
void addRaw(core::CGateway& g, core::CTMessageHandler<T>* handler) {
    g.add(std::unique_ptr<core::CChannel>(new core::CTRawChannel<T>(handler)));
    if (handler != nullptr) g.addTickEnd([handler] { handler->onTickEnd(); });
}

}  // namespace wiring
