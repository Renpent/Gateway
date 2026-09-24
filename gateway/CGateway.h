// 全チャネルをまとめて周期実行する。
//
// 1周（tick）でやることは4つ：
//   1. poll に待機時間 0 で聞き、来ているポートを知る
//   2. そのポートだけ読み切って復号し、手元（HLA やアプリ）へ渡す
//   3. 手元から出てきたものを符号化して送る
//   4. 周期の終わりの処理（setTickEnd で登録したもの。制御コマンドの反映に使う）
//
// ソケットは全部ノンブロッキングなので、tick は必ず有限時間で戻る。待つのはこのクラスの
// 最後の sleep だけで、そこが周期を決めている。

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../net/CPoller.h"
#include "CChannel.h"
#include "TLoopStats.h"

namespace gw {

class CGateway {
public:
    /// チャネルを登録する。open は openAll でまとめて行う。
    void add(std::unique_ptr<CChannel> channel);

    /// 全チャネルの受信ポートを bind し、送信先を設定する。
    /// peerHost が空なら受信専用で立ち上がる。
    [[nodiscard]] bool openAll(const std::string& peerHost);

    /// tick() の最後に1回呼ぶ処理を登録する。**制御コマンドはここで反映する。**
    ///
    /// 受信（2）と送信（3）のループがどちらも終わったあとなので、ループの途中で状態を変えて
    /// 壊す心配が無い。受信中に届いたコマンドはその場ではキューに積むだけにして（app/CTToApp.h）、
    /// ここでまとめて処理する。効くのは次の周期の送信から。
    ///
    /// 1つだけ登録できる。登録しなければ何もしない。
    void setTickEnd(std::function<void()> fn) { m_tickEnd = std::move(fn); }

    /// 1周ぶん。ブロックしない。
    void tick();

    /// hz の周期で tick を回す。seconds 秒で終わる（0 なら止まらない）。
    void run(unsigned hz, unsigned seconds);

    [[nodiscard]] const TLoopStats& getLoopStats() const noexcept { return m_loop; }
    [[nodiscard]] const std::vector<std::unique_ptr<CChannel>>& getChannels() const noexcept {
        return m_channels;
    }

    /// 開いた直後の状態。ポート・レコード長・収容数・ペイロード上限を並べる。
    void printPlan() const;

    void printSummary() const;

private:
    std::vector<std::unique_ptr<CChannel>> m_channels;  ///< 登録されたチャネル。所有する
    std::vector<std::size_t> m_pollIndex;   ///< m_channels の添字 -> CPoller の添字
    CPoller m_poller;                        ///< 全チャネルのソケットをまとめて見張る
    TLoopStats m_loop;                       ///< 周期が守れているかの記録
    std::function<void()> m_tickEnd;        ///< tick() の最後に呼ぶ処理。空なら何もしない
    bool m_opened = false;                  ///< openAll が成功したか。false なら tick は何もしない
};

}  // namespace gw
