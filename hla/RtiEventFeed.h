// インタラクションを HLA から受け取る器。**ゲートウェイの中で唯一ロックを持つ場所。**
//
// オブジェクトと違って、インタラクションはこちらが読みに行くのではなく、RTI のコールバックが
// **RTI のスレッドで**勝手に持ってくる。周期ループとは別のスレッドなので、間にキューを1枚
// 挟んで、そこだけを排他する。
//
//     RTI のスレッド ──push()──→ [キュー] ──drain()──→ 周期ループのスレッド
//
// **変換はコールバックの中で済ませてから push すること。** RTI のパラメータハンドルや値バッファは
// コールバックの間しか有効でないのが普通で、ポインタを持ち越して後から復号することはできない。
// push に渡すのは詰め替え済みの T。
//
// **キューは深さで縛る。件数で周期を縛らない。** 周期ループが止まっている間にコールバックが
// 溜め続けるとメモリが伸びる一方なので、上限を超えた push は捨てて数える（新しいほうを捨てる）。
// 1周期に出す件数のほうは縛らない — 渡されたものはその周期で出し切る（ClassChannel の方針）。
//
// **この環境に RTI は無いので、push を呼ぶ者がいない。** 形を先に決めておくためのもの。

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <mutex>
#include <vector>

#include "FromHla.h"

namespace hla {

template <class T>
class RtiEventFeed : public FromHla<T> {
public:
    explicit RtiEventFeed(std::size_t maxQueued = 1024) : m_maxQueued(maxQueued) {}

    /// **RTI のスレッドから呼ばれる。** コールバックの中で T に詰め替えたものを渡す。
    /// 溢れていたら捨てて false。捨てた数は dropped() に出る。
    bool push(const T& record) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.size() >= m_maxQueued) {
            ++m_dropped;
            return false;
        }
        m_queue.push_back(record);
        ++m_pushed;
        return true;
    }

    /// 周期ループのスレッドから。**ロックの中でやるのはキューの差し替えだけ**で、
    /// out への挿入はロックの外。ここを雑にすると RTI のスレッドが周期ループに引きずられる。
    /// out はクリアしない（Events は持ち越すのが正しく、捨てる判断は ClassChannel のもの）。
    std::size_t drain(std::vector<T>& out) override {
        std::vector<T> taken;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            taken.swap(m_queue);
        }
        out.insert(out.end(),
                   std::make_move_iterator(taken.begin()),
                   std::make_move_iterator(taken.end()));
        m_drained += static_cast<std::uint64_t>(taken.size());
        return taken.size();
    }

    [[nodiscard]] std::uint64_t pushed() const noexcept { return m_pushed.load(); }
    [[nodiscard]] std::uint64_t dropped() const noexcept { return m_dropped.load(); }
    [[nodiscard]] std::uint64_t drained() const noexcept { return m_drained; }

private:
    std::mutex m_mutex;                    ///< m_queue を守る。push と drain の間だけ
    std::vector<T> m_queue;                ///< コールバックが積み、drain が丸ごと持っていく
    std::size_t m_maxQueued;               ///< キューの深さの上限。超えた push は捨てる
    std::atomic<std::uint64_t> m_pushed{0};   ///< 受け付けた件数（RTI スレッドが書く）
    std::atomic<std::uint64_t> m_dropped{0};  ///< 溢れて捨てた件数（RTI スレッドが書く）
    std::uint64_t m_drained = 0;           ///< 周期ループへ渡した件数（ループスレッドだけが触る）
};

}  // namespace hla
