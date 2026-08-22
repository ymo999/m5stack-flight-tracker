/**
 * airline_dict.cpp
 * 航空会社辞書（ICAOコード→名称変換）の実装
 *
 * ============================================================
 * データ出典およびライセンスについて
 * ============================================================
 * 本データベースに登録されている航空会社コード（IATA/ICAOコード）・
 * 航空会社名は、以下の公的機関・業界団体が公開している「客観的な事実データ」を
 * 手動で集計・検証したものである。
 *
 * - 日本国内データ: 国土交通省 航空局 航空会社コード一覧
 *   https://www.mlit.go.jp/koku/content/001514113.xlsx
 * - 国際データ: IATA（国際航空運送協会）「Current Airline Members」（公式企業ディレクトリ）
 *   https://www.iata.org/en/about/members/airline-list/
 *
 * 上記機関に対する自動スクレイピング等、利用規約に反する行為は行っていない。
 * （手作業でのコピー＆ペーストによりデータを取得）
 * また本アプリは各機関の公式サービスではないため、データの正確性を保証するものではない。
 * ============================================================
 */

#include "airline_dict.h"

#include <string.h>

// 航空会社辞書データ（国際79社＋国内20社の計99社）
// ICAOコードの昇順にソート済みであること（二分探索の前提条件）
static const AirlineEntry AIRLINE_TABLE[] = {
    {"AAL", "American Airlines"},
    {"AAR", "Asiana Airlines"},
    {"ACA", "Air Canada"},
    {"ADO", "AIRDO"},
    {"AFR", "Air France"},
    {"AHK", "Air Hong Kong"},
    {"AHX", "Amakusa Airlines"},
    {"AIC", "Air India"},
    {"AKX", "ANA Wings"},
    {"AMU", "Air Macau"},
    {"AMX", "Aeromexico"},
    {"ANA", "All Nippon Airways"},
    {"ANG", "Air Niugini"},
    {"ANZ", "Air New Zealand"},
    {"APJ", "Peach Aviation"},
    {"APZ", "Air Premia"},
    {"AUA", "Austrian"},
    {"AZG", "Silk Way West Airlines"},
    {"BAW", "British Airways"},
    {"CAL", "China Airlines"},
    {"CCA", "Air China"},
    {"CDC", "Loong Air"},
    {"CEB", "Cebu Pacific"},
    {"CES", "China Eastern"},
    {"CHH", "Hainan Airlines"},
    {"CLX", "Cargolux"},
    {"CPA", "Cathay Pacific"},
    {"CRK", "Hong Kong Airlines"},
    {"CSC", "Sichuan Airlines"},
    {"CSH", "Shanghai Airlines"},
    {"CSN", "China Southern Airlines"},
    {"CSS", "SF Airlines"},
    {"CSZ", "Shenzhen Airlines"},
    {"CUK", "New Central Airservice"},
    {"CXA", "Xiamen Airlines"},
    {"DAL", "Delta Air Lines"},
    {"DKH", "Juneyao Airlines"},
    {"DLH", "Lufthansa"},
    {"ELY", "EL AL"},
    {"ESR", "Eastar Jet"},
    {"ETH", "Ethiopian Airlines"},
    {"EVA", "EVA Air"},
    {"FDA", "Fuji Dream Airlines"},
    {"FIN", "Finnair"},
    {"FJI", "Fiji Airways"},
    {"GCR", "Tianjin Airlines"},
    {"GIA", "Garuda Indonesia"},
    {"GTI", "Atlas Air"},
    {"HAL", "Hawaiian Airlines"},
    {"HGB", "Greater Bay Airlines"},
    {"HKE", "Hong Kong Express Airways"},
    {"HVN", "Vietnam Airlines"},
    {"HYT", "YTO Cargo Airlines"},
    {"IBE", "IBERIA"},
    {"IBX", "Ibex Airlines"},
    {"ITY", "ITA Airways"},
    {"JAC", "Japan Air Commuter"},
    {"JAL", "Japan Airlines"},
    {"JJA", "Jeju Air"},
    {"JJP", "Jetstar Japan"},
    {"JNA", "Jin Air"},
    {"JTA", "Japan Transocean Air"},
    {"KAL", "Korean Air"},
    {"KHV", "Air Cambodia"},
    {"KLM", "KLM"},
    {"LOT", "LOT Polish Airlines"},
    {"MAS", "Malaysia Airlines"},
    {"MGL", "MIAT Mongolian Airlines"},
    {"MNG", "Aero Mongolia"},
    {"MSR", "Egyptair"},
    {"MXD", "Batik Air Malaysia"},
    {"NTH", "Hokkaido Air System"},
    {"ORC", "Oriental Air Bridge"},
    {"PAC", "Polar Air Cargo"},
    {"PAL", "Philippine Airlines"},
    {"QDA", "Qingdao Airlines"},
    {"QFA", "Qantas"},
    {"QTR", "Qatar Airways"},
    {"RAC", "Ryukyu Air Commuter"},
    {"RBA", "Royal Brunei"},
    {"SAS", "SAS"},
    {"SFJ", "StarFlyer"},
    {"SIA", "Singapore Airlines"},
    {"SJO", "Spring Japan"},
    {"SJX", "STARLUX Airlines"},
    {"SKY", "Skymark Airlines"},
    {"SNJ", "Solaseed Air"},
    {"SWR", "SWISS"},
    {"TGW", "Scoot"},
    {"THA", "Thai Airways International"},
    {"THT", "Air Tahiti Nui"},
    {"THY", "Turkish Airlines"},
    {"TLM", "Thai Lion Air"},
    {"TZP", "ZIPAIR Tokyo"},
    {"UAE", "Emirates"},
    {"UAL", "United Airlines"},
    {"UZB", "Uzbekistan Airways"},
    {"VJC", "Vietjet"},
    {"WJA", "WestJet"},
};

static const int AIRLINE_COUNT = sizeof(AIRLINE_TABLE) / sizeof(AirlineEntry);

// ============================================================
// ICAOコードから航空会社名を取得する（二分探索）
// ============================================================
const char* getAirlineName(const char* icaoCode) {
    int low = 0;
    int high = AIRLINE_COUNT - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        int cmp = strcmp(icaoCode, AIRLINE_TABLE[mid].code);

        if (cmp == 0) {
            return AIRLINE_TABLE[mid].name;
        } else if (cmp < 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    // 見つからない場合はコードをそのまま返す（フォールバック）
    return icaoCode;
}