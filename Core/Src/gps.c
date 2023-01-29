/*
 * gps.c
 *
 *  Created on: Jan 29, 2023
 *      Author: ailton.fidelix
 */

#include "gps.h"

void gps_init(void) {
	MX_USART3_UART_Init();
	HAL_GPIO_WritePin(NEO_RST_GPIO_Port, NEO_RST_Pin, GPIO_PIN_SET);
}

void gps_reset(void) {
	HAL_GPIO_WritePin(NEO_RST_GPIO_Port, NEO_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(2000);
	HAL_GPIO_WritePin(NEO_RST_GPIO_Port, NEO_RST_Pin, GPIO_PIN_SET);
}
