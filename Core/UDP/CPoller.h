// 複数のソケットをまとめて見張る。
//
// 周期ごとに全ポートを recvfrom で叩く代わりに、poll に1回で聞いて**来ているポートだけ**読む。
// 待機時間 0 で呼ぶ前提（待つのは周期ループの仕事）。

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace udp {

class CUdpSocket;

class CPoller {
public:
    /// 監視対象に加え、その添字を返す。readable() にはこの添字を渡す。
    std::size_t add(const CUdpSocket& sock);

    /// 読めるソケットがあれば true。timeoutMs = 0 なら待たずに戻る。
    bool poll(int timeoutMs);

    /// 直前の poll() で読めたか。
    [[nodiscard]] bool readable(std::size_t index) const;

private:
    std::vector<std::intptr_t> m_handles;  ///< 監視対象のハンドル（add した順）
    std::vector<unsigned char> m_ready;    ///< 直前の poll で読めたか（1 = 読める）
};

}  // namespace udp
