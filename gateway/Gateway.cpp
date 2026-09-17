#include "Gateway.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

namespace gw {
namespace {

using Clock = std::chrono::steady_clock;

/// "HLAobjectRoot.EmitterBeam.RadarBeam" -> "EmitterBeam.RadarBeam"
const char* trimRoot(const char* fomName) {
    const char* dot = std::strchr(fomName, '.');
    return dot ? dot + 1 : fomName;
}

double toMs(Clock::duration d) {
    return std::chrono::duration<double, std::milli>(d).count();
}

}  // namespace

void Gateway::add(std::unique_ptr<Channel> channel) {
    m_channels.push_back(std::move(channel));
}

bool Gateway::openAll(const std::string& peerHost) {
    m_pollIndex.clear();
    for (auto& ch : m_channels) {
        // 1件も入らないレコードは、開いてから毎周期黙って捨てられる。ここで止める。
        if (ch->capacityInRecords() == 0) {
            std::printf("[%s] レコード %zu B が1件もペイロードに入りません。"
                        "ジャンボフレーム（MTU 9000）にするか、"
                        "ICD の配列上限を下げてください。\n",
                        trimRoot(ch->binding().fomName), ch->recordSize());
            return false;
        }
        if (!ch->open(peerHost)) {
            std::printf("[%s] ポート %u を開けません: %s\n",
                        ch->binding().fomName, ch->binding().port,
                        ch->lastError().c_str());
            return false;
        }
        m_pollIndex.push_back(m_poller.add(ch->socket()));
    }
    m_opened = true;
    return true;
}

void Gateway::tick() {
    if (!m_opened) return;

    // 待機時間 0。全ポートを順に recvfrom で叩くのではなく、poll に一度で聞く。
    // クラスが増えても システムコールは1回のままで、空振りの recvfrom が積み上がらない。
    const int ready = m_poller.poll(0);

    if (ready > 0) {
        for (std::size_t i = 0; i < m_channels.size(); ++i) {
            if (m_poller.readable(m_pollIndex[i])) m_channels[i]->pumpIn();
        }
    }

    // 送信は poll と無関係に毎周期。HLA 側から出てくるものは
    // ソケットの読み取り可否とは関係がない。
    for (auto& ch : m_channels) ch->pumpOut();
}

void Gateway::run(unsigned hz, unsigned seconds) {
    if (hz == 0) hz = 1;

    const auto period = std::chrono::nanoseconds(1000000000LL / hz);
    const auto start = Clock::now();
    const auto deadline = start + std::chrono::seconds(seconds);

    for (auto& ch : m_channels) ch->setLoopRate(hz);

    auto next = start + period;

    while (seconds == 0 || Clock::now() < deadline) {
        const auto tickStart = Clock::now();
        tick();
        const auto tickEnd = Clock::now();

        ++m_loop.ticks;
        const double busy = toMs(tickEnd - tickStart);
        if (busy > m_loop.maxTickMs) m_loop.maxTickMs = busy;

        if (tickEnd > next) {
            // 1周の処理が周期に収まらなかった。取り戻そうとして詰めて回すと悪化するので、
            // 落とした周期は諦めて基準を打ち直す。溜め込まないことを優先する。
            ++m_loop.overruns;
            next = tickEnd + period;
            continue;
        }

        std::this_thread::sleep_until(next);

        const double late = toMs(Clock::now() - next);
        if (late > m_loop.maxLateMs) m_loop.maxLateMs = late;
        m_loop.sumLateMs += late;

        next += period;
    }
}

void Gateway::printPlan(unsigned hz) const {
    std::printf("%-28s %6s %8s %8s %8s %10s\n",
                "クラス", "port", "1件(B)", "1発(件)", "送信Hz", "種別");
    for (const auto& ch : m_channels) {
        const ClassBinding& b = ch->binding();
        std::printf("%-28s %6u %8zu %8zu %8u %10s\n",
                    trimRoot(b.fomName), b.port,
                    ch->recordSize(), ch->capacityInRecords(),
                    (b.rateHz == 0 || b.rateHz >= hz) ? hz : b.rateHz,
                    b.delivery == Delivery::Snapshot ? "状態" : "イベント");
    }
}

void Gateway::printSummary() const {
    std::printf("\n%-28s %10s %10s %10s %8s %8s %8s\n",
                "クラス", "port", "送信件数", "受信件数", "class違い", "異常", "積み残し");
    for (const auto& ch : m_channels) {
        const SubscriberStats& s = ch->inStats();
        std::printf("%-28s %10u %10llu %10llu %8llu %8llu %8zu\n",
                    trimRoot(ch->binding().fomName),
                    ch->binding().port,
                    static_cast<unsigned long long>(ch->sentTotal()),
                    static_cast<unsigned long long>(s.records),
                    static_cast<unsigned long long>(s.wrongClass),
                    static_cast<unsigned long long>(s.malformed + s.skipped),
                    ch->backlog());
    }

    const double avgLate = m_loop.ticks ? m_loop.sumLateMs / static_cast<double>(m_loop.ticks) : 0.0;
    std::printf("\n周期        : %llu 周（超過 %llu 回）\n",
                static_cast<unsigned long long>(m_loop.ticks),
                static_cast<unsigned long long>(m_loop.overruns));
    std::printf("1周の処理   : 最大 %.3f ms\n", m_loop.maxTickMs);
    std::printf("起床の遅れ  : 平均 %.3f ms / 最大 %.3f ms\n", avgLate, m_loop.maxLateMs);
}

}  // namespace gw
