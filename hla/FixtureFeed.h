// フィクスチャが作った値を毎周期そのまま渡す供給元。RTI の代わり。
//
// **StubRadarBeamFeed を一般化したもの。** クラスが2つ目になった時点で、違うのは
// 「i 番目の値をどう作るか」だけだと分かったので、そこだけを関数ポインタで受ける。
// クラスを増やしても増えるのは <Class>Fixture.h ひとつで、供給元は実体化するだけでよい。
//
// ConstantFeed との違いは値が毎回変わること。往復をバイト比較するならこちら、
// ポートの振り分けだけ見たいなら ConstantFeed で足りる。

#pragma once

#include <cstddef>
#include <vector>

#include "FromHla.h"

namespace hla {

template <class T>
class FixtureFeed : public FromHla<T> {
public:
    /// i 番目の値を作る関数。**照合側（VerifyingReceiver）に同じものを渡すこと。**
    /// 「送ったはずの値」の定義が2箇所に分かれると、往復照合が意味を失う。
    using Fixture = T (*)(std::size_t);

    FixtureFeed(Fixture make, std::size_t perDrain = 4)
        : m_make(make), m_perDrain(perDrain) {}

    std::size_t drain(std::vector<T>& out) override {
        for (std::size_t n = 0; n < m_perDrain; ++n) out.push_back(m_make(m_produced++));
        return m_perDrain;
    }

    [[nodiscard]] std::size_t produced() const noexcept { return m_produced; }

private:
    Fixture m_make;                 ///< i 番目の値を作る関数
    std::size_t m_perDrain;         ///< 1回の drain で作る件数
    std::size_t m_produced = 0;     ///< これまでに作った累計件数（m_make の添字）
};

}  // namespace hla
