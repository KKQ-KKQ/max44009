//
// MAX44009 ambient light sensor
// prototyping code to enable the sensor and read the current value
//
// Written by Larry Bank - 11/30/2017
// Copyright (c) 2017 BitBank Software, Inc.
// bitbank@pobox.com
//
// Modified by KIRA Ryouta - 07/23/2019
// Copyright (c) 2019 KIRA Ryouta
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "max44009.h"

static int file_i2c = 0;

//
// Opens a file system handle to the I2C device
// Sets the device into continuous sampling mode
// returns 0 for success, 1 for failure
//
int max44009Init(int iChannel, int iAddr, char config)
{
char filename[32];
 
	sprintf(filename, "/dev/i2c-%d", iChannel);
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "Failed to open the i2c bus; not running as root?\n");
		file_i2c = 0;
		return 1;
	}

	if (ioctl(file_i2c, I2C_SLAVE, iAddr) < 0)
	{
		fprintf(stderr, "Failed to acquire bus access or talk to slave\n");
		file_i2c = 0;
		return 1;
	}

	if (max44009WriteConfig(config) != 0)
	{
		return 1;
	}

	return 0;
} /* max44009Init() */

int max44009DeInit()
{
int ret;
	ret = close(file_i2c);
	file_i2c = -1;
	return ret;
}

static int read1Byte(char addr, unsigned char *data)
{
unsigned char ucTemp[1];
int i,rc;

	ucTemp[0] = addr;
	rc = write(file_i2c, ucTemp, 1);
	i = read(file_i2c, data, 1);
	if (rc != 1 || i != 1) {
		return 1;
	}
	return 0;
}

int max44009WriteConfig(char config)
{
int rc;
unsigned char ucTemp[2];

	ucTemp[0] = 0x02; // configuration register
	if (read1Byte(0x02, &ucTemp[1]))
	{
		fprintf(stderr, "Error reading config\n");
		return 1;
	}
	if ((ucTemp[1] & 0xc0) != (config & 0xc0) ||
		(((config & 0x40) != 0) && (ucTemp[1] != config)))
	{
		ucTemp[1] = config;
		rc = write(file_i2c, ucTemp, 2);
		if (rc != 2)
		{
			fprintf(stderr, "Error configuring sensor\n");
			return 1;
		}
	}

	return 0;
}

//
// Read the ambient light value in Lux
//
float max44009ReadValue(void)
{
unsigned char ucTemp[3];
int iTim, iMantissa, iExponent;
long long llValue;
float fLux;

//
// We need to read registers 0x3 and 0x4 for the LUX value
// On most systems, a 2-byte read from 0x3 succeeds, but on RPI boards
// there is something wrong with their I2C hardware and it doesn't work
// so we split the operation into 2 separate reads
//
	if (read1Byte(0x2, &ucTemp[0])) return 0.0;
	if (read1Byte(0x3, &ucTemp[1])) return 0.0;
	if (read1Byte(0x4, &ucTemp[2])) return 0.0;

	if (ucTemp[0] & 0x8) {
		iTim = 3 + (ucTemp[0] & 0x7);
	} else {
		iTim = (ucTemp[0] & 0x7);
	}

	iMantissa = ((ucTemp[1] & 0xf)<< 4) | (ucTemp[2] & 0xf);
	iExponent = iTim + (ucTemp[1] >> 4);
	llValue = (long long)iMantissa << iExponent;
	fLux = (float)llValue * 0.045f;
	return fLux;

} /* max44009ReadValue() */
