// レコードをデータグラムに詰めて送る。
//
// 12バイトヘッダ（classId・件数・1件の長さ）の後ろに固定長レコードを並べる。
// 符号化は生成コード（icd::DatagramWriter）が行う。

#pragma once

#include <cstdint>
#include <vector>

#include "../../ICD/icd_codec.h"
#include "CUdpSocket.h"

namespace udp {

template <class T>
class CTUdpSender {
public:
    CTUdpSender(std::uint32_t classId, std::size_t payload)
        : m_classId(classId), m_buf(payload),
          m_writer(m_buf.data(), m_buf.size(), classId) {}

    /// 1件積む。データグラムが一杯なら先に送ってから積む。
    void publish(const T& record, CUdpSocket& sock) {
        if (m_writer.add(record)) return;
        flush(sock);
        // 1件がペイロードに収まることは生成ヘッダの static_assert が保証している。
        (void)m_writer.add(record);
    }

    /// 積んである分を送る。**送れなかった分は捨てる**（UDP なので再送はしない）。
    void flush(CUdpSocket& sock) {
        if (m_writer.empty()) return;
        (void)sock.send(m_buf.data(), m_writer.finish());
        m_writer = icd::DatagramWriter<T>(m_buf.data(), m_buf.size(), m_classId);
    }

private:
    std::uint32_t m_classId;            ///< データグラム先頭に入れる classId
    std::vector<unsigned char> m_buf;   ///< 送信バッファ。長さは payload
    icd::DatagramWriter<T> m_writer;    ///< m_buf にレコードを積む。flush のたびに作り直す
};

}  // namespace udp
