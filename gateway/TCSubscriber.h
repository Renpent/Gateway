// データグラムを受けてレコードに戻す側。
//
// クラスに依存しない部分だけをここに置く。T ごとの違い（何バイトか、どう復号するか）は
// すべて生成コードが知っているので、このテンプレートは1つで足りる。
// 数えた結果は TSubscriberStats。

#pragma once

#include <cstdint>
#include <vector>

#include "../icd/icd_codec.h"
#include "../net/CUdpSocket.h"
#include "TSubscriberStats.h"

namespace gw {

template <class T>
class TCSubscriber {
public:
    /// **1周期に届いていたものは、その周期で読み切る。** 件数の上限は設けない。
    ///
    /// 無限には回らない。ソケットの受信バッファは有限なので、空になれば receive が 0 を返して
    /// 抜ける。相手がこちらの排出より速く送り続けるなら、溢れはカーネル側で起きていて、
    /// ここで打ち切っても救えない。
    ///
    /// 持ち越さないのはログのため。次の周期に回すと「このレコードはどの周期に届いたのか」が
    /// 突き合わせで曖昧になり、周期がずれているように見える。
    explicit TCSubscriber(std::uint32_t classId, std::size_t payload = icd::kDefaultPayload)
        : m_classId(classId), m_buf(payload) {}

    /// 来ているデータグラムを読み切る。ブロックしない。
    /// 戻り値は取り出せたレコード数、ソケットエラーで -1。
    template <class Fn>
    long drain(CUdpSocket& sock, Fn&& fn) {
        long delivered = 0;
        for (;;) {
            const long got = sock.receive(m_buf.data(), m_buf.size());
            if (got < 0) return -1;
            if (got == 0) return delivered;      // もう何も来ていない
            delivered += handle(static_cast<std::size_t>(got), fn);
        }
    }

    [[nodiscard]] const TSubscriberStats& stats() const noexcept { return m_stats; }

private:
    template <class Fn>
    long handle(std::size_t len, Fn& fn) {
        icd::DatagramReader<T> reader;
        const icd::Result rc =
            icd::DatagramReader<T>::open(m_buf.data(), len, m_classId, reader);

        // どちらも待ち直せばよいだけなので、統計に載せて次のデータグラムへ進む。
        if (rc == icd::Result::WrongClass) { ++m_stats.wrongClass; return 0; }
        if (rc != icd::Result::Ok) { ++m_stats.malformed; return 0; }

        ++m_stats.datagrams;

        long delivered = 0;
        T record{};
        while (reader.hasNext()) {
            bool skipped = false;
            if (reader.next(record, skipped) != icd::Result::Ok) break;
            if (skipped) { ++m_stats.skipped; continue; }
            fn(record);
            ++delivered;
        }
        m_stats.records += static_cast<std::uint64_t>(delivered);
        return delivered;
    }

    std::uint32_t m_classId;            ///< 期待する classId。違えば wrongClass として捨てる
    std::vector<unsigned char> m_buf;   ///< 受信バッファ。長さは payload
    TSubscriberStats m_stats;            ///< 受けた / 捨てた件数の内訳
};

}  // namespace gw
