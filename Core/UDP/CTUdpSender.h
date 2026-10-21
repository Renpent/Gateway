// レコードをデータグラムに詰めて送る。
//
// 12バイトヘッダ（classId・件数・1件の長さ）の後ろに固定長レコードを並べる。
// 符号化は生成コード（icd::DatagramWriter）が行う。
// 1データグラム送るごとに、どうなったかを TSendReport で知らせる（ログは呼び出し側が書く）。

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../../ICD/icd_codec.h"
#include "CUdpSocket.h"

/// 送ったデータグラム1つの結果。
struct TSendReport {
    bool m_sent = false;        ///< false なら送れずに捨てた
    unsigned m_count = 0;       ///< 入っていたレコードの数
    std::size_t m_bytes = 0;    ///< データグラムの長さ
};

template <class T>
class CTUdpSender {
public:
    CTUdpSender(std::uint32_t classId, std::size_t payload)
        : m_classId(classId), m_buf(payload),
          m_writer(m_buf.data(), m_buf.size(), classId) {}

    /// 1件積む。データグラムが一杯なら先に送ってから積む。送ったら report(const TSendReport&) を呼ぶ。
    template <class Report>
    void publish(const T& record, CUdpSocket& sock, Report&& report) {
        if (m_writer.add(record)) return;
        flush(sock, report);
        // 1件がペイロードに収まることは生成ヘッダの static_assert が保証している。
        (void)m_writer.add(record);
    }

    /// 積んである分を送る。**送れなかった分は捨てる**（UDP なので再送はしない）。
    template <class Report>
    void flush(CUdpSocket& sock, Report&& report) {
        if (m_writer.empty()) return;
        TSendReport rep;
        rep.m_count = m_writer.count();
        rep.m_bytes = m_writer.finish();
        rep.m_sent = sock.send(m_buf.data(), rep.m_bytes);
        m_writer = icd::DatagramWriter<T>(m_buf.data(), m_buf.size(), m_classId);
        report(rep);
    }

private:
    std::uint32_t m_classId;            ///< データグラム先頭に入れる classId
    std::vector<unsigned char> m_buf;   ///< 送信バッファ。長さは payload
    icd::DatagramWriter<T> m_writer;    ///< m_buf にレコードを積む。flush のたびに作り直す
};
