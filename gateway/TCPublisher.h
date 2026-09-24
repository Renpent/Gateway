// レコードをデータグラムに詰めて送る側。
//
// クラスに依存しない部分だけをここに置く。T ごとの違い（何バイトか、どう符号化するか）は
// すべて生成コードが知っているので、このテンプレートは1つで足りる。

#pragma once

#include <cstdint>
#include <vector>

#include "../icd/icd_codec.h"
#include "../net/CUdpSocket.h"

namespace gw {

template <class T>
class TCPublisher {
public:
    TCPublisher(std::uint32_t classId, std::size_t payload = icd::kDefaultPayload)
        : m_classId(classId), m_buf(payload),
          m_writer(m_buf.data(), m_buf.size(), classId) {}

    /// 1データグラムに何件入るか。レコードが固定長なので、1件も詰める前に分かる。
    [[nodiscard]] std::size_t getCapacityInRecords() const noexcept {
        return m_writer.capacityInRecords();
    }

    /// 1件積む。いっぱいなら先に送ってから積み直す。
    /// 1件がペイロードに収まらない場合だけ false を返す — ICD の上限設定を見直すこと。
    [[nodiscard]] bool publish(const T& record, CUdpSocket& sock) {
        if (m_writer.add(record)) return true;
        if (!flush(sock)) return false;
        return m_writer.add(record);
    }

    /// 溜まっている分を送る。空なら何もしない。
    [[nodiscard]] bool flush(CUdpSocket& sock) {
        if (m_writer.empty()) return true;

        const std::uint32_t packed = m_writer.count();
        const std::size_t len = m_writer.finish();
        const bool ok = sock.send(m_buf.data(), len);

        // DatagramWriter に reset はない。使い捨てのつもりの型なので、詰め直す。
        m_writer = icd::DatagramWriter<T>(m_buf.data(), m_buf.size(), m_classId);

        if (ok) { ++m_datagrams; m_records += packed; }
        return ok;
    }

    [[nodiscard]] std::uint64_t getDatagramsSent() const noexcept { return m_datagrams; }
    [[nodiscard]] std::uint64_t getRecordsSent() const noexcept { return m_records; }

private:
    std::uint32_t m_classId;            ///< データグラム先頭に入れる classId
    std::vector<unsigned char> m_buf;   ///< 送信バッファ。長さは payload
    icd::DatagramWriter<T> m_writer;    ///< m_buf に直接レコードを積む。使い捨てなので flush で作り直す
    std::uint64_t m_datagrams = 0;      ///< 送ったデータグラム数
    std::uint64_t m_records = 0;        ///< 送ったレコード数
};

}  // namespace gw
