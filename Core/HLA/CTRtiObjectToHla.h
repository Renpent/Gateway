// 受け取ったレコードを HLA のオブジェクトへ書く器。
//
// レコードにはインスタンスの同一性が無いので、KeyOf でどのインスタンスかを決める。
// 初めて見る鍵なら Create で登録し、以後は同じインスタンスに Update する（属性を書いて update）。
//
// **インスタンスの削除は扱わない。** 一度登録したインスタンスは残り続ける。
//
// 鍵の型は既定で int。ほかの型にするときは3つ目の引数で指定する（std::uint64_t、std::string、
// std::tuple など。std::map のキーにするので < で比べられること）。
// **KeyOf の戻り値は鍵の型とぴったり同じにすること**（関数ポインタで受けるため）。
// Create の引数は鍵の型から変換できれば通る（int の値渡しでも const int& でもよい）。

#pragma once

#include <functional>
#include <map>
#include <utility>

#include "CTToHla.h"

template <class T, class ObjPtr, class Key = int>
class CTRtiObjectToHla : public CTToHla<T> {
public:
    using KeyOf = Key (*)(const T&);                        ///< レコード -> インスタンスの鍵
    using Create = std::function<ObjPtr(const Key&)>;       ///< 初見の鍵で登録。失敗なら空を返す
    using Update = void (*)(const T&, const ObjPtr&);        ///< 属性を書いて update する

    CTRtiObjectToHla(KeyOf keyOf, Create create, Update update)
        : m_keyOf(keyOf), m_create(std::move(create)), m_update(update) {}

    void accept(const T& record) override {
        const Key key = m_keyOf(record);

        auto it = m_instances.find(key);
        if (it == m_instances.end()) {
            ObjPtr obj = m_create(key);
            if (!obj) return;   // 登録できなければ捨てる。次に届いたときに再び試す
            it = m_instances.insert(std::make_pair(key, obj)).first;
        }
        m_update(record, it->second);
    }

private:
    KeyOf m_keyOf;                      ///< レコード -> インスタンスの鍵
    Create m_create;                    ///< 初めて見る鍵でのインスタンス登録
    Update m_update;                    ///< 属性の書き込みと update
    std::map<Key, ObjPtr> m_instances;  ///< 鍵 -> 登録したインスタンス
};

