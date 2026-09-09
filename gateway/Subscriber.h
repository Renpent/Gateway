// データグラムを受けてレコードに戻す側。
//
// 受信は「捨てる判断」が仕事の半分なので、何をどう捨てたかを数えて外から見えるようにしてある。
// 黙って落とされるパケットが一番デバッグしにくい。

#ifndef HLAGW_SUBSCRIBER_H
#define HLAGW_SUBSCRIBER_H

#include <cstdint>
#include <vector>

#include "../icd/icd_codec.h"
#include "../net/UdpSocket.h"

namespace gw {

/// 受信側で起きたことの内訳。
struct SubscriberStats {
    std::uint64_t datagrams = 0;      ///< 正しく開けたデータグラム
    std::uint64_t records = 0;        ///< 取り出せたレコード
    std::uint64_t wrongClass = 0;     ///< classId 不一致 — ポートの向き先を疑う
    std::uint64_t malformed = 0;      ///< ヘッダが壊れている / 短すぎる
    std::uint64_t skipped = 0;        ///< 個々のレコードが復号できなかった
    std::uint64_t backlogged = 0;     ///< 1周期で読み切れず次に持ち越した回数
};

template <class T>
class Subscriber {
public:
    /// maxPerDrain: 1周期に読むデータグラムの上限。
    /// **この上限が無いと周期が守れない。** 相手が溢れさせてくると drain が戻らなくなり、
    /// 20 Hz のつもりのループが何秒も1周に費やす。読み切れなかったぶんは OS の受信バッファに
    /// 残るので、次の周期で続きから読める（統計の backlogged がその回数）。
    explicit Subscriber(std::uint32_t classId,
                        std::size_t payload = icd::kDefaultPayload,
                        std::size_t maxPerDrain = 64)
        : classId_(classId), buf_(payload), maxPerDrain_(maxPerDrain) {}

    /// 来ているデータグラムを読み切る。ブロックしない。
    /// 戻り値は取り出せたレコード数、ソケットエラーで -1。
    template <class Fn>
    long drain(UdpSocket& sock, Fn&& fn) {
        long delivered = 0;
        for (std::size_t i = 0; i < maxPerDrain_; ++i) {
            const long got = sock.receive(buf_.data(), buf_.size());
            if (got < 0) return -1;
            if (got == 0) return delivered;      // もう何も来ていない
            delivered += handle(static_cast<std::size_t>(got), fn);
        }
        ++stats_.backlogged;
        return delivered;
    }

    [[nodiscard]] const SubscriberStats& stats() const noexcept { return stats_; }

private:
    template <class Fn>
    long handle(std::size_t len, Fn& fn) {
        icd::DatagramReader<T> reader;
        const icd::Result rc =
            icd::DatagramReader<T>::open(buf_.data(), len, classId_, reader);

        // どちらも待ち直せばよいだけなので、統計に載せて次のデータグラムへ進む。
        if (rc == icd::Result::WrongClass) { ++stats_.wrongClass; return 0; }
        if (rc != icd::Result::Ok) { ++stats_.malformed; return 0; }

        ++stats_.datagrams;

        long delivered = 0;
        T record{};
        while (reader.hasNext()) {
            bool skipped = false;
            if (reader.next(record, skipped) != icd::Result::Ok) break;
            if (skipped) { ++stats_.skipped; continue; }
            fn(record);
            ++delivered;
        }
        stats_.records += static_cast<std::uint64_t>(delivered);
        return delivered;
    }

    std::uint32_t classId_;
    std::vector<unsigned char> buf_;
    std::size_t maxPerDrain_;
    SubscriberStats stats_;
};

}  // namespace gw

#endif  // HLAGW_SUBSCRIBER_H
