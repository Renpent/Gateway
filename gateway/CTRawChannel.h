// FOM に無い独自データ1種類ぶんの CChannel。**相手が決めた形式のまま受ける。受信専用。**
//
// ヘッダは無く、1データグラム = 1メッセージ。バイト列は T の parse() が読む。
// 形式に合わないもの（parse が false）は捨てる。
//
// T に求めるもの（app/ に手書きする。実例は app/TCommand.h）：
//   static constexpr const char*   kName;   表示名
//   static constexpr std::uint16_t kPort;   受信ポート。ICD のクラスと重ならないこと
//   bool parse(const unsigned char* data, std::size_t len, T& out);
//       T と同じ名前空間に置く（ADL で拾う）。例外は投げない。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "udp/CUdpSocket.h"
#include "CChannel.h"
#include "CTMessageHandler.h"

namespace gw {

template <class T>
class CTRawChannel final : public CChannel {
public:
    /// handler が null なら受けた分を捨てる。借りているだけなので、このチャネルより長く生きていること。
    /// 受信バッファは UDP の最大長で取る（相手の最大長を知らなくても切り詰めない）。
    explicit CTRawChannel(CTMessageHandler<T>* handler)
        : m_handler(handler), m_buf(udp::CUdpSocket::kMaxDatagram) {}

    [[nodiscard]] const char* getName() const noexcept override { return T::kName; }
    [[nodiscard]] std::uint16_t getPort() const noexcept override { return T::kPort; }
    [[nodiscard]] udp::CUdpSocket& getSocket() noexcept override { return m_sock; }

    /// 受信専用なので送信先は持たない。
    [[nodiscard]] bool open(const std::string& /*peerHost*/) override {
        return m_sock.open(T::kPort, "", 0);
    }

    void pumpIn() override {
        for (;;) {
            const long got = m_sock.receive(m_buf.data(), m_buf.size());
            if (got <= 0) return;

            T message{};   // 毎回作り直す。parse が途中で失敗したときに前の中身が残らないように
            if (!parse(m_buf.data(), static_cast<std::size_t>(got), message)) continue;
            if (m_handler != nullptr) m_handler->accept(message);
        }
    }

    void pumpOut() override {}

private:
    CTMessageHandler<T>* m_handler;     ///< アプリ側の受け口。借り物で、null なら捨てる
    udp::CUdpSocket m_sock;             ///< このメッセージ専用のソケット（受信のみ）
    std::vector<unsigned char> m_buf;   ///< 受信バッファ。UDP の最大長
};

}  // namespace gw
