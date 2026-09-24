// 周期が守れているかの記録。
//
// **守れていないことに気づけないのが一番困る**ので測っておく。周期そのものは絶対時刻で
// 刻んでいるのでレートはずれないが、起床の遅れと1周の処理時間は環境で大きく変わる。

#pragma once

#include <cstdint>

namespace gw {

struct TLoopStats {
    std::uint64_t ticks = 0;         ///< 回した周期の数
    std::uint64_t overruns = 0;      ///< 1周の処理が周期を超えた回数
    double maxTickMs = 0.0;          ///< 1周の処理時間の最大
    double maxLateMs = 0.0;          ///< 起床が予定からどれだけ遅れたかの最大
    double sumLateMs = 0.0;          ///< 起床の遅れの合計（平均を出すため）
};

}  // namespace gw
