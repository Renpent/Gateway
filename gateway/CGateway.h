// 全チャネルをまとめて周期実行する。
//
// 1周（tick）でやることは4つ：
//   1. poll に待機時間 0 で聞き、来ているポートを知る
//   2. そのポートだけ読み切って復号し、手元（HLA やアプリ）へ渡す
//   3. 手元から出てきたものを符号化して送る
//   4. addTickEnd で登録した周期末の処理を、登録した順に呼ぶ
//
// ソケットは全部ノンブロッキングなので、tick は必ず有限時間で戻る。待つのはこのクラスの
// 最後の sleep だけで、そこが周期を決めている。

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "udp/CPoller.h"
#include "CChannel.h"
#include "TLoopStats.h"

namespace gw {

class CGateway {
public:
    /// チャネルを登録する。open は openAll でまとめて行う。
    void add(std::unique_ptr<CChannel> channel);

    /// 周期の最後（受信・送信どちらのループも終わったあと）に呼ぶ処理を足す。
    /// いくつでも足せて、足した順に毎周期呼ばれる。
    ///
    /// **チャネルの機能にはしていない。** 周期末にやることがあるのは、独自データのハンドラ
    /// （受信中に積んだコマンドの処理）など一部だけで、大半のチャネル（ICD のクラス）は受けて
    /// 変換して流すだけ。チャネルに持たせると、ほとんどが空の実装になる。
    ///
    /// 独自データのハンドラは、配線の addRaw がチャネルと一緒にここへ登録するので、
    /// ハンドラを足しても登録し忘れることはない。
    void addTickEnd(std::function<void()> fn) { m_tickEnds.push_back(std::move(fn)); }

    /// 全チャネルの受信ポートを bind し、送信先を設定する。
    /// peerHost が空なら受信専用で立ち上がる。
    [[nodiscard]] bool openAll(const std::string& peerHost);

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
    std::vector<std::function<void()>> m_tickEnds;      ///< 周期の最後に呼ぶ処理。足した順
    std::vector<std::size_t> m_pollIndex;   ///< m_channels の添字 -> udp::CPoller の添字
    udp::CPoller m_poller;                  ///< 全チャネルのソケットをまとめて見張る
    TLoopStats m_loop;                      ///< 周期が守れているかの記録
    bool m_opened = false;                  ///< openAll が成功したか。false なら tick は何もしない
};

}  // namespace gw
