/**
 * state_machine.cpp
 * 画面状態（SystemMode）管理・状態遷移関連の実装
 */

#include "state_machine.h"

// 現在の画面状態の実体
// 起動時の分岐ロジック（Wi-Fi接続・APIキー・基準地点の登録状況による初期状態の判定）は未実装のため、
// 暫定的にMODE_WIFI_SETUP固定とする
SystemMode currentMode = MODE_WIFI_SETUP;

// ============================================================
// 状態管理機構の初期化
// ============================================================
void initStateMachine() {
    currentMode = MODE_WIFI_SETUP;
}

// ============================================================
// 現在の画面状態に応じて、対応するハンドラ関数を呼び出す
// ============================================================
void updateStateMachine() {
    switch (currentMode) {
        case MODE_WIFI_SETUP:      handleWiFiSetupView();  break;
        case MODE_FLIGHT_VIEW:     handleFlightView();     break;
        case MODE_MENU_VIEW:       handleMenuView();       break;
        case MODE_CONFIG_VIEW:     handleConfigView();     break;
        case MODE_SCAN_RANGE_VIEW: handleScanRangeView();  break;
        case MODE_QR_VIEW:         handleQrView();         break;
        case MODE_CONFIRM_DIALOG:  handleConfirmDialog();  break;
        case MODE_ERROR_VIEW:      handleErrorView();      break;
        case MODE_NO_FLIGHTS_VIEW: handleNoFlightsView();  break;
        case MODE_LOADING:         handleLoadingView();    break;
    }
}

// ============================================================
// 各画面状態（SystemMode）に対応するハンドラ関数
// 画面の描画・ボタン処理の分岐を担当する
// ============================================================
void handleWiFiSetupView() {
    /* TODO : 処理内容の記述（Wi-Fi設定関連画面の描画・ボタン処理、5.2・5.9参照） */
}

void handleFlightView() {
    /* TODO : 処理内容の記述（機体情報表示画面の描画・ボタン処理、5.2・5.6参照） */
}

void handleMenuView() {
    /* TODO : 処理内容の記述（設定メニュー画面の描画・ボタン処理、5.2・5.7参照） */
}

void handleConfigView() {
    /* TODO : 処理内容の記述（設定内容一覧画面の描画・ボタン処理、5.2・5.7.1参照） */
}

void handleScanRangeView() {
    /* TODO : 処理内容の記述（SCAN RANGE選択画面の描画・ボタン処理、5.2・5.7.3参照） */
}

void handleQrView() {
    /* TODO : 処理内容の記述（QRコード誘導画面の描画・ボタン処理、5.2・5.8参照） */
}

void handleConfirmDialog() {
    /* TODO : 処理内容の記述（確認ダイアログの描画・ボタン処理、5.2・5.7.2参照） */
}

void handleErrorView() {
    /* TODO : 処理内容の記述（エラー画面の描画・ボタン処理、5.2・5.10参照） */
}

void handleNoFlightsView() {
    /* TODO : 処理内容の記述（機体0件画面の描画・ボタン処理、5.2・5.10参照） */
}

void handleLoadingView() {
    /* TODO : 処理内容の記述（ローディング画面の描画、5.2・5.11参照） */
}