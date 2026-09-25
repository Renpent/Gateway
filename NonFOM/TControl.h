// 制御文字列。FOM に無い独自データ。
//
// **形式（仮）：1データグラム = 1制御で、中身は char の並び。** ヘッダは無い。
// 最初の NUL で切り、末尾の改行を落とす。中身は英数字の前提で、文字の検査はしない。
//
// 相手の仕様が決まったら parse と kPort を合わせること。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace nonfom {

struct TControl {
    std::string text;   ///< 制御の本体。NUL と末尾の改行は除いてある

    static constexpr const char*   kName = "Control";   ///< 表示名
    static constexpr std::uint16_t kPort = 24101;       ///< 受信ポート（仮）。ICD のクラスと重ならないこと
};

/// 1データグラムを制御1つとして読む。空なら false（捨てられる）。
/// CTRawChannel が ADL で拾うので、TControl と同じ名前空間に置く。
inline bool parse(const unsigned char* data, std::size_t len, TControl& out) {
    std::size_t n = 0;
    while (n < len && data[n] != '\0') ++n;
    while (n > 0 && (data[n - 1] == '\n' || data[n - 1] == '\r')) --n;

    if (n == 0) return false;
    out.text.assign(reinterpret_cast<const char*>(data), n);
    return true;
}

}  // namespace nonfom
