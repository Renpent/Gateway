// HLA ⇄ UDP ゲートウェイ（参照実装）。
//
// この環境には RTI が無いので、HLA 側だけをスタブに置き換えてある。それ以外 — ICD コーデック、
// データグラムの組み立て、複数ポートの監視、UDP の送受信 — は本番と同じコードが動く。
// Windows と Linux のどちらでもビルドでき、同じ振る舞いをする。
//
//   HLAGateway loopback [Hz] [秒]         自分宛に送って自分で受け、往復後の値を元と照合
//   HLAGateway run <宛先IP|none> [Hz] [秒]  実運用の形。none なら受信専用

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#include "gateway/ClassIds.h"
#include "gateway/Gateway.h"
#include "hla/StubSource.h"
// 配線はここだけが全クラスを名指しする。生成物なので、ICD にクラスを足せば自動で追随する。
#include "icd/icd_classes.h"
#include "platform/Platform.h"

namespace {

using Beam = icdfom::RadarBeam;

/// レコード1件を符号化した結果。往復の照合に使う。
/// 未使用の配列末尾はゼロ埋めされる約束なので、同じ値なら必ず同じバイト列になる。
std::vector<unsigned char> encodedBytes(const Beam& b) {
    std::vector<unsigned char> out(Beam::kEncodedSize, 0);
    icd::Writer w(out.data(), out.size());
    icdfom::encode(w, b);
    if (!w.ok()) out.clear();
    return out;
}

/// 受け取った RadarBeam を「送ったはずの値」と1バイトずつ突き合わせる。
class VerifyingSink : public hla::Sink<Beam> {
public:
    void accept(const Beam& b) override {
        const Beam expected = hla::makeRadarBeam(received_);
        if (encodedBytes(b) != encodedBytes(expected)) {
            if (mismatched_ < 5) {
                std::printf("  !! %zu 件目が往復で一致しません\n", received_);
            }
            ++mismatched_;
        }
        if (received_ == 0) dumpFirst(b);
        ++received_;
    }

    [[nodiscard]] std::size_t received() const noexcept { return received_; }
    [[nodiscard]] std::size_t mismatched() const noexcept { return mismatched_; }

private:
    static void dumpFirst(const Beam& b) {
        const std::vector<unsigned char> bytes = encodedBytes(b);
        const std::size_t n = bytes.size() < 48 ? bytes.size() : 48;
        std::printf("先頭レコードのバイト列（先頭 %zu / %zu バイト・ビッグエンディアン）\n",
                    n, bytes.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (i % 16 == 0) std::printf("  %04zx  ", i);
            std::printf("%02x ", bytes[i]);
            if (i % 16 == 15) std::printf("\n");
        }
        if (n % 16 != 0) std::printf("\n");
        std::printf("\n");
    }

    std::size_t received_ = 0;
    std::size_t mismatched_ = 0;
};

/// 3クラスぶんのチャネルを組み立てる。
///
/// ここが「どのクラスをどちら向きに流すか」を決める唯一の場所。実運用では publish だけ /
/// subscribe だけのクラスがあるので、source と sink はどちらも null を許す。
struct Wiring {
    hla::StubRadarBeamSource beamSource{4};
    hla::ConstantSource<icdfom::RadioReceiver> receiverSource{1};
    hla::ConstantSource<icdfom::MinefieldData> minefieldSource{1};

    hla::CountingSink<icdfom::RadioReceiver> receiverSink;
    hla::CountingSink<icdfom::MinefieldData> minefieldSink;
    VerifyingSink beamSink;

    void build(gw::Gateway& g, bool verify) {
        g.add(std::unique_ptr<gw::Channel>(
            new gw::ClassChannel<Beam>(gw::kRadarBeam, &beamSource,
                                       verify ? static_cast<hla::Sink<Beam>*>(&beamSink)
                                              : nullptr)));
        g.add(std::unique_ptr<gw::Channel>(
            new gw::ClassChannel<icdfom::RadioReceiver>(
                gw::kRadioReceiver, &receiverSource, &receiverSink)));
        g.add(std::unique_ptr<gw::Channel>(
            new gw::ClassChannel<icdfom::MinefieldData>(
                gw::kMinefieldData, &minefieldSource, &minefieldSink)));
    }
};

int runGateway(const std::string& peer, unsigned hz, unsigned seconds, bool verify) {
    gw::Gateway gateway;
    Wiring wiring;
    wiring.build(gateway, verify);

    if (!gateway.openAll(peer)) return 1;

    std::printf("周期        : %u Hz（1周 %.1f ms）\n", hz, 1000.0 / hz);
    std::printf("送信先      : %s\n\n",
                peer.empty() ? "なし（受信専用）" : peer.c_str());
    gateway.printPlan(hz);
    std::printf("\n");

    gateway.run(hz, seconds);
    gateway.printSummary();

    if (!verify) return 0;

    std::printf("\n往復照合    : %zu 件受信 / 不一致 %zu 件\n",
                wiring.beamSink.received(), wiring.beamSink.mismatched());

    // 送ったぶんが全部戻るとは限らない — 最後の周期ぶんはループを抜けた後に届く。
    // 中身が1件でも壊れていないこと、受信が皆無でないことを合格条件にする。
    const bool ok = wiring.beamSink.mismatched() == 0 && wiring.beamSink.received() > 0;
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
    gw::initPlatform();

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

    gw::shutdownPlatform();
    return rc;
}
