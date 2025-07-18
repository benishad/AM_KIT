/*
 * web_page.c
 *
 *  Created on: Jul 11, 2025
 *      Author: DONGYOONLEE
 */


 #include "web_page.h"
 #include "main.h"




char g_cWifiListHtml[WIFI_HTML_MAX] = "<li>Scanning…</li>";
volatile int g_nWifiListReady = 0;



// ──────────────────────────────────────────────────────────────────────────────



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

const char htmlBody_inline[] =
  "<!DOCTYPE html><html><head>"
    "<meta charset=\"utf-8\">"
    "<title>ESP32 Web Page</title>"
    "<style>"
      "body {"
        "margin: 0; padding: 0;"
        "font-family: Helvetica, sans-serif;"
        "background: #f5f5f5; color: #333;"
        "text-align: center;"
      "}"
      "h1 { margin: 40px 0; font-size: 2em; color: #2a7ae2; }"
      "p { max-width: 600px; margin: 20px auto; line-height: 1.6; }"
      "button {"
        "margin: 10px; padding: 12px 24px;"
        "font-size: 1em; border: none; border-radius: 4px;"
        "background: #2a7ae2; color: white; cursor: pointer;"
        "transition: background 0.3s;"
      "}"
      "button:hover { background: #1f5cbf; }"
      "@media (max-width: 480px) {"
        "h1 { font-size: 1.5em; }"
        "button { padding: 10px 20px; font-size: 0.9em; }"
      "}"
    "</style>"
  "</head><body>"
    "<h1>ESP32 Web Page</h1>"
    "<p>Hello from STM32!</p>"
    "<button onclick=\"location.reload()\">Reload</button>"
    "<button onclick=\"window.close()\">QUIT</button>"
  "</body></html>";

const int htmlBody_inlineLen = sizeof(htmlBody_inline) - 1;

const char htmlBody_inline_2[] =
  "<!DOCTYPE html><html><head>"
    "<meta charset=\"utf-8\">"
    "<title>ESP32 Web Page</title>"
    "<style>"
      "body {"
        "margin: 0; padding: 0;"
        "font-family: Helvetica, sans-serif;"
        "background: #f5f5f5; color: #333;"
        "text-align: center;"
      "}"
      "h1 { margin: 40px 0; font-size: 2em; color: #2a7ae2; }"
      "p { max-width: 600px; margin: 20px auto; line-height: 1.6; }"
      "button {"
        "margin: 10px; padding: 12px 24px;"
        "font-size: 1em; border: none; border-radius: 4px;"
        "background: #2a7ae2; color: white; cursor: pointer;"
        "transition: background 0.3s;"
      "}"
      "button:hover { background: #1f5cbf; }"
      "@media (max-width: 480px) {"
        "h1 { font-size: 1.5em; }"
        "button { padding: 10px 20px; font-size: 0.9em; }"
      "}"
    "</style>"
    "<script>"
      "window.addEventListener('load', fetchNetworks);"
      "function ledDevice() {"
        "fetch('/led')"
          ".then(response => response.text())"
          ".then(text => console.log(text))"
          ".catch(err => console.error(err));"
      "}"
      "function fetchNetworks() {"
        "fetch('/scan')"
          ".then(res => res.text())"
          ".then(html => {"
            "document.getElementById('wifiList').innerHTML = html;"
          "})"
          ".catch(err => console.error(err));"
      "}"
    "</script>"
  "</head><body>"
    "<h1>ESP32 Web Page</h1>"
    "<p>Hello from STM32!</p>"
    "<h2>Available Wi‑Fi Networks</h2>"
    "<ul id=\"wifiList\"><li>Scanning…</li></ul>"
    "<button onclick=\"location.reload()\">Reload</button>"
    "<button onclick=\"ledDevice()\">LED</button>"
  "</body></html>";

const int htmlBody_inline_2Len = sizeof(htmlBody_inline_2) - 1;


const char htmlBody_inline_3[] =
  "<!DOCTYPE html><html><head>"
    "<meta charset=\"utf-8\">"
    "<title>ESP32 Web Page</title>"
    "<style>"
      "body {"
        "margin: 0; padding: 0;"
        "font-family: Helvetica, sans-serif;"
        "background: #f5f5f5; color: #333;"
        "text-align: center;"
      "}"
      "h1 { margin: 40px 0; font-size: 2em; color: #2a7ae2; }"
      "p { max-width: 600px; margin: 20px auto; line-height: 1.6; }"
      "button {"
        "margin: 10px; padding: 12px 24px;"
        "font-size: 1em; border: none; border-radius: 4px;"
        "background: #2a7ae2; color: white; cursor: pointer;"
        "transition: background 0.3s;"
      "}"
      "button:hover { background: #1f5cbf; }"
      "@media (max-width: 480px) {"
        "h1 { font-size: 1.5em; }"
        "button { padding: 10px 20px; font-size: 0.9em; }"
      "}"
    "</style>"
    "<script>"
      "function ledDevice() {"
        "fetch('/led')"
          ".then(response => response.text())"
          ".then(text => console.log(text))"
          ".catch(err => console.error(err));"
      "}"
      "function startScanLoop() {"
        "const list  = document.getElementById('wifiList');"
        "list.innerHTML = '<li>Scanning…</li>';"
        "const loop = setInterval(() => {"
          "fetch('/scan', {cache:'no-store'})"
            ".then(r => r.text())"
            ".then(html => {"
              "if (html.includes('<li>')) {"
                "list.innerHTML = html;"   /* 성공 → 목록 표시 */
                "clearInterval(loop);"     /* 루프 종료 */
              "}"
            "})"
            ".catch(() => {});"            /* 실패 무시하고 재시도 */
        "}, 1500);"                         /* 1.5 초 주기 */
      "}"
      "window.addEventListener('load', startScanLoop);"
    "</script>"
  "</head><body>"
    "<h1>Welcome to AMKIT</h1>"
    "<p>ANDAMIORO</p>"
    "<h2>Available Wi‑Fi Networks</h2>"
    "<ul id=\"wifiList\"><li>Scanning…</li></ul>"
    "<button onclick=\"location.reload()\">Reload</button>"
    "<button onclick=\"ledDevice()\">LED</button>"
  "</body></html>";

const int htmlBody_inline_3Len = sizeof(htmlBody_inline_3) - 1;



const char htmlBody_inline_4[] =
"<!DOCTYPE html><html><head>"
"<meta charset=\"utf-8\">"
"<title>Welcome to AMKIT</title>"
"<style>"
"body{margin:0;padding:0;font-family:Helvetica,Arial,sans-serif;"
"background:#f5f5f5;color:#333;text-align:center}"
"h1{margin:40px 0;font-size:2em;color:#2a7ae2}"
"h2{margin-top:24px}"
"ul{list-style:none;padding:0}"
"li{margin:6px 0}"
"label{cursor:pointer}"
"input[type=radio]{margin-right:6px}"
"input[type=password]{padding:8px 10px;width:180px;border:1px solid #aaa;"
"border-radius:4px}"
"button{margin:12px 6px;padding:12px 24px;font-size:1em;border:0;"
"border-radius:4px;background:#2a7ae2;color:#fff;cursor:pointer;"
"transition:background .3s}"
"button:hover{background:#1f5cbf}"
"@media(max-width:480px){h1{font-size:1.5em}"
"button{padding:10px 20px;font-size:.9em}}"
"</style>"

"<script>"
"let selectedSSID='';"                                   /* 현재 선택 SSID  */
"function ledDevice(){fetch('/led?toggle=1').catch(()=>{});}"

/* ---------- 스캔 루프 ---------- */
"function startScanLoop(){"
" const list=document.getElementById('wifiList');"
" list.innerHTML='<li>Scanning…</li>';"
" const loop=setInterval(()=>{"
"  fetch('/scan',{cache:'no-store'})"
"   .then(r=>r.text()).then(html=>{"
"     if(html.includes('<li')){"
"       list.innerHTML=html;"           /* 목록 삽입 */
"       /* 각 li에 data-ssid 속성 읽어 클릭 이벤트 부여 */"
"       document.querySelectorAll('#wifiList li').forEach(li=>{"
"         li.addEventListener('click',()=>{"
"           selectedSSID=li.dataset.ssid||li.textContent;"
"           document.getElementById('sel').textContent=selectedSSID;"
"         });"
"       });"
"       clearInterval(loop);"           /* 성공 → 루프 종료 */
"     }"
"   }).catch(()=>{});"
" },1500);"
"}"

/* ---------- 연결 요청 ---------- */
"function connectWifi(){"
" if(!selectedSSID){alert('SSID를 선택하세요');return;}"
" const pw=document.getElementById('pwd').value;"
" const url='/connect?ssid='+encodeURIComponent(selectedSSID)"
"           +'&pw='+encodeURIComponent(pw);"
" fetch(url)"
"   .then(()=>alert('연결 요청 전송됨'))"
"   .catch(()=>alert('전송 실패'));"
"}"

"window.addEventListener('load',startScanLoop);"
"</script>"
"</head><body>"
"<h1>Welcome to AMKIT</h1>"
"<p>ANDAMIRO</p>"

"<h2>Available Wi‑Fi Networks</h2>"
"<ul id=\"wifiList\"><li>Scanning…</li></ul>"

"<p>선택 SSID: <strong id=\"sel\">(없음)</strong></p>"
"<input type=\"password\" id=\"pwd\" placeholder=\"비밀번호(열린망은 빈칸)\">"
"<br>"
"<button onclick=\"connectWifi()\">Connect</button>"
"<button onclick=\"location.reload()\">Reload</button>"
"<button onclick=\"ledDevice()\">LED</button>"

"</body></html>";

const int htmlBody_inline_4Len = sizeof(htmlBody_inline_4) - 1;



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



// ──────────────────────────────────────────────────────────────────────────────