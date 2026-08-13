/**
 * state_machine.h
 * 画面状態（SystemMode）管理・状態遷移関連の関数宣言
 */

#ifndef STATE_MACHINE_H                     // インクルードガード（二重定義防止）
#define STATE_MACHINE_H

// 画面状態（システムの状態）
enum SystemMode {
    MODE_WIFI_SETUP,                        // Wi-Fi設定関連（AP接続案内／接続成功／接続失敗）
    MODE_FLIGHT_VIEW,                       // 機体情報表示
    MODE_MENU_VIEW,                         // 設定メニュー（SETTINGS）
    MODE_CONFIG_VIEW,                       // 設定内容一覧（CONFIG）
    MODE_SCAN_RANGE_VIEW,                   // SCAN RANGE選択
    MODE_QR_VIEW,                           // QRコード誘導（APIキー／基準地点）
    MODE_CONFIRM_DIALOG,                    // 確認ダイアログ
    MODE_ERROR_VIEW,                        // エラー表示
    MODE_NO_FLIGHTS_VIEW,                   // 機体0件
    MODE_LOADING                            // データ取得中
};

// state_machine.cpp で定義されている現在の画面状態を共有する
extern SystemMode currentMode;

// 画面の再描画が必要かどうかを示すフラグ
// currentModeの変化時や、表示内容の更新時にtrueへ設定する
// 各ハンドラは、このフラグがtrueの時のみ描画処理を実行し、falseに戻す
// ※毎ループでの再描画は無駄な処理、かつちらつきの原因になるため
extern bool needsRedraw;

// 状態管理機構の初期化（起動時の初期状態を設定する）
void initStateMachine();

// 現在の画面状態（currentMode）に応じて、対応するハンドラ関数を呼び出す
// loop()から毎回呼び出される想定
void updateStateMachine();

// 各画面状態（SystemMode）に対応するハンドラ関数
// 画面の描画・ボタン処理の分岐を担当する（中身は各画面の実装ステップで記述）
void handleWiFiSetupView();                 // Wi-Fi設定関連（AP接続案内／接続成功／接続失敗）
void handleFlightView();                    // 機体情報表示
void handleMenuView();                      // 設定メニュー（SETTINGS）
void handleConfigView();                    // 設定内容一覧（CONFIG）
void handleScanRangeView();                 // SCAN RANGE選択
void handleQrView();                        // QRコード誘導（APIキー／基準地点）
void handleConfirmDialog();                 // 確認ダイアログ
void handleErrorView();                     // エラー表示
void handleNoFlightsView();                 // 機体0件
void handleLoadingView();                   // データ取得中

#endif