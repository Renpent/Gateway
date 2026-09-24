# HLAGateway（簡易版）

HLA と UDP を橋渡しするゲートウェイの参照実装。
[ICDgenerator](https://github.com/Renpent/ICDgenerator) が FOM から生成する ICD のとおりに
バイト列を組み立てて送受信する。

**この版は動作に要るものだけを残した簡易版。** 送受信の件数・周期の遅れ・キューの深さといった
監視と、送れなかったインタラクションの持ち越しは入っていない。送れなかったもの・形式に合わない
ものは、数えずに捨てる。

**この環境に RTI は無い。** HLA 側の送信元は `stub/` の代用品。受信した分は、サンプルの
`Designator` だけモックが表示し、ほかは捨てている。
Windows と Linux のどちらでもビルドでき、同じように動く。

## 動かす

```
HLAGateway <宛先IP|none> [Hz] [秒]
```

- 宛先を `none` にすると受信専用
- Hz を省くと 20
- 秒を省くか 0 にすると止まらない

## 1周（tick）でやること

```
1. poll（待機時間 0）で、データが来ているポートを知る
2. そのポートだけ読み切って復号し、HLA やアプリへ渡す
3. HLA 側から出てきた分を符号化して送る      ← 全クラス、毎周期
4. addTickEnd で登録した処理を、足した順に呼ぶ
5. 次の周期まで sleep                        ← 待つのはここだけ
```

- **ソケットは全部ノンブロッキング。** 1本が待ちに入ると全クラスが止まるため
- **周期は絶対時刻で刻む。** 起床が多少遅れても、平均の周期はずれない。1周が周期を超えたときは
  取り戻そうとせず、基準を打ち直す
- **送信レートは周期そのもの。** クラスごとの間引きは無い。ICD の `Rate` 列は参考値で、コードは読まない
- **1周期に出す件数の上限は無い。** 渡された分はその周期で全部送り、届いていた分はその周期で全部読む。
  送れなかった分は捨て、次の周期には持ち越さない

## 層

```
main.cpp          起動
app/              配線（スタブ用 CWiring / 本番用 CRtiWiring）と、FOM に無い独自データの型・処理
rti/              本番の変換関数（ツールキットの型 ⇄ ICD の型）と、world を持つ CDb
gateway/          周期ループ（CGateway）とチャネル（ICD のクラス用・独自データ用）
  udp/            ソケット、poll、データグラムの送信（CTUdpSender）と受信（CTUdpReceiver）
  hla/            HLA 側の継ぎ目（CTFromHla / CTToHla）と、RTI を呼ぶ器
stub/             RTI の代わりに値を作る代用品。**本番には持っていかない**
  toolkit/        偽の HLA ツールキット
platform/         コンソールの文字コードとタイマ分解能（Windows のみ）
icd/              ICDgenerator の生成物。手で編集しない
```

**依存は上から下への一方向。**

- `gateway/` は `app/` を見ない
- `udp/` と `hla/` は互いを知らない
- OS を知っているのは `gateway/udp/*.cpp` と `platform/*.cpp` だけ
- ツールキットの型が出てくるのは `rti/` と、それを繋ぐ `app/CRtiWiring.h` だけ

**名前空間はフォルダで決まる。**

| フォルダ | 名前空間 |
|---|---|
| `gateway/` | `gw` |
| `gateway/udp/` | `udp` |
| `gateway/hla/` | `hla` |
| `app/` | `app` |
| `rti/` | `rti`（ツールキットは別名 `tk`） |
| `stub/` | `stub` |
| `stub/toolkit/` | `toolkit` |
| `platform/` | `platform` |
| `icd/`（生成物） | `icd` / `icdfom` |

周期ループから見えるのは `CChannel` だけで、実装は2つある。

| | `CTClassChannel<T>` | `CTRawChannel<T>` |
|---|---|---|
| 何 | ICD のクラス | FOM に無い独自データ |
| バイト列 | 12バイトヘッダ + 固定長レコード | 相手が決めた形式。1データグラム = 1メッセージ |
| 型 | 生成物（`icd/`） | 手書き（`app/`） |
| 手元の相手 | HLA（`hla::CTFromHla` / `CTToHla`） | アプリ（`gw::CTMessageHandler`） |
| 向き | 送受信 | 受信のみ |

## 書き方の決まり

| 種類 | 接頭辞 | 例 |
|---|---|---|
| クラス | `C` | `CGateway`、`CUdpSocket` |
| クラステンプレート | `CT` | `CTClassChannel<T>`、`CTFromHla<T>` |
| 構造体・列挙 | `T` | `TClassBinding`、`app::TCommand` |

- **ファイル名は型名と同じ。** 1ファイル1クラス
- **生成物（`icd/`）には接頭辞を付けない。** FOM のクラス名のまま
- **メンバを返すだけの関数は `get〜`、設定するだけの関数は `set〜`**
- **派生の名前は、基底の名前（接頭辞を除く）で終える。** `CTFromHla` の実装は `〜FromHla`、
  `CTMessageHandler` の実装は `〜Handler`
- **メンバ変数は `m_` を付け、行末に `///<` で説明を書く**
- **ソースは UTF-8（BOM なし）+ CRLF。** `README.md` と `CMakeLists.txt` だけは LF。
  MSVC には `/utf-8` を渡してある

## クラスを1つ足す（スタブで試す）

**手で書くのは配線（`app/CWiring.h`）と、RTI が無い間の代用品だけ。** ID・ポート・ペイロードは
生成物の定数から決まるので、どこにも数字を書かない。

サンプルとして `Designator`（レーザー指示器、1件 115 B、ID 5 / port 24005）を足してある。
以下はその手順。

### ① ICDgenerator で生成する

1. GUI で FOM を開き、対象クラス（`EmbeddedSystem.Designator`）にチェックを入れる
2. ID/Port/Rate ダイアログで ID と Port を振る（ほかのクラス・独自データと重ならない番号）
3. MTU を選ぶ（1500 / 9000）
4. C++ 生成の出力先をこのリポジトリの `icd/` にして生成する

これで次のものが出る。include を手で足す必要はない。

- `icd/object/Designator.h/.cpp`（インタラクションなら `icd/interaction/`）
- `icd/icd_types.h/.cpp` に、このクラスが使う型が足される
- `icd/icd_classes.h` にこのクラスの include が足される

**生成が止まるのは2通り：** 可変レコードを含むクラスと、1件が MTU に収まらないクラス。
どちらもクラス名と理由が出る。

### ② ビルド設定に `.cpp` を足す

`icd/object/Designator.cpp` を `HLAGateway.vcxproj`（と `.filters`）と `CMakeLists.txt` に足す。
ヘッダは `.vcxproj` と `.filters` にだけ足す。

### ③ 送信元と受け口を用意する

RTI が無い間は `stub/` に代用品を置く。

**送信元（HLA → UDP）：** 値を作る関数を1本書き、`stub::CTFixtureFromHla` に渡す。
値に意味が要らなければ、関数を書かずに `stub::CTConstantFromHla<T>{1}`（既定構築の値を毎周期1件）でもよい。

```cpp
// stub/DesignatorFixture.h
inline icdfom::Designator makeDesignator(std::size_t i) {
    icdfom::Designator d{};
    d.HostObjectIdentifier = objectId("DSG-" + std::to_string(i % 2));
    d.DesignatorSpotLocation.X = 4.0e6 + 10.0 * static_cast<double>(i / 2);
    ...
    return d;
}
```

**受け口（UDP → HLA）：** `hla::CTToHla<T>` を実装する。名前は `〜ToHla` で終える。
サンプルの `stub/CDesignatorToHla.h` はモックで、本番の `hla::CTRtiObjectToHla` と同じ流れ
（鍵でインスタンスを決める → 初見なら登録 → 属性を書く）を、HLA に書く代わりに表示で真似している。

```cpp
class CDesignatorToHla : public hla::CTToHla<icdfom::Designator> {
public:
    void accept(const icdfom::Designator& d) override {
        const std::string key(d.HostObjectIdentifier.begin(), d.HostObjectIdentifier.end());
        if (m_registered.insert(key).second) std::printf("[HLA・モック] 登録: %s\n", key.c_str());
        std::printf("[HLA・モック] 更新: %s  照射点=(...)\n", key.c_str());
    }
private:
    std::set<std::string> m_registered;
};
```

- **`accept()` はブロックしないこと。** 周期ループのスレッドから呼ばれる
- **レコードはこの呼び出しの間だけ有効。** 後で使うならコピーする

### ④ 配線に足す（`app/CWiring.h`）

メンバを足す：

```cpp
stub::CTFixtureFromHla<icdfom::Designator> designatorFromHla{stub::makeDesignator, 1};
stub::CDesignatorToHla designatorToHla;
```

`build()` に1行足す：

```cpp
addClass<icdfom::Designator>(g, &designatorFromHla, &designatorToHla);   // 送受信
```

片方向のクラスは、要らないほうに `nullptr` を渡す。

```cpp
addClass<icdfom::Foo>(g, &fooFromHla, nullptr);   // 送信のみ
addClass<icdfom::Bar>(g, nullptr, &barToHla);     // 受信のみ
```

**`T`（`<icdfom::Designator>`）は必ず明示すること。** `nullptr` からは型が決まらない。

### ⑤ 動かす

自分宛てに送れば、送ったものを自分で受けてモックが表示する：

```
HLAGateway 127.0.0.1 20 1
```

```
[HLA・モック] 登録: DSG-0
[HLA・モック] 更新: DSG-0  照射点=(4000000.0, 2000000.0, 4500000.0)  出力=10.0 W
[HLA・モック] 登録: DSG-1
[HLA・モック] 更新: DSG-1  照射点=(4000000.0, 2001000.0, 4500000.0)  出力=11.0 W
[HLA・モック] 更新: DSG-0  照射点=(4000010.0, 2000000.0, 4500000.0)  出力=10.0 W
```

### 本番では

`stub::` の2つを `hla::CTRti...` の器に、配線を `app/CRtiWiring.h` にする（次の節）。

## クラスを1つ足す（本番）

RTI に繋ぐときに書くファイルと、その雛形。**サンプルは Designator（オブジェクト）と
WeaponFire（インタラクション）** で、偽のツールキットに対して動作を確認してある。

### ファイル群

| ファイル | 何 | クラスを足すとき |
|---|---|---|
| `rti/Toolkit.h` | ツールキットの入口。rti/ はここ経由でだけツールキットを見る | 触らない（本物に繋ぐときに1回だけ差し替え） |
| `rti/CDb.h` | world を持つシングルトン | 触らない |
| `rti/TypeConv.h/.cpp` | 入れ子のレコードと ID の変換（型ごと、クラス間で共有） | まだ無い型が出てきたら1組足す |
| `rti/<Class>Rti.h/.cpp` | **そのクラスの変換関数** | **1クラスにつき1組、新しく書く** |
| `rti/CInteractionCallback.h` | インタラクションの受信コールバック（全クラスで1つ） | **インタラクションならキューと override を1つずつ足す** |
| `app/CRtiWiring.h` | 本番の配線 | **メンバ2本と1行を足す** |
| `stub/toolkit/Toolkit.h` | 偽のツールキット（本番には持っていかない） | 試すなら、そのクラスを足す |

加えて、スタブのときと同じく ICDgenerator で `icd/` を生成し、`.cpp` をビルド設定に足す
（「クラスを1つ足す（スタブで試す）」の①②）。

### オブジェクトを足す（`rti/DesignatorRti.h/.cpp` が雛形）

書く関数は5本。Fetch と Create は world を `CDb` から読む。

```cpp
namespace rti {
std::vector<tk::DesignatorPtr> getRemoteDesignator();                      // Fetch
icdfom::Designator toIcd(const tk::DesignatorPtr& p);                       // HLA → ICD
std::string keyOf(const icdfom::Designator& r);                             // どのインスタンスか
tk::DesignatorPtr registerDesignator(const std::string& key);               // Create
void updateDesignator(const icdfom::Designator& r, const tk::DesignatorPtr& p);   // ICD → HLA、update
}
```

中身は属性ごとの代入を並べるだけ。入れ子のレコードと ID は `TypeConv` の `toIcd` / `toRti` に任せ、
列挙は `static_cast` する。

```cpp
icdfom::Designator toIcd(const tk::DesignatorPtr& p) {
    icdfom::Designator r{};
    r.EntityIdentifier      = toIcd(p->getEntityIdentifier());          // 入れ子のレコード
    r.HostObjectIdentifier  = toIcd(p->getHostObjectIdentifier());      // ID（文字列 → 文字の配列）
    r.CodeName              = static_cast<icdfom::DesignatorCodeNameEnum16>(p->getCodeName());   // 列挙
    r.DesignatorOutputPower = p->getDesignatorOutputPower();            // そのまま
    ...
    return r;
}

void updateDesignator(const icdfom::Designator& r, const tk::DesignatorPtr& p) {
    p->setEntityIdentifier(toRti(r.EntityIdentifier));
    ...
    p->update();
}
```

配線（`app/CRtiWiring.h`）：

```cpp
hla::CTRtiObjectFromHla<icdfom::Designator, tk::DesignatorPtr> designatorFromHla{
    &rti::getRemoteDesignator, &rti::toIcd};
hla::CTRtiObjectToHla<icdfom::Designator, tk::DesignatorPtr> designatorToHla{
    &rti::keyOf, &rti::registerDesignator, &rti::updateDesignator};
...
addClass<icdfom::Designator>(g, &designatorFromHla, &designatorToHla);   // build() に
```

### インタラクションを足す（`rti/WeaponFireRti.h/.cpp` と `rti/CInteractionCallback.h` が雛形）

**HLA から来る向きは RTI のコールバック。** ツールキットが生成するコールバックのクラスには
FOM の全インタラクションの仮想関数が並んでいるので、`rti/CInteractionCallback.h` でそれを1回だけ
継承し、**流すものだけ実装する。**

1インタラクションにつき書くもの：

**① 変換関数3本（`rti/<Class>Rti.h/.cpp`）**

```cpp
namespace rti {
icdfom::WeaponFire toIcd(const tk::WeaponFire& i);                  // 受信したパラメータ → レコード
void fillRti(const icdfom::WeaponFire& r, tk::WeaponFire* i);       // レコード → 送信するパラメータ
void sendWeaponFire(const icdfom::WeaponFire& r);                   // Send：fillRti して sendInteraction
}
```

**② `rti/CInteractionCallback.h` に、キュー1つと override 1つ**

```cpp
class CInteractionCallback : public tk::InteractionCallback {
public:
    hla::CTRtiInteractionFromHla<icdfom::WeaponFire> weaponFireFromHla;   // キュー

    void onWeaponFire(const tk::WeaponFire& interaction) override {       // RTI のスレッド
        weaponFireFromHla.push(toIcd(interaction));
    }
};
```

- パラメータはコールバックの間しか有効でないのが普通なので、この場でレコードに写す
- `push()` はロック付きなので、RTI のスレッドから呼んでよい
- ゲートウェイのほかのもの（チャネルやソケット）には触らない

**③ 配線（`app/CRtiWiring.h`）に、送信のメンバ1つと build() の1行**

```cpp
hla::CTRtiInteractionToHla<icdfom::WeaponFire> fireToHla{&rti::sendWeaponFire};
...
addClass<icdfom::WeaponFire>(g, &interactions.weaponFireFromHla, &fireToHla);   // build() に
```

コールバックの登録（`subscribe()`）は全インタラクションで1回なので、増えても変わらない。

**用語：** レコードは UDP（ICD）側の1件（`icdfom::` の型）、パラメータ・属性は HLA 側
（ツールキットの型）の値。

### main の形

world は join してからできるので、`CDb` に置いてから回す。**world が無い間にゲートウェイを回さないこと**
（null チェックせずに辿るため）。

```cpp
app::CRtiWiring wiring;          // CGateway より先に宣言する（チャネルが借りているため）
gw::CGateway gateway;
wiring.build(gateway);

/* join */
rti::CDb::getInstance().setWorld(world);
wiring.subscribe();              // インタラクションの受信コールバックを登録
if (gateway.openAll(peer)) gateway.run(20, 0);
/* resign */
rti::CDb::getInstance().setWorld(nullptr);
// wiring はここより後まで生きていること（コールバックをツールキットに貸しているため）
```

いまの `main.cpp` はスタブの配線（`CWiring`）を使っている。`CRtiWiring` はビルドには入っているが、
まだ main からは使っていない。

### 本物のツールキットに繋ぐとき

1. `rti/Toolkit.h` の2行を、本物のヘッダの include と名前空間の別名に差し替える
2. `rti/` の変換関数を本物の綴りに合わせる。**偽物で仮に決めたのは次の4つ：**
   - 属性はアクセサ（`getXxx` / `setXxx`）で触る
   - 入れ子のレコードは FOM と同じ名前の構造体で返る
   - 列挙は整数、`RTIobjectId` は `std::string`
   - インタラクションの受信は、全インタラクションの仮想関数が並んだ `InteractionCallback` を継承し、
     `setInteractionCallback` で登録する（関数名と登録の仕方は仮）
3. `stub/` を消し、main を `CRtiWiring` に切り替える

**RTIobjectId の上限に注意。** ICD では上限（RPR FOM では16文字）のある配列になっている。
これを超える ID のレコードは符号化に失敗し、送られない。上限は ICDgenerator の配列上限ダイアログで決める。

### スレッド

ゲートウェイ本体は単一スレッドで、ロックがあるのは `CTRtiInteractionFromHla` だけ。

- **コールバックの中で `T` に詰め替えてから `push()` する。** RTI のパラメータのバッファは、
  コールバックの間しか有効でないのが普通のため
- 周期ループが `drain()` でまとめて持っていく
- `CDb` の world を書き換えるのは周期ループが止まっているときだけなので、`CDb` にロックは要らない

### 変換に加えて、別の処理もしたいクラス

**そのクラスの Fetch の中に書く。** 取ってきたデータがそこにあるので、取り直す必要がない。

```cpp
std::vector<tk::DesignatorPtr> getRemoteDesignator() {
    auto list = CDb::getInstance().getWorld()->getObjectManager()->getRemoteDesignator();
    CDesignatorExtra::apply(list);   // 別の処理
    return list;                     // こちらは UDP に変換されて送られる
}
```

**Fetch が呼ばれるのは、そのチャネルが送信するときだけ。** 次のときには呼ばれない：

- `none` で起動したとき
- 送信元が `nullptr` のとき

どんなときも毎周期やりたい処理は、`CGateway::addTickEnd` に登録する。

### 決まっていないこと

- **鍵の選び方（`keyOf`）。** サンプルは `HostObjectIdentifier`。1つの母体に指示器が複数あるなら、
  ほかのフィールドと組にする
- **部分更新の扱い。** ICD は常に全属性ぶんの箱を送るので、送信側が持っていない属性はゼロで届く。
  前回値で埋めるか既定値で埋めるかは `updateDesignator` の中で決める
- **インスタンスの削除。** `CTRtiObjectToHla` は一度登録したインスタンスを消さない

## FOM に無いデータを受ける

別のシステムが送ってくるコマンドのように、**相手が形式を決めていて、12バイトヘッダも無い** UDP データも
受けられる。

1. **型を `app/` に手書きする。** `kName`・`kPort` と、同じ名前空間の
   `bool parse(const unsigned char*, std::size_t, T&)` を持たせる。
   形式に合わなければ `false` を返す（捨てられる）
2. **受け口を書く。** `gw::CTMessageHandler<T>` を実装する
3. **配線に1行足す。** `addRaw<T>(g, &handler);`

   ハンドラの `onTickEnd()` も、このとき周期末処理に登録される。

**受け口では積むだけにして、処理は `onTickEnd()` で行う。** `accept()` は受信の最中に呼ばれるので、
そこでゲートウェイの状態を変えると回しているループが壊れる。コマンドが効くのは次の周期から。

### コマンド文字列（`app::TCommand`）

形式は仮で、1データグラム = 1コマンド、中身は char の並び。最初の NUL で切り、末尾の改行を落とす。
`"STOP"`・`"STOP\0\0…"`・`"STOP\r\n"` はどれも `"STOP"` として受ける。

処理は `app/CCommandHandler.h` の `handle()` に書く（いまは表示するだけ）。受信専用で起動して、
別の端末から送れば試せる：

```sh
python -c "import socket; socket.socket(2,2).sendto(b'STOP', ('127.0.0.1', 24100))"
```

### 制御文字列（`app::TControl`）

形式は `TCommand` と同じ（仮）で、ポートは 24101（仮）。受け口は `app/CControlHandler.h`。

**`handle()` はモック。** 知っている制御を受けたら、表示してモックの状態（`running`）を切り替えるだけ：

| 制御 | モックの動き |
|---|---|
| `START` | `running` を 1 にする |
| `STOP` | `running` を 0 にする |
| `RESET` | `running` を 0 に戻す |
| `STATUS` | `running` を表示する |
| それ以外 | 「知らない制御」と表示して何もしない |

本物の処理が決まったら `handle()` の中身を差し替える。

### 気をつけること

- **ポートは ICD のクラスと同じ番号空間。** 重複していると `openAll` が開く前に断る。重複しても
  bind は成功し（`SO_REUSEADDR`）、どちらに届くかは OS で逆になる（Windows は先に bind したほう、
  Linux は後のほう）ため
- **0バイトのデータグラムは来ない前提。** 「何も来ていない」と区別できない

## ビルド

| | |
|---|---|
| Windows | `HLAGateway.sln`（VS2022 / v143 / x64） |
| Linux | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` |

- **C++17 が必須。** 生成コードが使っている
- リンクするもの：Windows は `ws2_32` と `winmm`、Linux は `Threads::Threads`
- ソースを増減したら `HLAGateway.vcxproj`（と `.filters`）と `CMakeLists.txt` の両方を直すこと

## `icd/` の再生成

ICD が変わったら、ICDgenerator の C++ 生成の出力先をこのリポジトリの `icd/` にして、
**丸ごと差し替える。** 手で入れた変更は消える。

```
icd/
  icd_codec.h        ランタイム（全クラス共通）
  icd_types.h/.cpp   選んだクラスが使う型
  icd_classes.h      全クラスのまとめ include（配線用）
  object/            オブジェクトクラス（<Class>.h / .cpp）
  interaction/       インタラクションクラス
```

- **生成器は既存のファイルを消さない。** 選択から外したクラスは手で消す
- **1件が MTU に収まらないクラスは ICDgenerator が生成を拒否する。** 生成ヘッダの `static_assert` も
  コンパイル時に止める

現在の選択（MTU 9000）：

| クラス | classId | port | 1件 | 種別 |
|---|---:|---:|---:|---|
| `EmitterBeam.RadarBeam` | 1 | 24001 | 139 B | オブジェクト |
| `EmbeddedSystem.RadioReceiver` | 2 | 24002 | 62 B | オブジェクト |
| `EmbeddedSystem.MinefieldData` | 3 | 24003 | 1902 B | オブジェクト |
| `WeaponFire` | 4 | 24004 | 134 B | インタラクション |
| `EmbeddedSystem.Designator`（サンプル） | 5 | 24005 | 115 B | オブジェクト |

RPR FOM の `BaseEntity` 配下（Aircraft など）は可変レコードを含むので、生成器が拒否する。
実運用の FOM には可変レコードが無い。
