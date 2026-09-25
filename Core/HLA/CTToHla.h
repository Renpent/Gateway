// UDP → HLA 向きの継ぎ目。UDP から復元した T を HLA へ出す。
//
// RTI に対しては publish する側になる（UDP 側から見ると受信）。

#pragma once

namespace hla {

template <class T>
class CTToHla {
public:
    virtual ~CTToHla() = default;

    /// 周期ループのスレッドから呼ばれる。**ブロックしないこと。**
    /// record はこの呼び出しの間だけ有効。後で使うならコピーすること。
    virtual void accept(const T& record) = 0;
};

}  // namespace hla
