/**
 * @file state_machine.h
 * @brief 画面状態（SystemMode）管理・状態遷移関連の関数宣言
 */

#ifndef STATE_MACHINE_H                     // インクルードガード（二重定義防止）
#define STATE_MACHINE_H

// ------------------------------------------------------
// 画面状態管理（全画面共通）
// ------------------------------------------------------

/**
 * @brief 画面状態（システムの状態）
 */
enum SystemMode {
    MODE_WIFI_SETUP,                        ///< Wi-Fi設定関連（AP接続案内／接続成功／接続失敗）
    MODE_FLIGHT_VIEW,                       ///< 機体情報表示
    MODE_MENU_VIEW,                         ///< 設定メニュー（SETTINGS）
    MODE_CONFIG_VIEW,                       ///< 設定内容一覧（CONFIG）
    MODE_SCAN_RANGE_VIEW,                   ///< SCAN RANGE選択
    MODE_QR_VIEW,                           ///< QRコード誘導（APIキー／基準地点）
    MODE_CONFIRM_DIALOG,                    ///< 確認ダイアログ
    MODE_ERROR_VIEW,                        ///< エラー表示
    MODE_NO_FLIGHTS_VIEW,                   ///< 機体0件
    MODE_LOADING,                           ///< データ取得中
    MODE_CONNECTION_FAILED                  ///< Wi-Fi接続失敗
};

/**
 * @brief state_machine.cpp で定義されている現在の画面状態を共有する
 */
extern SystemMode currentMode;

/**
 * @brief 画面の再描画が必要かどうかを示すフラグ
 * 
 * currentModeの変化時や、表示内容の更新時にtrueへ設定する
 * 各画面処理では、このフラグがtrueの時のみ描画処理を実行し、falseに戻す
 * ※毎ループでの再描画は無駄な処理、かつちらつきの原因になるため
 */
extern bool needsRedraw;

/**
 * @brief 現在アクティブな画面におけるカーソル位置（項目選択画面で共通使用）
 *
 * @note リセット（0代入）は、カーソルを持つ画面を新たに開く遷移でのみ行う
 * 
 * 例1：各画面のSETによるSETTINGSへの遷移の場合...handleFlightView()等の遷移元側で実施する
 * 例2：SETTINGSから一時的に別画面へ移りBACKで戻ってくる経路...リセットしない（選択位置を維持するため）
 */
extern int cursorIndex;

/**
 * @brief 状態管理機構の初期化（起動時の初期状態を設定する）
 */
void initStateMachine();

/**
 * @brief 現在の画面状態（currentMode）に応じて、対応するハンドラ関数を呼び出す
 * 
 * @note loop()から毎回呼び出される想定
 */
void updateStateMachine();

// ------------------------------------------------------
// 各画面での処理を行うための関数
// ------------------------------------------------------

/**
 * @name 各画面状態（SystemMode）に対応する処理を行うための関数
 * 
 * @brief 画面の描画・ボタン処理の分岐を担当する（中身は各画面の実装で記述）
 * 
 */
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
void handleConnectionFailedView();          // 接続失敗

// ------------------------------------------------------
// Wi-Fi設定画面専用（MODE_WIFI_SETUP）
// ------------------------------------------------------
 
/**
 * @brief Wi-Fi設定関連画面の表示フェーズ
 */
enum WiFiSetupPhase {
    WIFI_PHASE_NONE,        ///< MODE_WIFI_SETUP以外の状態では意味を持たない
    WIFI_PHASE_GUIDE,       ///< AP接続案内（APモード起動中）
    WIFI_PHASE_FAILED       ///< 接続失敗
};

/**
 * @brief AP接続案内画面の遷移元（BACKの有無・遷移先の決定に使用）
 * 
 * ConfirmTarget・MenuCallerの考え方を踏襲する
 */
enum WiFiSetupCaller {
    WIFI_CALLER_NONE,       ///< MODE_WIFI_SETUP以外の状態では意味を持たない
    WIFI_CALLER_INIT,       ///< 初回起動 → BACKなし
    WIFI_CALLER_SETTINGS,   ///< SETTINGSから → BACK: MENU_VIEW
    WIFI_CALLER_RECONNECT   ///< 再接続フローから → BACK: FLIGHT_VIEW
};

/**
 * @brief state_machine.cpp で定義されている、Wi-Fi設定関連画面の表示フェーズを共有する
 * 
 * @note MODE_WIFI_SETUP以外の状態では意味を持たない（WIFI_PHASE_NONEを想定）
 */
extern WiFiSetupPhase wifiSetupPhase;

/**
 * @brief state_machine.cpp で定義されている、AP接続案内画面の遷移元を共有する
 * 
 * @note MODE_WIFI_SETUP以外の状態では意味を持たない（WIFI_CALLER_NONEを想定）
 */
extern WiFiSetupCaller wifiSetupCaller;

// ------------------------------------------------------
// SETTINGS画面専用（MODE_MENU_VIEW）
// ------------------------------------------------------

/**
 * @brief SETTINGS画面の項目種別（cursorIndexとの対応付け）
 * 
 * @note 制約：SETTINGS_ITEM_RESET_ALLは常にenumの末尾に置くこと
 * （SETTINGS_ITEM_COUNTの自動算出、およびsettingsItems[]配列の並び順制約[ui_handler_menu.cpp参照]は
 * SETTINGS_ITEM_RESET_ALLが末尾であることを前提としているため）
 */
enum SettingsItemIndex {
    SETTINGS_ITEM_LOCATION,                 ///< LOCATION（基準地点設定）
    SETTINGS_ITEM_API_KEY,                  ///< API KEY
    SETTINGS_ITEM_WIFI,                     ///< Wi-Fi再設定
    SETTINGS_ITEM_SCAN_RANGE,               ///< SCAN RANGE選択
    SETTINGS_ITEM_SHOW_CONFIG,              ///< SHOW CONFIG（設定内容一覧）
    SETTINGS_ITEM_RESET_ALL                 ///< RESET ALL
};

/**
 * @def SETTINGS_ITEM_COUNT
 * @brief SETTINGS画面の項目数
 *
 * 「末尾の値+1」として自動算出する
 *
 * @note 新しい項目を追加してもこの行の変更は不要。ただし追加時は必ずSETTINGS_ITEM_RESET_ALLより前に挿入すること
 */
#define SETTINGS_ITEM_COUNT (SETTINGS_ITEM_RESET_ALL + 1)

/**
 * @brief SETTINGS画面の1項目分のデータ（enum値と表示文字列のペア）
 */
struct SettingsItemEntry {
    SettingsItemIndex index;                ///< この項目のenum値
    const char* label;                      ///< 画面に表示する文字列
};

/**
 * @brief SETTINGS画面の遷移元
 *
 * 確認ダイアログのConfirmTargetの考え方を踏襲する
 */
enum MenuCaller {
    MENUCALLER_NONE,                        ///< 未設定（初期値）
    MENUCALLER_FLIGHT,                      ///< 機体情報表示（FLIGHT_VIEW）からの遷移
    MENUCALLER_NOFLIGHT,                    ///< 機体0件画面からの遷移
    MENUCALLER_ERROR                        ///< エラー画面（キャッシュなし）からの遷移
};

/**
 * @brief state_machine.cppで定義されている、SETTINGS画面の遷移元を共有する
 *
 * @note この変数はMODE_MENU_VIEW以外の状態では意味を持たない（MENUCALLER_NONEを想定）
 */
extern MenuCaller menuCaller;

// ------------------------------------------------------
// SCAN RANGE画面専用（MODE_SCAN_RANGE_VIEW）
// ------------------------------------------------------

/**
 * @brief SCAN RANGE画面専用のカーソル位置（cursorIndexと区別するため個別で定義）
 */
extern int scanRangeCursorIndex;

// ------------------------------------------------------
// 確認ダイアログ専用（MODE_CONFIRM_DIALOG）
// ------------------------------------------------------

/**
 * @brief 確認ダイアログの種別
 * 
 * 「同一の描画内容でも、遷移元によってCANCEL/CONFIRM後の遷移先が異なる」ケースに対応するため、
 * 遷移元ごとに個別の値を用意する（汎用的な「直前の画面」変数は使わない方針）
 */
enum ConfirmTarget {
    CONFIRM_NONE,                           ///< ダイアログ非表示中
    CONFIRM_RESET,                          ///< SETTINGSからのRESET ALL確認
    CONFIRM_WIFI_SETTINGS,                  ///< SETTINGSからのWi-Fi再設定確認
    CONFIRM_WIFI_RECONNECT,                 ///< 機体再取得時の接続失敗からのWi-Fi再接続確認
    CONFIRM_REFRESH                         ///< FLIGHT_VIEW最終機体からのデータ再取得確認
};

/**
 * @brief state_machine.cpp で定義されている、現在表示中の確認ダイアログの種別を共有する
 * 
 * @note この変数はMODE_CONFIRM_DIALOG以外の状態では意味を持たない（CONFIRM_NONEを想定）
 */
extern ConfirmTarget currentConfirm;

#endif