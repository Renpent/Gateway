// HLA → UDP 向きの継ぎ目。RTI が反映した属性更新を T に詰めて渡す。
//
// 名前に向きが入っているのは、ここが一番読み間違えられるところだから。HLA の用語では
// **これを実装するとき RTI に対してやるのは subscribe** で、UDP 側の相手は udp::CTUdpSender
// （送信）になる。publish / subscribe が橋の両側で逆向きになるので、Source のような
// 相対的な名前だとどちらの視点か分からなくなる。
//
// **この環境に RTI は無い**ので、ここには RTI を呼ぶコードが1行も無い。それでも継ぎ目を先に
// 切ってあるのは、上の層（CTUdpSender / CTUdpReceiver / 生成コーデック）が RTI の型を一切知らない
// 状態を保つため。
//
// **スレッドの取り決めは Federate.h にまとめてある。実装する前に必ず読むこと。**

#pragma once

#include <cstddef>
#include <vector>

namespace hla {

template <class T>
class CTFromHla {
public:
    virtual ~CTFromHla() = default;

    /// 前回の呼び出し以降に届いたぶんを out の末尾に足し、足した件数を返す。
    ///
    /// **ブロックしないこと。** 呼び出し側が送信レートを決めていて、ここで待たれると
    /// その周期ぶん全クラスが遅れる。キューが空なら 0 を返して即座に戻る。
    virtual std::size_t drain(std::vector<T>& out) = 0;
};

}  // namespace hla
