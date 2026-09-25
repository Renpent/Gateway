#include "CGateway.h"

#include <chrono>
#include <cstdio>
#include <thread>

namespace core {

void CGateway::add(std::unique_ptr<CChannel> channel) {
    m_channels.push_back(std::move(channel));
}

bool CGateway::openAll(const std::string& peerHost) {
    // **ポートの重複は bind では捕まらない**（SO_REUSEADDR のため2本目も成功する）。
    // 重複すると片方にしか届かず、どちらに届くかは OS で逆になる（Windows は先、Linux は後）。
    for (std::size_t i = 0; i < m_channels.size(); ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            if (m_channels[i]->getPort() == m_channels[j]->getPort()) {
                std::printf("ポート %u が重複しています: [%s] と [%s]\n",
                            m_channels[i]->getPort(), m_channels[j]->getName(),
                            m_channels[i]->getName());
                return false;
            }
        }
    }

    m_pollIndex.clear();
    for (auto& ch : m_channels) {
        if (!ch->open(peerHost)) {
            std::printf("[%s] ポート %u を開けません: %s\n",
                        ch->getName(), ch->getPort(), ch->getSocket().getLastError().c_str());
            return false;
        }
        m_pollIndex.push_back(m_poller.add(ch->getSocket()));
    }
    m_opened = true;
    return true;
}

void CGateway::tick() {
    if (!m_opened) return;

    if (m_poller.poll(0)) {
        for (std::size_t i = 0; i < m_channels.size(); ++i) {
            if (m_poller.readable(m_pollIndex[i])) m_channels[i]->pumpIn();
        }
    }

    // 送信は poll と関係なく毎周期。
    for (auto& ch : m_channels) ch->pumpOut();

    for (auto& fn : m_tickEnds) fn();
}

void CGateway::run(unsigned hz, unsigned seconds) {
    using Clock = std::chrono::steady_clock;
    if (hz == 0) hz = 1;

    const auto period = std::chrono::nanoseconds(1000000000LL / hz);
    const auto deadline = Clock::now() + std::chrono::seconds(seconds);
    auto next = Clock::now() + period;

    while (seconds == 0 || Clock::now() < deadline) {
        tick();

        // 絶対時刻で刻むので、起床が多少遅れても平均の周期はずれない。
        // 1周が周期を超えたときは取り戻そうとせず、基準を打ち直す（詰めて回すと悪化する）。
        const auto now = Clock::now();
        if (now > next) {
            next = now + period;
            continue;
        }
        std::this_thread::sleep_until(next);
        next += period;
    }
}

}  // namespace core
