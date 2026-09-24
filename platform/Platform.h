// OS ごとの初期化。どちらも Windows でだけ効き、Linux では何もしない。

#pragma once

namespace platform {

/// main の先頭で1回呼ぶ。Windows では次の2つを行う：
///   1. コンソールを UTF-8 にする（ソースも文字列リテラルも UTF-8 なので、合わせないと化ける）
///   2. タイマ分解能を 1 ms に上げる（既定の 15.6 ms だと 50 ms 周期の起床が平均 7 ms 遅れる）
void initPlatform();

/// 終了時に呼ぶ。上げたタイマ分解能を戻す（システム全体の設定なので上げっぱなしにしない）。
void shutdownPlatform();

}  // namespace platform
