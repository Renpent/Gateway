// **本番には持っていかないファイル。** stub/ は RTI が無いこの環境でゲートウェイを動かし、
// 往復を検証するための代用品だけが入っている。実 RTI に繋ぐときは stub/ ごと消せて、
// 直す先は app/CWiring.h の1ファイルで済む。
//
// 受け取ったレコードを「送ったはずの値」と1バイトずつ突き合わせる受け口。
//
// 比較を**再符号化したバイト列**で行うのは、可変長配列の未使用部分がゼロ埋めされる約束が
// あるから。同じ値なら必ず同じバイト列になるので、構造体を1メンバずつ比べる必要がない。
//
// **n 件目の受信を n 件目の送信と比べているので、欠落と破損を区別できない。** 途中で1つ
// 落ちると以降が全部ずれて不一致の山になる。ループバックでは末尾しか落ちないので成立するが、
// 実ネットワークで使うならレコードに通し番号を持たせて突き合わせること
// （WeaponFireFixture の EventIdentifier.EventCount がその例）。

#pragma once

#include <cstddef>
#include <cstdio>
#include <vector>

#include "../icd/icd_codec.h"
#include "../hla/TCToHla.h"

namespace stub {

template <class T>
class TCVerifyingToHla : public hla::TCToHla<T> {
public:
    /// **供給側（TCFixtureFromHla）と同じ関数を渡すこと。** 「送ったはずの値」の定義が2箇所に
    /// 分かれると、往復照合は何も確かめていないのと同じになる。
    using Fixture = T (*)(std::size_t);

    explicit TCVerifyingToHla(Fixture make) : m_make(make) {}

    void accept(const T& rec) override {
        if (encodedBytes(rec) != encodedBytes(m_make(m_received))) {
            if (m_mismatched < 5) {
                std::printf("  !! %s の %zu 件目が往復で一致しません\n", T::kFomName, m_received);
            }
            ++m_mismatched;
        }
        if (m_received == 0) dumpFirst(rec);
        ++m_received;
    }

    [[nodiscard]] std::size_t received() const noexcept { return m_received; }
    [[nodiscard]] std::size_t mismatched() const noexcept { return m_mismatched; }

private:
    /// レコード1件を符号化した結果。失敗したら空を返すので、比較は必ず不一致になる。
    /// encode を修飾せずに呼ぶのは ADL のため — 生成コードが T と同じ名前空間に置いている。
    static std::vector<unsigned char> encodedBytes(const T& rec) {
        std::vector<unsigned char> out(T::kEncodedSize, 0);
        icd::Writer w(out.data(), out.size());
        encode(w, rec);
        if (!w.ok()) out.clear();
        return out;
    }

    static void dumpFirst(const T& rec) {
        const std::vector<unsigned char> bytes = encodedBytes(rec);
        const std::size_t n = bytes.size() < 48 ? bytes.size() : 48;
        std::printf("%s の先頭レコード（先頭 %zu / %zu バイト・ビッグエンディアン）\n",
                    T::kFomName, n, bytes.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (i % 16 == 0) std::printf("  %04zx  ", i);
            std::printf("%02x ", bytes[i]);
            if (i % 16 == 15) std::printf("\n");
        }
        if (n % 16 != 0) std::printf("\n");
        std::printf("\n");
    }

    Fixture m_make;                 ///< 期待値を作る関数。供給側と同一でなければならない
    std::size_t m_received = 0;     ///< 受け取った件数。照合する期待値の添字も兼ねる
    std::size_t m_mismatched = 0;   ///< 往復で元とバイト列が違った件数
};

}  // namespace stub
