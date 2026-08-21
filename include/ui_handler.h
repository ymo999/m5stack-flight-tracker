/**
 * @file ui_handler.h
 * @brief UI描画関連の関数宣言
 */

#ifndef UI_HANDLER_H                        // インクルードガード（二重定義防止）
#define UI_HANDLER_H

#include <Arduino.h>
#include <M5Unified.h>                      // TFT_WHITE等の色定数（デフォルト引数で使用）のため
                                            //  ※M5Unified.h自体に多重インクルード防止機構があるため、
                                            //  　各.cpp側で個別にincludeしても問題は生じない

/**
 * @def SCAN_RANGE_ITEM_COUNT
 * @brief SCAN RANGE画面の項目数（複数ファイルから参照されるため定義）
 *
 * NARROW・WIDEの2項目（scanRangeItems[]の要素数と一致）
 */
#define SCAN_RANGE_ITEM_COUNT 2             // NARROW・WIDEの2項目（scanRangeItems[]の要素数と一致）

// ------------------------------------------------------
// レイアウト共通定数（描画・入力判定の両方から参照）
// ------------------------------------------------------

/**
 * @note 以下3つのBUTTON_AREA_*マクロは、drawButtonLabels()（描画）とinput_handler.cpp
 * （タッチ判定、CoreS3対応）の両方で使用する。値を変更する場合は、両者の整合性が
 * 崩れないよう必ず両方を確認すること
 */
#define BUTTON_AREA_Y       205    // ボタンエリアの上端（区切り線のY座標）
#define BUTTON_AREA_MARGIN  5      // 左右マージン
#define BUTTON_AREA_WIDTH   103    // 1ボタンあたりの幅（(320-BUTTON_AREA_MARGIN*2)/3）

// ------------------------------------------------------
// 文字列・データ変換系ヘルパー（描画を伴わない）
// ------------------------------------------------------

/**
 * @brief 進行方向（度数、0〜360）を8方位の英字表記に変換する
 *
 * @param[in] heading 進行方向（度数、0〜360）
 * @return 8方位の英字表記（"N"、"NE"等）
 */
const char* getDirectionLabel(float heading);

/**
 * @brief 時刻情報をMM/DD HH:MM形式の文字列に整形する（取得日時の表示用）
 *
 * @param[in] timeInfo 整形対象の時刻情報格納先
 * @return MM/DD HH:MM形式の文字列
 */
String formatUpdateTime(const struct tm& timeInfo);

/**
 * @brief 指定文字数に収まらない場合、末尾3文字分を"..."に置き換える
 *
 * @param[in] text 対象の文字列
 * @param[in] maxLen 収める最大文字数
 * @return 整形後の文字列
 */
String truncateText(const char* text, int maxLen);

/**
 * @brief 整数値を3桁ごとにカンマ区切りした文字列に変換する（高度表示用）
 *
 * @param[in] value 変換対象の整数値
 * @return カンマ区切りされた文字列
 */
String addThousandsSeparator(int value);

/**
 * @brief 文字列の先頭・末尾4文字を残し、中間を"****...****"でマスクする（APIキー表示用）
 *
 * @note 短すぎる場合（8文字未満）は全体を"****"で置き換える
 *
 * @param[in] text マスク対象の文字列
 * @return マスク後の文字列
 */
String maskSecret(const String& text);

// ------------------------------------------------------
// 共通描画パーツ（複数画面から呼ばれる、画面の一部分を描画）
// ------------------------------------------------------

/**
 * @brief 画面全体の初期化（背景クリア・共通テキスト設定）
 *
 * @note 各画面のdrawXxxView()の冒頭で呼び出すこと
 */
void initScreenDrawing();

/**
 * @brief タイトルを持たない画面共通の外枠を描画する（確認ダイアログ・機体0件画面・ローディング画面等）
 */
void drawOuterFrame();

/**
 * @brief タイトルを持つ画面共通の外枠を描画する
 */
void drawOuterFrameWithTitle();

/**
 * @brief タイトル（中央揃え）を描画する
 * 
 * @param[in] title タイトルのラベル
 */
void drawTitle(const char* title);

/**
 * @brief 3つのボタンラベルを描画する
 *
 * @param[in] labelA 左ボタンのラベル（nullptrの場合は描画しない）
 * @param[in] labelB 中央ボタンのラベル（nullptrの場合は描画しない）
 * @param[in] labelC 右ボタンのラベル（nullptrの場合は描画しない）
 */
void drawButtonLabels(const char* labelA, const char* labelB, const char* labelC);

/**
 * @brief 電池残量アイコンを描画する
 *
 * @note levelが範囲外（0〜100以外）の場合は描画しない
 *
 * @param[in] x 描画位置のX座標
 * @param[in] y 描画位置のY座標
 * @param[in] level 電池残量（%）
 */
void drawBatteryIcon(int x, int y, int level);

/**
 * @brief カーソル選択状態を反映した項目を描画する（背景＋テキストの両方を描画）
 *
 * @param[in] x 描画位置のX座標
 * @param[in] y 描画位置のY座標
 * @param[in] width 描画エリアの幅
 * @param[in] height 描画エリアの高さ
 * @param[in] text 表示する文字列
 * @param[in] isSelected 選択中かどうか
 * @param[in] textColor 通常時の文字色（デフォルト: TFT_WHITE）
 * @param[in] forceBlackOnSelect 選択時に文字色を黒へ強制するか（デフォルト: true）
 * RESET ALL等、選択時も文字色を保ちたい項目はfalseを指定する
 */
void drawCursorHighlight(int x, int y, int width, int height, const char* text, 
                        bool isSelected, uint16_t textColor = TFT_WHITE, bool forceBlackOnSelect = true);   

// ------------------------------------------------------
// 画面全体描画（1画面＝1関数）
// ------------------------------------------------------

/**
 * @brief Wi-Fi設定関連画面のうち、AP接続案内画面を描画する
 *
 * @note BACKラベルの有無はwifiSetupCallerを直接参照して判定する
 * （WIFI_CALLER_INITの場合のみ、戻り先がないためBACKを表示しない）
 */
void drawWiFiSetupGuide();

/**
 * @brief Wi-Fi設定関連画面のうち、接続失敗画面を描画する
 *
 * @note ボタンラベルの出し分けはtotalFlightCountを直接参照して判定する
 * （キャッシュがある場合のみ、機体情報表示へ戻るBACKとWi-Fi再設定のWi-Fiを併記する）
 */
void drawWiFiSetupFailed();

/**
 * @brief 機体情報表示画面（FLIGHT_VIEW）を描画する
 */
void drawFlightView();

/**
 * @brief 設定メニュー画面（SETTINGS）を描画する
 */
void drawSettingsView();

/**
 * @brief SCAN RANGE選択画面を描画する
 */
void drawScanRangeView();

/**
 * @brief 設定内容一覧画面（CONFIG）を描画する
 */
void drawConfigView();

/**
 * @brief 確認ダイアログを描画する
 *
 * @param[in] title メインメッセージ（"Change Wi-Fi settings?"等）
 * @param[in] message1 詳細説明の1行目
 * @param[in] message2 詳細説明の2行目（不要な場合はnullptr）
 */
void drawConfirmDialog(const char* title, const char* message1, const char* message2);

/**
 * @brief エラー画面を描画する
 *
 * @param[in] message レスポンスのerror.message（またはHTTP/JSON解析エラー発生時のオリジナルメッセージ）
 * @param[in] code レスポンスのerror.code（同上）
 */
void drawErrorView(const char* message, const char* code);

/**
 * @brief 機体0件画面を描画する
 */
void drawNoFlightsView();

/**
 * @brief ローディング画面を描画する
 *
 * @param[in] message 処理段階を示すメッセージ（"Connecting to Wi-Fi..."等）
 */
void drawLoadingScreen(const char* message);

/**
 * @brief Wi-Fi接続失敗の通知画面（CONNECTION_FAILED_VIEW）を描画する
 *
 * @note データ再取得時の接続失敗時のWi-Fi設定関連の接続失敗画面（drawWiFiSetupFailed）とは、レイアウト・文言が異なる別画面
 */
void drawConnectionFailedView();

/**
 * @brief 設定画面へのQRコード誘導画面（APIキー設定／基準地点設定）を描画する
 *
 * @note タイトル以外は左揃え（x=18）で描画する（drawWiFiSetupGuide()と同じ方針）
 *
 * @param[in] title       画面タイトル（"API KEY SETUP" 等）
 * @param[in] path        設定ページのパス（"/api_key" や "/location"）
 * @param[in] extraInfo   現在の設定値等（不要な場合は nullptr）
 * @param[in] statusMsg   待機中等のステータスメッセージ（不要な場合は nullptr）
 * @param[in] buttonLabel BACKラベル（nullptr ならボタン非表示）
 */
void drawSetupQRScreen(const char* title, const char* path, const char* extraInfo,
                        const char* statusMsg, const char* buttonLabel);

#endif