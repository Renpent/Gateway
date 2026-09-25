// インタラクションを HLA から受け取る器。**ゲートウェイで唯一ロックを持つ場所。**
//
// インタラクションは RTI のコールバックが**RTI のスレッドで**持ってくるので、キューを挟む：
//
//     RTI のスレッド ──push()──→ [キュー] ──drain()──→ 周期ループのスレッド
//
// **push には変換済みの T を渡すこと。** RTI のパラメータのバッファはコールバックの間しか
// 有効でないため、コールバックの中で T に詰め替える。

#pragma once

#include <iterator>
#include <mutex>
#include <vector>

#include "CTFromHla.h"

namespace hla {

template <class T>
class CTRtiInteractionFromHla : public CTFromHla<T> {
public:
    /// RTI のスレッドから呼ぶ。
    void push(const T& record) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_back(record);
    }

    /// 周期ループのスレッドから呼ばれる。ロック中はキューの入れ替えだけにして、RTI 側を待たせない。
    void drain(std::vector<T>& out) override {
        std::vector<T> taken;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            taken.swap(m_queue);
        }
        out.insert(out.end(), std::make_move_iterator(taken.begin()),
                   std::make_move_iterator(taken.end()));
    }

private:
    std::mutex m_mutex;       ///< m_queue を守る
    std::vector<T> m_queue;   ///< コールバックが積み、drain が丸ごと持っていく
};

}  // namespace hla
