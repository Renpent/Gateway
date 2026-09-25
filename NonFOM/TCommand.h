// コマンド文字列。FOM に無い独自データ。
//
// **形式（仮）：1データグラム = 1コマンドで、中身は char の並び。** ヘッダは無い。
// 最初の NUL で切り、末尾の改行を落とす。なので次の3つはどれも "STOP" になる：
//
//   "STOP"          文字列の長さぶんだけ送られてきた
//   "STOP\0\0\0…"   固定長の char 配列を丸ごと送られてきた
//   "STOP\r\n"      行として送られてきた
//
// 相手の仕様が決まったら parse と kPort を合わせること。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace nonfom {

struct TCommand {
    std::string text;   ///< コマンド本体。NUL と末尾の改行は除いてある

    static constexpr const char*   kName = "Command";   ///< 表示名
    static constexpr std::uint16_t kPort = 24100;       ///< 受信ポート（仮）。ICD のクラスと重ならないこと
};

/// 1データグラムをコマンド1つとして読む。空なら false（捨てられる）。
/// CTRawChannel が ADL で拾うので、TCommand と同じ名前空間に置く。
inline bool parse(const unsigned char* data, std::size_t len, TCommand& out) {
    std::size_t n = 0;
    while (n < len && data[n] != '\0') ++n;
    while (n > 0 && (data[n - 1] == '\n' || data[n - 1] == '\r')) --n;

    if (n == 0) return false;
    out.text.assign(reinterpret_cast<const char*>(data), n);
    return true;
}

}  // namespace nonfom
