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

// 画面全体の初期化（背景クリア・共通テキスト設定）。各画面のdrawXxxView()の冒頭で呼び出す
void initScreenDrawing();

// タイトルを持たない画面共通の外枠を描画する（確認ダイアログ・機体0件画面・ローディング画面等）
void drawOuterFrame();

// 3つのボタンラベルを描画する（nullptrの位置は描画しない）
void drawButtonLabels(const char* labelA, const char* labelB, const char* labelC);

// 電池残量アイコンを描画する（levelが範囲外の場合は描画しない）
void drawBatteryIcon(int x, int y, int level);

// カーソル選択状態を反映した項目を描画する（背景＋テキストの両方を描画）
// forceBlackOnSelect: 選択時に文字色を黒へ強制するか（RESET ALL等、選択時も文字色を保ちたい項目はfalseを指定）
void drawCursorHighlight(int x, int y, int width, int height, const char* text, 
                        bool isSelected, uint16_t textColor = TFT_WHITE, bool forceBlackOnSelect = true);   

// ------------------------------------------------------
// 画面全体描画（1画面＝1関数）
// ------------------------------------------------------

// 機体情報表示画面（FLIGHT_VIEW）を描画する
void drawFlightView();

// 設定メニュー画面（SETTINGS）を描画する
void drawSettingsView();

// 設定内容一覧画面（CONFIG）を描画する
void drawConfigView();

// 確認ダイアログを描画する
// title    : メインメッセージ（"Change Wi-Fi settings?" 等）
// message1 : 詳細説明の1行目
// message2 : 詳細説明の2行目（不要な場合は nullptr）
void drawConfirmDialog(const char* title, const char* message1, const char* message2);

// SCAN RANGE選択画面を描画する
void drawScanRangeView();

// エラー画面を描画する
// message : レスポンスのerror.message（またはHTTP/JSON解析エラー発生時のオリジナルメッセージ）
// code    : レスポンスのerror.code（同上）
void drawErrorView(const char* message, const char* code);

// 機体0件画面を描画する
void drawNoFlightsView();

// ローディング画面を描画する
// message : 処理段階を示すメッセージ（"Connecting to Wi-Fi..." 等）
void drawLoadingScreen(const char* message);

#endif