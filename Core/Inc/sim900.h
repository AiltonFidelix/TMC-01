/*
 * gprs.h
 *
 *  Created on: Jan 29, 2023
 *      Author: ailton.fidelix
 */

#ifndef INC_SIM900_H_
#define INC_SIM900_H_

#include "gpio.h"
#include "usart.h"

#define APN "iot4u.br"

void sim900_init(void);
void sim900_set_mode(void);
HAL_StatusTypeDef sim900_send_http_post(char *json);


#endif /* INC_SIM900_H_ */

/**
 * CMD ---> RESP
 *
 * Commands to test device
 * ATE0\r ---> \r\nOK\r\n
 * AT\r ---> \r\nOK\r\n
 *
 * Get signal quality
 * AT+CSQ\r --->  \r\n+CSQ: xx,x\r\n\r\nOK\r\n
 *
 * Set GPRS mode commands
 * AT+SAPBR=3,1,"CONTYPE","GPRS"\r ---> \r\nOK\r\n
 * AT+SAPBR=3,1,"APN","iot4u.br"\r ---> \r\nOK\r\n
 * AT+SAPBR=1,1\r ---> \r\nOK\r\n
 */
