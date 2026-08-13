/**
 * system_status.h
 * 電池残量などシステム状態関連の変数・関数宣言
 */

#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H                     // インクルードガード（二重定義防止）

// マクロ
#define BATTERY_LOW_THRESHOLD 20            // この値以下で警告色（赤）表示にする閾値

// extern変数宣言
extern int batteryLevel;                    // 電池残量（%）。M5.Power非対応時は-1（センチネル値）

// 関数宣言
void updateBatteryLevel();

#endif
