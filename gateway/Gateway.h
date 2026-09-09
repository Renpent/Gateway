// 全チャネルをまとめて周期実行する。
//
// 1周（tick）でやることは3つ：
//   1. poll に待機時間 0 で聞き、来ているポートを知る
//   2. そのポートだけ読み切って復号し、HLA 側へ渡す
//   3. HLA 側から出てきたものを符号化して送る
//
// ソケットは全部ノンブロッキングなので、tick は必ず有限時間で戻る。待つのはこのクラスの
// 最後の sleep だけで、そこが周期を決めている。

#ifndef HLAGW_GATEWAY_H
#define HLAGW_GATEWAY_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../net/Poller.h"
#include "Channel.h"

namespace gw {

/// 周期が守れているかの記録。守れていないことに気づけないのが一番困るので測っておく。
struct LoopStats {
    std::uint64_t ticks = 0;
    std::uint64_t overruns = 0;      ///< 1周の処理が周期を超えた回数
    double maxTickMs = 0.0;          ///< 1周の処理時間の最大
    double maxLateMs = 0.0;          ///< 起床が予定からどれだけ遅れたかの最大
    double sumLateMs = 0.0;
};

class Gateway {
public:
    /// チャネルを登録する。open は openAll でまとめて行う。
    void add(std::unique_ptr<Channel> channel);

    /// 全チャネルの受信ポートを bind し、送信先を設定する。
    /// peerHost が空なら受信専用で立ち上がる。
    [[nodiscard]] bool openAll(const std::string& peerHost);

    /// 1周ぶん。ブロックしない。
    void tick();

    /// hz の周期で tick を回す。seconds 秒で終わる（0 なら止まらない）。
    void run(unsigned hz, unsigned seconds);

    [[nodiscard]] const LoopStats& loopStats() const noexcept { return loop_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Channel>>& channels() const noexcept {
        return channels_;
    }

    /// 開いた直後の状態。レコード長・収容数・送信レートを並べる。
    void printPlan(unsigned hz) const;

    void printSummary() const;

private:
    std::vector<std::unique_ptr<Channel>> channels_;
    std::vector<std::size_t> pollIndex_;   ///< channels_ の添字 -> Poller の添字
    Poller poller_;
    LoopStats loop_;
    bool opened_ = false;
};

}  // namespace gw

#endif  // HLAGW_GATEWAY_H
