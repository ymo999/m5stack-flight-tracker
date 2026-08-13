/**
 * system_status.cpp
 * 電池残量などシステム状態関連の実装
 */

#include "system_status.h"

#include <M5Unified.h>

// 電池残量（%）の実体。初期値は-1（未対応・未取得を表すセンチネル値）
int batteryLevel = -1;

// ============================================================
// 電池残量を取得し、batteryLevelを更新する
// ============================================================
void updateBatteryLevel() {
    batteryLevel = M5.Power.getBatteryLevel();
    Serial.printf("[TEST] battery: %d\n", batteryLevel);    // 一時テストコード：戻り値確認用
    // 非対応の場合はbatteryLevelを-1のまま維持する
}