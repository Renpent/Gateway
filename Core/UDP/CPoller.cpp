// CPoller の実装。Windows の WSAPoll は POSIX の poll と同じ形で使えるので、差は冒頭の別名だけ。

#include "CPoller.h"

#include "CUdpSocket.h"

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
using pollfd_t = WSAPOLLFD;
static int pollWait(pollfd_t* fds, unsigned long n, int timeoutMs) {
    return ::WSAPoll(fds, static_cast<ULONG>(n), timeoutMs);
}
#else
#  include <poll.h>
using pollfd_t = struct pollfd;
static int pollWait(pollfd_t* fds, unsigned long n, int timeoutMs) {
    return ::poll(fds, static_cast<nfds_t>(n), timeoutMs);
}
#endif

namespace udp {

std::size_t CPoller::add(const CUdpSocket& sock) {
    m_handles.push_back(sock.getOsHandle());
    m_ready.push_back(0);
    return m_handles.size() - 1;
}

bool CPoller::poll(int timeoutMs) {
    for (unsigned char& r : m_ready) r = 0;

    // WSAPoll は要素数 0 だとエラーになるので、手前で返す。
    if (m_handles.empty()) return false;

    std::vector<pollfd_t> fds(m_handles.size());
    for (std::size_t i = 0; i < m_handles.size(); ++i) {
        fds[i].fd = static_cast<decltype(fds[i].fd)>(m_handles[i]);
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    if (pollWait(fds.data(), static_cast<unsigned long>(fds.size()), timeoutMs) <= 0) return false;

    bool any = false;
    for (std::size_t i = 0; i < fds.size(); ++i) {
        // POLLERR / POLLHUP でも読みに行く。recvfrom が失敗を返し、読み飛ばされる。
        if (fds[i].revents & (POLLIN | POLLERR | POLLHUP)) {
            m_ready[i] = 1;
            any = true;
        }
    }
    return any;
}

bool CPoller::readable(std::size_t index) const {
    return index < m_ready.size() && m_ready[index] != 0;
}

}  // namespace udp
