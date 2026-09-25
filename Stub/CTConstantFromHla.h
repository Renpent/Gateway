// **本番には持っていかないファイル。** Stub/ は RTI が無いこの環境で送信側を動かすための
// 代用品で、実 RTI に繋ぐときはフォルダごと消せる（直すのは Wiring/CWiring.h だけ）。
//
// 既定構築の値（可変長配列は 0 要素）を毎周期 perDrain 件渡す供給元。値に意味は無い。

#pragma once

#include <cstddef>
#include <vector>

#include "../Core/HLA/CTFromHla.h"

namespace stub {

template <class T>
class CTConstantFromHla : public hla::CTFromHla<T> {
public:
    explicit CTConstantFromHla(std::size_t perDrain) : m_perDrain(perDrain) {}

    void drain(std::vector<T>& out) override {
        for (std::size_t n = 0; n < m_perDrain; ++n) out.push_back(T{});
    }

private:
    std::size_t m_perDrain;   ///< 1回の drain で渡す件数
};

}  // namespace stub
