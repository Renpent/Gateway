// world を持つシングルトン。
//
// world はフェデレーションに join してからできるので、配線を作るときにはまだ無い。
// 変換関数は world を掴まず、**呼ばれたときにここから読む。**
//
// 書き換えるのは周期ループが止まっているときだけ（join 後・run の前 / run の後・resign の前）
// なので、ロックは要らない。

#pragma once

#include "Toolkit.h"

namespace rti {

class CDb {
public:
    static CDb& getInstance() {
        static CDb db;
        return db;
    }

    [[nodiscard]] tk::World* getWorld() const noexcept { return m_world; }
    void setWorld(tk::World* world) noexcept { m_world = world; }

private:
    CDb() = default;

    tk::World* m_world = nullptr;   ///< join 後に置き、resign 前に外す
};

}  // namespace rti
