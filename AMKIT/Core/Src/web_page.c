/*
 * web_page.c
 *
 *  Created on: Jul 11, 2025
 *      Author: DONGYOONLEE
 */


 #include "web_page.h"
 #include "main.h"



// IOS safari엔진은 HTTP/1.1에서 Content-Length 헤더나 
// Transfer-Encoding: chunked 없이 응답 바디의 끝을 정확히 알 수 없으면, 
// 바디가 아직 더 남아 있다고 판단해서 렌더링을 보류한다고 함.
// 그래서 호환성을 위해 Content-Length 헤더를 반드시 포함해야 함.
// 또한, HTML 바디는 UTF-8로 인코딩되어야 함.
// 아래는 Content-Length 헤더로 정확한 바디 길이를 명시했음.
// 웹 페이지 HTML 소스
// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
//    _    _ _______ __  __ _      
//   | |  | |__   __|  \/  | |     
//   | |__| |  | |  | \  / | |     
//   |  __  |  | |  | |\/| | |     
//   | |  | |  | |  | |  | | |____ 
//   |_|  |_|  |_|  |_|  |_|______|
//                                 
// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
const char htmlBody[] =
    "<!DOCTYPE html><html><body>"
    "<h1>ESP32 Web Page</h1>"
    "<p>Hello from STM32!</p>"
    "</body></html>";
    
const int htmlBodyLen = sizeof(htmlBody) - 1;

const char htmlBody_2[] = 
    "<!DOCTYPE html><html><head>"
    "<meta charset=\"utf-8\">"
    "<title>ESP32 Web Page</title>"
    "<link rel=\"stylesheet\" href=\"/style.css\">"
    "</head><body>"
        "<h1>ESP32 Web Page</h1>"
        "<p>Hello from STM32!</p>"
        "<button onclick=\"location.reload()\">Reload</button>"
    "</body></html>";

const int htmlBody_2Len = sizeof(htmlBody_2) - 1;



// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
//     _____  _____ _____ 
//    / ____|/ ____/ ____|
//   | |    | (___| (___  
//   | |     \___ \\___ \ 
//   | |____ ____) |___) |
//    \_____|_____/_____/ 
//                        
// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
const char cssStyle[] =
"/* Base Layout */\n"
"body {\n"
"  margin: 0;\n"
"  padding: 0;\n"
"  font-family: \"Helvetica\", sans-serif;\n"
"  background: #f5f5f5;\n"
"  color: #333;\n"
"  text-align: center;\n"
"}\n\n"
"/* Title Style */\n"
"h1 {\n"
"  margin-top: 40px;\n"
"  font-size: 2em;\n"
"  color: #2a7ae2;\n"
"}\n\n"
"/* Paragraph Style */\n"
"p {\n"
"  margin: 20px auto;\n"
"  max-width: 600px;\n"
"  line-height: 1.6;\n"
"}\n\n"
"/* Button Style */\n"
"button {\n"
"  margin: 10px;\n"
"  padding: 12px 24px;\n"
"  font-size: 1em;\n"
"  border: none;\n"
"  border-radius: 4px;\n"
"  background: #2a7ae2;\n"
"  color: white;\n"
"  cursor: pointer;\n"
"  transition: background 0.3s;\n"
"}\n\n"
"/* Buttonqks Hover */\n"
"button:hover {\n"
"  background: #1f5cbf;\n"
"}\n\n"
"/* Responsive (Mobile) */\n"
"@media (max-width: 480px) {\n"
"  h1 {\n"
"    font-size: 1.5em;\n"
"  }\n"
"  button {\n"
"    padding: 10px 20px;\n"
"    font-size: 0.9em;\n"
"  }\n"
"}\n";

const int cssStyleLen = sizeof(cssStyle) - 1;