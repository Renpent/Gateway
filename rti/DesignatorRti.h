// Designator（オブジェクトクラス）の本番の変換。**オブジェクトを1つ足すときの雛形。**
//
// 配線（app/CRtiWiring.h）にはこの5本を渡す：
//
//   HLA → UDP  hla::CTRtiObjectFromHla{&getRemoteDesignator, &toIcd}
//   UDP → HLA  hla::CTRtiObjectToHla{&keyOf, &registerDesignator, &writeBack}

#pragma once

#include <string>
#include <vector>

#include "../icd/object/Designator.h"
#include "Toolkit.h"

namespace rti {

/// Fetch：受信したインスタンスの一覧を取る。
std::vector<tk::DesignatorPtr> getRemoteDesignator();

/// Convert：1インスタンス -> 1レコード。
icdfom::Designator toIcd(const tk::DesignatorPtr& p);

/// KeyOf：このレコードがどのインスタンスのものか。
/// **鍵の選び方は ICD 側の判断で、まだ決まっていない。** ここでは HostObjectIdentifier にしてある
/// （1つの母体に指示器が複数あるなら、ほかのフィールドと組にする）。
std::string keyOf(const icdfom::Designator& r);

/// Create：初めて見る鍵でインスタンスを登録する。失敗したら空。
tk::DesignatorPtr registerDesignator(const std::string& key);

/// Write：属性を書いて update する。
void writeBack(const icdfom::Designator& r, const tk::DesignatorPtr& p);

}  // namespace rti
