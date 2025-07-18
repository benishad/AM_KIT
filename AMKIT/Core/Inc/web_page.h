/*
 * web_page.h
 *
 *  Created on: Jul 11, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_WEB_PAGE_H_
#define INC_WEB_PAGE_H_



#define WIFI_HTML_MAX  1024        // 테스트로 일단 1024 설정, 부족시 증가
extern char g_cWifiListHtml[]; // WiFi 리스트 HTML
extern volatile int g_nWifiListReady; // WiFi 리스트 준비 상태


extern const char   htmlBody[];
extern const int htmlBodyLen;

extern const char   htmlBody_2[];
extern const int htmlBody_2Len;

extern const char   htmlBody_inline[];
extern const int htmlBody_inlineLen;

extern const char htmlBody_inline_2[];
extern const int htmlBody_inline_2Len;

extern const char htmlBody_inline_3[];
extern const int htmlBody_inline_3Len;

extern const char htmlBody_inline_4[];
extern const int htmlBody_inline_4Len;


extern const char cssStyle[];
extern const int cssStyleLen;


#endif /* INC_WEB_PAGE_H_ */
