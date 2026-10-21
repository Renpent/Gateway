// ICD のクラス1つぶんの CChannel。
//
// 送信：毎周期 fromHla から取り出した分を全部送る。**送れなかった分は捨てる**（持ち越さない）。
// 受信：届いた分を全部復号して toHla へ渡す。
// ログ：1データグラムごとに1行、送信も受信も log/<クラス名>_<起動時刻>.log に書く（CChannelLog）。

#pragma once

#include <cstring>
#include <string>
#include <vector>

#include "CChannel.h"
#include "CChannelLog.h"
#include "HLA/CTFromHla.h"
#include "HLA/CTToHla.h"
#include "TClassBinding.h"
#include "UDP/CTUdpReceiver.h"
#include "UDP/CTUdpSender.h"
#include "UDP/CUdpSocket.h"

template <class T>
class CTClassChannel final : public CChannel {
public:
    /// fromHla が null なら送信しない、toHla が null なら受けた分を捨てる。
    /// **どちらも借りているだけ**なので、このチャネルより長く生きていること。
    CTClassChannel(const TClassBinding& bind, CTFromHla<T>* fromHla, CTToHla<T>* toHla)
        : m_bind(bind), m_name(withoutRoot(bind.fomName)), m_fromHla(fromHla), m_toHla(toHla),
          m_sender(bind.classId, bind.payload), m_receiver(bind.classId, bind.payload) {}

    [[nodiscard]] const char* getName() const noexcept override { return m_name; }
    [[nodiscard]] std::uint16_t getPort() const noexcept override { return m_bind.port; }
    [[nodiscard]] CUdpSocket& getSocket() noexcept override { return m_sock; }

    [[nodiscard]] bool open(const std::string& peerHost) override {
        if (!m_sock.open(m_bind.port, peerHost, m_bind.port)) return false;
        m_log.open(m_name);
        m_log.write("開始 port=%u classId=%u 1件=%u B 送信先=%s", static_cast<unsigned>(m_bind.port),
                    static_cast<unsigned>(m_bind.classId), static_cast<unsigned>(icd::fixedSize<T>),
                    peerHost.empty() ? "なし（受信のみ）" : peerHost.c_str());
        return true;
    }

    void pumpIn() override {
        m_receiver.drain(m_sock, [this](const T& record) {
            if (m_toHla != nullptr) m_toHla->accept(record);
        }, [this](const TRecvReport& r) {
            if (r.m_bytes < static_cast<long>(icd::kHeaderSize)) {
                m_log.error("受 捨てた %ld B 理由=ヘッダ（12 B）に満たない", r.m_bytes);
                return;
            }
            if (r.m_result != icd::Result::Ok) {
                m_log.error("受 捨てた %ld B 理由=%s（届いたヘッダ classId=%u 件数=%u 1件=%u B）",
                            r.m_bytes, resultName(r.m_result), static_cast<unsigned>(r.m_classId),
                            static_cast<unsigned>(r.m_count), static_cast<unsigned>(r.m_recordSize));
                return;
            }
            m_log.write("受 %u件 %ld B", static_cast<unsigned>(r.m_count), r.m_bytes);
            if (r.m_skipped > 0) m_log.error("受 復号できない %u件を飛ばした", r.m_skipped);
        });
    }

    void pumpOut() override {
        if (m_fromHla == nullptr || !m_sock.canSend()) return;

        m_outbox.clear();
        m_fromHla->drain(m_outbox);
        const auto report = [this](const TSendReport& r) {
            if (r.m_sent) m_log.write("送 %u件 %u B", r.m_count, static_cast<unsigned>(r.m_bytes));
            else m_log.error("送 失敗 %u件を捨てた", r.m_count);
        };
        for (const T& record : m_outbox) m_sender.publish(record, m_sock, report);
        m_sender.flush(m_sock, report);   // 次の周期まで抱えない
    }

private:
    /// "HLAobjectRoot.EmitterBeam.RadarBeam" -> "EmitterBeam.RadarBeam"
    static const char* withoutRoot(const char* fomName) noexcept {
        const char* dot = std::strchr(fomName, '.');
        return dot ? dot + 1 : fomName;
    }

    static const char* resultName(icd::Result rc) noexcept {
        switch (rc) {
        case icd::Result::Truncated:      return "ヘッダの件数ぶんの長さが無い";
        case icd::Result::WrongClass:     return "classId 違い";
        case icd::Result::RecordTooSmall: return "1件の長さが足りない（相手と ICD が食い違っている）";
        default:                          return "壊れたヘッダ";
        }
    }

    TClassBinding m_bind;               ///< ID / ポート / 上限（生成物の定数から）
    const char* m_name;                 ///< 表示名。m_bind.fomName の根を除いた部分を指す
    CTFromHla<T>* m_fromHla;       ///< HLA 側の供給元。借り物で、null なら送信しない
    CTToHla<T>* m_toHla;           ///< HLA 側の受け口。借り物で、null なら受信を捨てる
    CUdpSocket m_sock;             ///< このクラス専用のソケット（送受信とも1本）
    CTUdpSender<T> m_sender;       ///< レコード → データグラム
    CTUdpReceiver<T> m_receiver;   ///< データグラム → レコード
    std::vector<T> m_outbox;            ///< その周期に送るレコード。毎周期空にして使い回す
    CChannelLog m_log;             ///< このクラスのログファイル
};

