// 復元したレコードを HLA へ戻す器。**RTI の型を1つも名指ししない。**
//
// 送信側と違って、受信側には器だけでは決まらないことが1つある — **このレコードはどの
// インスタンスのものか。** ICD のレコードにはインスタンスの同一性が入っていないので、
// どのフィールドを鍵にするかを ICD 側で決めて keyOf として渡す必要がある。
//
// この器が引き受けるのは「鍵 -> インスタンス」の対応表と、初めて見る鍵での新規登録まで。
// 属性を実際に書くのと updateAttributeValues を呼ぶのは、ツールキットの API を知っている
// write に任せる。
//
// **この環境に RTI は無いので、ここは実体化されない。**
//
// **インスタンスの削除は扱わない。** 対応表の項目は一度入ったら消えないので、相手が送るのを
// やめたローカルオブジェクトは残り続ける。削除が起きない前提での割り切りで、前提が変わったら
// ここに消す口を足すことになる（届かなくなって一定時間で deleteObjectInstance するか、
// ICD に削除を表すフィールドを足すか）。
//
// **インタラクションをこの器で受けないこと。** 宛先を探す必要がないので、対応表がまるごと
// 無駄になる。あちらは RtiInteractionToHla — accept() で詰め替えて sendInteraction を呼ぶだけ。
//
// 未決のまま残っていること：**部分更新の扱い。** ICD は常に全属性ぶんの箱を送るので、
// 送信側が持っていなかった属性はゼロで届く。それを前回値で埋めるか既定値で埋めるかは
// write の中で決めることになる。

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>

#include "ToHla.h"

namespace hla {

template <class T, class ObjPtr, class Key = std::string>
class RtiObjectToHla : public ToHla<T> {
public:
    /// このレコードがどのインスタンスのものかを決める。**ICD 側の設計判断。**
    /// 1つの装置が複数の子を持つ型なら、鍵は複数フィールドの組になる。
    using KeyOf = Key (*)(const T&);

    /// 初めて見る鍵に対してインスタンスを用意する（registerObjectInstance 相当）。
    /// フェデレートを掴む必要があるので std::function。空を返したら登録失敗として数える。
    using Create = std::function<ObjPtr(const Key&)>;

    /// 属性を書き、必要なら update まで行う。**状態を持たない自由関数**である想定。
    /// セッタが即反映のツールキットもあるので、update を呼ぶかどうかはここに委ねる。
    using Write = void (*)(const T&, const ObjPtr&);

    RtiObjectToHla(KeyOf keyOf, Create create, Write write)
        : m_keyOf(keyOf), m_create(std::move(create)), m_write(write) {}

    /// **ブロックしないこと。** 周期ループのスレッドから呼ばれる。
    /// **record は使い回されている。** 保持するならコピーすること。
    void accept(const T& record) override {
        const Key key = m_keyOf(record);

        typename std::map<Key, ObjPtr>::iterator it = m_instances.find(key);
        if (it == m_instances.end()) {
            ObjPtr obj = m_create(key);
            if (!obj) { ++m_refused; return; }   // 登録できなかった。黙って捨てない
            it = m_instances.insert(std::make_pair(key, obj)).first;
            ++m_registered;
        }

        m_write(record, it->second);
        ++m_written;
    }

    [[nodiscard]] std::size_t instances() const noexcept { return m_instances.size(); }
    [[nodiscard]] std::uint64_t registered() const noexcept { return m_registered; }
    [[nodiscard]] std::uint64_t refused() const noexcept { return m_refused; }
    [[nodiscard]] std::uint64_t written() const noexcept { return m_written; }

private:
    KeyOf m_keyOf;                    ///< レコード -> インスタンスの鍵
    Create m_create;                  ///< 初めて見る鍵でのインスタンス登録
    Write m_write;                    ///< 属性の書き込み（必要なら update まで）
    std::map<Key, ObjPtr> m_instances;  ///< 鍵 -> 自分が登録したインスタンス
    std::uint64_t m_registered = 0;   ///< 新規に登録した数
    std::uint64_t m_refused = 0;      ///< 登録できず捨てた数
    std::uint64_t m_written = 0;      ///< 書き込んだ回数
};

}  // namespace hla
