# M5Stack 航空機スキャンシステム

![Platform](https://img.shields.io/badge/platform-M5Stack%20Basic-000000)
![Language](https://img.shields.io/badge/language-C%2B%2B-00599C)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D)
![Build](https://img.shields.io/badge/build-PlatformIO-FF7F00)
![API](https://img.shields.io/badge/API-AirLabs-2E8B57)
![Status](https://img.shields.io/badge/status-in%20development-yellow)
![AI Assisted](https://img.shields.io/badge/development-AI%20assisted-9cf)

M5Stack（ESP32）を使用し、指定した地点の周辺を飛行中の航空機情報を取得・表示する個人開発ガジェットです。

---

## 概要

ボタン操作をトリガーに、周辺空域の航空機情報を [AirLabs API](https://airlabs.co/) から取得し、M5Stackのディスプレイに一覧表示します。詳細情報（発着予定・実績時刻等）は、画面に表示するQRコードからスマートフォンで [FlightAware](https://ja.flightaware.com/) にアクセスして確認する方式を採用しています。

常時通信を行わない非リアルタイム運用とし、Wi-Fiは必要なタイミングのみ接続することで、API無料枠とバッテリーの両方を効率的に使う設計です。

---

## 主な機能

- 周辺機体スキャン（bbox指定によるAirLabs APIリクエスト、NARROW/WIDEの2段階切替）
- 機体情報表示（便名・航空会社・高度・速度・進行方向・距離・スコーク等）
- FlightAware連携用QRコード[^1]表示（詳細情報はスマートフォン側で確認）
- Wi-Fi未設定時の自動設定案内（キャプティブポータル方式）
- 静的IP設定対応（DHCPのないネットワーク環境でも接続可能）
- APIキー・取得地点（緯度経度）のWebページ経由での登録・変更
- 設定内容一覧表示・全設定リセット（確認ダイアログ付き）
- 機体情報キャッシュ・残りAPIリクエスト数の保持
- Wi-Fi低消費電力運用（通常時OFF、必要時のみON）

---

## 動作環境

| 項目 | 内容 |
|---|---|
| 対象デバイス | M5Stack Basic |
| 開発環境 | VS Code + PlatformIO |
| フレームワーク | Arduino |
| 主要ライブラリ | M5Unified, ArduinoJson, TinyGPSPlus |

---

## セットアップ

1. このリポジトリをクローンする
2. VS Code + PlatformIO 拡張機能をインストールする
3. `platformio.ini` の依存ライブラリを取得する（PlatformIOが自動解決）
4. M5Stack BasicをUSB接続し、ビルド・書き込みを行う
5. 初回起動時、画面の案内に従いWi-Fi・APIキー・取得地点を設定する

---

## 使い方

1. 起動後、ボタン操作でスキャン（機体情報取得）を実行する
2. 一覧から機体を選び、簡易情報（便名・高度・速度等）を確認する
3. 詳細情報が必要な場合は、画面のQRコードをスマートフォンで読み取り、FlightAwareで確認する
4. 設定変更（Wi-Fi・APIキー・取得地点）は設定メニューから行う

---

## プロジェクト構成

```
Your-Project-Folder/
├── data/                // Webページ用HTML（Wi-Fi設定・APIキー設定・基準地点設定）
├── include/            // ヘッダファイル（関数宣言・構造体・定数定義）
├── src/                 // 実装ファイル（main.cpp 他）
├── platformio.ini
└── docs/                // 仕様書・設計資料
```

詳細な仕様は `docs/` 内の「プロジェクト仕様書」を参照してください。

---

## AI協働について

本プロジェクトは、要件定義・設計・仕様書作成・実装の各フェーズにおいて、AIアシスタント（Claude、Gemini等）との対話を活用しながら進めています。

- 仕様の検討・整理、フロー設計、コード実装のたたき台作成などにAIを活用
- 技術的な決定（採用ライブラリ、保存方式、通信方式等）は、AIからの助言を参考に開発者が最終判断
- 仕様書・設計資料は、複数のAIチャットでのやり取りを統合・整理して作成

AIの助言はあくまで参考情報であり、実機での動作確認・最終的な設計判断は開発者が行っています。

---

## ステータス

現在、要件定義・設計フェーズを経て実装フェーズに着手した段階です。詳細な進捗・工程は別途工程管理ドキュメントで管理しています。

---

## ライセンス

未定（個人開発プロジェクトのため、現時点では非公開/検討中）

---

## 注釈

[^1]: QRコードは株式会社デンソーウェーブの登録商標です。
