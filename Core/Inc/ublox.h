/*
 * ublox.h
 *
 *  Created on: Jan 29, 2023
 *      Author: ailton.fidelix
 */

#ifndef INC_UBLOX_H_
#define INC_UBLOX_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gpio.h"
#include "usart.h"

enum _fixtype {
	FIX_TYPE_NONE, FIX_TYPE_GPS, FIX_TYPE_DIFF
};

enum _fix {
	FIX_NONE = 1, FIX_2D, FIX_3D
};

enum _op_mode {
	MODE_MANUAL, MODE_AUTOMATIC
};

struct Satellite {
	uint8_t prn;
	int16_t elevation;
	int16_t azimuth;
	uint8_t snr; //signal to noise ratio
};

struct Datetime {
	uint8_t day, month, year;
	uint8_t hours, minutes, seconds;
	uint16_t millis;
	bool valid; //1 = yes, 0 = no
};

struct GPSData {
	float altitude;
	float latitude;
	float longitude;
	float vert_speed;
	uint16_t speed;
	uint16_t course;
	uint16_t knots;
	int8_t sats_in_use;
	int8_t sats_in_view;
	struct Datetime datetime;
	bool ok;
};

void ublox_init(void);
void ublox_reset(void);
struct GPSData ublox_read(void);

#endif /* INC_UBLOX_H_ */
