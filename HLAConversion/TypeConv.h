// 入れ子のレコードと ID の変換。**クラスではなく型ごと**なので、複数のクラスで共有する。
//
//   toIcd(ツールキットの値) -> ICD の値     HLA → UDP
//   toRti(ICD の値)         -> ツールキットの値  UDP → HLA
//
// クラスを足して、まだ無い型が出てきたらここに1組足す。

#pragma once

#include <string>

#include "../ICD/icd_types.h"
#include "Toolkit.h"

namespace hlaconv {

// RTIobjectId（ICD では文字の配列）<-> 文字列。
// **ICD の上限（RTIobjectId は16文字）を超える ID は、そのレコードごと送られない。**
// 上限は ICDgenerator の配列上限ダイアログで決める。
icdfom::RTIobjectId toIcd(const std::string& s);
std::string toRti(const icdfom::RTIobjectId& v);

icdfom::EntityIdentifierStruct toIcd(const tk::EntityIdentifierStruct& v);
tk::EntityIdentifierStruct toRti(const icdfom::EntityIdentifierStruct& v);

icdfom::RelativePositionStruct toIcd(const tk::RelativePositionStruct& v);
tk::RelativePositionStruct toRti(const icdfom::RelativePositionStruct& v);

icdfom::WorldLocationStruct toIcd(const tk::WorldLocationStruct& v);
tk::WorldLocationStruct toRti(const icdfom::WorldLocationStruct& v);

icdfom::AccelerationVectorStruct toIcd(const tk::AccelerationVectorStruct& v);
tk::AccelerationVectorStruct toRti(const icdfom::AccelerationVectorStruct& v);

icdfom::VelocityVectorStruct toIcd(const tk::VelocityVectorStruct& v);
tk::VelocityVectorStruct toRti(const icdfom::VelocityVectorStruct& v);

icdfom::EventIdentifierStruct toIcd(const tk::EventIdentifierStruct& v);
tk::EventIdentifierStruct toRti(const icdfom::EventIdentifierStruct& v);

icdfom::EntityTypeStruct toIcd(const tk::EntityTypeStruct& v);
tk::EntityTypeStruct toRti(const icdfom::EntityTypeStruct& v);

}  // namespace hlaconv
