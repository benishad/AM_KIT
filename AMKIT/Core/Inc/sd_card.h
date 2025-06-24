/*
 * sd_card.h
 *
 *  Created on: Jun 17, 2025
 *      Author: PROGRAM
 */

#ifndef INC_SD_CARD_H_
#define INC_SD_CARD_H_





// =========================================================
void SD_Card_Boot(void);
void SD_Card_Log(const char *logMessage);

const char* SD_Card_Get_WiFi_SSID(void);
const char* SD_Card_Get_WiFi_Password(void);


#endif /* INC_SD_CARD_H_ */
