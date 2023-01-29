/*
 * gprs.h
 *
 *  Created on: Jan 29, 2023
 *      Author: ailton.fidelix
 */

#ifndef INC_GPRS_H_
#define INC_GPRS_H_

#include "usart.h"

#define APN "iot4u.br"

void gprs_init(void);
bool gprs_set_mode(void);
bool gprs_check_cmd(char *cmd, char *resp);
bool gprs_send_http_post(char *json);


#endif /* INC_GPRS_H_ */

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
