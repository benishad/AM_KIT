/*
 * sim.h
 *
 *  Created on: Jul 14, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_SIM_H_
#define INC_SIM_H_

#include <stdint.h>
#include <stddef.h>

void SIM_M_PWR_KEY_Init(void);
void SIM_PWR_ON(void);
void SIM_PWR_OFF(void);
void SIM_Toggle_PWR(void);

void SIM_USIM_RESET_Init(void);
void SIM_USIM_RESET_Set(void);
void SIM_USIM_RESET_Clear(void);

size_t SIM_UART_ReadData(char *buf, size_t buf_len, uint32_t overall_timeout_ms, uint32_t byte_timeout_ms);

const char* SIM_AT_Send_Command_Sync_Get_Result(const char* cmd);

int SIM_AT_WaitReady(uint32_t timeout_ms);

#endif /* INC_SIM_H_ */
