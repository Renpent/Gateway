// 受け取った件数だけ数える受け口。中身は見ない。
//
// 「ポートの振り分けが合っているか」だけを確かめたいとき用。中身まで見たいなら
// VerifyingReceiver のように、送信側と同じ値を作って突き合わせる必要がある。

#pragma once

#include <cstddef>

#include "ToHla.h"

namespace hla {

template <class T>
class CountingReceiver : public ToHla<T> {
public:
    void accept(const T&) override { ++m_count; }
    [[nodiscard]] std::size_t count() const noexcept { return m_count; }

private:
    std::size_t m_count = 0;    ///< 受け取った件数
};

}  // namespace hla
