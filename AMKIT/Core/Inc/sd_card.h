/*
 * sd_card.h
 *
 *  Created on: Jun 17, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_SD_CARD_H_
#define INC_SD_CARD_H_





// =========================================================
int SD_Card_Boot(void);
void SD_Card_Log(const char *logMessage);

const char* SD_Card_Get_WiFi_SSID(void);
const char* SD_Card_Get_WiFi_Password(void);

int SD_Card_Is_Exist(void);

enum SDStatus
{
    SD_OK = 0,                // SD 카드 작업 성공
    SD_ERROR,                 // SD 카드 작업 실패
};

#endif /* INC_SD_CARD_H_ */
