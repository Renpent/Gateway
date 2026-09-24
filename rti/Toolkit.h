// HLA ツールキットの入口。**rti/ の他のファイルはツールキットをこのファイル経由でだけ見る。**
//
// 今は stub/ の偽物を指している。本物に繋ぐときは、この2行を差し替える：
//
//   #include <本物のツールキットのヘッダ>
//   namespace tk = 本物の名前空間;

#pragma once

#include "../stub/toolkit/Toolkit.h"

namespace tk = ::toolkit;
