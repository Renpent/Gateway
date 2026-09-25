// HLA ⇄ UDP ゲートウェイ（簡易版）。
//
//   HLAGateway <宛先IP|none> [Hz] [秒]
//
// 宛先が none なら受信専用。秒を省くか 0 にすると止まらない。
// 配線（どのクラスを流すか）は Wiring/CWiring.h。

#include <cstdio>
#include <cstdlib>
#include <string>

#include "Core/CGateway.h"
#include "Platform/Platform.h"
#include "Wiring/CWiring.h"

namespace {

unsigned toUnsigned(const char* s, unsigned fallback) {
    if (s == nullptr) return fallback;
    const long v = std::strtol(s, nullptr, 10);
    return v >= 0 ? static_cast<unsigned>(v) : fallback;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::printf("使い方: HLAGateway <宛先IP|none> [Hz] [秒]\n");
        return 2;
    }

    platform::initPlatform();

    const std::string peer = (std::string(argv[1]) == "none") ? "" : argv[1];
    const unsigned hz = toUnsigned(argc > 2 ? argv[2] : nullptr, 20);
    const unsigned seconds = toUnsigned(argc > 3 ? argv[3] : nullptr, 0);

    int rc = 1;
    {
        // **CWiring を先に宣言すること。** gateway のチャネルが wiring のメンバを借りているので、
        // 破棄（宣言の逆順）で gateway が先に消えるようにする。
        wiring::CWiring wiring;
        core::CGateway gateway;
        wiring.build(gateway);

        if (gateway.openAll(peer)) {
            std::printf("%u Hz で開始（送信先: %s）\n", hz, peer.empty() ? "なし" : peer.c_str());
            gateway.run(hz, seconds);
            rc = 0;
        }
    }

    platform::shutdownPlatform();
    return rc;
}
