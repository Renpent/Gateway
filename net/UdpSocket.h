// UDP ソケット。
//
// このプロジェクトで OS を知っているのは net/ と platform/ の .cpp だけで、ヘッダには
// winsock も <sys/socket.h> も現れない。ハンドルを std::intptr_t で持っているのはそのためで、
// Windows の SOCKET（符号なし整数ハンドル）と POSIX の int fd を1つの型で受けられる。
//
// **常にノンブロッキング。** 待つかどうかを決めるのは Poller の仕事で、ソケット自身は
// 「今あるものを渡す / 今出せるものを出す」しかしない。周期実行のループから呼ばれるので、
// 1本のソケットが待ちに入ると全クラスが止まってしまう。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace gw {

class UdpSocket {
public:
    UdpSocket() = default;
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&& other) noexcept;
    UdpSocket& operator=(UdpSocket&& other) noexcept;

    /// 受信ポートと送信先をまとめて設定する。UDP なので1本で両方できる。
    ///   bindPort  0 なら bind しない（送信専用）
    ///   peerHost  空なら送信先を持たない（受信専用）
    [[nodiscard]] bool open(std::uint16_t bindPort,
                            const std::string& peerHost,
                            std::uint16_t peerPort);

    void close() noexcept;
    [[nodiscard]] bool isOpen() const noexcept { return m_handle >= 0; }
    [[nodiscard]] bool canSend() const noexcept { return m_peerPort != 0; }

    /// OS が持っているハンドルの**値**。ただし型は移植用の器で、OS の型そのもの
    /// （Windows の SOCKET、POSIX の int fd）ではない。本来の型に戻すのは .cpp の asSocket()。
    /// 使うのは net/ の中だけ — Poller が poll に渡すため。
    [[nodiscard]] std::intptr_t osHandle() const noexcept { return m_handle; }

    /// データグラムを1つ送る。UDP に部分送信はない — 全部行くか失敗するか。
    [[nodiscard]] bool send(const unsigned char* data, std::size_t len);

    /// UDP で1データグラムに載る最大のバイト数（IPv4）。受信バッファをこの長さにしておけば、
    /// 相手が何を送ってきても切り詰めは起きない。
    static constexpr std::size_t kMaxDatagram = 65507;

    /// データグラムを1つ受ける。ブロックしない。
    ///   >0  受信バイト数
    ///    0  今は何も来ていない
    ///   -1  エラー（lastError() を見ること）
    /// cap は必ず最大ペイロード以上にすること。長いデータグラムは切り詰められ、
    /// UDP では残りを後から取れない。形式が分からない相手なら kMaxDatagram にする。
    ///
    /// **0バイトのデータグラムは「何も来ていない」と区別できない。** 送ってくる相手はいない
    /// 前提で、ICD のクラスも独自データも空のメッセージを定義していない。もし届いたら、
    /// 数えられずに読み捨てられ、そのポートに続いて届いていたぶんは次の周期に回る
    /// （Windows / Linux の両方で実測）。区別が要るようになったら、戻り値を
    /// 「受け取った・何も無い・失敗」の状態にして、長さを別に返す形にすればよい。
    [[nodiscard]] long receive(unsigned char* buf, std::size_t cap);

    /// 直近の失敗の理由。
    /// **send と receive が同じ1本を書く。** ソケット自体は全二重で、送信と受信を別スレッドに
    /// 割っても OS 側は安全だが、このメンバ（と統計カウンタ）だけはそのとき競合する。
    /// 今は単一スレッドなので問題にならない。
    [[nodiscard]] const std::string& lastError() const noexcept { return m_error; }

private:
    bool fail(const char* what);

    std::intptr_t m_handle = -1;    ///< ソケットのハンドル値（-1 = 未オープン）
    std::string m_error;            ///< 直近の失敗理由

    // 送信先。sockaddr_in をヘッダに出さないために生の形で持つ。
    std::uint32_t m_peerAddr = 0;   ///< 送信先アドレス（ネットワークバイトオーダー）
    std::uint16_t m_peerPort = 0;   ///< 送信先ポート（0 = 送信先なし。ネットワークバイトオーダー）
};

}  // namespace gw
