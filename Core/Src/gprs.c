/*
 * gprs.c
 *
 *  Created on: Jan 29, 2023
 *      Author: ailton.fidelix
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "gprs.h"
//#include "debug.h"

#define DEBUG

static char TAG[] = "gprs";

/**
 * @brief Initial configuration to gprs USART2
 */
void gprs_init(void) {
	MX_USART2_UART_Init();

	gprs_check_cmd("ATE0\r", "\r\nOK\r\n");
	gprs_check_cmd("AT\r", "\r\nOK\r\n");

	//gprs_set_mode();

//#ifdef DEBUG
//	debug(TAG, "gprs module init\n");
//#endif
}

bool gprs_set_mode(void) {

	gprs_check_cmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r", "\r\nOK\r\n");

	char setAPN[100];
	sprintf(setAPN, "AT+SAPBR=3,1,\"APN\",\"%s\"\r", APN);
	gprs_check_cmd(setAPN, "\r\nOK\r\n");

	gprs_check_cmd("AT+SAPBR=1,1\r", "\r\nOK\r\n");

}

HAL_StatusTypeDef grps_send_cmd(char *cmd) {
	return HAL_UART_Transmit(&huart2, cmd, strlen(cmd), 100);
}

HAL_StatusTypeDef gprs_check_response(char *resp) {
	uint16_t len = strlen(resp);
	char receive[255];

	if(HAL_UART_Receive(&huart2, &receive, len, 2000) == HAL_TIMEOUT)
		return HAL_TIMEOUT;

	for(int i = 0; i < len; i++){
		if(receive[i] != resp[i])
			return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef gprs_check_cmd(char *cmd, char *resp) {
	grps_send_cmd(cmd);
	return gprs_check_response(resp);
}
/*
 bool gprs_send_http_post(char *json) {

 }
 */
