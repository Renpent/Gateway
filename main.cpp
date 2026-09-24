// HLA ⇄ UDP ゲートウェイ（参照実装）。
//
// この環境には RTI が無いので、HLA 側だけをスタブに置き換えてある。それ以外 — ICD コーデック、
// データグラムの組み立て、複数ポートの監視、UDP の送受信 — は本番と同じコードが動く。
// Windows と Linux のどちらでもビルドでき、同じ振る舞いをする。
//
//   HLAGateway loopback [Hz] [秒]         自分宛に送って自分で受け、往復後の値を元と照合
//   HLAGateway run <宛先IP|none> [Hz] [秒]  実運用の形。none なら受信専用
//
// **配線は app/CWiring.h。** クラスを増やすときに触るのはそちらで、このファイルではない。

#include <cstdio>
#include <cstdlib>
#include <string>

#include "app/CWiring.h"
#include "gateway/CGateway.h"
#include "platform/Platform.h"

namespace {

int runGateway(const std::string& peer, unsigned hz, unsigned seconds, bool verify) {
    // **CWiring を先に宣言すること。** 破棄は宣言の逆順なので、この順なら gateway が先に
    // 消え、そのチャネルが借りている継ぎ目（CTFromHla / CTToHla の実体）はあとから消える。
    // 逆にすると、チャネルより先に参照先が無くなる。今はチャネルのデストラクタが
    // それらを参照しないので実害は出ていないが、それに頼っている状態をなくしておく。
    app::CWiring wiring;
    gw::CGateway gateway;
    wiring.build(gateway, verify);

    if (!gateway.openAll(peer)) return 1;

    std::printf("周期        : %u Hz（1周 %.1f ms）\n", hz, 1000.0 / hz);
    std::printf("送信先      : %s\n\n",
                peer.empty() ? "なし（受信専用）" : peer.c_str());
    gateway.printPlan();
    std::printf("\n");

    gateway.run(hz, seconds);
    gateway.printSummary();

    if (!verify) return 0;

    // どのクラスを照合したかを知っているのは CWiring だけ。ここは合計しか見ない。
    const app::CWiring::TVerifyResult v = wiring.verifyResult();
    std::printf("\n往復照合    : %zu 件受信 / 不一致 %zu 件\n", v.received, v.mismatched);

    // 送ったぶんが全部戻るとは限らない — 最後の周期ぶんはループを抜けた後に届く。
    // 中身が1件でも壊れていないこと、受信が皆無でないことを合格条件にする。
    const bool ok = v.mismatched == 0 && v.received > 0;
    std::printf("\n%s\n", ok ? "OK — 受信したレコードはすべて送信時と同じバイト列でした。"
                             : "NG — 上の内訳を確認してください。");
    return ok ? 0 : 1;
}

unsigned toUnsigned(const char* s, unsigned fallback) {
    if (s == nullptr) return fallback;
    const long v = std::strtol(s, nullptr, 10);
    return v > 0 ? static_cast<unsigned>(v) : fallback;
}

void usage() {
    std::printf(
        "HLAGateway — HLA/UDP ゲートウェイの参照実装\n\n"
        "  HLAGateway loopback [Hz] [秒]          自分宛に送受信し、往復後の値を照合\n"
        "  HLAGateway run <宛先IP|none> [Hz] [秒]  実運用の形。none なら受信専用\n\n"
        "既定は 20 Hz / 3 秒。\n");
}

}  // namespace

int main(int argc, char** argv) {
    platform::initPlatform();

    const std::string mode = (argc > 1) ? argv[1] : "loopback";
    int rc = 2;

    if (mode == "loopback") {
        rc = runGateway("127.0.0.1",
                        toUnsigned(argc > 2 ? argv[2] : nullptr, 20),
                        toUnsigned(argc > 3 ? argv[3] : nullptr, 3),
                        /*verify=*/true);
    } else if (mode == "run" && argc >= 3) {
        const std::string peer = (std::string(argv[2]) == "none") ? "" : argv[2];
        rc = runGateway(peer,
                        toUnsigned(argc > 3 ? argv[3] : nullptr, 20),
                        toUnsigned(argc > 4 ? argv[4] : nullptr, 3),
                        /*verify=*/false);
    } else {
        usage();
    }

    platform::shutdownPlatform();
    return rc;
}
