// コマンド文字列の受け口。**受け取ったコマンドの処理は handle() に書く。**
//
// accept() は受信の最中に呼ばれるので積むだけにして、周期の最後（onTickEnd）に処理する。
// そのためコマンドが効くのは次の周期から。どちらも周期ループのスレッドなのでロックは要らない。

#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include "../gateway/CTMessageHandler.h"
#include "TCommand.h"

namespace app {

class CCommandHandler : public gw::CTMessageHandler<app::TCommand> {
public:
    void accept(const app::TCommand& command) override { m_pending.push_back(command.text); }

    /// 届いた順に処理して、キューを空にする。
    void onTickEnd() override {
        for (const std::string& text : m_pending) handle(text);
        m_pending.clear();
    }

private:
    /// コマンド1つぶんの処理。いまは表示するだけ。**ブロックしないこと。**
    void handle(const std::string& text) {
        std::printf("コマンド受信: \"%s\"\n", text.c_str());
    }

    std::vector<std::string> m_pending;   ///< accept が積み、onTickEnd が空にする
};

}  // namespace app
