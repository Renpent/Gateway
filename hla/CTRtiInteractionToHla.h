// 復元したインタラクションを HLA へ送る器。**探さない。詰め替えて送るだけ。**
//
// オブジェクト（CTRtiObjectToHla）は「このレコードはどのインスタンスのものか」を鍵で探して
// 属性を書き、update する。インタラクションにその段は無い — 宛先というものが存在せず、
// sendInteraction を1回呼べば終わる。だから対応表も鍵も持たない、完全にステートレスな器になる。
//
// 詰め替えと sendInteraction はツールキットの API を知っている send に任せる。
// **send の中で RTI を呼ぶのは正しい**（それが目的）。ただし周期ループのスレッドから呼ばれるので、
// RTI 側がその呼び出しで待たされる作りなら、そのぶん周期が延びる。
//
// **この環境に RTI は無いので、ここは実体化されない。**

#pragma once

#include <cstdint>
#include <functional>
#include <utility>

#include "CTToHla.h"

namespace hla {

template <class T>
class CTRtiInteractionToHla : public CTToHla<T> {
public:
    /// 詰め替えて sendInteraction まで行う。フェデレートを掴む必要があるので std::function。
    using Send = std::function<void(const T&)>;

    explicit CTRtiInteractionToHla(Send send) : m_send(std::move(send)) {}

    /// **record は使い回されている。** send の中で保持するならコピーすること。
    void accept(const T& record) override {
        m_send(record);
        ++m_sent;
    }

    [[nodiscard]] std::uint64_t getSent() const noexcept { return m_sent; }

private:
    Send m_send;                ///< 詰め替え + sendInteraction
    std::uint64_t m_sent = 0;   ///< 送った件数
};

}  // namespace hla
