// HLA ツールキットの入口。**Wiring/FOM/ の他のファイルはツールキットをこのファイル経由でだけ見る。**
//
// 今は Stub/ の偽物を指している。本物に繋ぐときは、この2行を差し替える：
//
//   #include <本物のツールキットのヘッダ>
//   namespace tk = 本物の名前空間;
//
// **tk は相手（ツールキット）の名前空間を指す別名で、こちらの名前空間ではない。** こちらの手書きの
// コードは名前空間を使わない（相手のヘッダがグローバルで using namespace するとぶつかるため）。
// 相手のヘッダがグローバルで using namespace していても、tk:: と書いた所はそのまま通る。

#pragma once

#include "../../Stub/Toolkit/Toolkit.h"

namespace tk = ::toolkit;
