// FOM に無い独自データ1種類ぶんの Channel。**相手が決めた形式のまま受ける。**
//
// ClassChannel との違いは、バイト列の形式を誰が決めたか：
//
//   ClassChannel  12バイトヘッダ + 固定長レコード。ICD としてこちらで決めた形式。相手は HLA
//   RawChannel    ヘッダ無し。1データグラム = 1メッセージで、中身は相手の仕様どおり。相手はアプリ
//
// ヘッダが無いので classId による取り違えの検出も無い。**このポートにこの形式が来る、
// という取り決めだけが頼り**で、違うものが来たときに分かるのは parse が失敗したときだけ。
// そのため失敗は必ず数えて、統計の「異常」列に出す。
//
// 形式を知っているのは T と、T と同じ名前空間に置く parse() だけ。このクラスはバイト列を
// 受け取って parse に渡し、成功したら ToApp へ、失敗したら数えて捨てる。
//
// **受信専用。** 送る必要が出たら FromApp と、parse の逆を足す。pumpOut は今は何もしない。
//
// T に求めるもの（手書き。ICDgenerator は関わらない）：
//
//   static constexpr const char*   kName;   表示名
//   static constexpr std::uint16_t kPort;   受信ポート
//   bool parse(const unsigned char* data, std::size_t len, T& out);
//       T と同じ名前空間に置く（ADL で拾う）。len は常に 1 以上 — 空のデータグラムは
//       来ない前提で、来ても receive() が「何も来ていない」として読み捨てる（net/UdpSocket.h）。
//       形式に合わなければ false を返すこと。**例外は投げない。**
//
// 番号を配線に書かないのは ClassChannel と同じ理由で、T の定数から決まる。
// **ポートは ICD のクラスと同じ番号空間。** 手書きの番号は ICDgenerator のダイアログの
// 重複検出を通らないので、Gateway::openAll が開く前に全チャネルを突き合わせて断る。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../app/ToApp.h"
#include "../net/UdpSocket.h"
#include "Channel.h"
#include "SubscriberStats.h"

namespace gw {

template <class T>
class RawChannel final : public Channel {
public:
    /// toApp が null なら受けたものを数えて捨てる。借りているだけで、寿命はこのチャネルより
    /// 長くなければならない（配線側が値で持つ。ClassChannel と同じ）。
    ///
    /// 受信バッファは UDP の最大長で取る。**相手の形式の最大長を知らなくても切り詰めが起きない**
    /// ようにするため。ポート1つにつき 64 KB。
    explicit RawChannel(app::ToApp<T>* toApp)
        : m_toApp(toApp), m_buf(UdpSocket::kMaxDatagram) {}

    [[nodiscard]] const char* name() const noexcept override { return T::kName; }
    [[nodiscard]] std::uint16_t port() const noexcept override { return T::kPort; }
    [[nodiscard]] std::size_t payload() const noexcept override { return m_buf.size(); }
    [[nodiscard]] const char* kindLabel() const noexcept override { return "独自（受信のみ）"; }
    [[nodiscard]] UdpSocket& socket() noexcept override { return m_sock; }

    /// 受信専用なので送信先は持たない。loopback でも run でも、bind するだけ。
    [[nodiscard]] bool open(const std::string& /*peerHost*/) override {
        return m_sock.open(T::kPort, "", 0);
    }

    std::size_t pumpOut() override { return 0; }

    /// 来ているデータグラムを読み切る。1つ = 1メッセージ。
    std::size_t pumpIn() override {
        std::size_t delivered = 0;
        for (;;) {
            const long got = m_sock.receive(m_buf.data(), m_buf.size());
            if (got <= 0) break;   // 何も無い / 失敗

            // 毎回作り直す。前のメッセージの中身が parse の失敗時に残らないように。
            T message{};
            // parse は T の名前空間から ADL で拾う。
            if (!parse(m_buf.data(), static_cast<std::size_t>(got), message)) {
                ++m_stats.malformed;
                continue;
            }
            ++m_stats.datagrams;
            ++m_stats.records;
            if (m_toApp != nullptr) m_toApp->accept(message);
            ++delivered;
        }
        return delivered;
    }

    /// 可変長。1データグラムに1メッセージ（Channel の取り決めで 0 が「可変」）。
    [[nodiscard]] std::size_t recordSize() const noexcept override { return 0; }
    [[nodiscard]] std::size_t capacityInRecords() const noexcept override { return 1; }

    [[nodiscard]] std::uint64_t sentTotal() const noexcept override { return 0; }
    [[nodiscard]] std::size_t backlog() const noexcept override { return 0; }
    [[nodiscard]] std::uint64_t deferrals() const noexcept override { return 0; }
    [[nodiscard]] const SubscriberStats& inStats() const noexcept override { return m_stats; }
    [[nodiscard]] const std::string& lastError() const noexcept override {
        return m_sock.lastError();
    }

private:
    app::ToApp<T>* m_toApp;             ///< アプリ側の受け口。借り物で、null なら数えて捨てる
    UdpSocket m_sock;                   ///< このメッセージ専用のソケット（受信のみ）
    std::vector<unsigned char> m_buf;   ///< 受信バッファ。UDP の最大長なので切り詰めは起きない
    SubscriberStats m_stats;            ///< datagrams/records = 受理、malformed = parse 失敗
};

}  // namespace gw
