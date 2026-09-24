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
static int lastErrno() { return WSAGetLastError(); }
#else
#  include <cerrno>
#  include <poll.h>
using pollfd_t = struct pollfd;
static int pollWait(pollfd_t* fds, unsigned long n, int timeoutMs) {
    return ::poll(fds, static_cast<nfds_t>(n), timeoutMs);
}
static int lastErrno() { return errno; }
#endif

namespace udp {

std::size_t CPoller::add(const CUdpSocket& sock) {
    m_handles.push_back(sock.getOsHandle());
    m_ready.push_back(0);
    return m_handles.size() - 1;
}

int CPoller::poll(int timeoutMs) {
    for (unsigned char& r : m_ready) r = 0;

    // WSAPoll は要素数 0 でエラーを返す。POSIX の poll は単なるタイマになるが、
    // どちらでも「見るものが無い」の答えは 0 で同じなので、手前で返す。
    if (m_handles.empty()) return 0;

    std::vector<pollfd_t> fds(m_handles.size());
    for (std::size_t i = 0; i < m_handles.size(); ++i) {
        fds[i].fd = static_cast<decltype(fds[i].fd)>(m_handles[i]);
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    const int n = pollWait(fds.data(), static_cast<unsigned long>(fds.size()), timeoutMs);
    if (n < 0) {
#ifndef _WIN32
        // シグナルで起こされただけ。周期ループから見れば「何も来ていない」と同じ。
        if (lastErrno() == EINTR) return 0;
#endif
        m_error = "poll に失敗（errno=" + std::to_string(lastErrno()) + "）";
        return -1;
    }
    if (n == 0) return 0;

    int readable = 0;
    for (std::size_t i = 0; i < fds.size(); ++i) {
        // POLLIN 以外に POLLERR / POLLHUP でも読みに行く。recvfrom がその理由を返すので、
        // ここで種類を判定せずに一度読ませたほうが、扱いが1箇所に集まる。
        if (fds[i].revents & (POLLIN | POLLERR | POLLHUP)) {
            m_ready[i] = 1;
            ++readable;
        }
    }
    return readable;
}

bool CPoller::readable(std::size_t index) const {
    return index < m_ready.size() && m_ready[index] != 0;
}

}  // namespace udp
