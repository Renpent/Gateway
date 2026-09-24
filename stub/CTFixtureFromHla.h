// **本番には持っていかないファイル。** stub/ は RTI が無いこの環境で送信側を動かすための
// 代用品で、実 RTI に繋ぐときはフォルダごと消せる（直すのは app/CWiring.h だけ）。
//
// フィクスチャ関数が作る値を毎周期 perDrain 件渡す供給元。i 件目は make(i)。

#pragma once

#include <cstddef>
#include <vector>

#include "../gateway/hla/CTFromHla.h"

namespace stub {

template <class T>
class CTFixtureFromHla : public hla::CTFromHla<T> {
public:
    using Fixture = T (*)(std::size_t);   ///< i 件目の値を作る関数

    CTFixtureFromHla(Fixture make, std::size_t perDrain) : m_make(make), m_perDrain(perDrain) {}

    void drain(std::vector<T>& out) override {
        for (std::size_t n = 0; n < m_perDrain; ++n) out.push_back(m_make(m_next++));
    }

private:
    Fixture m_make;           ///< i 件目の値を作る関数
    std::size_t m_perDrain;   ///< 1回の drain で渡す件数
    std::size_t m_next = 0;   ///< 次に作る値の添字
};

}  // namespace stub
