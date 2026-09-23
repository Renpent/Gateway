// RTIobjectId を作る小さな道具。
//
// FOM の RTIobjectId は null 終端の文字配列（std::vector<HLAASCIIchar>）なので、文字列
// リテラルからそのまま代入できない。フィクスチャが増えるたびに同じ変換を書くことになるので
// ここに出してある。

#pragma once

#include <string>

#include "../icd/icd_types.h"

namespace hla {

inline icdfom::RTIobjectId objectId(const std::string& s) {
    return icdfom::RTIobjectId(s.begin(), s.end());
}

}  // namespace hla
