// Designator（オブジェクトクラス）の本番の変換。**オブジェクトを1つ足すときの雛形。**
//
// 配線（Wiring/CRtiWiring.h）にはこの5本を渡す：
//
//   HLA → UDP  CTRtiObjectFromHla{&CDesignatorRti::getRemoteDesignator, &CDesignatorRti::toIcd}
//   UDP → HLA  CTRtiObjectToHla{&CDesignatorRti::keyOf, &CDesignatorRti::registerDesignator,
//                               &CDesignatorRti::updateDesignator}

#pragma once

#include <string>
#include <vector>

#include "../../ICD/Object/Designator.h"
#include "Toolkit.h"

/// Designator の変換を static 関数として並べるだけのクラス（名前空間の代わり）。
class CDesignatorRti {
public:
    /// Fetch：受信したインスタンスの一覧を取る。
    static std::vector<tk::DesignatorPtr> getRemoteDesignator();

    /// Convert：1インスタンス -> 1レコード。
    static icdfom::Designator toIcd(const tk::DesignatorPtr& p);

    /// KeyOf：このレコードがどのインスタンスのものか。
    /// **鍵の選び方は ICD 側の判断で、まだ決まっていない。** ここでは HostObjectIdentifier にしてある
    /// （1つの母体に指示器が複数あるなら、ほかのフィールドと組にする）。
    static std::string keyOf(const icdfom::Designator& r);

    /// Create：初めて見る鍵でインスタンスを登録する。失敗したら空。
    static tk::DesignatorPtr registerDesignator(const std::string& key);

    /// Update：属性を書いて update する。
    static void updateDesignator(const icdfom::Designator& r, const tk::DesignatorPtr& p);
};
