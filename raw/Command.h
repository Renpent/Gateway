// コマンド文字列。FOM に無い独自データの1つ目。
//
// **形式（仮）: 1データグラム = 1コマンドで、中身は char の並び。** ヘッダも長さの前置も無い。
// 相手の仕様が固まったら parse と kPort をそれに合わせること。
//
// C/C++ の送信側でよくある形を3つとも受けられるようにしてある：
//
//   "STOP"              文字列の長さぶんだけ送る
//   "STOP\0\0\0…"       char cmd[64] のような固定長バッファを丸ごと送る（NUL で詰めてある）
//   "STOP\r\n"          行として送る（改行付き）
//
// どれも "STOP" として渡す。**最初の NUL で切り、末尾の改行を落とす。** 残りが空なら形式違反。
//
// 文字の中身は見ない — 0x80 以上（日本語の Shift_JIS や UTF-8）もそのまま通す。何が正しい
// コマンドかを判断するのは受け取った側（app/CommandToApp.h）で、ここは「文字列として
// 取り出せるか」だけを決める。

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace raw {

struct Command {
    std::string text;   ///< コマンド本体。NUL と末尾の改行は除いてある

    static constexpr const char*   kName = "Command";   ///< 統計表に出る名前
    static constexpr std::uint16_t kPort = 24100;       ///< 受信ポート（仮）。ICD のクラスと重ならないこと
};

/// 1データグラムをコマンド1つとして読む。空なら false（統計の「異常」に数えられる）。
/// RawChannel が ADL で拾うので、Command と同じ名前空間に置いてある。
inline bool parse(const unsigned char* data, std::size_t len, Command& out) {
    // 最初の NUL まで。固定長の char 配列を丸ごと送ってくる相手は、後ろが NUL で埋まっている。
    std::size_t n = 0;
    while (n < len && data[n] != '\0') ++n;

    // 行として送ってくる相手のために、末尾の改行を落とす。
    while (n > 0 && (data[n - 1] == '\n' || data[n - 1] == '\r')) --n;

    if (n == 0) return false;
    out.text.assign(reinterpret_cast<const char*>(data), n);
    return true;
}

}  // namespace raw
