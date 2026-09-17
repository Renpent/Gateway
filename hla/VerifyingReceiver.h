// 受け取った RadarBeam を「送ったはずの値」と1バイトずつ突き合わせる受け口。
//
// 比較を**再符号化したバイト列**で行うのは、可変長配列の未使用部分がゼロ埋めされる約束が
// あるから。同じ値なら必ず同じバイト列になるので、構造体を1メンバずつ比べる必要がない。
//
// **n 件目の受信を n 件目の送信と比べているので、欠落と破損を区別できない。** 途中で1つ
// 落ちると以降が全部ずれて不一致の山になる。ループバックでは末尾しか落ちないので成立するが、
// 実ネットワークで使うならレコードに通し番号を持たせて突き合わせること。

#pragma once

#include <cstddef>
#include <cstdio>
#include <vector>

#include "../icd/RadarBeam.h"
#include "../icd/icd_codec.h"
#include "RadarBeamFixture.h"
#include "ToHla.h"

namespace hla {

class VerifyingReceiver : public ToHla<icdfom::RadarBeam> {
public:
    void accept(const icdfom::RadarBeam& b) override {
        const icdfom::RadarBeam expected = makeRadarBeam(m_received);
        if (encodedBytes(b) != encodedBytes(expected)) {
            if (m_mismatched < 5) {
                std::printf("  !! %zu 件目が往復で一致しません\n", m_received);
            }
            ++m_mismatched;
        }
        if (m_received == 0) dumpFirst(b);
        ++m_received;
    }

    [[nodiscard]] std::size_t received() const noexcept { return m_received; }
    [[nodiscard]] std::size_t mismatched() const noexcept { return m_mismatched; }

private:
    /// レコード1件を符号化した結果。失敗したら空を返すので、比較は必ず不一致になる。
    static std::vector<unsigned char> encodedBytes(const icdfom::RadarBeam& b) {
        std::vector<unsigned char> out(icdfom::RadarBeam::kEncodedSize, 0);
        icd::Writer w(out.data(), out.size());
        icdfom::encode(w, b);
        if (!w.ok()) out.clear();
        return out;
    }

    static void dumpFirst(const icdfom::RadarBeam& b) {
        const std::vector<unsigned char> bytes = encodedBytes(b);
        const std::size_t n = bytes.size() < 48 ? bytes.size() : 48;
        std::printf("先頭レコードのバイト列（先頭 %zu / %zu バイト・ビッグエンディアン）\n",
                    n, bytes.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (i % 16 == 0) std::printf("  %04zx  ", i);
            std::printf("%02x ", bytes[i]);
            if (i % 16 == 15) std::printf("\n");
        }
        if (n % 16 != 0) std::printf("\n");
        std::printf("\n");
    }

    std::size_t m_received = 0;     ///< 受け取った件数。照合する期待値の添字も兼ねる
    std::size_t m_mismatched = 0;   ///< 往復で元とバイト列が違った件数
};

}  // namespace hla
