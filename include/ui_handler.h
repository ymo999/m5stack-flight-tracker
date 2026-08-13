/**
 * ui_handler.h
 * UI描画関連の関数宣言
 */

#ifndef UI_HANDLER_H                        // インクルードガード（二重定義防止）
#define UI_HANDLER_H

#include <Arduino.h>

// 進行方向（度数、0〜360）を8方位の英字表記に変換する
const char* getDirectionLabel(float heading);

// 時刻情報をMM/DD HH:MM形式の文字列に整形する（取得日時の表示用）
String formatUpdateTime(const struct tm& timeInfo);

// 指定文字数に収まらない場合、末尾3文字分を "..." に置き換える
String truncateText(const char* text, int maxLen);

// 整数値を3桁ごとにカンマ区切りした文字列に変換する（高度表示用）
String addThousandsSeparator(int value);

// 3つのボタンラベルを描画する（nullptrの位置は描画しない）
void drawButtonLabels(const char* labelA, const char* labelB, const char* labelC);

// 電池残量アイコンを描画する（levelが範囲外の場合は描画しない）
void drawBatteryIcon(int x, int y, int level);

// カーソル選択状態を反映した項目を描画する（背景＋テキストの両方を描画）
void drawCursorHighlight(int x, int y, int width, int height, const char* text, bool isSelected, uint16_t textColor = TFT_WHITE);

// 機体情報表示画面（FLIGHT_VIEW）を描画する
void drawFlightView();

#endif