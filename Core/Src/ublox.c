/*
 * gps.c
 *
 *  Created on: Jan 29, 2023
 *      Author: ailton.fidelix
 */

#include "ublox.h"

// Global variables
enum _fixtype fixtype; //0 = no fix, 1 = satellite only, 2 = differential fix
enum _fix fix;
enum _op_mode op_mode;

struct Datetime datetime;
struct Satellite sats[12];

char buf[120];

float latitude, longitude, altitude, vert_speed;
float pdop, hdop, vdop; //positional, horizontal and vertical dilution of precision

uint32_t latlng_age, alt_age;
uint32_t time_age, date_age;
uint32_t sats_age;

//these units are in hundredths
//so a speed of 5260 means 52.60km/h
uint16_t speed, course, knots;

int16_t speed_age, course_age, knots_age;
int16_t fixtype_age;
int16_t fix_age;
int16_t dop_age;

uint8_t pos;

int8_t sats_in_use;
int8_t sats_in_view;
// End global variables

// Functions prototypes
bool next(char *out, int len, char *str, char token);
bool encode(char c);
bool process_buf();
bool check_checksum(void);
uint8_t parse_hex(char c);

void read_gga(void);
void read_gsa(void);
void read_gsv(void);
void read_rmc(void);
void read_vtg(void);
// End functions prototypes

void ublox_init(void) {
	MX_USART3_UART_Init();
	HAL_GPIO_WritePin(NEO_RST_GPIO_Port, NEO_RST_Pin, GPIO_PIN_SET);
}

void ublox_reset(void) {
	HAL_GPIO_WritePin(NEO_RST_GPIO_Port, NEO_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(2000);
	HAL_GPIO_WritePin(NEO_RST_GPIO_Port, NEO_RST_Pin, GPIO_PIN_SET);
}

struct GPSData ublox_read(void) {
	struct GPSData data;

	while (HAL_UART_GetState(&huart3) == HAL_UART_STATE_BUSY_RX) {
		char c;
		if (HAL_UART_Receive(&huart3, (uint8_t*)&c, 1, 200) == HAL_TIMEOUT) {
			data.ok = false;
			return data;
		}
		encode(c);
	}

	data.altitude = altitude;
	data.latitude = latitude;
	data.longitude = longitude;
	data.vert_speed = vert_speed;
	data.speed = speed;
	data.course = course;
	data.knots = knots;
	data.sats_in_use = sats_in_use;
	data.sats_in_view = sats_in_view;
	data.datetime = datetime;
	data.ok = true;

	return data;
}

bool next(char *out, int len, char *str, char token) {
	uint8_t count = 0;

	if (str[0] == 0)
		return false;

	while (true) {
		if (str[count] == '\0') {
			out[count] = '\0';
			str = &str[count];
			return true;
		}

		if (str[count] == token) {
			out[count] = '\0';
			count++;
			str = &str[count];
			return true;
		}

		if (count < len)
			out[count] = str[count];

		count++;
	}
	return false;
}

bool encode(char c) {
	buf[pos] = c;
	pos++;

	if (c == '\n') //linefeed
			{
		bool ret = process_buf();
		memset(buf, '\0', 120);
		pos = 0;
		return ret;
	}

	if (pos >= 120) //avoid a buffer overrun
			{
		memset(buf, '\0', 120);
		pos = 0;
	}
	return false;
}

bool process_buf() {
	if (!check_checksum()) //if checksum is bad
	{
		return false; //return
	}

	//otherwise, what sort of message is it
	if (strncmp(buf, "$GNGGA", 6) == 0) {
		read_gga();
	}
	if (strncmp(buf, "$GNGSA", 6) == 0) {
		read_gsa();
	}
	if (strncmp(buf, "$GPGSV", 6) == 0) {
		read_gsv();
	}
	if (strncmp(buf, "$GNRMC", 6) == 0) {
		read_rmc();
	}
	if (strncmp(buf, "$GNVTG", 6) == 0) {
		read_vtg();
	}
	return true;
}

void read_gga(void) {
	int counter = 0;
	char token[20];

	while (next(token, 20, buf, ',')) {
		switch (counter) {
		case 1: { // time
			float time = atof(token);
			int hms = (int) time;

			datetime.millis = time - hms;
			datetime.seconds = fmod(hms, 100);
			hms /= 100;
			datetime.minutes = fmod(hms, 100);
			hms /= 100;
			datetime.hours = hms;

			time_age = HAL_GetTick();
		}
			break;
		case 2: { // latitude
			float llat = atof(token);
			int ilat = llat / 100;
			double mins = fmod(llat, 100);
			latitude = ilat + (mins / 60);
		}
			break;
		case 3: { // north/south
			if (token[0] == 'S')
				latitude = -latitude;
		}
			break;
		case 4: { //longitude
			float llong = atof(token);
			int ilat = llong / 100;
			double mins = fmod(llong, 100);
			longitude = ilat + (mins / 60);
		}
			break;
		case 5: { // east/west
			if (token[0] == 'W')
				longitude = -longitude;
			latlng_age = HAL_GetTick();
		}
			break;
		case 6: {
//			fixtype = _fixtype(atoi(token));
			fixtype = atoi(token);
		}
			break;
		case 7: {
			sats_in_use = atoi(token);
		}
			break;
		case 8: {
			hdop = atoi(token);
		}
			break;
		case 9: {
			float new_alt = atof(token);
			vert_speed = (new_alt - altitude)
					/ ((HAL_GetTick() - alt_age) / 1000.0);
			altitude = atof(token);
			alt_age = HAL_GetTick();
		}
			break;
		}
		counter++;
	}
}

void read_gsa() {
	int counter = 0;
	char token[20];

	while (next(token, 20, buf, ',')) {
		switch (counter) {
		case 1: { //operating mode
			if (token[0] == 'A')
				op_mode = MODE_AUTOMATIC;
			if (token[0] == 'M')
				op_mode = MODE_MANUAL;
		}
			break;
		case 2: {
//			fix = _fix(atoi(token));
			fix = atoi(token);
			fix_age = HAL_GetTick();
		}
			break;
		case 14: {
			pdop = atof(token);
		}
			break;
		case 15: {
			hdop = atof(token);
		}
			break;
		case 16: {
			vdop = atof(token);
			dop_age = HAL_GetTick();
		}
			break;
		}
		counter++;
	}
}

void read_gsv() {
	char token[20];

	next(token, 20, buf, ',');
	next(token, 20, buf, ',');

	next(token, 20, buf, ',');
	int16_t mn = atoi(token); //msg number

	next(token, 20, buf, ',');
	sats_in_view = atoi(token); //number of sats

	int8_t j = (mn - 1) * 4;

	for (int8_t i = 0; i <= 3; i++) {
		next(token, 20, buf, ',');
		sats[j + i].prn = atoi(token);

		next(token, 20, buf, ',');
		sats[j + i].elevation = atoi(token);

		next(token, 20, buf, ',');
		sats[j + i].azimuth = atoi(token);

		next(token, 20, buf, ',');
		sats[j + i].snr = atoi(token);
	}
	sats_age = HAL_GetTick();
}

void read_rmc() {
	int16_t counter = 0;
	char token[20];

	while (next(token, 20, buf, ',')) {
		switch (counter) {
		case 1: //time
		{
			float time = atof(token);
			int hms = (int) time;

			datetime.millis = time - hms;
			datetime.seconds = fmod(hms, 100);
			hms /= 100;
			datetime.minutes = fmod(hms, 100);
			hms /= 100;
			datetime.hours = hms;

			time_age = HAL_GetTick();
		}
			break;
		case 2: {
			if (token[0] == 'A')
				datetime.valid = true;
			if (token[0] == 'V')
				datetime.valid = false;
		}
			break;
			/*
			 case 3:
			 {
			 float llat = atof(token);
			 int ilat = llat/100;
			 double latmins = fmod(llat, 100);
			 latitude = ilat + (latmins/60);
			 }
			 break;
			 case 4:
			 {
			 if(token[0] == 'S')
			 latitude = -latitude;
			 }
			 break;
			 case 5:
			 {
			 float llong = atof(token);
			 float ilat = llong/100;
			 double lonmins = fmod(llong, 100);
			 longitude = ilat + (lonmins/60);
			 }
			 break;
			 case 6:
			 {
			 if(token[0] == 'W')
			 longitude = -longitude;
			 latlng_age = millis();
			 }
			 break;
			 */
		case 8: {
			course = atof(token);
			course_age = HAL_GetTick();
		}
			break;
		case 9: {
			uint32_t date = atoi(token);
			datetime.year = fmod(date, 100);
			date /= 100;
			datetime.month = fmod(date, 100);
			datetime.day = date / 100;
			date_age = HAL_GetTick();
		}
			break;
		}
		counter++;
	}
}

void read_vtg() {
	int counter = 0;
	char token[20];

	while (next(token, 20, buf, ',')) {
		switch (counter) {
		case 1: {
			course = (atof(token) * 100);
			course_age = HAL_GetTick();
		}
			break;
		case 5: {
			knots = (atof(token) * 100);
			knots_age = HAL_GetTick();
		}
			break;
		case 7: {
			speed = (atof(token) * 100);
			speed_age = HAL_GetTick();
		}
			break;
		}
		counter++;
	}
}

bool check_checksum(void) {
	if (buf[strlen(buf) - 5] == '*') {
		uint16_t sum = parse_hex(buf[strlen(buf) - 4]) * 16;
		sum += parse_hex(buf[strlen(buf) - 3]);

		for (uint8_t i = 1; i < (strlen(buf) - 5); i++)
			sum ^= buf[i];
		if (sum != 0)
			return false;

		return true;
	}
	return false;
}

uint8_t parse_hex(char c) {
	if (c < '0')
		return 0;
	if (c <= '9')
		return c - '0';
	if (c < 'A')
		return 0;
	if (c <= 'F')
		return (c - 'A') + 10;
	return 0;
}
