/*
 * real_time.c
 *
 *  Created on: Jun 24, 2025
 *      Author: DONGYOONLEE
 */


#include "real_time.h"
#include "main.h"



// RTC 시간 설정
// PAT_UTC_Time pAtUtcTime; UTC 시간 구조체 포인터를 사용하여 시간 동기화 함수


// 월 문자열을 숫자로 변환
int Month_String_To_Number(const char* monthStr)
{
    
    if (strcmp(monthStr, "Jan") == 0) return RTC_MONTH_JANUARY;
    if (strcmp(monthStr, "Feb") == 0) return RTC_MONTH_FEBRUARY;
    if (strcmp(monthStr, "Mar") == 0) return RTC_MONTH_MARCH;
    if (strcmp(monthStr, "Apr") == 0) return RTC_MONTH_APRIL;
    if (strcmp(monthStr, "May") == 0) return RTC_MONTH_MAY;
    if (strcmp(monthStr, "Jun") == 0) return RTC_MONTH_JUNE;
    if (strcmp(monthStr, "Jul") == 0) return RTC_MONTH_JULY;
    if (strcmp(monthStr, "Aug") == 0) return RTC_MONTH_AUGUST;
    if (strcmp(monthStr, "Sep") == 0) return RTC_MONTH_SEPTEMBER;
    if (strcmp(monthStr, "Oct") == 0) return RTC_MONTH_OCTOBER;
    if (strcmp(monthStr, "Nov") == 0) return RTC_MONTH_NOVEMBER;
    if (strcmp(monthStr, "Dec") == 0) return RTC_MONTH_DECEMBER;

    return -1; // 잘못된 월 문자열
}

// 요일 문자열을 숫자로 변환
int DayOfWeek_String_To_Number(const char* dayStr)
{
    if (strcmp(dayStr, "Mon") == 0) return RTC_WEEKDAY_MONDAY;
    if (strcmp(dayStr, "Tue") == 0) return RTC_WEEKDAY_TUESDAY;
    if (strcmp(dayStr, "Wed") == 0) return RTC_WEEKDAY_WEDNESDAY;
    if (strcmp(dayStr, "Thu") == 0) return RTC_WEEKDAY_THURSDAY;
    if (strcmp(dayStr, "Fri") == 0) return RTC_WEEKDAY_FRIDAY;
    if (strcmp(dayStr, "Sat") == 0) return RTC_WEEKDAY_SATURDAY;
    if (strcmp(dayStr, "Sun") == 0) return RTC_WEEKDAY_SUNDAY;

    return -1; // 잘못된 요일 문자열
}

// 저장된 UTC 시간으로 RTC 시간을 설정하는 함수
int RTC_Set_UTC(void)
{
    int result = 0;
    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터
    // ──────────────────────────────────────────────────────────────────────────────

    g_Time.Hours = (uint8_t)pAtUtcTime->sHour;       // 시
    g_Time.Minutes = (uint8_t)pAtUtcTime->sMinute;   // 분
    g_Time.Seconds = (uint8_t)pAtUtcTime->sSecond;   // 초
    g_Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE; // 일광 절약 시간 없음
    g_Time.StoreOperation = RTC_STOREOPERATION_RESET; // 저장 작업 없음

    g_Date.WeekDay = (uint8_t)pAtUtcTime->sDayOfWeek; // 요일
    g_Date.Month = (uint8_t)pAtUtcTime->sMonth;       // 월
    g_Date.Date = (uint8_t)pAtUtcTime->sDay;          // 일
    g_Date.Year = (uint8_t)(pAtUtcTime->sYear % 100);   // 연

    // 
    __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);   // RTC 쓰기 보호 해제

    // RTC 시간 설정
    if (HAL_RTC_SetTime(&hrtc, &g_Time, RTC_FORMAT_BIN) != HAL_OK)
    {
        // 초기화 실패 처리
        Error_Handler();
        result = RTC_ERROR; // 실패 코드
    }
    // RTC 날짜 설정
    if (HAL_RTC_SetDate(&hrtc, &g_Date, RTC_FORMAT_BIN) != HAL_OK)
    {
        // 초기화 실패 처리
        Error_Handler();
        result = RTC_ERROR; // 실패 코드
    }

    __HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc); // RTC 쓰기 보호 재설정

    if (result == RTC_ERROR)
    {
        return result; // 오류 발생 시 즉시 반환
    }

    result = RTC_OK; // 성공 코드    
    
    return result;
}

// 저장된 연도를 문자열로 반환하는 함수
const char* RTC_Get_Synced_Year_String(void)
{
    static char yearString[5]; // 정적 버퍼로 연도 문자열 저장
    
    HAL_RTC_GetDate(&hrtc, &g_Date, RTC_FORMAT_BIN);
    
    // snprintf(yearString, sizeof(yearString), "%04d", g_Date.Year + 2000);
    snprintf(yearString, sizeof(yearString), "%02d", (int)g_Date.Year );
    return yearString; // 연도 문자열 반환
}

// UTC 연도를 문자열로 반환하는 함수
const char* UTC_Get_Synced_Year_String(void)
{
    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터}
    
    static char yearString[5]; // 정적 버퍼로 연도 문자열 저장
    snprintf(yearString, sizeof(yearString), "%04d", pAtUtcTime->sYear); // 4자리 연도로 포맷
    return yearString; // 연도 문자열 반환
}


// 저장된 월을 문자열로 반환하는 함수
const char* RTC_Get_Synced_Month_String(void)
{
    static char monthString[4]; // 정적 버퍼로 월 문자열 저장

    HAL_RTC_GetDate(&hrtc, &g_Date, RTC_FORMAT_BIN);

    snprintf(monthString, sizeof(monthString), "%02d", g_Date.Month); // 2자리 월로 포맷
    
    return monthString; // 월 문자열 반환
}
// UTC 월을 문자열로 반환하는 함수
const char* UTC_Get_Synced_Month_String(void)
{
    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터
    
    static char monthString[4]; // 정적 버퍼로 월 문자열 저장
    snprintf(monthString, sizeof(monthString), "%02d", pAtUtcTime->sMonth); // 2자리 월로 포맷
    return monthString; // 월 문자열 반환
}

// 저장된 일을 문자열로 반환하는 함수
const char* RTC_Get_Synced_Date_String(void)
{
    static char dateString[4]; // 정적 버퍼로 일 문자열 저장

    HAL_RTC_GetDate(&hrtc, &g_Date, RTC_FORMAT_BIN);

    snprintf(dateString, sizeof(dateString), "%02d", g_Date.Date); // 2자리 일로 포맷
    
    return dateString; // 일 문자열 반환
}
// UTC 일을 문자열로 반환하는 함수
const char* UTC_Get_Synced_Date_String(void)
{
    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터
    
    static char dateString[4]; // 정적 버퍼로 일 문자열 저장
    snprintf(dateString, sizeof(dateString), "%02d", pAtUtcTime->sDay); // 2자리 일로 포맷
    return dateString; // 일 문자열 반환
}

// 저장된 시를 문자열로 반환하는 함수
const char* RTC_Get_Synced_Hour_String(void)
{
    static char hourString[4]; // 정적 버퍼로 시 문자열 저장

    HAL_RTC_GetTime(&hrtc, &g_Time, RTC_FORMAT_BIN);
    
    snprintf(hourString, sizeof(hourString), "%02d", g_Time.Hours); // 2자리 시로 포맷
    
    return hourString; // 시 문자열 반환
}
// UTC 시를 문자열로 반환하는 함수
const char* UTC_Get_Synced_Hour_String(void)
{
    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터
    
    static char hourString[3]; // 정적 버퍼로 시 문자열 저장
    snprintf(hourString, sizeof(hourString), "%02d", pAtUtcTime->sHour); // 2자리 시로 포맷
    return hourString; // 시 문자열 반환
}

// 저장됨 분을 문자열로 반환하는 함수
const char* RTC_Get_Synced_Minute_String(void)
{
    static char minuteString[4]; // 정적 버퍼로 분 문자열 저장
    
    HAL_RTC_GetTime(&hrtc, &g_Time, RTC_FORMAT_BIN);
    
    snprintf(minuteString, sizeof(minuteString), "%02d", g_Time.Minutes); // 2자리 분으로 포맷
    
    return minuteString; // 분 문자열 반환
}
// UTC 분을 문자열로 반환하는 함수
const char* UTC_Get_Synced_Minute_String(void)
{
    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터
    
    static char minuteString[3]; // 정적 버퍼로 분 문자열 저장
    snprintf(minuteString, sizeof(minuteString), "%02d", pAtUtcTime->sMinute); // 2자리 분으로 포맷
    return minuteString; // 분 문자열 반환
}

// 저장된 초를 문자열로 반환하는 함수
const char* RTC_Get_Synced_Second_String(void)
{
    static char secondString[4]; // 정적 버퍼로 초 문자열 저장
    
    HAL_RTC_GetTime(&hrtc, &g_Time, RTC_FORMAT_BIN);
    
    snprintf(secondString, sizeof(secondString), "%02d", g_Time.Seconds); // 2자리 초로 포맷
    
    return secondString; // 초 문자열 반환
}
// UTC 초를 문자열로 반환하는 함수
const char* UTC_Get_Synced_Second_String(void)  
{
    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터
    
    static char secondString[3]; // 정적 버퍼로 초 문자열 저장
    snprintf(secondString, sizeof(secondString), "%02d", pAtUtcTime->sSecond); // 2자리 초로 포맷
    return secondString; // 초 문자열 반환
}



// 동기화된 시간을 문자열로 반환하는 함수
const char* RTC_Get_Synced_Time_String(void)
{
    static char timeString[32]; // 정적 버퍼로 시간 문자열 저장
    
    HAL_RTC_GetTime(&hrtc, &g_Time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &g_Date, RTC_FORMAT_BIN);

    snprintf(timeString, sizeof(timeString), "%02d-%02d-%02d %02d:%02d:%02d",
             g_Date.Year, g_Date.Month, g_Date.Date,
             g_Time.Hours, g_Time.Minutes, g_Time.Seconds);
    return timeString; // 시간 문자열 반환
}