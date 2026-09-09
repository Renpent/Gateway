// レコードをデータグラムに詰めて送る側。
//
// クラスに依存しない部分だけをここに置く。T ごとの違い（何バイトか、どう符号化するか）は
// すべて生成コードが知っているので、このテンプレートは1つで足りる。

#ifndef HLAGW_PUBLISHER_H
#define HLAGW_PUBLISHER_H

#include <cstdint>
#include <vector>

#include "../icd/icd_codec.h"
#include "../net/UdpSocket.h"

namespace gw {

template <class T>
class Publisher {
public:
    Publisher(std::uint32_t classId, std::size_t payload = icd::kDefaultPayload)
        : classId_(classId), buf_(payload),
          writer_(buf_.data(), buf_.size(), classId) {}

    /// 1データグラムに何件入るか。レコードが固定長なので、1件も詰める前に分かる。
    [[nodiscard]] std::size_t capacityInRecords() const noexcept {
        return writer_.capacityInRecords();
    }

    /// 1件積む。いっぱいなら先に送ってから積み直す。
    /// 1件がペイロードに収まらない場合だけ false を返す — ICD の上限設定を見直すこと。
    [[nodiscard]] bool publish(const T& record, UdpSocket& sock) {
        if (writer_.add(record)) return true;
        if (!flush(sock)) return false;
        return writer_.add(record);
    }

    /// 溜まっている分を送る。空なら何もしない。
    [[nodiscard]] bool flush(UdpSocket& sock) {
        if (writer_.empty()) return true;

        const std::uint32_t packed = writer_.count();
        const std::size_t len = writer_.finish();
        const bool ok = sock.send(buf_.data(), len);

        // DatagramWriter に reset はない。使い捨てのつもりの型なので、詰め直す。
        writer_ = icd::DatagramWriter<T>(buf_.data(), buf_.size(), classId_);

        if (ok) { ++datagrams_; records_ += packed; }
        return ok;
    }

    [[nodiscard]] std::uint64_t datagramsSent() const noexcept { return datagrams_; }
    [[nodiscard]] std::uint64_t recordsSent() const noexcept { return records_; }

private:
    std::uint32_t classId_;
    std::vector<unsigned char> buf_;
    icd::DatagramWriter<T> writer_;
    std::uint64_t datagrams_ = 0;
    std::uint64_t records_ = 0;
};

}  // namespace gw

#endif  // HLAGW_PUBLISHER_H
