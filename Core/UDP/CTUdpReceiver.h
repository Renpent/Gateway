// データグラムを受けてレコードに戻す。
//
// classId が違うもの・壊れているものは捨てる。復号は生成コード（icd::DatagramReader）が行う。
// 1データグラムごとに、どうなったかを TRecvReport で知らせる（ログは呼び出し側が書く）。

#pragma once

#include <cstdint>
#include <vector>

#include "../../ICD/icd_codec.h"
#include "CUdpSocket.h"

/// 受けたデータグラム1つの結果。ヘッダの3つは、届いたデータグラムに書かれていた値。
struct TRecvReport {
    long m_bytes = 0;                       ///< データグラムの長さ
    icd::Result m_result = icd::Result::Ok; ///< Ok 以外なら丸ごと捨てた
    std::uint32_t m_classId = 0;            ///< ヘッダの classId（12バイトに満たなければ 0）
    std::uint32_t m_count = 0;              ///< ヘッダの件数
    std::uint32_t m_recordSize = 0;         ///< ヘッダの1件の長さ
    unsigned m_skipped = 0;                 ///< 復号できずに飛ばしたレコードの数
};

template <class T>
class CTUdpReceiver {
public:
    CTUdpReceiver(std::uint32_t classId, std::size_t payload)
        : m_classId(classId), m_buf(payload) {}

    /// 来ているデータグラムを全部読み、1レコードごとに fn(record) を、
    /// 1データグラムごとに report(const TRecvReport&) を呼ぶ。ブロックしない。
    template <class Fn, class Report>
    void drain(CUdpSocket& sock, Fn&& fn, Report&& report) {
        for (;;) {
            const long got = sock.receive(m_buf.data(), m_buf.size());
            if (got <= 0) return;

            TRecvReport rep;
            rep.m_bytes = got;
            if (static_cast<std::size_t>(got) >= icd::kHeaderSize) {
                rep.m_classId = icd::loadU32(m_buf.data());
                rep.m_count = icd::loadU32(m_buf.data() + 4);
                rep.m_recordSize = icd::loadU32(m_buf.data() + 8);
            }

            icd::DatagramReader<T> reader;
            rep.m_result = icd::DatagramReader<T>::open(m_buf.data(), static_cast<std::size_t>(got),
                                                        m_classId, reader);
            if (rep.m_result == icd::Result::Ok) {
                T record{};
                while (reader.hasNext()) {
                    bool skipped = false;
                    if (reader.next(record, skipped) != icd::Result::Ok) break;
                    if (skipped) ++rep.m_skipped;
                    else fn(record);
                }
            }
            report(rep);
        }
    }

private:
    std::uint32_t m_classId;            ///< 期待する classId。違うデータグラムは捨てる
    std::vector<unsigned char> m_buf;   ///< 受信バッファ。長さは payload
};
