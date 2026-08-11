/**
 * web_handler.h
 * Webサーバー・設定ページ配信関連の関数宣言
 */

#ifndef WEB_HANDLER_H                               // インクルードガード（二重定義防止）
#define WEB_HANDLER_H

// 初回設定ページ（wifi.html）を配信する（GET "/"）
void handleSetupPage();

// フォーム送信内容を受信し、バリデーション後に保存する（POST "/save"）
void handleSave();

#endif