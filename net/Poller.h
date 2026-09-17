// 複数のソケットをまとめて見張る。
//
// クラスごとに1ポート割り当てる設計なので、周期ごとに全ポートを順に recvfrom で叩くと、
// 何も来ていないポートのぶんだけシステムコールが無駄になる。poll に一度で聞いて、
// **来ているポートだけ**読みに行く。
//
// 待機時間 0 で呼ぶ前提。待つのはこのクラスではなく周期ループの仕事で、
// poll はあくまで「今どれが読めるか」を1回のシステムコールで答えるために使う。
//
// Windows の WSAPoll は POSIX の poll と同じ pollfd 構造・同じ意味で使えるので、
// 分岐は .cpp の冒頭の別名定義だけで済む。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace gw {

class UdpSocket;

class Poller {
public:
    /// 監視対象に加え、その添字を返す。readable() にはこの添字を渡す。
    std::size_t add(const UdpSocket& sock);

    /// 読めるソケットの数を返す。0 は「今は何も来ていない」、-1 はエラー。
    /// timeoutMs = 0 なら待たずに即座に戻る。
    ///
    /// 名前が poll なのは、これが poll / WSAPoll そのものだから。wait という名前にすると
    /// 常用の呼び方 poll(0) が「0 だけ待つ」と読めて、実際にやっていること（待たずに
    /// 今の状態を聞く）と食い違う。待つのはこのクラスではなく周期ループの仕事。
    [[nodiscard]] int poll(int timeoutMs);

    /// 直前の poll() の結果。
    [[nodiscard]] bool readable(std::size_t index) const;

    [[nodiscard]] std::size_t size() const noexcept { return m_handles.size(); }
    [[nodiscard]] const std::string& lastError() const noexcept { return m_error; }

private:
    std::vector<std::intptr_t> m_handles;  ///< 監視対象のハンドル（add した順）
    std::vector<unsigned char> m_ready;    ///< 直前の poll で読めたか（1 = 読める）。vector<bool> のプロキシを避ける
    std::string m_error;                   ///< 直近の失敗理由
};

}  // namespace gw
