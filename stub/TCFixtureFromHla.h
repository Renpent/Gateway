// **本番には持っていかないファイル。** stub/ は RTI が無いこの環境でゲートウェイを動かし、
// 往復を検証するための代用品だけが入っている。実 RTI に繋ぐときは stub/ ごと消せて、
// 直す先は app/CWiring.h の1ファイルで済む。
//
// フィクスチャが作った値を毎周期そのまま渡す供給元。RTI の代わり。
//
// **TCFixtureFromHla を一般化したもの。** クラスが2つ目になった時点で、違うのは
// 「i 番目の値をどう作るか」だけだと分かったので、そこだけを関数ポインタで受ける。
// クラスを増やしても増えるのは <Class>Fixture.h ひとつで、供給元は実体化するだけでよい。
//
// TCConstantFromHla との違いは値が毎回変わること。往復をバイト比較するならこちら、
// ポートの振り分けだけ見たいなら TCConstantFromHla で足りる。

#pragma once

#include <cstddef>
#include <vector>

#include "../hla/TCFromHla.h"

namespace stub {

template <class T>
class TCFixtureFromHla : public hla::TCFromHla<T> {
public:
    /// i 番目の値を作る関数。**照合側（TCVerifyingToHla）に同じものを渡すこと。**
    /// 「送ったはずの値」の定義が2箇所に分かれると、往復照合が意味を失う。
    using Fixture = T (*)(std::size_t);

    TCFixtureFromHla(Fixture make, std::size_t perDrain = 4)
        : m_make(make), m_perDrain(perDrain) {}

    std::size_t drain(std::vector<T>& out) override {
        for (std::size_t n = 0; n < m_perDrain; ++n) out.push_back(m_make(m_produced++));
        return m_perDrain;
    }

    [[nodiscard]] std::size_t getProduced() const noexcept { return m_produced; }

private:
    Fixture m_make;                 ///< i 番目の値を作る関数
    std::size_t m_perDrain;         ///< 1回の drain で作る件数
    std::size_t m_produced = 0;     ///< これまでに作った累計件数（m_make の添字）
};

}  // namespace stub
