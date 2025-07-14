/*
 * real_time.h
 *
 *  Created on: Jun 24, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_REAL_TIME_H_
#define INC_REAL_TIME_H_


enum RTCStatus
{
    RTC_OK = 0,                // RTC 작업 성공
    RTC_ERROR,                 // RTC 작업 실패
};


int Month_String_To_Number(const char* monthStr);
int DayOfWeek_String_To_Number(const char* dayStr);

int RTC_Set_UTC(void);

const char* RTC_Get_Synced_Year_String(void);
const char* UTC_Get_Synced_Year_String(void);

const char* RTC_Get_Synced_Month_String(void);
const char* UTC_Get_Synced_Month_String(void);

const char* RTC_Get_Synced_Date_String(void);
const char* UTC_Get_Synced_Date_String(void);

const char* RTC_Get_Synced_Hour_String(void);
const char* UTC_Get_Synced_Hour_String(void);

const char* RTC_Get_Synced_Minute_String(void);
const char* UTC_Get_Synced_Minute_String(void);

const char* RTC_Get_Synced_Second_String(void);
const char* UTC_Get_Synced_Second_String(void);

const char* RTC_Get_Synced_Time_String(void);

#endif /* INC_REAL_TIME_H_ */
