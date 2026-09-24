// 全チャネルをまとめて周期実行する。
//
// 1周（tick）でやること：
//   1. poll（待機時間 0）で、データが来ているポートを知る
//   2. そのポートだけ読み切って復号し、手元（HLA やアプリ）へ渡す
//   3. 手元から出てきた分を符号化して送る
//   4. addTickEnd で登録した処理を、登録した順に呼ぶ
//
// ソケットは全部ノンブロッキングなので tick は必ず有限時間で戻る。待つのは run() の sleep だけ。

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "udp/CPoller.h"
#include "CChannel.h"

namespace gw {

class CGateway {
public:
    /// チャネルを登録する。開くのは openAll。
    void add(std::unique_ptr<CChannel> channel);

    /// 周期の最後に呼ぶ処理を足す。いくつでも足せて、足した順に毎周期呼ばれる。
    void addTickEnd(std::function<void()> fn) { m_tickEnds.push_back(std::move(fn)); }

    /// 全チャネルを開く。peerHost が空なら受信専用。失敗したら理由を表示して false。
    [[nodiscard]] bool openAll(const std::string& peerHost);

    /// 1周ぶん。ブロックしない。
    void tick();

    /// hz の周期で tick を回す。seconds 秒で終わる（0 なら止まらない）。
    void run(unsigned hz, unsigned seconds);

private:
    std::vector<std::unique_ptr<CChannel>> m_channels;  ///< 登録されたチャネル。所有する
    std::vector<std::function<void()>> m_tickEnds;      ///< 周期の最後に呼ぶ処理。足した順
    std::vector<std::size_t> m_pollIndex;               ///< m_channels の添字 -> m_poller の添字
    udp::CPoller m_poller;                              ///< 全チャネルのソケットをまとめて見張る
    bool m_opened = false;                              ///< openAll が成功したか。false なら tick は何もしない
};

}  // namespace gw
