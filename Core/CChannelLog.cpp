#include "CChannelLog.h"

#include <chrono>
#include <cstdarg>
#include <ctime>

#ifdef _WIN32
#  include <direct.h>
#else
#  include <sys/stat.h>
#endif

static const char* const kDir = "log";

static std::tm localTime(std::time_t t) {
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return tm;
}

/// ファイル名に入れる起動時刻。最初に開いたときに決め、全チャネルで同じものを使う。
static const std::string& startStamp() {
    static const std::string stamp = [] {
        const std::tm tm = localTime(std::time(nullptr));
        char buf[32];
        std::strftime(buf, sizeof buf, "%Y%m%d-%H%M%S", &tm);
        return std::string(buf);
    }();
    return stamp;
}

CChannelLog::~CChannelLog() {
    if (m_file != nullptr) std::fclose(m_file);
}

bool CChannelLog::open(const char* name) {
    if (m_file != nullptr) return true;   // openAll をやり直しても同じファイルに書き続ける
    m_name = name;
#ifdef _WIN32
    (void)_mkdir(kDir);         // 既にあれば失敗するが、それでよい
#else
    (void)mkdir(kDir, 0755);
#endif
    const std::string path = std::string(kDir) + "/" + name + "_" + startStamp() + ".log";
#ifdef _WIN32
    if (fopen_s(&m_file, path.c_str(), "w") != 0) m_file = nullptr;
#else
    m_file = std::fopen(path.c_str(), "w");
#endif
    if (m_file == nullptr) {
        std::printf("[%s] ログファイル %s を開けません（ログなしで続けます）\n", name, path.c_str());
        return false;
    }
    return true;
}

void CChannelLog::write(const char* fmt, ...) {
    if (m_file == nullptr) return;
    char line[512];
    std::va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    put(line);
}

void CChannelLog::error(const char* fmt, ...) {
    char line[512];
    std::va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    put(line);
    // 全体の出力。実環境のログ関数に差し替えるならここ。
    std::printf("[%s] %s\n", m_name.c_str(), line);
}

void CChannelLog::put(const char* line) {
    if (m_file == nullptr) return;

    using Clock = std::chrono::system_clock;
    const auto now = Clock::now();
    const std::tm tm = localTime(Clock::to_time_t(now));
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch()).count() % 1000;

    char stamp[32];
    std::strftime(stamp, sizeof stamp, "%Y-%m-%d %H:%M:%S", &tm);
    std::fprintf(m_file, "%s.%03d %s\n", stamp, static_cast<int>(ms), line);
    std::fflush(m_file);   // 落ちたときに最後の数行が残るように
}
