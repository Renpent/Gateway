// 自動生成 — 編集しないこと。
// HLAobjectRoot.EmbeddedSystem.RadioReceiver
//
// ICDgenerator が FOM から生成。C++17 / ビッグエンディアン / レコードは固定長。

#include "RadioReceiver.h"

namespace icdfom {

icd::Result decode(icd::Reader& r, RadioReceiver& v) {
    if (const icd::Result rc = decode(r, v.EntityIdentifier); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.HostObjectIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.RelativePosition); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.RadioIndex); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.ReceivedPower); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = icd::decodeBounded(r, v.ReceivedTransmitterIdentifier, 16); rc != icd::Result::Ok) return rc;
    if (const icd::Result rc = decode(r, v.ReceiverOperationalStatus); rc != icd::Result::Ok) return rc;
    return icd::Result::Ok;
}

void encode(icd::Writer& w, const RadioReceiver& v) {
    encode(w, v.EntityIdentifier);
    icd::encodeBounded(w, v.HostObjectIdentifier, 16);
    encode(w, v.RelativePosition);
    encode(w, v.RadioIndex);
    encode(w, v.ReceivedPower);
    icd::encodeBounded(w, v.ReceivedTransmitterIdentifier, 16);
    encode(w, v.ReceiverOperationalStatus);
}

std::size_t encodedSize(const RadioReceiver& v) {
    (void)v;
    return 62;
}

}  // namespace icdfom
