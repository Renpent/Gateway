// データグラムを受けてレコードに戻す。
//
// classId が違うもの・壊れているものは捨てる。復号は生成コード（icd::DatagramReader）が行う。

#pragma once

#include <cstdint>
#include <vector>

#include "../../ICD/icd_codec.h"
#include "CUdpSocket.h"

namespace udp {

template <class T>
class CTUdpReceiver {
public:
    CTUdpReceiver(std::uint32_t classId, std::size_t payload)
        : m_classId(classId), m_buf(payload) {}

    /// 来ているデータグラムを全部読み、1レコードごとに fn(record) を呼ぶ。ブロックしない。
    template <class Fn>
    void drain(CUdpSocket& sock, Fn&& fn) {
        for (;;) {
            const long got = sock.receive(m_buf.data(), m_buf.size());
            if (got <= 0) return;

            icd::DatagramReader<T> reader;
            if (icd::DatagramReader<T>::open(m_buf.data(), static_cast<std::size_t>(got),
                                             m_classId, reader) != icd::Result::Ok) {
                continue;   // classId 違い・壊れたヘッダ
            }

            T record{};
            while (reader.hasNext()) {
                bool skipped = false;
                if (reader.next(record, skipped) != icd::Result::Ok) break;
                if (!skipped) fn(record);
            }
        }
    }

private:
    std::uint32_t m_classId;            ///< 期待する classId。違うデータグラムは捨てる
    std::vector<unsigned char> m_buf;   ///< 受信バッファ。長さは payload
};

}  // namespace udp
