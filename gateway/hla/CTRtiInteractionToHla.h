// 受け取ったインタラクションを HLA へ送る器。
//
// オブジェクトと違って宛先を探す必要が無いので、Send（パラメータを詰めて sendInteraction）を
// 呼ぶだけ。

#pragma once

#include <functional>
#include <utility>

#include "CTToHla.h"

namespace hla {

template <class T>
class CTRtiInteractionToHla : public CTToHla<T> {
public:
    using Send = std::function<void(const T&)>;   ///< パラメータを詰めて sendInteraction

    explicit CTRtiInteractionToHla(Send send) : m_send(std::move(send)) {}

    void accept(const T& record) override { m_send(record); }

private:
    Send m_send;   ///< 詰め替え + sendInteraction
};

}  // namespace hla
