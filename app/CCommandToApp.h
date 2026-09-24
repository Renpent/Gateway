// コマンド文字列の受け口。**受け取ったコマンドの処理はここの handle() に書く。**
//
// 受け取るのと処理するのを分けてある：
//
//   accept()        tick() の途中（UDP 受信の最中）に呼ばれる。**キューに積むだけ。**
//   applyPending()  tick() の最後に呼ばれる（CGateway::setTickEnd で登録）。積まれた順に handle する
//
// 分けているのは、コマンドがゲートウェイの動きを変えるものでも安全にするため。受信の最中に
// 状態を変えると、回しているループが壊れる。**効くのは次の周期から**（20 Hz なら 50 ms 後）。
// 積むのも処理するのも周期ループのスレッドなので、キューにロックは要らない。

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "../raw/TCommand.h"
#include "TCToApp.h"

namespace app {

class CCommandToApp : public TCToApp<raw::TCommand> {
public:
    /// tick() の途中で呼ばれる。ここでは積むだけで、何も実行しない。
    void accept(const raw::TCommand& command) override {
        m_pending.push_back(command.text);
        ++m_received;
    }

    /// tick() の最後に呼ぶ。受け取った順に処理して、キューを空にする。
    void applyPending() {
        for (const std::string& text : m_pending) {
            handle(text);
            ++m_handled;
        }
        m_pending.clear();
    }

    [[nodiscard]] std::uint64_t received() const noexcept { return m_received; }
    [[nodiscard]] std::uint64_t handled() const noexcept { return m_handled; }
    [[nodiscard]] std::size_t pending() const noexcept { return m_pending.size(); }

private:
    /// **コマンド1つぶんの処理。ここに中身を書く。**
    ///
    /// いまは受け取ったことを表示するだけ。ここは周期ループのスレッドで、どのループも
    /// 回っていないので、ゲートウェイの状態を変える処理を書いてもよい。ただし**ブロックしない
    /// こと** — ここで待つと、その周期ぶん全ポートが遅れる。
    void handle(const std::string& text) {
        std::printf("コマンド受信: \"%s\"\n", text.c_str());
    }

    std::vector<std::string> m_pending;  ///< accept が積み、applyPending が空にする
    std::uint64_t m_received = 0;        ///< 受け取った件数（形式違反は TCRawChannel 側で数える）
    std::uint64_t m_handled = 0;         ///< 処理した件数
};

}  // namespace app
