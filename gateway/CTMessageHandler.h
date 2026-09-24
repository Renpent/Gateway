// UDP → アプリ 向きの継ぎ目。FOM に無い独自データを、それを使う側のロジックへ渡す。
//
// 形は hla::CTToHla と同じ（accept が1本）だが、**別の型にしてある。** 独自データは RTI を
// 1ミリも通らないので、hla::CTToHla を実装させると「HLA へ」という嘘の名前が1つ増える。
// 実装の名前はすべて Handler で終える（CCommandHandler など）。
//
// **ゲートウェイ自身の制御に使うときの注意。** accept は周期ループのスレッドで、tick() の
// **途中**（前半の UDP 受信）に呼ばれる。ここで CGateway のチャネル一覧を変えたり止めたりすると、
// いま回しているループそのものが壊れる。アプリのロジックへ渡すだけならこの制約は無い。
//
// **決めてある形：accept ではキューに積むだけ。反映は onTickEnd() でまとめて行う。**
//
//     tick():  [UDP 受信 → HLA へ]  [HLA から → UDP 送信]  [onTickEnd]
//                   └ accept で積む                            └ ここで効かせる
//
// onTickEnd() は CTRawChannel 経由で**毎周期かならず呼ばれる**。どこかに登録する必要は無い。
// 効き始めるのは次の周期から（20 Hz なら 50 ms 後）。前半・後半のあいだで反映すれば同じ周期の
// 送信から効くが、そこでチャネルやソケットを変えると後半のループと poll の対応表がずれる。
// 積むのも反映するのも周期ループのスレッドなので、このキューにロックは要らない
// （RTI スレッドから積まれる CTRtiInteractionFromHla とはそこが違う）。
//
// 実例は app/CCommandHandler.h（accept で積み、onTickEnd で1件ずつ handle する）。
//
// 送る向き（アプリ → UDP）が要るようになったら、対になるものをここに足す。

#pragma once

namespace gw {

template <class T>
class CTMessageHandler {
public:
    virtual ~CTMessageHandler() = default;

    /// **ブロックしないこと。** 周期ループのスレッドから呼ばれ、ここで待つと全ポートが遅れる。
    /// message はこの呼び出しの間だけ有効。後で使うならコピーすること。
    virtual void accept(const T& message) = 0;

    /// 周期の最後に1回呼ばれる。その周期に accept で積んだものをここで処理する。
    /// accept の中で処理を済ませるハンドラなら、何もしなくてよい（既定は何もしない）。
    virtual void onTickEnd() {}
};

}  // namespace gw
