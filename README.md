# HLAGateway（簡易版）

HLA と UDP を橋渡しするゲートウェイの参照実装。
[ICDgenerator](https://github.com/Renpent/ICDgenerator) が FOM から生成する ICD のとおりに
バイト列を組み立てて送受信する。

**この版は動作に要るものだけを残した簡易版。** 送受信の件数・周期の遅れ・キューの深さといった
監視と、送れなかったインタラクションの持ち越しは入っていない。送れなかったもの・形式に合わない
ものは、数えずに捨てる。

**この環境に RTI は無い。** HLA 側の送信元は `stub/` の代用品で、受信した分は捨てている。
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
app/              配線（CWiring）と、FOM に無い独自データの型・処理（TCommand, CCommandHandler）
gateway/          周期ループ（CGateway）とチャネル（ICD のクラス用・独自データ用）
  udp/            ソケット、poll、データグラムの送信（CTUdpSender）と受信（CTUdpReceiver）
  hla/            HLA 側の継ぎ目（CTFromHla / CTToHla）と、RTI を呼ぶ器
stub/             RTI の代わりに値を作る代用品。**本番には持っていかない**
platform/         コンソールの文字コードとタイマ分解能（Windows のみ）
icd/              ICDgenerator の生成物。手で編集しない
```

**依存は上から下への一方向。**

- `gateway/` は `app/` を見ない
- `udp/` と `hla/` は互いを知らない
- OS を知っているのは `gateway/udp/*.cpp` と `platform/*.cpp` だけ
- RTI の型はどこにも出てこない

**名前空間はフォルダで決まる。**

| フォルダ | 名前空間 |
|---|---|
| `gateway/` | `gw` |
| `gateway/udp/` | `udp` |
| `gateway/hla/` | `hla` |
| `app/` | `app` |
| `stub/` | `stub` |
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

## クラスを1つ足す

**触るのは `app/CWiring.h` だけ。** ID・ポート・ペイロードは生成物の定数から決まる。

1. **ICDgenerator で生成する。** クラスにチェックを入れ、ID/Port を振り、MTU を選んで C++ 生成する。
   `icd/icd_classes.h` も追随するので、include を足す必要はない
2. **メンバを足す。** 送信元を足す（いまは `stub/` の代用品）：

   ```cpp
   stub::CTConstantFromHla<icdfom::Aircraft> aircraftFromHla{1};
   ```

3. **`build()` に1行足す。**

   ```cpp
   add<icdfom::Aircraft>(g, &aircraftFromHla, nullptr);   // 送信のみ
   add<icdfom::Bar>(g, nullptr, &barToHla);               // 受信のみ
   add<icdfom::Foo>(g, &fooFromHla, &fooToHla);           // 送受信
   ```

   `T` は必ず明示すること（`nullptr` からは型が決まらない）。

## 本番（RTI）での形

`stub::` の代用品を `hla::CTRti...` の器に差し替える。HLA ツールキットが
`worldPtr->getXxxManager()->getYyy()` の形で、`update()` や `sendInteraction()` が戻り値の
ポインタのメソッドである前提。world は join してからできるので、シングルトン（下の例では `CDb`）に置き、
**呼ばれたときに読む。**

```cpp
// オブジェクト：毎周期こちらから取りに行く
hla::CTRtiObjectFromHla<icdfom::Aircraft, their::AircraftPtr> aircraftFromHla{
    [] { return CDb::getInstance().getWorld()->getObjectManager()->getRemoteAircraft(); },  // Fetch
    &toIcd                                          // 1インスタンス → 1レコード
};
hla::CTRtiObjectToHla<icdfom::Aircraft, their::AircraftPtr> aircraftToHla{
    &keyOf,                                         // どのインスタンスか
    [](const std::string& k) {                      // 初見なら登録
        return CDb::getInstance().getWorld()->getObjectManager()->registerAircraft(k); },
    &writeBack                                      // p->setXxx(...); p->update();
};

// インタラクション：RTI のコールバックから押し込まれる
hla::CTRtiInteractionFromHla<icdfom::WeaponFire> fireFromHla;   // コールバックの中で push() を呼ぶ
hla::CTRtiInteractionToHla<icdfom::WeaponFire>   fireToHla{
    [](const icdfom::WeaponFire& r) {
        auto* i = CDb::getInstance().getWorld()->getInteractionManager()->getWeaponFire();
        fillRti(r, i);
        i->sendInteraction(); }
};
```

クラスごとに書くのは変換関数だけ：

- オブジェクト：`toIcd` / `keyOf` / `writeBack` の3本
- インタラクション：`toIcd` / `fillRti` の2本

**起動と終了の順番。** world が無い間にゲートウェイを回さないこと（world を null チェックせずに辿るため）。

```
起動:  配線を作る → join → CDb に world を置く → openAll → run
終了:  run が戻る → resign → CDb の world を外す
```

### スレッド

ゲートウェイ本体は単一スレッドで、ロックがあるのは `CTRtiInteractionFromHla` だけ。
インタラクションは RTI のスレッドのコールバックで届くので、次のようにする：

- **コールバックの中で `T` に詰め替えてから `push()` する。** RTI のパラメータのバッファは、
  コールバックの間しか有効でないのが普通のため
- 周期ループが `drain()` でまとめて持っていく

### 変換に加えて、別の処理もしたいクラス

**そのクラスの Fetch の中に書く。** 取ってきたデータがそこにあるので、取り直す必要がない。

```cpp
hla::CTRtiObjectFromHla<icdfom::RadarBeam, their::RadarBeamPtr> beamFromHla{
    [] {
        auto beams = CDb::getInstance().getWorld()->getObjectManager()->getRemoteRadarBeam();
        CRadarBeamExtra::apply(beams);   // 別の処理
        return beams;                    // こちらは UDP に変換されて送られる
    },
    &toIcd };
```

**Fetch が呼ばれるのは、そのチャネルが送信するときだけ。** 次のときには呼ばれない：

- `none` で起動したとき
- 送信元が `nullptr` のとき

どんなときも毎周期やりたい処理は、`CGateway::addTickEnd` に登録する。

### 決まっていないこと

- **部分更新の扱い。** ICD は常に全属性ぶんの箱を送るので、送信側が持っていない属性はゼロで届く。
  前回値で埋めるか既定値で埋めるかは `writeBack` の中で決める
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

RPR FOM の `BaseEntity` 配下（Aircraft など）は可変レコードを含むので、生成器が拒否する。
実運用の FOM には可変レコードが無い。
