#include "CPlatform.h"

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <timeapi.h>
// MSVC はこれでリンクできる。MinGW / gcc は pragma comment を解さないので
// （-Werror=unknown-pragmas で落ちる）、ビルド側で -lwinmm を渡す。
#  ifdef _MSC_VER
#    pragma comment(lib, "winmm.lib")
#  endif
#endif

#ifdef _WIN32
static bool g_timerRaised = false;   ///< timeBeginPeriod が成功したか（戻すときに見る）
#endif

void CPlatform::init() {
#ifdef _WIN32
    // 65001 = UTF-8。標準出力がファイルへリダイレクトされているとコンソールが無いので
    // 失敗するが、その場合は UTF-8 のバイト列がそのまま書かれるだけで問題ない。
    ::SetConsoleOutputCP(CP_UTF8);

    g_timerRaised = (::timeBeginPeriod(1) == TIMERR_NOERROR);
#endif
}

void CPlatform::shutdown() {
#ifdef _WIN32
    if (g_timerRaised) {
        ::timeEndPeriod(1);
        g_timerRaised = false;
    }
#endif
}

