// ICD のクラス1つぶんの CChannel。
//
// 送信：毎周期 fromHla から取り出した分を全部送る。**送れなかった分は捨てる**（持ち越さない）。
// 受信：届いた分を全部復号して toHla へ渡す。

#pragma once

#include <cstring>
#include <string>
#include <vector>

#include "hla/CTFromHla.h"
#include "hla/CTToHla.h"
#include "udp/CUdpSocket.h"
#include "udp/CTUdpSender.h"
#include "udp/CTUdpReceiver.h"
#include "CChannel.h"
#include "TClassBinding.h"

namespace gw {

template <class T>
class CTClassChannel final : public CChannel {
public:
    /// fromHla が null なら送信しない、toHla が null なら受けた分を捨てる。
    /// **どちらも借りているだけ**なので、このチャネルより長く生きていること。
    CTClassChannel(const TClassBinding& bind, hla::CTFromHla<T>* fromHla, hla::CTToHla<T>* toHla)
        : m_bind(bind), m_name(withoutRoot(bind.fomName)), m_fromHla(fromHla), m_toHla(toHla),
          m_sender(bind.classId, bind.payload), m_receiver(bind.classId, bind.payload) {}

    [[nodiscard]] const char* getName() const noexcept override { return m_name; }
    [[nodiscard]] std::uint16_t getPort() const noexcept override { return m_bind.port; }
    [[nodiscard]] udp::CUdpSocket& getSocket() noexcept override { return m_sock; }

    [[nodiscard]] bool open(const std::string& peerHost) override {
        return m_sock.open(m_bind.port, peerHost, m_bind.port);
    }

    void pumpIn() override {
        m_receiver.drain(m_sock, [this](const T& record) {
            if (m_toHla != nullptr) m_toHla->accept(record);
        });
    }

    void pumpOut() override {
        if (m_fromHla == nullptr || !m_sock.canSend()) return;

        m_outbox.clear();
        m_fromHla->drain(m_outbox);
        for (const T& record : m_outbox) m_sender.publish(record, m_sock);
        m_sender.flush(m_sock);   // 次の周期まで抱えない
    }

private:
    /// "HLAobjectRoot.EmitterBeam.RadarBeam" -> "EmitterBeam.RadarBeam"
    static const char* withoutRoot(const char* fomName) noexcept {
        const char* dot = std::strchr(fomName, '.');
        return dot ? dot + 1 : fomName;
    }

    TClassBinding m_bind;               ///< ID / ポート / 上限（生成物の定数から）
    const char* m_name;                 ///< 表示名。m_bind.fomName の根を除いた部分を指す
    hla::CTFromHla<T>* m_fromHla;       ///< HLA 側の供給元。借り物で、null なら送信しない
    hla::CTToHla<T>* m_toHla;           ///< HLA 側の受け口。借り物で、null なら受信を捨てる
    udp::CUdpSocket m_sock;             ///< このクラス専用のソケット（送受信とも1本）
    udp::CTUdpSender<T> m_sender;       ///< レコード → データグラム
    udp::CTUdpReceiver<T> m_receiver;   ///< データグラム → レコード
    std::vector<T> m_outbox;            ///< その周期に送るレコード。毎周期空にして使い回す
};

}  // namespace gw
