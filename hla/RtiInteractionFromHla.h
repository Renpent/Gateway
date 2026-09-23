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

/// キューの深さの上限。**全クラス共通の1つの値で足りる。クラスごとに測って決めない。**
///
/// 理由は3つ。
///
/// 1. **使わなければ1バイトも要らない。** m_queue は普通の std::vector で、積まれたぶんしか
///    確保しない。しかも drain() は swap で丸ごと持っていくので、毎周期 capacity ごと 0 に
///    戻る。この定数は「ここまでなら伸びてよい」という天井であって、確保量ではない。
///    idle のクラスを50個並べても消費はゼロ。
///
/// 2. **効いてくる状況が1つしかない。** drain() は毎周期キューを空にするので、深さが伸びるのは
///    「RTI が1周期のあいだに渡してくる件数」がこの値を超えたときだけ。20 Hz なら 50 ms に
///    2048 件。実運用のインタラクション頻度から見て、これを超えるクラスがあるなら、それは
///    上限の調整ではなく設計の見直しが要る状況。
///
/// 3. **超えても黙って壊れない。** 溢れた push は false を返して dropped() に載り、
///    最大どこまで伸びたかは maxDepth() で後から読める。**先に見積もるのではなく、
///    流してから maxDepth() を見ればよい。**
///
/// 大きめに取ってあるのはそのため。個別に詰める価値が出るのは、1件が極端に大きいクラスで
/// 実際に天井に張り付いたときだけで、そのときだけコンストラクタに数値を渡せばよい。
inline constexpr std::size_t kInteractionQueueDepth = 2048;

template <class T>
class RtiInteractionFromHla : public FromHla<T> {
public:
    /// **既定のままでよい。** 数値を渡すのは、maxDepth() が天井に張り付いたクラスが
    /// 実際に出てきたときだけ（kInteractionQueueDepth の説明を参照）。
    explicit RtiInteractionFromHla(std::size_t maxQueued = kInteractionQueueDepth)
        : m_maxQueued(maxQueued) {}

    /// **RTI のスレッドから呼ばれる。** コールバックの中で T に詰め替えたものを渡す。
    /// 溢れていたら捨てて false。捨てた数は dropped() に出る。
    bool push(const T& record) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.size() >= m_maxQueued) {
            ++m_dropped;
            return false;
        }
        m_queue.push_back(record);
        // 最大どこまで伸びたか。**上限を見積もらずに済ませるための数字**で、流したあとに
        // これを見れば、そのクラスに上限の調整が要るかどうかが分かる。
        if (m_queue.size() > m_maxDepth.load(std::memory_order_relaxed)) {
            m_maxDepth.store(m_queue.size(), std::memory_order_relaxed);
        }
        ++m_pushed;
        return true;
    }

    /// 周期ループのスレッドから。**ロックの中でやるのはキューの差し替えだけ**で、
    /// out への挿入はロックの外。ここを雑にすると RTI のスレッドが周期ループに引きずられる。
    /// out はクリアしない（インタラクションは持ち越すのが正しく、捨てる判断は ClassChannel のもの）。
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

    /// 実際に積み上がった最大の深さ。**上限が妥当かを後から確かめるための値。**
    /// これが maxQueued に届いていないなら、そのクラスに個別の調整は要らない。
    [[nodiscard]] std::size_t maxDepth() const noexcept {
        return m_maxDepth.load(std::memory_order_relaxed);
    }

    /// この器が使っている上限。
    [[nodiscard]] std::size_t maxQueued() const noexcept { return m_maxQueued; }

private:
    std::mutex m_mutex;                    ///< m_queue を守る。push と drain の間だけ
    std::vector<T> m_queue;                ///< コールバックが積み、drain が丸ごと持っていく
    std::size_t m_maxQueued;               ///< キューの深さの上限。超えた push は捨てる
    std::atomic<std::size_t> m_maxDepth{0};   ///< 到達した最大の深さ（RTI スレッドが書く）
    std::atomic<std::uint64_t> m_pushed{0};   ///< 受け付けた件数（RTI スレッドが書く）
    std::atomic<std::uint64_t> m_dropped{0};  ///< 溢れて捨てた件数（RTI スレッドが書く）
    std::uint64_t m_drained = 0;           ///< 周期ループへ渡した件数（ループスレッドだけが触る）
};

}  // namespace hla
