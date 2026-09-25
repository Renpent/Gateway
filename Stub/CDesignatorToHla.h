// **本番には持っていかないファイル。** Stub/ は RTI が無いこの環境でゲートウェイを動かすための
// 代用品で、実 RTI に繋ぐときはフォルダごと消せる（直すのは Wiring/CWiring.h だけ）。
//
// Designator の受け口（モック）。UDP から受けたレコードを HLA へ書く代わりに表示する。
//
// 本番の hla::CTRtiObjectToHla と同じ流れにしてある：
//   1. 鍵（HostObjectIdentifier）でどのインスタンスかを決める
//   2. 初めて見る鍵なら登録する（本番では registerObjectInstance）
//   3. 属性を書いて update する（本番では p->setXxx(...); p->update();）

#pragma once

#include <cstdio>
#include <set>
#include <string>

#include "../Core/HLA/CTToHla.h"
#include "../ICD/Object/Designator.h"

namespace stub {

class CDesignatorToHla : public hla::CTToHla<icdfom::Designator> {
public:
    void accept(const icdfom::Designator& d) override {
        const std::string key(d.HostObjectIdentifier.begin(), d.HostObjectIdentifier.end());

        if (m_registered.insert(key).second) {
            std::printf("[HLA・モック] 登録: %s\n", key.c_str());
        }
        std::printf("[HLA・モック] 更新: %s  照射点=(%.1f, %.1f, %.1f)  出力=%.1f W\n",
                    key.c_str(), d.DesignatorSpotLocation.X, d.DesignatorSpotLocation.Y,
                    d.DesignatorSpotLocation.Z, static_cast<double>(d.DesignatorOutputPower));
    }

private:
    std::set<std::string> m_registered;   ///< 登録済みの鍵（本番では鍵 -> インスタンスの対応表）
};

}  // namespace stub
