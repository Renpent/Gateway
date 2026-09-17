// UDP → HLA 向きの継ぎ目。UDP から復元した T を HLA に押し込む。
//
// 名前に向きが入っている理由は FromHla.h と同じ。HLA の用語では **これを実装するとき
// RTI に対してやるのは publish** で、UDP 側の相手は gw::Subscriber（受信）になる。
//
// **この環境に RTI は無い**ので、ここには RTI を呼ぶコードが1行も無い。
//
// **スレッドの取り決めは Federate.h にまとめてある。実装する前に必ず読むこと。**

#pragma once

namespace hla {

template <class T>
class ToHla {
public:
    virtual ~ToHla() = default;

    /// **ブロックしないこと。** 周期ループのスレッドから呼ばれる。
    virtual void accept(const T& record) = 0;
};

}  // namespace hla
