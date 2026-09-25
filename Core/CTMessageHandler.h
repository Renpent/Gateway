// UDP → アプリ 向きの受け口。FOM に無い独自データを、それを使う側へ渡す。
//
// **accept ではキューに積むだけにして、処理は onTickEnd() で行う。** accept は tick() の途中
// （UDP 受信の最中）に呼ばれるので、そこでゲートウェイの状態を変えると回しているループが壊れる。
//
//     tick():  [UDP 受信 → HLA へ]  [HLA から → UDP 送信]  [周期末の処理]
//                   └ accept で積む                            └ onTickEnd で処理する
//
// onTickEnd() は配線の addRaw が CGateway::addTickEnd に登録するので、毎周期必ず呼ばれる。
// どちらも周期ループのスレッドから呼ばれるので、キューにロックは要らない。

#pragma once

namespace core {

template <class T>
class CTMessageHandler {
public:
    virtual ~CTMessageHandler() = default;

    /// **ブロックしないこと。** message はこの呼び出しの間だけ有効。
    virtual void accept(const T& message) = 0;

    /// 周期の最後に毎回呼ばれる。
    virtual void onTickEnd() {}
};

}  // namespace core
