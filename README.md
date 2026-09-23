# HLAGateway

HLA と UDP を橋渡しするゲートウェイの**参照実装**。
[ICDgenerator](https://github.com/Renpent/ICDgenerator) が FOM から生成する ICD の
とおりにバイト列を組み立てて送受信する。

**この環境に RTI は無い。** HLA 側だけをスタブに差し替えてあり、それ以外
（ICD コーデック・複数ポートの監視・データグラムの組み立て・UDP 送受信・周期実行）は
本番と同じコードが動く。**Windows と Linux のどちらでもビルドでき、同じ振る舞いをする。**

## 動かす

```
HLAGateway loopback [Hz] [秒]           自分宛に送受信し、往復後の値を元と1バイトずつ照合
HLAGateway run <宛先IP|none> [Hz] [秒]  実運用の形。none なら受信専用
```

既定は 20 Hz / 3 秒。

`loopback` は往復したレコードを**再符号化して元とバイト比較**する。可変長配列の未使用部分が
ゼロ埋めされる約束のおかげで、同じ値なら必ず同じバイト列になるので、この比較が成立する。

## 1周（tick）でやること

```
1. poll に待機時間 0 で聞き、来ているポートを知る        ← システムコール1回
2. そのポートだけ読み切って復号し、HLA 側へ渡す           ← ノンブロッキング
3. HLA 側から出てきたものを符号化して送る                ← 全クラス、毎周期
4. 次の周期まで sleep                                    ← 待つのはここだけ
```

**ソケットは全部ノンブロッキング**なので tick は必ず有限時間で戻る。1本のソケットが待ちに
入ると全クラスが止まってしまうため。

**全ポートを順に recvfrom で叩かない。** クラスが増えても poll は1回で、空振りの recvfrom が
積み上がらない。Windows の `WSAPoll` は POSIX の `poll` と同じ `pollfd` 構造・同じ意味で
使えるので、分岐は `net/Poller.cpp` 冒頭の別名定義だけで済んでいる。

### 周期がずれたら

`next += period` で**絶対時刻**を刻む。起床が多少遅れても平均レートは正確に保たれる。
1周の処理が周期を超えたときだけ、取り戻そうとせず基準を打ち直す（詰めて回すと悪化するので、
落とした周期は諦めて溜め込まないほうを選ぶ）。回数は `超過` として出る。

### 送信レートは周期そのもの

クラスごとの間引きは無い。`tick()` を叩く側が決めた周期で、全クラスが毎周期出る。
ICD の `Rate` 列は「受信側が期待してよい更新頻度」を書く参考値で、コードは読まない。

以前は `Rate` 列を「何周に1回送るか」に変換して間引いていた。外した理由は、本番では周期処理側が
`tick()` を呼ぶので全クラスがその周期に沿うことになり、クラスごとの値を持つ意味が無くなったため。
帯域は増える — 測ったときの3クラスで間引きありの 7.6 KB/s が 49 KB/s になり、ほぼ `MinefieldData`（1件
1902 B）が毎周期出るぶん — が、それを承知のうえでの判断。必要になったら `Rate` を生成に戻せばよい。

### オブジェクトとインタラクションは backlog の扱いが正反対

`ClassBinding::kind` で区別する。**取り違えると静かに壊れる**ので型にしてある。

| | `ClassKind::Object` | `ClassKind::Interaction` |
|---|---|---|
| 何 | オブジェクトクラスの属性 | インタラクションクラス |
| HLA 側 | `getRemoteXXX()` でその瞬間の全体像が取れる | コールバックでイベントとして飛んでくる |
| 落ちたら | 次の周期が現在値を運ぶ。**自己回復する** | **二度と戻らない** |
| 送り残し | 次の周期には古い。**捨てて撮り直す** | **持ち越す** |
| 排他 | 不要（周期ループが自分で読みに行く） | RTI スレッドとの間にキュー1枚 |

**1周期あたりの件数上限は置いていない。** 送信は渡されたものをその周期で出し切り、受信は届いて
いたものをその周期で読み切る。止まるのは、送信ならソケットが受け付けなかったとき、受信なら
受信バッファが空になったときだけ。出し切れなかったぶんは `積み残し` 列に、そうなった周期の数は
`持ち越し` 列に出る。同じ数字でも意味は正反対で、イベントならその回数だけ次の周期へ繰り越して
おり、状態なら同じ回数だけ捨てている。

**この分岐は長いあいだ一度も実行されていなかった**（インタラクションのクラスが無かったため）。
`WeaponFire` を通したときに、そこに2つ不具合が出た。**ソケットに断られた周期で、持ち越すはずの
イベントがきっかり1データグラムぶん消えていた** — `Publisher::publish` はレコードを
データグラムに積んだ時点で true を返すので、その後の flush が断られると積んだぶんは捨てられる
のに、outbox からは「送った」ものとして削られていた（500件のバーストで 434件しか届かない）。
いまは `Publisher::recordsSent()` の増分だけを削っている。もう1つは表示の側で、オブジェクトが
残りを捨てた周期に `積み残し` を書き戻しておらず、**捨てたはずの件数を残り続けているかのように
報告していた**。どちらもループバックでは出ない — OS はループバックの送信を断らないので、
検出には送信失敗を人為的に起こす必要がある。

以前は両方向に上限を置いていた（受信64・送信8データグラム）が外した。**送信側の上限はオブジェクトに
対して有害だった** — 次の周期の頭で残りを捨てる設計なので、上限を超えた末尾が毎周期おなじように
落ち続け、インスタンス数が上限を超えたフェデレーションでは末尾が永久に送られなかった。
**受信側は持ち越しがログを濁す** — 「このレコードはどの周期に届いたのか」が突き合わせで曖昧になる。

**インタラクションで積み残しが減らないなら、そのクラスは供給に追いついていない。** 上げるのは周期そのものか
MTU（＝1発の件数）で、放っておくとキューが伸び続ける。

## 層

```
main.cpp        起動とモード分岐だけ
app/            配線。「どのクラスをどちら向きに流すか」を決める場所
gateway/        クラス非依存の送受信と周期ループ。ID・ポート・MTU は生成物の定数から導き、手書きの数字は無い
hla/            HLA 側との継ぎ目。インタフェースと、RTI を呼ぶ実装
stub/           RTI が無い環境で動かすための代用品。**本番には持っていかない**
net/            ソケットと poll
platform/       コンソールの文字コードとタイマ分解能
icd/            ICDgenerator の生成物。手で編集しない
```

依存は上から下への一方向で、**逆流させないこと**が唯一の構造上の規則。

### 本番に要るもの / 要らないもの

`stub/` に入っているのは RTI の代わりに値を作る供給元と、往復を照合する受け口だけで、
**実 RTI に繋ぐときはディレクトリごと消せる**。名前空間も `stub::` に分けてあるので、
どこで代用品を使っているかは型を見れば分かる。

`stub/` を include しているのは `app/Wiring.h` だけなので、**消したときに直すのはそのファイル
1つ**。`hla::RtiObjectFromHla` などの本番用の器はすでに `hla/` にあり、配線の右辺を
差し替えるだけで繋がる形にしてある。

`main.cpp` の `loopback` モードも検証用（自分宛に送って往復を照合する）。実運用で使うのは
`run` のほうで、こちらは照合せず `stub/` の照合器も繋がない。

- `gateway/` と `hla/` はソケットの型を知らない（`UdpSocket` と `Poller` しか見えない）
- `gateway/` `net/` は RTI の型を知らない
- `icd/` は何も知らない。OS ヘッダも RTI も include していない

OS を知っているのは **`net/*.cpp` と `platform/*.cpp` だけ**。ヘッダには winsock も
`<sys/socket.h>` も現れない（`UdpSocket` がハンドルを `std::intptr_t` で持っているのはそのため
で、Windows の `SOCKET` と POSIX の `int fd` を1つの型で受けられる）。この向きを守っている
限り、移植で書き換わるのは `net/` `platform/` と `hla/` の実装だけになる。

クラスごとにレコード型が違うが、周期ループから見えるのは `Channel`（`gateway/Channel.h`）という
型を持たない抽象だけ。型が要るのは `ClassChannel<T>`（`gateway/ClassChannel.h`）の内側
— 生成コーデックを呼ぶ場所 — に閉じている。

### 1ファイル1クラス

手書きのコードは**クラス1つにつきファイル1つ**。継承しているものは基底と派生で分ける
（`Channel` / `ClassChannel`、`FromHla` / `RtiObjectFromHla`）。入れ子クラスは例外で、
外側と同じファイルでよい。

**派生の名前は基底で終える。** `FromHla<T>` の実装はすべて `〜FromHla`、`ToHla<T>` の実装は
すべて `〜ToHla`。`RtiObjectFromHla` を見れば「RTI のオブジェクトクラスを HLA から汲み出す
FromHla」と読めるので、Feed や Receiver のような語を別に覚えなくてよい。以前は
`RtiSnapshotFeed` / `RtiEventReceiver` だったが、Feed がどちら向きか読めないうえ、
Snapshot / Event はオブジェクト / インタラクションへの訳が毎回必要だった。

例外は1つ。`SubscriberStats` は `Channel::inStats()` がテンプレートでない参照を返すので
`Subscriber<T>` の中に置けない（入れ子にすると `T` ごとに別の型になる）。

`Federate.h` と `icd/icd_classes.h` はクラスを持たないまとめ include で、
前者は FromHla/ToHla 共通のスレッド取り決めを、後者は全生成クラスを1行で入れる役目を持つ。

### メンバ変数の書き方

手書きクラスのメンバは **`m_` 接頭辞**を付け、**宣言行の末尾に `///<` で何の値かを短く**書く。

```cpp
std::uint32_t m_classId;            ///< 期待する classId。違えば wrongClass として捨てる
std::vector<unsigned char> m_buf;   ///< 受信バッファ。長さは payload
```

**生成コードのレコードのフィールドは `m_` を付けない。** FOM のフィールド名そのままであることが、
ICD の行から grep で辿れる条件であり、参照モードでツール側のメンバ名と一致する条件でもある。
そちらには `///< FOM: <名前> : <型>` が生成時に付く。

ソースは手書き・生成とも **UTF-8（BOM なし）+ CRLF**。BOM は付けていない — MSVC には
`.vcxproj` と `CMakeLists.txt` の両方から `/utf-8` を渡してあり、gcc は元から UTF-8 として
読むので、BOM が要る経路が無い。`README.md` と `CMakeLists.txt` だけは LF。

### 全クラスを名指しするのは配線1箇所だけ

`app/Wiring.h` が `icd/icd_classes.h`（生成物のまとめ include）を1行入れる。ICD にクラスを足せば
このヘッダが追随するので、手で並べたリストがずれることがない。**1クラスだけを扱うコードは
そのクラスのヘッダを直接** include すること（`stub/WeaponFireFixture.h` がその例）。

**classId で分岐するディスパッチャは無い。** 1クラス1ポートなので、ポートが決まればクラスが
決まり、受信側は自分の `T` で復号するだけで済む。ポートを共有していたら
`switch (classId)` が要り、そこが全型を1ファイルに集める上に ID 表と復号処理が癒着していた。

コストは実測で1クラスあたり前処理40行（`icd_types.h` を共有する薄い殻なので）。
`<vector>` 単体が 14,404 行なので、50クラスでも埋もれる。

## クラスを1つ足す

**触るのは `app/Wiring.h` だけ。** ID もポートも種別も生成物から決まるので、配線に数字は出てこない。

### ① ICDgenerator 側

GUI で対象クラスにチェック → ID/Port ダイアログで番号を振る（`連番を振る` は見えている行に効く）
→ MTU を選ぶ → C++ 生成。これで `icd/<Class>.h` に `kClassId` `kPort` `kPayload`
`kIsInteraction` `kFomName` が入り、まとめ include の `icd/icd_classes.h` も追随する。
**Wiring に include を足す必要はない。**

生成が止まるのは2通りだけで、どちらもクラス名と理由を出す — 可変レコードを含む場合と、
1件が MTU に収まらない場合。

### ② メンバを2本

いま（RTI が無い状態）なら `stub/` の代用品を繋ぐ：

```cpp
// HLA → UDP
stub::FixtureFromHla<icdfom::Aircraft> aircraftFromHla{stub::makeAircraft, 4};
// UDP → HLA
stub::CountingToHla<icdfom::Aircraft>  aircraftToHla;
```

値に意味が要らないなら `stub::ConstantFromHla<T>{1}` で足りる（既定構築の値を毎周期1件）。
バイト単位で往復照合したいときだけ `stub::VerifyingToHla<T>{stub::makeAircraft}` にし、
`stub/<Class>Fixture.h` に `makeAircraft(i)` を1本書く（`stub/WeaponFireFixture.h` が雛形）。
**照合器を足したら `Wiring::verifyResult()` の合計にも1つ加えること** — 配線で2箇所書くのは
ここだけ。

### ③ `build()` に1行

```cpp
add<icdfom::Aircraft>(g, &aircraftFromHla, &aircraftToHla);
```

片方向のクラスは、要らないほうのメンバを作らず `nullptr` を渡す。`T` は必ず明示すること
（`nullptr` からは型が決まらない）。

```cpp
add<icdfom::Foo>(g, &fooFromHla, nullptr);    // 送信のみ
add<icdfom::Bar>(g, nullptr,     &barToHla);  // 受信のみ
```

### 本番（RTI）での形

`stub::` が `hla::Rti...` に変わる。**オブジェクトとインタラクションで形が違う**のは、
前者が毎周期こちらから取りに行く（pull）のに対し、後者は RTI のコールバックから
押し込まれる（push）から。

```cpp
// オブジェクト
hla::RtiObjectFromHla<icdfom::Aircraft, their::AircraftPtr> aircraftFromHla{
    [this] { return m_fed.getRemoteAircraft(); },   // Fetch: getRemoteXXX() を呼ぶだけ
    &toIcd                                          // Convert: 1インスタンス → 1レコード
};
hla::RtiObjectToHla<icdfom::Aircraft, their::AircraftPtr> aircraftToHla{
    &keyOf,                                                              // どのインスタンスか
    [this](const std::string& k) { return m_fed.registerAircraft(k); },  // 初見なら登録
    &writeBack                                                           // 属性を書いて update
};

// インタラクション
hla::RtiInteractionFromHla<icdfom::WeaponFire> fireFromHla;   // 上限は既定でよい
hla::RtiInteractionToHla<icdfom::WeaponFire>   fireToHla{
    [this](const icdfom::WeaponFire& r) { m_fed.sendWeaponFire(toRti(r)); }
};
```

`Fetch` と `Create` だけ `std::function`（フェデレートを掴む必要がある）、残りは状態を持たない
自由関数の想定で関数ポインタ。**クラスごとに書く中身はこの関数だけ** — オブジェクトなら
`toIcd` / `keyOf` / `writeBack` の3本、インタラクションなら `toIcd` / `toRti` の2本。

`fireFromHla` は Fetch を取らない代わりに、**`receiveInteraction` のコールバック側にこの
ポインタを渡して、その中で `push()` を呼ぶ**配線が別に要る。ゲートウェイで唯一ロックを持つ
場所がここ。

### コンストラクタの数字

**どちらも書かなくてよい。** 見た目は似ているが意味は別物で、片方は試験の都合、
もう片方は全クラス共通の定数になっている。

| | 何の数 | 既定 | 書くとき |
|---|---|---|---|
| `FixtureFromHla{make, 4}` | **1周期に何件でっち上げるか** | 4 | 試験の負荷を変えたいとき |
| `RtiInteractionFromHla{}` | キューの深さの上限 | `hla::kInteractionQueueDepth`（2048） | まず無い（下記） |

前者は `stub/` の中だけの話で、本番には存在しない。増やせばそのクラスの送信件数がそのまま増える。

後者は **`hla/RtiInteractionFromHla.h` の `kInteractionQueueDepth` 1つを全クラスで使う。
クラスごとに流量を見積もって数値を入れる運用にはしない。**

- **使わなければ1バイトも要らない。** キューは普通の `std::vector` で積まれたぶんしか確保せず、
  `drain()` が swap で持っていくので毎周期 capacity ごと 0 に戻る。この定数は天井であって
  確保量ではない。idle のクラスを50個並べても消費はゼロ（確認済み）。
- **効くのは1つの状況だけ。** `drain()` が毎周期空にするので、深さが伸びるのは「RTI が1周期の
  あいだに渡してくる件数」がこの値を超えたときだけ。20 Hz なら 50 ms に 2048 件。
- **超えても黙って壊れない。** 溢れた `push()` は false を返して `dropped()` に載る。
  **`maxDepth()` が到達した最大の深さを覚えている**ので、見積もるのではなく流してから確かめられる。
  これが `maxQueued()` に届いていないクラスは、調整しなくてよいと分かる。

個別に数値を渡す価値が出るのは、実際に天井へ張り付いたクラスが出てきたときだけ。

### 増えたときにどこが伸びるか

```
1クラス            → Wiring.h に3行（メンバ2 + build 1）
往復照合もするなら  → + verifyResult() に1つ
本番用の変換関数    → オブジェクト3本 / インタラクション2本
```

50クラスでも `Wiring.h` は150行程度で、`main.cpp` と `gateway/` は一切変わらない。
伸びるのは変換関数のほうで、そこは ICDgenerator で生成したい（アクセサの綴りが決まり次第）。

## 移植するときに書くもの

`hla/Federate.h` の2つのインタフェースを RTI の API で実装する。**名前に向きが入っている**
のは、ここが一番読み違えられるところだから — HLA の publish / subscribe とは逆に見える：

| | 向き | RTI 側 | UDP 側の相手 |
|---|---|---|---|
| `FromHla<T>` | HLA → UDP | **subscribe**。`reflectAttributeValues` で来たものを `T` に詰める | `gw::Publisher`（送信） |
| `ToHla<T>` | UDP → HLA | **publish**。復元した `T` を `updateAttributeValues` で出す | `gw::Subscriber`（受信） |

`ClassChannel` は `fromHla` / `toHla` のどちらも null を許す。publish だけ、subscribe だけの
クラスが実運用にはあるため。

**排他が要るのはこの継ぎ目だけ。** ゲートウェイ本体は単一スレッドで、`tick()` が受信を全部
片付けてから送信する直列実行になっている（UDP ソケットは全二重で、送信用と受信用のバッファも
別々に持っているので、そもそも競合しない）。

インタラクションの実装は RTI のコールバックスレッドから来るので、キューを1枚挟んでそこに
lock を置く。**変換はコールバックの中で済ませること** — RTI のパラメータハンドルや値バッファは
コールバックの間しか有効でないのが普通で、ポインタを持ち越して後で復号することはできない。
復号済みの構造体をキューに積む。

`drain()` / `accept()` の中から RTI を呼び返さないこと。コールバック中の再入を禁じている
RTI 実装があり、握ったままだと簡単にデッドロックする。

実装側で必要になるがインタフェースに現れないもの：

- **インスタンスハンドルと `T` の対応表。** HLA はオブジェクトインスタンス単位、ICD はレコード単位
- **属性ハンドルとメンバの対応。** 生成コードはメンバの並び順を保証するが、ハンドルは実行時に取る
- **部分更新の扱い。** ICD は常に全属性ぶんの箱を送るので、届いていない属性を前回値で埋めるか
  既定値で埋めるかを決める必要がある

`ICDgenerator` の**参照モード**（`ExternalNamespace` を指定）を使うと、`icd_types.h` が型定義を
やめて HLA ツールの生成ヘッダを include するようになる。本番では型の定義がツール側と二重に
ならないので、そちらを使うことになるはず。

## ビルド

| | |
|---|---|
| Windows | `HLAGateway.sln`（VS2022 / v143 / x64） |
| Linux | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` |

**C++17 が必須。** 生成コードが inline 変数・変数テンプレート・`if` 初期化子を使っている。
VS の既定は C++14 なので `<LanguageStandard>stdcpp17</LanguageStandard>` を入れてある。

リンクするもの：Windows は `ws2_32`（ソケット）と `winmm`（`timeBeginPeriod`）、
Linux は `Threads::Threads`。

### 文字コード

**ソースも文字列リテラルも UTF-8 に統一している。** MSVC には `/utf-8` を渡し、実行時に
`SetConsoleOutputCP(CP_UTF8)` でコンソール側も合わせる（`platform/Platform.cpp`）。

これで両 OS の実行文字セットが揃うので、**CP932 に無い文字（em ダッシュなど）もリテラルに
書ける**。`/utf-8` を外すと MSVC の実行文字セットが CP932 に戻り、そういう文字は C4566 になる。

**BOM は付けていない。** コンパイラ側は元から要らない（`/utf-8` と gcc の既定）。以前
付けていたのは「BOM 無しの UTF-8 を Visual Studio で開いて保存し直すと CP932 に落ちることが
ある」ための保険だったが、それは `.editorconfig` の `charset = utf-8` で直接防げる。
保険としての BOM より、編集器に文字コードを明示するほうが確実で、差分にも出ない。

## 確認済み

| | 結果 |
|---|---|
| `g++ -std=c++17 -Wall -Wextra -Werror -pedantic`（MinGW） | 警告なし |
| MSBuild v143 `/std:c++17 /W4` | 警告なし |
| g++ 11.4 + CMake（WSL Ubuntu 22.04） | 警告なし |
| loopback（Windows / Linux 両方） | 往復不一致 0、16進ダンプが完全一致 |
| **Windows 送信 → Linux 受信** | classId 違い 0 / 異常 0。UDP なので取りこぼしはある |

起床精度（20 Hz = 50 ms 周期、`起床の遅れ` の実測）：

| | 平均 | 最大 |
|---|---:|---:|
| Linux (WSL) | 0.11 ms | 0.17 ms |
| Windows (MSVC) | 0.78 ms | 1.66 ms |
| Windows (MinGW) | 8.4 ms | 13.8 ms |

MinGW だけ悪いのは、libstdc++ の `sleep_until` の実装が `timeBeginPeriod(1)` の効果を
受けないため。**周期そのものはどれも正確**（絶対時刻で刻んでいるので、位相がぶれるだけで
レートはずれない）。本番が Linux なら気にしなくてよいし、Windows なら MSVC で建てること。

## `icd/` の再生成

ICD が変わったら、生成物を**丸ごと差し替える**。手を入れた変更は次の再生成で消える。

ICDgenerator の GUI で対象クラスを選び、C++ 生成の出力先をこのリポジトリの `icd/` にする。
クラスを増減したら `HLAGateway.vcxproj` と `CMakeLists.txt` のソース一覧を合わせること。
ID・ポート・MTU は生成物が持っている（各クラスの `kClassId` / `kPort` / `kPayload`）ので、
ゲートウェイ側に写す作業は無い。`app/Wiring.h` の `bindingOf<T>()` がそこから組み立てる。

### RPR FOM で生成できないクラスがある

`BaseEntity` 配下（Aircraft, GroundVehicle, Munition …）は判別子付きの可変レコード
（`SpatialVariantStruct` など）を含むため、生成器が拒否する。**実運用の FOM には可変レコードが
無い**ので本番では問題にならないが、この参照実装で RPR FOM を使う都合上、可変レコードを
含まない29クラスから選んである。

現在の選択：

| クラス | classId | port | 1件 | 1発 | 上限 | 種別 |
|---|---:|---:|---:|---:|---:|---|
| `EmitterBeam.RadarBeam` | 1 | 24001 | 139 B | 63 件 | 8900 B | 状態 |
| `EmbeddedSystem.RadioReceiver` | 2 | 24002 | 62 B | 143 件 | 8900 B | 状態 |
| `EmbeddedSystem.MinefieldData` | 3 | 24003 | 1902 B | 4 件 | 8900 B | 状態 |
| `WeaponFire` | 4 | 24004 | 134 B | 66 件 | 8900 B | イベント |

MTU は ICDgenerator の GUI で 1500 か 9000 を選ぶ（全クラス共通）。この表は 9000。

**種別はここで選んでいない。** `ClassKind` は生成された `kIsInteraction` から決まるので、
`WeaponFire` がイベントになるのは FOM でインタラクションだからであって、配線に書いた結果ではない。

`WeaponFire` を選んだのは、公開可能な73インタラクションのうち RPR FOM で最も素直なものだから。
`MunitionDetonation` が対になる候補だが `ParameterValueVariantStruct` を含むので生成器が拒否する
（73件中この1件だけ）。

**`MinefieldData` は 1500 MTU に収まらない。** 1件 1902 B で、1500 MTU のペイロード 1400 B では
1件も送れない。同種のことは配列上限を上げれば簡単に起きるので、三段で止まる：
ICDgenerator が MTU 1500 のまま生成しようとするとクラス名と超過量を出して**拒否**し、
生成ヘッダの `static_assert` が**コンパイル時**に止め、ゲートウェイは**起動時**にも理由付きで弾く
（毎周期黙って捨てられるのが一番困る）。

## バイト列の確認

`loopback` が先頭レコードを16進で出す。ICD の各シートと突き合わせられる。

```
  0000  00 00 00 00 3e 80 00 00 80 00 00 00 3e 00 00 00
  0010  01 00 00 64 42 2a 00 00 50 0a 94 af 00 06 45 4d
  0020  49 54 2d 30 00 00 00 00 00 00 00 00 00 00 00 00
```

| バイト | 値 | 意味 |
|---|---|---|
| `3e 80 00 00` | 0.25f | ビッグエンディアン（LE なら `00 00 80 3e`） |
| `00 64` | 100 | `BeamParameterIndex` |
| `00 06` | 6 | `EmitterSystemIdentifier_Count` |
| `45 4d 49 54 2d 30` | `"EMIT-0"` | 中身 |
| `00 …` | | 上限16文字までのゼロ埋め |
