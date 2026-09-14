// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmbeddedSystem.RadioReceiver
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#ifndef ICDFOM_RADIORECEIVER_H
#define ICDFOM_RADIORECEIVER_H

#include "icd_types.h"

namespace icdfom {

/// FOM: HLAobjectRoot.EmbeddedSystem.RadioReceiver
///
/// Members are in the ICD's row order, which is the order they occupy on the wire.
struct RadioReceiver {
    EntityIdentifierStruct EntityIdentifier;  ///< FOM: EntityIdentifier : EntityIdentifierStruct
    RTIobjectId HostObjectIdentifier;  ///< FOM: HostObjectIdentifier : RTIobjectId
    RelativePositionStruct RelativePosition;  ///< FOM: RelativePosition : RelativePositionStruct
    UnsignedInteger16 RadioIndex;  ///< FOM: RadioIndex : UnsignedInteger16
    PowerRatioDecibelMilliwattFloat32 ReceivedPower;  ///< FOM: ReceivedPower : PowerRatioDecibelMilliwattFloat32
    RTIobjectId ReceivedTransmitterIdentifier;  ///< FOM: ReceivedTransmitterIdentifier : RTIobjectId
    ReceiverOperationalStatusEnum16 ReceiverOperationalStatus;  ///< FOM: ReceiverOperationalStatus : ReceiverOperationalStatusEnum16
    static constexpr std::size_t kEncodedSize = 62;
    static constexpr std::uint32_t kClassId = 2;  // 抽出概要シートの ID 列
};

[[nodiscard]] icd::Result decode(icd::Reader& r, RadioReceiver& v);
void encode(icd::Writer& w, const RadioReceiver& v);

/// Reads the records batched into one datagram.
/// The class id stays a runtime argument so a deployment can override the ICD's number
/// without regenerating; pass kClassId to take it.
typedef icd::DatagramReader<RadioReceiver> RadioReceiverReader;

/// Packs records into one datagram until the next one will not fit.
typedef icd::DatagramWriter<RadioReceiver> RadioReceiverWriter;

}  // namespace icdfom

#endif  // ICDFOM_RADIORECEIVER_H
