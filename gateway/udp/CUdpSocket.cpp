// CUdpSocket の実装。Windows と POSIX の差は冒頭の関数に閉じ込めてある。

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
static bool setNonBlocking(socket_t s) {
    u_long on = 1;
    return ::ioctlsocket(s, FIONBIO, &on) == 0;
}

namespace {
/// Winsock の初期化と終了。プロセスで1回。
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
static bool setNonBlocking(socket_t s) {
    const int flags = ::fcntl(s, F_GETFL, 0);
    return flags >= 0 && ::fcntl(s, F_SETFL, flags | O_NONBLOCK) == 0;
}
#endif

namespace udp {
namespace {

socket_t asSocket(std::intptr_t h) { return static_cast<socket_t>(h); }

}  // namespace

CUdpSocket::~CUdpSocket() { close(); }

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
        // 再起動直後に同じポートで受信を上げ直せるように。
        // **このため同じポートの2本目の bind も成功する** — 重複は CGateway::openAll が断る。
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
    if (m_handle < 0 || !canSend()) return false;

    sockaddr_in peer{};
    peer.sin_family = AF_INET;
    std::memcpy(&peer.sin_addr, &m_peerAddr, sizeof m_peerAddr);
    peer.sin_port = m_peerPort;

    return ::sendto(asSocket(m_handle), reinterpret_cast<const char*>(data),
                    static_cast<int>(len), 0,
                    reinterpret_cast<sockaddr*>(&peer), sizeof peer) >= 0;
}

long CUdpSocket::receive(unsigned char* buf, std::size_t cap) {
    if (m_handle < 0) return -1;
    // 失敗はすべて「今は読めない」として扱う。Windows では、直前に送った先が閉じていると
    // ICMP port unreachable がここで WSAECONNRESET として返るが、これも読み飛ばしてよい。
    const auto got = ::recvfrom(asSocket(m_handle), reinterpret_cast<char*>(buf),
                                static_cast<int>(cap), 0, nullptr, nullptr);
    return static_cast<long>(got);
}

}  // namespace udp
