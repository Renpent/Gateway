// HLA 側との継ぎ目。
//
// **この環境に RTI は無い**ので、ここには RTI を呼ぶコードが1行も無い。それでも継ぎ目を先に
// 切っておくのは、上の層（Publisher / Subscriber / 生成コーデック）が RTI の型を一切知らない
// 状態を保つため。本番環境への移植は「この2つのインタフェースを RTI の API で実装する」
// だけになり、ゲートウェイ本体には手が入らない。
//
// 向きが2つあることに注意：
//   Source<T>  HLA → UDP。RTI が反映した属性更新を T に詰めて渡す（reflectAttributeValues）
//   Sink<T>    UDP → HLA。UDP から復元した T を HLA に押し込む（updateAttributeValues）
//
// 実装側で必要になるが、ここに現れないもの：
//   - オブジェクトインスタンスハンドルと T の対応表（HLA はインスタンス単位、ICD はレコード単位）
//   - 属性ハンドルとメンバの対応（生成コードはメンバ順を保証するが、ハンドルは実行時に取る）
//   - 部分更新の扱い。ICD は常に全属性ぶんの箱を送るので、
//     未受信の属性を「前回値」で埋めるか既定値で埋めるかを決める必要がある
//
// ─── スレッドの取り決め ───────────────────────────────────────────────
//
// **この2つのメソッドは周期ループのスレッドから呼ばれる。** ゲートウェイ側は単一スレッドで、
// tick() が受信を全部片付けてから送信する直列実行なので、ゲートウェイの内部に排他は無い
// （UDP ソケットは全二重で、送信用と受信用のバッファも別々に持っている）。
//
// **排他が要るのはこの継ぎ目だけ。** RTI のコールバック（reflectAttributeValues など）は
// RTI が持つスレッドで飛んでくるので、実装はキューを1枚挟んでそこに lock を置くこと：
//
//     RTI のスレッド ──→ [ロック付きキュー] ──→ drain() ──→ 周期ループのスレッド
//
// drain() / accept() の中で RTI を呼び返さないこと。RTI 実装によっては
// コールバック中の再入を禁じており、握ったままだと簡単にデッドロックする。
// ─────────────────────────────────────────────────────────────────────

#ifndef HLAGW_FEDERATE_H
#define HLAGW_FEDERATE_H

#include <cstddef>
#include <vector>

namespace hla {

/// HLA から出てくるレコードの供給元。
template <class T>
class Source {
public:
    virtual ~Source() = default;

    /// 前回の呼び出し以降に届いたぶんを out の末尾に足し、足した件数を返す。
    ///
    /// **ブロックしないこと。** 呼び出し側が送信レートを決めていて、ここで待たれると
    /// その周期ぶん全クラスが遅れる。キューが空なら 0 を返して即座に戻る。
    virtual std::size_t drain(std::vector<T>& out) = 0;
};

/// HLA へ戻すレコードの受け口。
template <class T>
class Sink {
public:
    virtual ~Sink() = default;
    virtual void accept(const T& record) = 0;
};

}  // namespace hla

#endif  // HLAGW_FEDERATE_H
