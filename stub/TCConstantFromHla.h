// **本番には持っていかないファイル。** stub/ は RTI が無いこの環境でゲートウェイを動かし、
// 往復を検証するための代用品だけが入っている。実 RTI に繋ぐときは stub/ ごと消せて、
// 直す先は app/CWiring.h の1ファイルで済む。
//
// 中身に意味を持たせない供給元。
//
// ポートが複数あって poll が正しく振り分けているかを見るためだけのもので、
// 値は既定構築のまま（可変長配列は 0 要素）。**毎回まったく同じ値**なので、
// これを流したクラスはバイト比較しても何も分からない。数えるだけの相手と組ませること。

#pragma once

#include <cstddef>
#include <vector>

#include "../hla/TCFromHla.h"

namespace stub {

template <class T>
class TCConstantFromHla : public hla::TCFromHla<T> {
public:
    explicit TCConstantFromHla(std::size_t perDrain = 1) : m_perDrain(perDrain) {}

    std::size_t drain(std::vector<T>& out) override {
        for (std::size_t n = 0; n < m_perDrain; ++n) out.push_back(T{});
        m_produced += m_perDrain;
        return m_perDrain;
    }

    [[nodiscard]] std::size_t getProduced() const noexcept { return m_produced; }

private:
    std::size_t m_perDrain;         ///< 1回の drain で作る件数
    std::size_t m_produced = 0;     ///< これまでに作った累計件数
};

}  // namespace stub
