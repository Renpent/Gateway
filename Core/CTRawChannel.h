// FOM に無い独自データ1種類ぶんの CChannel。**相手が決めた形式のまま受ける。受信専用。**
//
// ヘッダは無く、1データグラム = 1メッセージ。バイト列は T::parse() が読む。
// 形式に合わないもの（parse が false）は捨てる。
//
// T に求めるもの（Wiring/NonFOM/ に手書きする。実例は Wiring/NonFOM/TCommand.h）：
//   static constexpr const char*   kName;   表示名
//   static constexpr std::uint16_t kPort;   受信ポート。ICD のクラスと重ならないこと
//   static bool parse(const unsigned char* data, std::size_t len, T& out);
//       形式に合わなければ false を返す。例外は投げない。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "UDP/CUdpSocket.h"
#include "CChannel.h"
#include "CTMessageHandler.h"

template <class T>
class CTRawChannel final : public CChannel {
public:
    /// handler が null なら受けた分を捨てる。借りているだけなので、このチャネルより長く生きていること。
    /// 受信バッファは UDP の最大長で取る（相手の最大長を知らなくても切り詰めない）。
    explicit CTRawChannel(CTMessageHandler<T>* handler)
        : m_handler(handler), m_buf(CUdpSocket::kMaxDatagram) {}

    [[nodiscard]] const char* getName() const noexcept override { return T::kName; }
    [[nodiscard]] std::uint16_t getPort() const noexcept override { return T::kPort; }
    [[nodiscard]] CUdpSocket& getSocket() noexcept override { return m_sock; }

    /// 受信専用なので送信先は持たない。
    [[nodiscard]] bool open(const std::string& /*peerHost*/) override {
        return m_sock.open(T::kPort, "", 0);
    }

    void pumpIn() override {
        for (;;) {
            const long got = m_sock.receive(m_buf.data(), m_buf.size());
            if (got <= 0) return;

            T message{};   // 毎回作り直す。parse が途中で失敗したときに前の中身が残らないように
            if (!T::parse(m_buf.data(), static_cast<std::size_t>(got), message)) continue;
            if (m_handler != nullptr) m_handler->accept(message);
        }
    }

    void pumpOut() override {}

private:
    CTMessageHandler<T>* m_handler;     ///< アプリ側の受け口。借り物で、null なら捨てる
    CUdpSocket m_sock;             ///< このメッセージ専用のソケット（受信のみ）
    std::vector<unsigned char> m_buf;   ///< 受信バッファ。UDP の最大長
};

