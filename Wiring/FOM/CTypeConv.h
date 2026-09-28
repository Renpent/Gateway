// 入れ子のレコードと ID の変換。**クラスではなく型ごと**なので、複数のクラスで共有する。
//
//   toIcd(ツールキットの値) -> ICD の値     HLA → UDP
//   toRti(ICD の値)         -> ツールキットの値  UDP → HLA
//
// クラスを足して、まだ無い型が出てきたらここに1組足す。

#pragma once

#include <string>

#include "../../ICD/icd_types.h"
#include "Toolkit.h"

/// 型ごとの変換を static 関数として並べるだけのクラス（名前空間の代わり）。
class CTypeConv {
public:
    // RTIobjectId（ICD では文字の配列）<-> 文字列。
    // **ICD の上限（RTIobjectId は16文字）を超える ID は、そのレコードごと送られない。**
    // 上限は ICDgenerator の配列上限ダイアログで決める。
    static icdfom::RTIobjectId toIcd(const std::string& s);
    static std::string toRti(const icdfom::RTIobjectId& v);

    static icdfom::EntityIdentifierStruct toIcd(const tk::EntityIdentifierStruct& v);
    static tk::EntityIdentifierStruct toRti(const icdfom::EntityIdentifierStruct& v);

    static icdfom::RelativePositionStruct toIcd(const tk::RelativePositionStruct& v);
    static tk::RelativePositionStruct toRti(const icdfom::RelativePositionStruct& v);

    static icdfom::WorldLocationStruct toIcd(const tk::WorldLocationStruct& v);
    static tk::WorldLocationStruct toRti(const icdfom::WorldLocationStruct& v);

    static icdfom::AccelerationVectorStruct toIcd(const tk::AccelerationVectorStruct& v);
    static tk::AccelerationVectorStruct toRti(const icdfom::AccelerationVectorStruct& v);

    static icdfom::VelocityVectorStruct toIcd(const tk::VelocityVectorStruct& v);
    static tk::VelocityVectorStruct toRti(const icdfom::VelocityVectorStruct& v);

    static icdfom::EventIdentifierStruct toIcd(const tk::EventIdentifierStruct& v);
    static tk::EventIdentifierStruct toRti(const icdfom::EventIdentifierStruct& v);

    static icdfom::EntityTypeStruct toIcd(const tk::EntityTypeStruct& v);
    static tk::EntityTypeStruct toRti(const icdfom::EntityTypeStruct& v);
};
