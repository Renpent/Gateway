// ICD の「抽出概要」シート1行ぶん — ただし**もう手で写さない**。
//
// ID・ポート・ペイロード・種別・FOM 名は、すべて生成された各クラスの定数
// （kClassId / kPort / kPayload / kIsInteraction / kFomName）から bindingOf<T>() が組み立てる。
// ゲートウェイが自前で持っている数字はゼロで、ICD を更新して再生成すれば追随する。
//
// 以前は ClassIds.h に手で写した定数表があった。手写しは ICD と食い違えても何も言ってくれない —
// 食い違いは受信側の「class違い」カウンタになって初めて見える — ので、出どころを1つにした。
//
// Rate は無い。周期は tick() を叩く側が決めていて、全クラスがその周期で出る。
// ICD シートの Rate 列は「受信側が期待してよい頻度」を書く参考値で、コードは見ない。
//
// 1クラス = 1ポート。データグラムの classId はポートと情報が重複するが、
// ポートの設定ミスを受信側で捕まえられるので両方持つ。

#pragma once

#include <cstddef>
#include <cstdint>

#include "ClassKind.h"

namespace gw {

struct ClassBinding {
    std::uint32_t classId;      ///< データグラム先頭に入るクラス識別子
    std::uint16_t port;         ///< このクラス専用の UDP ポート（送受信とも同じ番号）
    std::size_t   payload;      ///< 1データグラムの上限。ICD の MTU から生成された値
    ClassKind     kind;         ///< オブジェクトかインタラクションか。送り残しの扱いが正反対
    const char*   fomName;      ///< FOM 上の完全名。ログ表示用
};

/// 生成されたクラス T の定数から、その ClassBinding を組み立てる。
///
/// T を明示するのは呼び側で、値は T から決まる。別クラスの binding を渡し間違える余地が無い。
/// 配備先でポートを変えたいときは、これを使わず ClassBinding を直接書いて ClassChannel に渡せばよい。
template <class T>
constexpr ClassBinding bindingOf() noexcept {
    return ClassBinding{
        T::kClassId,
        T::kPort,
        T::kPayload,
        T::kIsInteraction ? ClassKind::Interaction : ClassKind::Object,
        T::kFomName,
    };
}

}  // namespace gw
