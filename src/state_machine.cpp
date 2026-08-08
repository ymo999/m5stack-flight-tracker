/*
    state_machine.cpp
    状態管理機構に関する処理
*/

// 状態管理用（画面名）の定数定義
enum SystemMode {
    MODE_WIFI_SETUP,                        // Wi-Fi設定関連（AP接続案内／接続成功／接続失敗）
    MODE_FLIGHT_VIEW,                       // 機体情報表示
    MODE_MENU_VIEW,                         // 設定メニュー（SETTINGS）
    MODE_CONFIG_VIEW,                       // 設定内容一覧表示（CONFIG）
    MODE_SCAN_RANGE_VIEW,                   // 範囲切り替え
    MODE_QR_VIEW,                           // QRコード表示（APIキー／基準地点）
    MODE_CONFIRM_DIALOG,                    // 確認ダイアログ
    MODE_ERROR_VIEW,                        // エラー表示
    MODE_NO_FLIGHTS_VIEW,                   // 機体0件
    MODE_LOADING                            // データ取得中
};

