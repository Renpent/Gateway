// オブジェクトクラスを HLA から取り出す器。
//
// 毎周期 Fetch（getRemoteXXX() を呼ぶ関数）でその瞬間の全インスタンスを取り、
// Convert で1インスタンス → 1レコードに詰め替える。クラスごとに渡すのはこの2つだけ。
//
// ObjPtr は bool 文脈で空か判定できてコピーできるもの（生ポインタや shared_ptr）。

#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "CTFromHla.h"

namespace hla {

template <class T, class ObjPtr>
class CTRtiObjectFromHla : public CTFromHla<T> {
public:
    using Fetch = std::function<std::vector<ObjPtr>()>;   ///< getRemoteXXX() を呼ぶ
    using Convert = T (*)(const ObjPtr&);                   ///< 1インスタンス -> 1レコード

    CTRtiObjectFromHla(Fetch fetch, Convert convert)
        : m_fetch(std::move(fetch)), m_convert(convert) {}

    void drain(std::vector<T>& out) override {
        // ポインタは持ち越さず、この場で値に写し切る（次の周期までに消えることがあるため）。
        for (const ObjPtr& p : m_fetch()) {
            if (p) out.push_back(m_convert(p));
        }
    }

private:
    Fetch m_fetch;       ///< getRemoteXXX() を呼ぶ
    Convert m_convert;   ///< 1インスタンス -> 1レコードの詰め替え
};

}  // namespace hla
