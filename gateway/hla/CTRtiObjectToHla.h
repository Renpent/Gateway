// 受け取ったレコードを HLA のオブジェクトへ書く器。
//
// レコードにはインスタンスの同一性が無いので、KeyOf でどのインスタンスかを決める。
// 初めて見る鍵なら Create で登録し、以後は同じインスタンスに Write する。
//
// **インスタンスの削除は扱わない。** 一度登録したインスタンスは残り続ける。

#pragma once

#include <functional>
#include <map>
#include <string>
#include <utility>

#include "CTToHla.h"

namespace hla {

template <class T, class ObjPtr, class Key = std::string>
class CTRtiObjectToHla : public CTToHla<T> {
public:
    using KeyOf = Key (*)(const T&);                        ///< レコード -> インスタンスの鍵
    using Create = std::function<ObjPtr(const Key&)>;       ///< 初見の鍵で登録。失敗なら空を返す
    using Write = void (*)(const T&, const ObjPtr&);         ///< 属性を書いて update する

    CTRtiObjectToHla(KeyOf keyOf, Create create, Write write)
        : m_keyOf(keyOf), m_create(std::move(create)), m_write(write) {}

    void accept(const T& record) override {
        const Key key = m_keyOf(record);

        auto it = m_instances.find(key);
        if (it == m_instances.end()) {
            ObjPtr obj = m_create(key);
            if (!obj) return;   // 登録できなければ捨てる。次に届いたときに再び試す
            it = m_instances.insert(std::make_pair(key, obj)).first;
        }
        m_write(record, it->second);
    }

private:
    KeyOf m_keyOf;                      ///< レコード -> インスタンスの鍵
    Create m_create;                    ///< 初めて見る鍵でのインスタンス登録
    Write m_write;                      ///< 属性の書き込みと update
    std::map<Key, ObjPtr> m_instances;  ///< 鍵 -> 登録したインスタンス
};

}  // namespace hla
