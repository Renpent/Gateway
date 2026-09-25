// **本番には持っていかないファイル。** HLA ツールキットの代わり（偽物）。
//
// 本物の API で分かっているのは次の形だけなので、それ以外は仮に決めてある：
//
//   world->getObjectManager()->getRemoteDesignator()   受信したインスタンスの一覧
//   world->getObjectManager()->registerDesignator(名前)  自分のインスタンスを登録
//   world->getInteractionManager()->getWeaponFire()     送信用のインタラクション
//   p->getXxx() / p->setXxx(...) / p->update()          属性の読み書きと送信
//   i->setXxx(...) / i->sendInteraction()               パラメータの詰め込みと送信
//
// **仮に決めたところ**（本物の綴りが分かったら HLAConversion/ の変換関数を合わせる）：
//   - 属性はアクセサ（getXxx / setXxx）で触る
//   - 入れ子のレコードは FOM と同じ名前の構造体で返る
//   - 列挙は整数、RTIobjectId は std::string
//   - インタラクションの受信は、FOM の全インタラクションの仮想関数が並んだクラス
//     （InteractionCallback）を継承し、要るものだけ実装して setInteractionCallback で登録する。
//     関数名（onXxx）と登録の仕方は仮
//
// 外部ツールキットの見た目を真似るため、型名は接頭辞の決まりの対象外。
// 「試験用」と書いた関数は本物には無い（RTI の代わりに値を入れるためのもの）。

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace toolkit {

// ─── 入れ子のレコード ─────────────────────────────────────────────

struct FederateIdentifierStruct { std::uint16_t SiteID = 0; std::uint16_t ApplicationID = 0; };
struct EntityIdentifierStruct { FederateIdentifierStruct FederateIdentifier; std::uint16_t EntityNumber = 0; };
struct RelativePositionStruct { float BodyXDistance = 0, BodyYDistance = 0, BodyZDistance = 0; };
struct WorldLocationStruct { double X = 0, Y = 0, Z = 0; };
struct AccelerationVectorStruct { float XAcceleration = 0, YAcceleration = 0, ZAcceleration = 0; };
struct VelocityVectorStruct { float XVelocity = 0, YVelocity = 0, ZVelocity = 0; };
struct EventIdentifierStruct { std::uint16_t EventCount = 0; std::string IssuingObjectIdentifier; };
struct EntityTypeStruct {
    std::uint8_t EntityKind = 0, Domain = 0;
    std::uint16_t CountryCode = 0;
    std::uint8_t Category = 0, Subcategory = 0, Specific = 0, Extra = 0;
};

// 属性1つぶんの getXxx / setXxx を作る（偽物を短く書くためだけのマクロ）。
#define TOOLKIT_ATTR(Type, Name)                                          \
public:                                                                   \
    const Type& get##Name() const { return m_##Name; }                    \
    void set##Name(const Type& v) { m_##Name = v; }                       \
private:                                                                  \
    Type m_##Name{};

// ─── オブジェクト ───────────────────────────────────────────────────

class Designator {
    TOOLKIT_ATTR(EntityIdentifierStruct, EntityIdentifier)
    TOOLKIT_ATTR(std::string, HostObjectIdentifier)
    TOOLKIT_ATTR(RelativePositionStruct, RelativePosition)
    TOOLKIT_ATTR(std::uint16_t, CodeName)
    TOOLKIT_ATTR(std::string, DesignatedObjectIdentifier)
    TOOLKIT_ATTR(std::uint16_t, DesignatorCode)
    TOOLKIT_ATTR(float, DesignatorEmissionWavelength)
    TOOLKIT_ATTR(float, DesignatorOutputPower)
    TOOLKIT_ATTR(WorldLocationStruct, DesignatorSpotLocation)
    TOOLKIT_ATTR(std::uint8_t, DeadReckoningAlgorithm)
    TOOLKIT_ATTR(RelativePositionStruct, RelativeSpotLocation)
    TOOLKIT_ATTR(AccelerationVectorStruct, SpotLinearAccelerationVector)
public:
    void update() { ++m_updates; }                        ///< 属性を RTI へ送る
    int getUpdates() const { return m_updates; }          ///< 試験用：update された回数
private:
    int m_updates = 0;
};
using DesignatorPtr = std::shared_ptr<Designator>;

class ObjectManager {
public:
    /// 受信した（他のフェデレートの）インスタンスの一覧。
    std::vector<DesignatorPtr> getRemoteDesignator() const { return m_remote; }

    /// 自分のインスタンスを登録する。失敗したら空。
    DesignatorPtr registerDesignator(const std::string& name) {
        DesignatorPtr p = std::make_shared<Designator>();
        m_local.push_back({name, p});
        return p;
    }

    void addRemoteDesignator(const DesignatorPtr& p) { m_remote.push_back(p); }   ///< 試験用
    const std::vector<std::pair<std::string, DesignatorPtr>>& getLocalDesignators() const {
        return m_local;
    }                                                                              ///< 試験用

private:
    std::vector<DesignatorPtr> m_remote;
    std::vector<std::pair<std::string, DesignatorPtr>> m_local;
};

// ─── インタラクション ───────────────────────────────────────────────

class WeaponFire {
    TOOLKIT_ATTR(EventIdentifierStruct, EventIdentifier)
    TOOLKIT_ATTR(float, FireControlSolutionRange)
    TOOLKIT_ATTR(std::uint32_t, FireMissionIndex)
    TOOLKIT_ATTR(WorldLocationStruct, FiringLocation)
    TOOLKIT_ATTR(std::string, FiringObjectIdentifier)
    TOOLKIT_ATTR(std::uint16_t, FuseType)
    TOOLKIT_ATTR(VelocityVectorStruct, InitialVelocityVector)
    TOOLKIT_ATTR(std::string, MunitionObjectIdentifier)
    TOOLKIT_ATTR(EntityTypeStruct, MunitionType)
    TOOLKIT_ATTR(std::uint16_t, QuantityFired)
    TOOLKIT_ATTR(std::uint16_t, RateOfFire)
    TOOLKIT_ATTR(std::string, TargetObjectIdentifier)
    TOOLKIT_ATTR(std::uint16_t, WarheadType)
public:
    /// パラメータを RTI へ送る。試験用に、送った中身を控えておく。
    void sendInteraction() {
        // 履歴を外してから自分を控える（履歴ごと写すと、送るたびに入れ子で膨らむ）。
        std::vector<WeaponFire> log;
        log.swap(m_sent);
        log.push_back(*this);
        m_sent.swap(log);
    }
    const std::vector<WeaponFire>& getSent() const { return m_sent; }   ///< 試験用
private:
    std::vector<WeaponFire> m_sent;
};

/// 実装しないインタラクションの例（中身は省略）。
class MunitionDetonation {};

/// 受信のコールバック。ツールキット側で生成される、**FOM の全インタラクションの仮想関数が
/// 並んだクラス**のつもり（ここでは2つだけ）。使う側が継承して、要るものだけ実装する。
/// 実装しなかったものは何もしない。**RTI のスレッドから呼ばれる。**
class InteractionCallback {
public:
    virtual ~InteractionCallback() = default;
    virtual void onWeaponFire(const WeaponFire& /*interaction*/) {}
    virtual void onMunitionDetonation(const MunitionDetonation& /*interaction*/) {}
};

class InteractionManager {
public:
    WeaponFire* getWeaponFire() { return &m_weaponFire; }

    /// 受信のコールバックを登録する。callback は借りるだけ。
    void setInteractionCallback(InteractionCallback* callback) { m_callback = callback; }

    /// 試験用：RTI がインタラクションを受け取ったことにする。
    void receiveWeaponFire(const WeaponFire& i) { if (m_callback) m_callback->onWeaponFire(i); }
    void receiveMunitionDetonation(const MunitionDetonation& i) {
        if (m_callback) m_callback->onMunitionDetonation(i);
    }

private:
    WeaponFire m_weaponFire;
    InteractionCallback* m_callback = nullptr;
};

// ─── world ──────────────────────────────────────────────────────────

class World {
public:
    ObjectManager* getObjectManager() { return &m_objects; }
    InteractionManager* getInteractionManager() { return &m_interactions; }

private:
    ObjectManager m_objects;
    InteractionManager m_interactions;
};

#undef TOOLKIT_ATTR

}  // namespace toolkit
