// 1ポートぶんの送受信。周期ループ（CGateway）からはこの抽象だけが見える。
//
// 実装は2つ：
//   CTClassChannel<T>  ICD のクラス。12バイトヘッダ + 固定長レコード、相手は HLA
//   CTRawChannel<T>    FOM に無い独自データ。相手が決めた形式のまま、相手はアプリ

#pragma once

#include <cstdint>
#include <string>

#include "udp/CUdpSocket.h"

namespace gw {

class CChannel {
public:
    virtual ~CChannel() = default;

    /// メッセージに出す名前。
    [[nodiscard]] virtual const char* getName() const noexcept = 0;

    /// このチャネル専用の UDP ポート。**全チャネルで重複してはいけない。**
    [[nodiscard]] virtual std::uint16_t getPort() const noexcept = 0;

    [[nodiscard]] virtual udp::CUdpSocket& getSocket() noexcept = 0;

    /// 受信ポートを bind し、送信先を設定する。peerHost が空なら受信専用。
    [[nodiscard]] virtual bool open(const std::string& peerHost) = 0;

    /// 来ている分を読み切って手元（HLA やアプリ）へ渡す。poll が読めると言ったときだけ呼ぶ。
    virtual void pumpIn() = 0;

    /// 手元から出てきた分を送る。受信専用なら何もしない。
    virtual void pumpOut() = 0;
};

}  // namespace gw
