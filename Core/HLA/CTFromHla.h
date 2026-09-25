// HLA → UDP 向きの継ぎ目。HLA 側のデータを T に詰めて渡す。
//
// RTI に対しては subscribe する側になる（UDP 側から見ると送信）。
//
// **スレッド：** drain() は周期ループのスレッドから呼ばれる。RTI のコールバックは別のスレッドで
// 来るので、コールバックで受けるものはロック付きのキューを挟むこと（CTRtiInteractionFromHla）。

#pragma once

#include <vector>

namespace hla {

template <class T>
class CTFromHla {
public:
    virtual ~CTFromHla() = default;

    /// 前回以降に出てきた分を out の末尾に足す。**ブロックしないこと。**
    virtual void drain(std::vector<T>& out) = 0;
};

}  // namespace hla
