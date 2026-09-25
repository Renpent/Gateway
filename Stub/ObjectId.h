// **本番には持っていかないファイル。** Stub/ は RTI が無いこの環境で送信側を動かすための
// 代用品で、実 RTI に繋ぐときはフォルダごと消せる（直すのは Wiring/CWiring.h だけ）。
//
// 文字列から RTIobjectId（null 終端の文字配列）を作る。

#pragma once

#include <string>

#include "../ICD/icd_types.h"

namespace stub {

inline icdfom::RTIobjectId objectId(const std::string& s) {
    return icdfom::RTIobjectId(s.begin(), s.end());
}

}  // namespace stub
