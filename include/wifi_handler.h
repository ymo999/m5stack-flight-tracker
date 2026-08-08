/*
wifi_handler.h
Wi-Fi接続・APモード・キャプティブポータル関連の関数宣言
*/
#ifndef WIFI_HANDLER_H                          // インクルードガード（二重定義防止）
#define WIFI_HANDLER_H

void initWiFi();
void handleWiFiSetup();
bool isWiFiConnected();

#endif