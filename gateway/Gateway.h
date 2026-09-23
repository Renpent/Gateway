// 全チャネルをまとめて周期実行する。
//
// 1周（tick）でやることは3つ：
//   1. poll に待機時間 0 で聞き、来ているポートを知る
//   2. そのポートだけ読み切って復号し、HLA 側へ渡す
//   3. HLA 側から出てきたものを符号化して送る
//
// ソケットは全部ノンブロッキングなので、tick は必ず有限時間で戻る。待つのはこのクラスの
// 最後の sleep だけで、そこが周期を決めている。

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../net/Poller.h"
#include "Channel.h"
#include "LoopStats.h"

namespace gw {

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

    [[nodiscard]] const LoopStats& loopStats() const noexcept { return m_loop; }
    [[nodiscard]] const std::vector<std::unique_ptr<Channel>>& channels() const noexcept {
        return m_channels;
    }

    /// 開いた直後の状態。ポート・レコード長・収容数・ペイロード上限を並べる。
    void printPlan() const;

    void printSummary() const;

private:
    std::vector<std::unique_ptr<Channel>> m_channels;  ///< 登録されたチャネル。所有する
    std::vector<std::size_t> m_pollIndex;   ///< m_channels の添字 -> Poller の添字
    Poller m_poller;                        ///< 全チャネルのソケットをまとめて見張る
    LoopStats m_loop;                       ///< 周期が守れているかの記録
    bool m_opened = false;                  ///< openAll が成功したか。false なら tick は何もしない
};

}  // namespace gw
