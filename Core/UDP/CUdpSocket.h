// UDP ソケット1本。受信ポートと送信先を1本で持つ。
//
// **常にノンブロッキング。** 1本が待ちに入ると周期ループ全体が止まるため。
// 読めるかどうかの判定は CPoller が行う。
//
// OS の型をヘッダに出さないよう、ハンドルは std::intptr_t で持つ
// （Windows の SOCKET と POSIX の int fd を同じ型で受けるため）。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace udp {

class CUdpSocket {
public:
    CUdpSocket() = default;
    ~CUdpSocket();

    CUdpSocket(const CUdpSocket&) = delete;
    CUdpSocket& operator=(const CUdpSocket&) = delete;

    /// UDP で1データグラムに載る最大のバイト数（IPv4）。
    static constexpr std::size_t kMaxDatagram = 65507;

    /// bindPort で受信し、peerHost:peerPort へ送る。
    ///   bindPort  0 なら bind しない
    ///   peerHost  空なら送信先を持たない（受信専用）
    /// 失敗したら false。理由は getLastError()。
    [[nodiscard]] bool open(std::uint16_t bindPort,
                            const std::string& peerHost,
                            std::uint16_t peerPort);

    void close() noexcept;
    [[nodiscard]] bool canSend() const noexcept { return m_peerPort != 0; }

    /// CPoller に渡すためのハンドル値。
    [[nodiscard]] std::intptr_t getOsHandle() const noexcept { return m_handle; }

    /// データグラムを1つ送る。送れなければ false（待たない）。
    bool send(const unsigned char* data, std::size_t len);

    /// データグラムを1つ受ける。受信バイト数を返し、何も無いかエラーなら 0 以下。
    /// **cap は相手の最大長以上にすること。** 長いデータグラムは切り詰められる。
    /// 0バイトのデータグラムは「何も無い」と区別できない（送られてこない前提）。
    [[nodiscard]] long receive(unsigned char* buf, std::size_t cap);

    /// open が失敗した理由。
    [[nodiscard]] const std::string& getLastError() const noexcept { return m_error; }

private:
    bool fail(const char* what);

    std::intptr_t m_handle = -1;    ///< ソケットのハンドル値（-1 = 未オープン）
    std::string m_error;            ///< open が失敗した理由
    std::uint32_t m_peerAddr = 0;   ///< 送信先アドレス（ネットワークバイトオーダー）
    std::uint16_t m_peerPort = 0;   ///< 送信先ポート（0 = 送信先なし。ネットワークバイトオーダー）
};

}  // namespace udp
