// 制御文字列の受け口。**受け取った制御の処理は handle() に書く。**
//
// accept() は受信の最中に呼ばれるので積むだけにして、周期の最後（onTickEnd）に処理する。
// そのため制御が効くのは次の周期から。どちらも周期ループのスレッドなのでロックは要らない。
//
// **handle() はモック。** 何をするかが決まっていないので、知っている制御を受けたら
// 「実行した」と表示して状態を切り替えるだけにしてある。本物の処理はここを差し替える。

#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include "../Core/CTMessageHandler.h"
#include "TControl.h"

namespace nonfom {

class CControlHandler : public core::CTMessageHandler<nonfom::TControl> {
public:
    void accept(const nonfom::TControl& control) override { m_pending.push_back(control.text); }

    /// 届いた順に処理して、キューを空にする。
    void onTickEnd() override {
        for (const std::string& text : m_pending) handle(text);
        m_pending.clear();
    }

private:
    /// 制御1つぶんの処理（モック）。**ブロックしないこと。**
    void handle(const std::string& text) {
        if (text == "START") {
            m_running = true;
            std::printf("[制御・モック] START: 開始した（running=1）\n");
        } else if (text == "STOP") {
            m_running = false;
            std::printf("[制御・モック] STOP: 停止した（running=0）\n");
        } else if (text == "RESET") {
            m_running = false;
            std::printf("[制御・モック] RESET: 初期状態に戻した（running=0）\n");
        } else if (text == "STATUS") {
            std::printf("[制御・モック] STATUS: running=%d\n", m_running ? 1 : 0);
        } else {
            std::printf("[制御・モック] 知らない制御: \"%s\"（何もしない）\n", text.c_str());
        }
    }

    std::vector<std::string> m_pending;   ///< accept が積み、onTickEnd が空にする
    bool m_running = false;               ///< モックの状態。START で true、STOP / RESET で false
};

}  // namespace nonfom
