// ICD の「抽出概要」シート1行ぶん。
//
// FOM にはポートも更新レートも存在しないので生成器はここを埋められない。ICD 側で決めた値を
// **手で写す**ための型で、ID・ポート・更新レートが1つの型に並ぶので写し間違いが目で見つかる。
//
// 1クラス = 1ポート。データグラムの classId はポートと情報が重複するが、
// ポートの設定ミスを受信側で捕まえられるので両方持つ。
//
// 実際の値は ClassIds.h。

#pragma once

#include <cstddef>
#include <cstdint>

#include "Delivery.h"

namespace gw {

struct ClassBinding {
    std::uint32_t classId;      ///< データグラム先頭に入るクラス識別子
    std::uint16_t port;         ///< このクラス専用の UDP ポート（送受信とも同じ番号）
    unsigned      rateHz;       ///< 0 = 変化時のみ送信
    std::size_t   payload;      ///< 1データグラムの上限。経路の MTU で決まる
    Delivery      delivery;     ///< 状態かイベントか。送り残しの扱いが正反対
    const char*   fomName;      ///< FOM 上の完全名。ログ表示用
};

}  // namespace gw
