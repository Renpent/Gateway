#include "Gateway.h"

#include <chrono>
#include <cstdio>
#include <thread>

namespace gw {
namespace {

using Clock = std::chrono::steady_clock;

double toMs(Clock::duration d) {
    return std::chrono::duration<double, std::milli>(d).count();
}

}  // namespace

void Gateway::add(std::unique_ptr<Channel> channel) {
    m_channels.push_back(std::move(channel));
}

bool Gateway::openAll(const std::string& peerHost) {
    // **ポートの重複は bind では捕まらない。** UdpSocket は SO_REUSEADDR を立てているので
    // 2本目の bind も成功し、データグラムはどちらか一方にしか届かない。しかも**どちらに
    // 届くかが OS で逆**で、Windows は先に bind したほう、Linux は後のほうが受け取る
    // （両方で実測）。片方が黙って飢えるうえに、飢えるほうが環境で変わるので、開く前に止める。
    //
    // ICD のクラスは ICDgenerator のダイアログが重複を検出するが、手で番号を書くチャネル
    // （FOM に無い独自データ）はその網にかからない。ここが全チャネル共通の最後の網。
    for (std::size_t i = 0; i < m_channels.size(); ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            if (m_channels[i]->port() == m_channels[j]->port()) {
                std::printf("ポート %u が重複しています: [%s] と [%s]。"
                            "重複すると片方にしか届かず、どちらに届くかは OS で変わります。\n",
                            m_channels[i]->port(), m_channels[j]->name(), m_channels[i]->name());
                return false;
            }
        }
    }

    m_pollIndex.clear();
    for (auto& ch : m_channels) {
        // 1件も入らないレコードは、開いてから毎周期黙って捨てられる。ここで止める。
        // 可変長のチャネル（recordSize() == 0）は件数が常に 1 なので、ここには掛からない。
        if (ch->capacityInRecords() == 0) {
            std::printf("[%s] レコード %zu B が1件もペイロードに入りません。"
                        "ジャンボフレーム（MTU 9000）にするか、"
                        "ICD の配列上限を下げてください。\n",
                        ch->name(), ch->recordSize());
            return false;
        }
        if (!ch->open(peerHost)) {
            std::printf("[%s] ポート %u を開けません: %s\n",
                        ch->name(), ch->port(), ch->lastError().c_str());
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

    // 送信は poll と無関係に毎周期。手元（HLA やアプリ）から出てくるものは
    // ソケットの読み取り可否とは関係がない。受信専用のチャネルは何もしない。
    for (auto& ch : m_channels) ch->pumpOut();
}

void Gateway::run(unsigned hz, unsigned seconds) {
    if (hz == 0) hz = 1;

    const auto period = std::chrono::nanoseconds(1000000000LL / hz);
    const auto start = Clock::now();
    const auto deadline = start + std::chrono::seconds(seconds);

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

void Gateway::printPlan() const {
    // 最終列だけ幅を指定しない。printf の幅はバイト数で数えるので、CJK を混ぜると揃わない。
    std::printf("%-28s %6s %8s %8s %8s  %s\n",
                "クラス", "port", "1件(B)", "1発(件)", "上限(B)", "種別");
    for (const auto& ch : m_channels) {
        // 可変長（相手が決めた形式）は1件の大きさが決まっていないので、数字の代わりに書く。
        char size[16];
        if (ch->recordSize() == 0) std::snprintf(size, sizeof size, "%s", "可変");
        else                       std::snprintf(size, sizeof size, "%zu", ch->recordSize());

        std::printf("%-28s %6u %8s %8zu %8zu  %s\n",
                    ch->name(), ch->port(), size,
                    ch->capacityInRecords(), ch->payload(), ch->kindLabel());
    }
}

void Gateway::printSummary() const {
    // 持ち越し回数を出しているのは、**ClassKind によって意味が正反対**だから。インタラクション
    // ならその回数だけ次の周期へ繰り越しており、オブジェクトなら同じ回数だけ捨てている。
    // 積み残しは終わった瞬間の残り件数、持ち越しは出し切れなかった周期の数。
    std::printf("\n%-28s %10s %10s %10s %8s %8s %8s %8s\n",
                "クラス", "port", "送信件数", "受信件数", "class違い", "異常",
                "積み残し", "持ち越し");
    for (const auto& ch : m_channels) {
        const SubscriberStats& s = ch->inStats();
        std::printf("%-28s %10u %10llu %10llu %8llu %8llu %8zu %8llu\n",
                    ch->name(),
                    ch->port(),
                    static_cast<unsigned long long>(ch->sentTotal()),
                    static_cast<unsigned long long>(s.records),
                    static_cast<unsigned long long>(s.wrongClass),
                    static_cast<unsigned long long>(s.malformed + s.skipped),
                    ch->backlog(),
                    static_cast<unsigned long long>(ch->deferrals()));
    }

    const double avgLate = m_loop.ticks ? m_loop.sumLateMs / static_cast<double>(m_loop.ticks) : 0.0;
    std::printf("\n周期        : %llu 周（超過 %llu 回）\n",
                static_cast<unsigned long long>(m_loop.ticks),
                static_cast<unsigned long long>(m_loop.overruns));
    std::printf("1周の処理   : 最大 %.3f ms\n", m_loop.maxTickMs);
    std::printf("起床の遅れ  : 平均 %.3f ms / 最大 %.3f ms\n", avgLate, m_loop.maxLateMs);
}

}  // namespace gw
