// ICD のクラス1つぶんの番号。**手で書かず、生成物の定数から bindingOf<T>() で作る。**
//
// 1クラス = 1ポート。classId はポートと重複する情報だが、受信側でポートの設定ミスを
// 捨てられるように持つ。

#pragma once

#include <cstddef>
#include <cstdint>

namespace gw {

struct TClassBinding {
    std::uint32_t classId;   ///< データグラム先頭に入るクラス識別子
    std::uint16_t port;      ///< このクラス専用の UDP ポート（送受信とも同じ番号）
    std::size_t   payload;   ///< 1データグラムの上限（ICD の MTU から生成）
    const char*   fomName;   ///< FOM 上の完全名。メッセージ用
};

/// 生成されたクラス T の定数から TClassBinding を作る。
template <class T>
constexpr TClassBinding bindingOf() noexcept {
    return TClassBinding{T::kClassId, T::kPort, T::kPayload, T::kFomName};
}

}  // namespace gw
