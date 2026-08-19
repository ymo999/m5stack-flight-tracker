/**
 * @file system_status.h
 * @brief 電池残量などシステム状態関連の変数・関数宣言
 */

#ifndef SYSTEM_STATUS_H                     // インクルードガード（二重定義防止）
#define SYSTEM_STATUS_H

/**
 * @def BATTERY_LOW_THRESHOLD
 * @brief この値以下で警告色（赤）表示にする閾値
 */
#define BATTERY_LOW_THRESHOLD 20

/**
 * @brief 電池残量（%）
 *
 * @note M5.Power非対応時は-1（センチネル値）
 */
extern int batteryLevel;

/**
 * @brief 電池残量を取得し、batteryLevel（extern変数）を更新する
 *
 * @note 非対応機種の場合、batteryLevelは-1（センチネル値）のまま維持される
 */
void updateBatteryLevel();

/**
 * @brief 現在のM5Stack機種が、物理ボタンを持たない機種か判定する
 *
 * タッチによる仮想ボタンでの代替が必要かどうかの判定に使用する
 *
 * @note 現状対象としている、物理ボタンを持たない機種はCoreS3のみ。将来的に他の機種が対象になった場合は判定を追加すること
 *
 * @return true 物理ボタンを持たない機種（仮想ボタンでの代替が必要）
 * @return false 物理ボタンを持つ機種
 */
bool isVirtualButtonBoard();

#endif
