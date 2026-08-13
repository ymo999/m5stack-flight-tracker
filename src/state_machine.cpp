/**
 * state_machine.cpp
 * 画面状態（SystemMode）管理・状態遷移関連の実装
 */

#include <M5Unified.h>

#include "flight_data.h"
#include "state_machine.h"
#include "ui_handler.h"

// 現在の画面状態の実体
// 起動時の分岐ロジック（Wi-Fi接続・APIキー・基準地点の登録状況による初期状態の判定）は未実装のため、
// 暫定的にMODE_WIFI_SETUP固定とする
SystemMode currentMode = MODE_WIFI_SETUP;

// 画面の再描画が必要かどうかを示すフラグの実体
// 起動直後は必ず1回描画するため、初期値はtrueとする
bool needsRedraw = true;

// ============================================================
// 状態管理機構の初期化
// ============================================================
void initStateMachine() {
    currentMode = MODE_WIFI_SETUP;
    needsRedraw = true;
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

// Wi-Fi設定関連（AP接続案内／接続成功／接続失敗）
void handleWiFiSetupView() {
    /* TODO : 処理内容の記述（Wi-Fi設定関連画面の描画・ボタン処理、5.2・5.9参照） */
}

// 機体情報表示
void handleFlightView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (M5.BtnA.wasPressed()) {
        // PREV：前の機体へ。1機目の場合は最終機体へループ（再取得は促さない）
        currentDisplayIndex--;
        if (currentDisplayIndex < 0) {
            currentDisplayIndex = totalFlightCount - 1;
        }
        needsRedraw = true;
    }

    if (M5.BtnB.wasPressed()) {
        // NEXT：次の機体へ。最終機体の場合は再取得の確認ダイアログへ遷移
        if (currentDisplayIndex >= totalFlightCount - 1) {
            currentMode = MODE_CONFIRM_DIALOG;
        } else {
            currentDisplayIndex++;
        }
        needsRedraw = true;
    }

    if (M5.BtnC.wasPressed()) {
        // SET：設定メニュー画面へ
        currentMode = MODE_MENU_VIEW;
        needsRedraw = true;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawFlightView();
        needsRedraw = false;
    }
}

// 設定メニュー（SETTINGS）
void handleMenuView() {
    /* TODO : 処理内容の記述（設定メニュー画面の描画・ボタン処理、5.2・5.7参照） */
}

// 設定内容一覧（CONFIG）
void handleConfigView() {
    /* TODO : 処理内容の記述（設定内容一覧画面の描画・ボタン処理、5.2・5.7.1参照） */
}

// SCAN RANGE選択
void handleScanRangeView() {
    /* TODO : 処理内容の記述（SCAN RANGE選択画面の描画・ボタン処理、5.2・5.7.3参照） */
}

// QRコード誘導（APIキー／基準地点）
void handleQrView() {
    /* TODO : 処理内容の記述（QRコード誘導画面の描画・ボタン処理、5.2・5.8参照） */
}

// 確認ダイアログ
void handleConfirmDialog() {
    /* TODO : 処理内容の記述（確認ダイアログの描画・ボタン処理、5.2・5.7.2参照） */
}

// エラー表示
void handleErrorView() {
    /* TODO : 処理内容の記述（エラー画面の描画・ボタン処理、5.2・5.10参照） */
}

// 機体0件
void handleNoFlightsView() {
    /* TODO : 処理内容の記述（機体0件画面の描画・ボタン処理、5.2・5.10参照） */
}

// データ取得中
void handleLoadingView() {
    /* TODO : 処理内容の記述（ローディング画面の描画、5.2・5.11参照） */
}