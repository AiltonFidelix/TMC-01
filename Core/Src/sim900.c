/*
 * gprs.c
 *
 *  Created on: Jan 29, 2023
 *      Author: ailton.fidelix
 */

#include "sim900.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

//#include "debug.h"

//#define DEBUG

static char TAG[] = "SIM900";

HAL_StatusTypeDef sim900_check_cmd(char *cmd, char *resp);

/**
 * @brief Initial configuration to gprs USART2
 */
void sim900_init(void) {
	MX_USART2_UART_Init();


	HAL_GPIO_WritePin(SIM_PWR_GPIO_Port, SIM_PWR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SIM_RST_GPIO_Port, SIM_RST_Pin, GPIO_PIN_SET);

	sim900_check_cmd("ATE0\r", "\r\nOK\r\n");
	sim900_check_cmd("AT\r", "\r\nOK\r\n");

	//gprs_set_mode();

//#ifdef DEBUG
//	debug(TAG, "gprs module init\n");
//#endif
}

void sim900_set_mode(void) {

	sim900_check_cmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r", "\r\nOK\r\n");

	char setAPN[100];
	sprintf(setAPN, "AT+SAPBR=3,1,\"APN\",\"%s\"\r", APN);
	sim900_check_cmd(setAPN, "\r\nOK\r\n");

	sim900_check_cmd("AT+SAPBR=1,1\r", "\r\nOK\r\n");

}

HAL_StatusTypeDef sim900_send_cmd(char *cmd) {
	return HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 100);
}

HAL_StatusTypeDef gprs_check_response(char *resp) {
	uint16_t len = strlen(resp);
	char receive[255];

	if(HAL_UART_Receive(&huart2, (uint8_t*)&receive, len, 2000) == HAL_TIMEOUT)
		return HAL_TIMEOUT;

	for(int i = 0; i < len; i++){
		if(receive[i] != resp[i])
			return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef sim900_check_cmd(char *cmd, char *resp) {
	sim900_send_cmd(cmd);
	return gprs_check_response(resp);
}
/*
 bool gprs_send_http_post(char *json) {

 }
 */
