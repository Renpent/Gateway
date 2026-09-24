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
2. そのポートだけ読み切って復号し、HLA やアプリへ渡す     ← ノンブロッキング
3. HLA 側から出てきたものを符号化して送る                ← 全クラス、毎周期
4. 周期の終わりの処理（受け取ったコマンドの実行）         ← setTickEnd で登録
5. 次の周期まで sleep                                    ← 待つのはここだけ
```

**ソケットは全部ノンブロッキング**なので tick は必ず有限時間で戻る。1本のソケットが待ちに
入ると全クラスが止まってしまうため。

**全ポートを順に recvfrom で叩かない。** クラスが増えても poll は1回で、空振りの recvfrom が
積み上がらない。Windows の `WSAPoll` は POSIX の `poll` と同じ `pollfd` 構造・同じ意味で
使えるので、分岐は `net/CPoller.cpp` 冒頭の別名定義だけで済んでいる。

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

`TClassBinding::kind` で区別する。**取り違えると静かに壊れる**ので型にしてある。

| | `TClassKind::Object` | `TClassKind::Interaction` |
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
イベントがきっかり1データグラムぶん消えていた** — `CTPublisher::publish` はレコードを
データグラムに積んだ時点で true を返すので、その後の flush が断られると積んだぶんは捨てられる
のに、outbox からは「送った」ものとして削られていた（500件のバーストで 434件しか届かない）。
いまは `CTPublisher::getRecordsSent()` の増分だけを削っている。もう1つは表示の側で、オブジェクトが
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
app/            配線。「どのクラスをどちら向きに流すか」を決める場所と、アプリ側への継ぎ目（CTToApp）
gateway/        型に依存しない送受信と周期ループ。ICD のクラス用と、FOM に無い独自データ用の2種類のチャネル
hla/            HLA 側との継ぎ目。インタフェースと、RTI を呼ぶ実装
stub/           RTI が無い環境で動かすための代用品。**本番には持っていかない**
raw/            FOM に無い独自データの型（手書き）。相手が決めた形式を parse で読む
net/            ソケットと poll
platform/       コンソールの文字コードとタイマ分解能
icd/            ICDgenerator の生成物。手で編集しない（共有ファイル + object/ + interaction/）
```

依存は上から下への一方向で、**逆流させないこと**が唯一の構造上の規則。

### 本番に要るもの / 要らないもの

`stub/` に入っているのは RTI の代わりに値を作る供給元と、往復を照合する受け口だけで、
**実 RTI に繋ぐときはディレクトリごと消せる**。名前空間も `stub::` に分けてあるので、
どこで代用品を使っているかは型を見れば分かる。

`stub/` を include しているのは `app/CWiring.h` だけなので、**消したときに直すのはそのファイル
1つ**。`hla::CTRtiObjectFromHla` などの本番用の器はすでに `hla/` にあり、配線の右辺を
差し替えるだけで繋がる形にしてある。

`main.cpp` の `loopback` モードも検証用（自分宛に送って往復を照合する）。実運用で使うのは
`run` のほうで、こちらは照合せず `stub/` の照合器も繋がない。

- `gateway/` と `hla/` はソケットの型を知らない（`CUdpSocket` と `CPoller` しか見えない）
- `gateway/` `net/` は RTI の型を知らない
- `icd/` は何も知らない。OS ヘッダも RTI も include していない

OS を知っているのは **`net/*.cpp` と `platform/*.cpp` だけ**。ヘッダには winsock も
`<sys/socket.h>` も現れない（`CUdpSocket` がハンドルを `std::intptr_t` で持っているのはそのため
で、Windows の `SOCKET` と POSIX の `int fd` を1つの型で受けられる）。この向きを守っている
限り、移植で書き換わるのは `net/` `platform/` と `hla/` の実装だけになる。

ポートごとにデータの型が違うが、周期ループから見えるのは `CChannel`（`gateway/CChannel.h`）という
型を持たない抽象だけ。型が要るのは実装の内側に閉じていて、実装は2つある。

| | `CTClassChannel<T>` | `CTRawChannel<T>` |
|---|---|---|
| 何 | ICD のクラス | FOM に無い独自データ |
| 形式を決めたのは | こちら（ICD） | **相手** |
| バイト列 | 12バイトヘッダ + 固定長レコード | ヘッダ無し。1データグラム = 1メッセージ |
| 型の出どころ | ICDgenerator の生成物（`icd/`） | 手書き |
| 手元の相手 | HLA（`hla::CTFromHla` / `CTToHla`） | アプリ（`app::CTToApp`） |
| 向き | 送受信 | いまは受信のみ |

`CChannel` は以前 `binding()` で `TClassBinding`（classId・`TClassKind`・FOM 名）をそのまま見せていたが、
独自データにはそのどれも当てはまらないので、周期ループが本当に使う4つ — 名前・ポート・上限・
種別の表示 — だけを出す形にした。

### 型の名前の接頭辞

手書きのコードの型には、種類を表す接頭辞を付ける。**ファイル名も同じ名前にする。**

| 種類 | 接頭辞 | 例 |
|---|---|---|
| クラス | `C` | `CGateway`（`gateway/CGateway.h`）、`CUdpSocket`、`CCommandToApp` |
| クラステンプレート | `CT` | `CTClassChannel<T>`、`CTFromHla<T>`、`CTRtiObjectFromHla<T, Ptr>` |
| 構造体・列挙 | `T` | `TClassBinding`、`TClassKind`、`TLoopStats`、`raw::TCommand` |

振る舞いを持つものはクラス（`CWiring` は以前 `struct` だったが、配線を持って動くので
`class` にした）、値の入れ物は構造体。関数だけのファイル（`platform/Platform.h`、
`stub/RadarBeamFixture.h` など）と `main.cpp` には型が無いので、名前はそのまま。

**生成物（`icd/`）は対象外。** `icdfom::RadarBeam` のようにレコードの型名は FOM のクラス名そのままで、
ICD の行から grep で辿れることと、参照モードで HLA ツールの型名と一致することを優先している。

**メンバを返すだけの関数は `get〜`、設定するだけの関数は `set〜`。** 引数を取らず、値を返す
だけで副作用の無い関数がこれにあたる（`getName()`、`getBacklog()`、`getMaxDepth()`、
`CGateway::setTickEnd()`）。`CChannel` の仮想関数のように、実装によってメンバを返したり定数を
返したりするものも、役割がアクセサなので `get〜` に揃えてある（上書きする関数は基底と同じ名前で
なければならないので、名前は本体ではなく役割で決める）。

`get〜` にしていないもの：真偽を返す判定（`isOpen()`、`canSend()` — `is`/`can` の形のまま）、
動作（`pumpIn()`、`tick()`、`applyPending()`）、複数の受け口を集計する `verifyResult()`、
生成物 `icd/` の関数（`w.ok()`、`reader.hasNext()` など）。

表示名は型名と別。`raw::TCommand` の統計表の名前は `"Command"` のまま（ICD のクラスも、表示は
C++ の型名ではなく FOM 名）。

### 1ファイル1クラス

手書きのコードは**クラス1つにつきファイル1つ**。継承しているものは基底と派生で分ける
（`CChannel` / `CTClassChannel`、`CTFromHla` / `CTRtiObjectFromHla`）。入れ子クラスは例外で、
外側と同じファイルでよい。

**派生の名前は、基底の名前（接頭辞を除いた部分）で終える。** `CTFromHla<T>` の実装はすべて
名前が `FromHla` で終わり、`CTToHla<T>` の実装はすべて `ToHla` で終わる。`CTRtiObjectFromHla` を
見れば「RTI のオブジェクトクラスを HLA から汲み出す FromHla」と読めるので、Feed や Receiver の
ような語を別に覚えなくてよい。以前は
`RtiSnapshotFeed` / `RtiEventReceiver` だったが、Feed がどちら向きか読めないうえ、
Snapshot / Event はオブジェクト / インタラクションへの訳が毎回必要だった。

例外は1つ。`TSubscriberStats` は `CChannel::getInStats()` がテンプレートでない参照を返すので
`CTSubscriber<T>` の中に置けない（入れ子にすると `T` ごとに別の型になる）。

`Federate.h` と `icd/icd_classes.h` はクラスを持たないまとめ include で、
前者は CTFromHla/CTToHla 共通のスレッド取り決めを、後者は全生成クラスを1行で入れる役目を持つ。

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

`app/CWiring.h` が `icd/icd_classes.h`（生成物のまとめ include）を1行入れる。ICD にクラスを足せば
このヘッダが追随するので、手で並べたリストがずれることがない。**1クラスだけを扱うコードは
そのクラスのヘッダを直接** include すること（`stub/WeaponFireFixture.h` がその例）。

**classId で分岐するディスパッチャは無い。** 1クラス1ポートなので、ポートが決まればクラスが
決まり、受信側は自分の `T` で復号するだけで済む。ポートを共有していたら
`switch (classId)` が要り、そこが全型を1ファイルに集める上に ID 表と復号処理が癒着していた。

コストは実測で1クラスあたり前処理40行（`icd_types.h` を共有する薄い殻なので）。
`<vector>` 単体が 14,404 行なので、50クラスでも埋もれる。

## クラスを1つ足す

**触るのは `app/CWiring.h` だけ。** ID もポートも種別も生成物から決まるので、配線に数字は出てこない。

### ① ICDgenerator 側

GUI で対象クラスにチェック → ID/Port ダイアログで番号を振る（`連番を振る` は見えている行に効く）
→ MTU を選ぶ → C++ 生成。これで `icd/object/<Class>.h`（インタラクションなら
`icd/interaction/<Class>.h`）に `kClassId` `kPort` `kPayload`
`kIsInteraction` `kFomName` が入り、まとめ include の `icd/icd_classes.h` も追随する。
**CWiring に include を足す必要はない。**

生成が止まるのは2通りだけで、どちらもクラス名と理由を出す — 可変レコードを含む場合と、
1件が MTU に収まらない場合。

### ② メンバを2本

いま（RTI が無い状態）なら `stub/` の代用品を繋ぐ：

```cpp
// HLA → UDP
stub::CTFixtureFromHla<icdfom::Aircraft> aircraftFromHla{stub::makeAircraft, 4};
// UDP → HLA
stub::CTCountingToHla<icdfom::Aircraft>  aircraftToHla;
```

値に意味が要らないなら `stub::CTConstantFromHla<T>{1}` で足りる（既定構築の値を毎周期1件）。
バイト単位で往復照合したいときだけ `stub::CTVerifyingToHla<T>{stub::makeAircraft}` にし、
`stub/<Class>Fixture.h` に `makeAircraft(i)` を1本書く（`stub/WeaponFireFixture.h` が雛形）。
**照合器を足したら `CWiring::verifyResult()` の合計にも1つ加えること** — 配線で2箇所書くのは
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
hla::CTRtiObjectFromHla<icdfom::Aircraft, their::AircraftPtr> aircraftFromHla{
    [this] { return m_fed.getRemoteAircraft(); },   // Fetch: getRemoteXXX() を呼ぶだけ
    &toIcd                                          // Convert: 1インスタンス → 1レコード
};
hla::CTRtiObjectToHla<icdfom::Aircraft, their::AircraftPtr> aircraftToHla{
    &keyOf,                                                              // どのインスタンスか
    [this](const std::string& k) { return m_fed.registerAircraft(k); },  // 初見なら登録
    &writeBack                                                           // 属性を書いて update
};

// インタラクション
hla::CTRtiInteractionFromHla<icdfom::WeaponFire> fireFromHla;   // 上限は既定でよい
hla::CTRtiInteractionToHla<icdfom::WeaponFire>   fireToHla{
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
| `CTFixtureFromHla{make, 4}` | **1周期に何件でっち上げるか** | 4 | 試験の負荷を変えたいとき |
| `CTRtiInteractionFromHla{}` | キューの深さの上限 | `hla::kInteractionQueueDepth`（2048） | まず無い（下記） |

前者は `stub/` の中だけの話で、本番には存在しない。増やせばそのクラスの送信件数がそのまま増える。

後者は **`hla/CTRtiInteractionFromHla.h` の `kInteractionQueueDepth` 1つを全クラスで使う。
クラスごとに流量を見積もって数値を入れる運用にはしない。**

- **使わなければ1バイトも要らない。** キューは普通の `std::vector` で積まれたぶんしか確保せず、
  `drain()` が swap で持っていくので毎周期 capacity ごと 0 に戻る。この定数は天井であって
  確保量ではない。idle のクラスを50個並べても消費はゼロ（確認済み）。
- **効くのは1つの状況だけ。** `drain()` が毎周期空にするので、深さが伸びるのは「RTI が1周期の
  あいだに渡してくる件数」がこの値を超えたときだけ。20 Hz なら 50 ms に 2048 件。
- **超えても黙って壊れない。** 溢れた `push()` は false を返して `getDropped()` に載る。
  **`getMaxDepth()` が到達した最大の深さを覚えている**ので、見積もるのではなく流してから確かめられる。
  これが `getMaxQueued()` に届いていないクラスは、調整しなくてよいと分かる。

個別に数値を渡す価値が出るのは、実際に天井へ張り付いたクラスが出てきたときだけ。

### 増えたときにどこが伸びるか

```
1クラス            → CWiring.h に3行（メンバ2 + build 1）
往復照合もするなら  → + verifyResult() に1つ
本番用の変換関数    → オブジェクト3本 / インタラクション2本
```

50クラスでも `CWiring.h` は150行程度で、`main.cpp` と `gateway/` は一切変わらない。
伸びるのは変換関数のほうで、そこは ICDgenerator で生成したい（アクセサの綴りが決まり次第）。

## FOM に無いデータを受ける

FOM のクラスではない UDP データ — たとえば別のシステムが送ってくるコマンド — も同じゲートウェイで
受けられる。**形式は相手が決めていて、12バイトヘッダも固定長レコードも無い**、「このポートに
この形のデータが来る」ことだけが分かっている、という前提。

周期ループ・poll・統計表は ICD のクラスと共通で、**同じ1回の poll で両方を見る**。分けているのは
型と継ぎ目だけ。

### 足し方

**① 型を1つ手書きする。** 置き場所は `raw/<名前>.h`、名前空間は `raw`（`icd/` は再生成で丸ごと
差し替わるので、手書きを置いてはいけない）。求めるのは定数2つと関数1つ。
実例は `raw/TCommand.h`（下の「コマンド文字列」）。

```cpp
namespace raw {

struct TCommand {
    std::string text;

    static constexpr const char*   kName = "Command";   // 統計表に出る名前
    static constexpr std::uint16_t kPort = 24100;       // 受信ポート
};

/// 相手の仕様どおりに読む。形式に合わなければ false。例外は投げない。
/// len は常に 1 以上（空のデータグラムは来ない前提）。
bool parse(const unsigned char* data, std::size_t len, TCommand& out);

}  // namespace raw
```

`parse` は `TCommand` と同じ名前空間に置くこと（`CTRawChannel` が ADL で拾う）。相手の形式が
ビッグエンディアンのバイナリなら、`icd/icd_codec.h` の `icd::Reader` がそのまま使える。

**② 受け口を1つ書く。** `app::CTToApp<T>` を実装する。名前は `ToApp` で終える。
実例は `app/CCommandToApp.h`。

**③ 配線に1行。**

```cpp
addRaw<raw::TCommand>(g, &commandToApp);
```

ポートも名前も `T` の定数から決まるので、配線に数字は出てこない（ICD のクラスと同じ）。

### コマンド文字列（`raw::TCommand`）

いま配線してある独自データはこれ1つ。**1データグラム = 1コマンドで、中身は char の並び**、
ヘッダも長さの前置も無い、という仮の形式。相手の仕様が固まったら `raw/TCommand.h` の `parse` と
`kPort`（いまは 24100）を合わせる。

C/C++ の送信側でよくある3つの送り方を、どれも同じコマンドとして受ける：

| 送られてくるバイト列 | 渡る文字列 | |
|---|---|---|
| `STOP` | `"STOP"` | 文字列の長さぶんだけ |
| `STOP\0\0\0…` | `"STOP"` | `char cmd[64]` のような固定長バッファを丸ごと。**最初の NUL で切る** |
| `STOP\r\n` | `"STOP"` | 行として。**末尾の改行を落とす** |
| NUL だけ / 改行だけ | — | 形式違反。統計の「異常」に数える |

文字の中身は見ない。0x80 以上（日本語の Shift_JIS や UTF-8）もそのまま通す。何が正しいコマンドか
を判断するのは受け取った側。

**処理は `app/CCommandToApp.h` の `handle()` に書く。** いまは受け取ったことを表示するだけ：

```
コマンド受信: "START"
コマンド受信: "STOP"
```

`handle()` は `tick()` の最後に呼ばれる（受信中は積むだけ）。どのループも回っていないので、
ゲートウェイの状態を変える処理を書いてもよい。**ブロックだけはしないこと。**

手で試すなら、ゲートウェイを `run none` で立ち上げておいて、別の端末から送る：

```sh
# Linux
printf 'STOP' | nc -u -w0 127.0.0.1 24100
# Python（Windows / Linux）
python -c "import socket; socket.socket(2,2).sendto(b'STOP', ('127.0.0.1', 24100))"
```

### 気をつけること

**ヘッダが無いので、取り違えを検出する仕組みが parse しかない。** ICD のクラスなら classId が
「違うポートに向いている」を捕まえるが、独自データにはそれが無い。**形式に合わないものは
必ず `false` を返すこと** — 統計表の「異常」列に数えられ、黙って消えることはない。

**空のデータグラム（0バイト）は来ない前提。** `receive()` は 0 を「何も来ていない」の意味にも
使っているので、もし届いても parse には渡らず、数えられずに読み捨てられる（そのポートに続いて
届いていたぶんは次の周期に回る）。区別が要るようになったら `net/CUdpSocket.h` のコメントに戻し方を
書いてある。

**ポートは ICD のクラスと同じ番号空間。** 手書きの番号は ICDgenerator のダイアログの重複検出を
通らないので、`CGateway::openAll` が開く前に全チャネルを突き合わせ、重複があれば開かない。
これは実測で必要と分かったもので、**重複しても bind は失敗しない**（`SO_REUSEADDR` のため）うえに、
どちらが受け取るかが **Windows では先に bind したほう、Linux では後のほう**と逆になる。

**受信バッファは UDP の最大長（65,507 バイト）。** 相手の形式の最大長を知らなくても切り詰めは
起きない。ポート1つにつき 64 KB。

**ゲートウェイ自身の制御に使うなら、`accept` の中で直接変えないこと。** `accept` は `tick()` の
**途中**で呼ばれるので、そこでチャネルの一覧を変えたり止めたりすると回しているループが壊れる。
アプリのロジックへ渡すだけならこの制約は無い。

制御コマンドは **`accept` でキューに積むだけにして、`tick()` の最後に1箇所でまとめて反映する**：

```
tick():  [UDP 受信 → HLA へ]  [HLA から → UDP 送信]  [制御コマンドを反映]
              └ ここで積む                                └ ここで効かせる
```

効き始めるのは**次の周期から**（20 Hz なら 50 ms 後）。前半と後半のあいだで反映すれば同じ周期から
効くが、そこでチャネルやソケットを変えると、後半のループと poll の対応表がずれる。積むのも反映
するのも周期ループのスレッドなので、このキューにロックは要らない。

反映する場所は `CGateway::setTickEnd` で登録する（1つだけ）。`app/CWiring.h` がコマンドの受け口の
`applyPending()` を登録している。

前半（UDP→HLA）と後半（HLA→UDP）の順番は、**データの中継に関しては意味がほぼ無い**。2つの流れは
独立していて、UDP から来た値を HLA に出しても、それが同じ周期の `getRemoteXXX()` に返ってくる
ことはない（返るのは他のフェデレートのインスタンス）。順番が効くのは、受け取ったものが送り方を
変える制御コマンドだけ。

**送る向きはまだ無い。** 必要になったら `app::FromApp<T>` と、`parse` の逆を足す。

## 移植するときに書くもの

`hla/Federate.h` の2つのインタフェースを RTI の API で実装する。**名前に向きが入っている**
のは、ここが一番読み違えられるところだから — HLA の publish / subscribe とは逆に見える：

| | 向き | RTI 側 | UDP 側の相手 |
|---|---|---|---|
| `CTFromHla<T>` | HLA → UDP | **subscribe**。`reflectAttributeValues` で来たものを `T` に詰める | `gw::CTPublisher`（送信） |
| `CTToHla<T>` | UDP → HLA | **publish**。復元した `T` を `updateAttributeValues` で出す | `gw::CTSubscriber`（受信） |

`CTClassChannel` は `fromHla` / `toHla` のどちらも null を許す。publish だけ、subscribe だけの
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
| loopback（Windows / Linux 両方） | 往復不一致 0、16進ダンプが完全一致。オブジェクト3クラスとインタラクション1クラス |
| **Windows 送信 → Linux 受信** | classId 違い 0 / 異常 0。UDP なので取りこぼしはある |
| インタラクションの持ち越し | 送信失敗を注入して確認。500件のバーストを1件も落とさず出し切り、オブジェクト側は同じバーストを捨てる |
| キューの深さの上限 | idle なら確保ゼロ、`getMaxDepth()` が山を覚える、天井に当たれば `getDropped()` に出る |
| RTI の器4つ | 偽ツールキットで実体化して動作確認。実スレッドから 5000件 push して順序・取りこぼしとも一致 |
| 独自データの受信 | 見本の形式で確認。順番どおり届く、60,000 バイトも切り詰めない、形式違反を数える、ICD のクラスと同じ poll で動く、ポート重複を断る |
| コマンド文字列 | 実バイナリを `run none` で立ち上げ、別プロセスから送信（Windows / Linux）。長さぶん・改行付き・NUL 詰め・日本語の4件を処理、NUL だけの1件を異常に数え、空のデータグラムは数えずに読み捨てる（後続は全部届く）。受信中は積むだけで、処理が `tick()` の最後に回ることも確認 |
| ポート重複 | 重複ポートは開く前に断る（Windows / Linux で「どちらが受け取るか」が逆なことも実測） |

下6つは**リポジトリ外のハーネス**で回している（コマンド文字列は実バイナリでも確認）。クラステンプレートは実体化しないと中身が
型検査されず、送信失敗もループバックでは起こせないので、本体に入れずに別立てにしてある。

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

```
icd/
  icd_codec.h        ランタイム（全クラス共通）
  icd_types.h/.cpp   選んだクラスが使う列挙・レコード・配列の型
  icd_classes.h      全クラスのまとめ include（配線用）
  object/            オブジェクトクラス。1クラスにつき <Class>.h / <Class>.cpp
  interaction/       インタラクションクラス。同上
```

共有ファイルはルートに、クラスは FOM 上の種類で `object/` と `interaction/` に分けて置かれる。
クラスのヘッダは1つ上の `../icd_types.h` を include する。**生成器は既存のファイルを消さない**ので、
選択から外したクラスや、フォルダを分ける前（すべてルートに並んでいた頃）のファイルは手で消すこと。
ID・ポート・MTU は生成物が持っている（各クラスの `kClassId` / `kPort` / `kPayload`）ので、
ゲートウェイ側に写す作業は無い。`app/CWiring.h` の `bindingOf<T>()` がそこから組み立てる。

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

**種別はここで選んでいない。** `TClassKind` は生成された `kIsInteraction` から決まるので、
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
