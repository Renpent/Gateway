// CUdpSocket の実装。
//
// Windows と POSIX の差は4つしかない。初期化（WSAStartup）、閉じ方、ノンブロッキングの
// 設定の仕方、そして「今は何も無い」を表すエラー値。冒頭の薄い層に閉じ込めて、
// 以降の本体は1つのコードで書いてある。

#include "CUdpSocket.h"

#include <cstring>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
using socket_t = SOCKET;
static const socket_t kInvalidSocket = INVALID_SOCKET;
static int lastErrno() { return WSAGetLastError(); }
static void closeSocket(socket_t s) { ::closesocket(s); }
static bool wouldBlock(int e) { return e == WSAEWOULDBLOCK; }
static bool setNonBlocking(socket_t s) {
    u_long on = 1;
    return ::ioctlsocket(s, FIONBIO, &on) == 0;
}

namespace {
/// Winsock はプロセスごとに一度だけ起こせばよい。静的オブジェクトの寿命に任せる。
struct WinsockScope {
    WinsockScope() { WSADATA d; WSAStartup(MAKEWORD(2, 2), &d); }
    ~WinsockScope() { WSACleanup(); }
};
WinsockScope g_winsock;
}  // namespace

#else
#  include <arpa/inet.h>
#  include <cerrno>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <unistd.h>
using socket_t = int;
static const socket_t kInvalidSocket = -1;
static int lastErrno() { return errno; }
static void closeSocket(socket_t s) { ::close(s); }
static bool wouldBlock(int e) { return e == EAGAIN || e == EWOULDBLOCK; }
static bool setNonBlocking(socket_t s) {
    const int flags = ::fcntl(s, F_GETFL, 0);
    return flags >= 0 && ::fcntl(s, F_SETFL, flags | O_NONBLOCK) == 0;
}
#endif

namespace udp {
namespace {

/// 器から OS の型へ戻す。getOsHandle() が渡してくるのは値だけなので、型はここで付け直す。
socket_t asSocket(std::intptr_t h) { return static_cast<socket_t>(h); }

}  // namespace

CUdpSocket::~CUdpSocket() { close(); }

CUdpSocket::CUdpSocket(CUdpSocket&& other) noexcept
    : m_handle(other.m_handle), m_error(std::move(other.m_error)),
      m_peerAddr(other.m_peerAddr), m_peerPort(other.m_peerPort) {
    other.m_handle = -1;
}

CUdpSocket& CUdpSocket::operator=(CUdpSocket&& other) noexcept {
    if (this != &other) {
        close();
        m_handle = other.m_handle;
        m_error = std::move(other.m_error);
        m_peerAddr = other.m_peerAddr;
        m_peerPort = other.m_peerPort;
        other.m_handle = -1;
    }
    return *this;
}

bool CUdpSocket::fail(const char* what) {
    m_error = std::string(what) + " に失敗（errno=" + std::to_string(lastErrno()) + "）";
    return false;
}

void CUdpSocket::close() noexcept {
    if (m_handle >= 0) {
        closeSocket(asSocket(m_handle));
        m_handle = -1;
    }
    m_peerPort = 0;
}

bool CUdpSocket::open(std::uint16_t bindPort, const std::string& peerHost,
                     std::uint16_t peerPort) {
    close();

    in_addr peer{};
    if (!peerHost.empty() && ::inet_pton(AF_INET, peerHost.c_str(), &peer) != 1) {
        m_error = "送信先アドレスを解釈できません: " + peerHost;
        return false;
    }

    const socket_t s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == kInvalidSocket) return fail("socket");
    m_handle = static_cast<std::intptr_t>(s);

    if (bindPort != 0) {
        // 再起動直後に「アドレス使用中」で弾かれないように。UDP には待機状態が残らないので
        // 実害はないが、同じポートで受信を上げ直す運用では効く。
        int on = 1;
        (void)::setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                           reinterpret_cast<const char*>(&on), sizeof on);

        sockaddr_in local{};
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = INADDR_ANY;
        local.sin_port = ::htons(bindPort);

        if (::bind(s, reinterpret_cast<sockaddr*>(&local), sizeof local) != 0) {
            const bool r = fail("bind");
            close();
            return r;
        }
    }

    if (!setNonBlocking(s)) {
        const bool r = fail("ノンブロッキング設定");
        close();
        return r;
    }

    if (!peerHost.empty()) {
        std::memcpy(&m_peerAddr, &peer, sizeof m_peerAddr);
        m_peerPort = ::htons(peerPort);
    }
    return true;
}

bool CUdpSocket::send(const unsigned char* data, std::size_t len) {
    if (!isOpen()) { m_error = "ソケットが開いていません"; return false; }
    if (!canSend()) { m_error = "送信先が設定されていません"; return false; }

    sockaddr_in peer{};
    peer.sin_family = AF_INET;
    std::memcpy(&peer.sin_addr, &m_peerAddr, sizeof m_peerAddr);
    peer.sin_port = m_peerPort;

    const auto sent = ::sendto(asSocket(m_handle),
                               reinterpret_cast<const char*>(data),
                               static_cast<int>(len), 0,
                               reinterpret_cast<sockaddr*>(&peer), sizeof peer);
    if (sent < 0) {
        // 送信バッファが一杯。ノンブロッキングなので待たずに諦め、次の周期に回す。
        if (wouldBlock(lastErrno())) { m_error = "送信バッファが一杯です"; return false; }
        return fail("sendto");
    }
    if (static_cast<std::size_t>(sent) != len) {
        m_error = "データグラムが分割されました（ありえない）";
        return false;
    }
    return true;
}

long CUdpSocket::receive(unsigned char* buf, std::size_t cap) {
    if (!isOpen()) { m_error = "ソケットが開いていません"; return -1; }

    const auto got = ::recvfrom(asSocket(m_handle), reinterpret_cast<char*>(buf),
                                static_cast<int>(cap), 0, nullptr, nullptr);
    if (got < 0) {
        if (wouldBlock(lastErrno())) return 0;
#ifdef _WIN32
        // 直前に送った先が閉じていると ICMP port unreachable が返り、Windows では
        // それが「受信側」のエラーとして上がってくる。UDP は非接続なので無視してよい。
        if (lastErrno() == WSAECONNRESET) return 0;
#endif
        (void)fail("recvfrom");
        return -1;
    }
    return static_cast<long>(got);
}

}  // namespace udp
