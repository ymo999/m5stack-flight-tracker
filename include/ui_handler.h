/**
 * ui_handler.h
 * UI描画関連の関数宣言
 */

#ifndef UI_HANDLER_H                        // インクルードガード（二重定義防止）
#define UI_HANDLER_H

#include <Arduino.h>
#include <M5Unified.h>                      // TFT_WHITE等の色定数（デフォルト引数で使用）のため
                                            //  ※M5Unified.h自体に多重インクルード防止機構があるため、
                                            //  　各.cpp側で個別にincludeしても問題は生じない

// SETTINGS画面の項目数（複数ファイルから参照されるため定義）
#define SETTINGS_REGULAR_ITEM_COUNT 5       // RESET ALLを除いた通常項目数（settingsItems[]の要素数と一致）
#define SETTINGS_TOTAL_ITEM_COUNT 6         // RESET ALLを含めた総項目数（cursorIndexの範囲判定に使用）

// ------------------------------------------------------
// 文字列・データ変換系ヘルパー（描画を伴わない）
// ------------------------------------------------------

// 進行方向（度数、0〜360）を8方位の英字表記に変換する
const char* getDirectionLabel(float heading);

// 時刻情報をMM/DD HH:MM形式の文字列に整形する（取得日時の表示用）
String formatUpdateTime(const struct tm& timeInfo);

// 指定文字数に収まらない場合、末尾3文字分を "..." に置き換える
String truncateText(const char* text, int maxLen);

// 整数値を3桁ごとにカンマ区切りした文字列に変換する（高度表示用）
String addThousandsSeparator(int value);

// ------------------------------------------------------
// 共通描画パーツ（複数画面から呼ばれる、画面の一部分を描画）
// ------------------------------------------------------

// 画面全体の初期化（背景クリア・共通テキスト設定）。各画面のdrawXxxView()の冒頭で呼び出す
void initScreenDrawing();

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

#endif