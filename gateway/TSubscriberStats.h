// 受信側で起きたことの内訳。
//
// 受信は「捨てる判断」が仕事の半分なので、何をどう捨てたかを数えて外から見えるようにしてある。
// 黙って落とされるパケットが一番デバッグしにくい。
//
// **TCSubscriber<T> の入れ子にはできない。** CChannel::getInStats() がテンプレートでない参照を
// 返すからで、入れ子にすると T ごとに別の型になり、抽象側が戻り値の型を書けなくなる。
// 独立したファイルなのは好みではなく、この制約による。

#pragma once

#include <cstdint>

namespace gw {

struct TSubscriberStats {
    std::uint64_t datagrams = 0;      ///< 正しく開けたデータグラム
    std::uint64_t records = 0;        ///< 取り出せたレコード
    std::uint64_t wrongClass = 0;     ///< classId 不一致 — ポートの向き先を疑う
    std::uint64_t malformed = 0;      ///< ヘッダが壊れている / 短すぎる
    std::uint64_t skipped = 0;        ///< 個々のレコードが復号できなかった
};

}  // namespace gw
