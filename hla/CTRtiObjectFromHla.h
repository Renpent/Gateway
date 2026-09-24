// オブジェクト属性を HLA から汲み出す器。**RTI の型を1つも名指ししない。**
//
// getRemoteXXX() が「その瞬間の全インスタンス」を std::vector<XxxPtr> で返す、という形を
// 前提にしている。クラスごとに違うのは「どの getter を呼ぶか」と「どう詰め替えるか」の2つ
// だけなので、その2つだけを外から受け取る。**50クラスあってもこの器は1つで足りる。**
//
// 器を分けずにクラスごとの派生を書くと、ループ・null 判定・数え上げという定型が全クラスに
// 複製される。そこに間違いが入ると直す場所が50箇所になる。
//
// **この環境に RTI は無いので、ここは実体化されない。** 形を先に決めておくためのもので、
// 本番では app/CWiring.h がこれを実体化する。
//
// ObjPtr に求めるのは2つだけ：bool 文脈で空かどうか判定できること（生ポインタ・shared_ptr
// のどちらでもよい）と、コピーできること。**中身の触り方は知らない** — 参照を渡すので、
// デリファレンスの仕方は変換関数側が決める。

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

#include "CTFromHla.h"

namespace hla {

template <class T, class ObjPtr>
class CTRtiObjectFromHla : public CTFromHla<T> {
public:
    /// getRemoteXXX() を呼ぶだけ。フェデレートを掴む必要があるので std::function。
    using Fetch = std::function<std::vector<ObjPtr>()>;

    /// 1インスタンス -> 1レコード。**状態を持たない自由関数**である想定なので関数ポインタ。
    /// クラスごとに1本書く（ICDgenerator で生成できる見込み）。
    using Convert = T (*)(const ObjPtr&);

    CTRtiObjectFromHla(Fetch fetch, Convert convert)
        : m_fetch(std::move(fetch)), m_convert(convert) {}

    /// **out はクリアしない。** 末尾に足すだけで、捨てるかどうかは CTClassChannel が
    /// TClassKind を見て決める。ここでクリアするとインタラクションの持ち越しが消える。
    ///
    /// **ブロックしないこと。** 周期ループのスレッドから呼ばれるので、ここで待つと
    /// その周期ぶん全クラスが遅れる。
    std::size_t drain(std::vector<T>& out) override {
        // **この場で値に写し切る。** ポインタを持ち越すと、次の周期までに RTI 側が
        // インスタンスを消したときにぶら下がる。
        const std::vector<ObjPtr> objects = m_fetch();

        std::size_t taken = 0;
        for (const ObjPtr& p : objects) {
            if (!p) { ++m_skipped; continue; }   // 消えた直後のインスタンス
            out.push_back(m_convert(p));
            ++taken;
        }
        m_seen += static_cast<std::uint64_t>(objects.size());
        return taken;
    }

    [[nodiscard]] std::uint64_t getSeen() const noexcept { return m_seen; }
    [[nodiscard]] std::uint64_t getSkipped() const noexcept { return m_skipped; }

private:
    Fetch m_fetch;                  ///< getRemoteXXX() を呼ぶ
    Convert m_convert;              ///< 1インスタンス -> 1レコードの詰め替え
    std::uint64_t m_seen = 0;       ///< 受け取ったポインタの総数
    std::uint64_t m_skipped = 0;    ///< 空だったので飛ばした数
};

}  // namespace hla
