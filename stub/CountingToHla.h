// **本番には持っていかないファイル。** stub/ は RTI が無いこの環境でゲートウェイを動かし、
// 往復を検証するための代用品だけが入っている。実 RTI に繋ぐときは stub/ ごと消せて、
// 直す先は app/Wiring.h の1ファイルで済む。
//
// 受け取った件数だけ数える受け口。中身は見ない。
//
// 「ポートの振り分けが合っているか」だけを確かめたいとき用。中身まで見たいなら
// VerifyingToHla のように、送信側と同じ値を作って突き合わせる必要がある。

#pragma once

#include <cstddef>

#include "../hla/ToHla.h"

namespace stub {

template <class T>
class CountingToHla : public hla::ToHla<T> {
public:
    void accept(const T&) override { ++m_count; }
    [[nodiscard]] std::size_t count() const noexcept { return m_count; }

private:
    std::size_t m_count = 0;    ///< 受け取った件数
};

}  // namespace stub
