// チャネル1つぶんのログファイル。送信も受信も同じファイルに、時刻つきで1行ずつ書く。
//
// ファイルは log/<チャネル名>_<起動時刻>.log。起動ごとに新しいファイルになる（同じ回の全チャネルで
// 起動時刻は揃う）。開けなくてもゲートウェイは止めず、書き込みを黙って省く。
//
// **捨てた・送れなかったなどのエラーは error() で書く。** ファイルに加えて全体の出力にも1行出すので、
// どのチャネルで問題が起きたかを全体の出力から辿れる。
//
// スレッド：書くのは周期ループのスレッドだけなのでロックしない。

#pragma once

#include <cstdio>
#include <string>

class CChannelLog {
public:
    CChannelLog() = default;
    ~CChannelLog();

    CChannelLog(const CChannelLog&) = delete;
    CChannelLog& operator=(const CChannelLog&) = delete;

    /// log/ が無ければ作り、ファイルを開く。開けなければ理由を全体の出力に出して false。
    bool open(const char* name);

    /// 1行書く。行頭に時刻を付け、行末の改行も付ける。
    void write(const char* fmt, ...);

    /// 1行書き、全体の出力にも "[チャネル名] ..." で出す。
    void error(const char* fmt, ...);

private:
    void put(const char* line);

    std::FILE* m_file = nullptr;    ///< 開いたログファイル（nullptr = 書かない）
    std::string m_name;             ///< チャネル名。全体の出力の行頭に付ける
};
