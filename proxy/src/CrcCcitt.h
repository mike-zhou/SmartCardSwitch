/*
 * CrcCcitt.h
 *
 *  Created on: Mar 19, 2019
 *      Author: mikez
 */

#ifndef CRCCCITT_H_
#define CRCCCITT_H_

#include <stddef.h>

class CrcCcitt
{
public:
	CrcCcitt()
	{
		crc_tabccitt_init = false;
	}

	unsigned short GetCRC(unsigned char * pData, unsigned int length)
	{
		return crc_ccitt_generic( pData, length, 0xFFFF);
	}

private:
	static const unsigned short CRC_POLY_CCITT	= 0x1021;

	bool crc_tabccitt_init;
	unsigned short crc_tabccitt[256];

	unsigned short crc_ccitt_generic( const unsigned char *input_str, int num_bytes, unsigned short start_value )
	{
		unsigned short crc;
		unsigned short tmp;
		unsigned short short_c;
		const unsigned char *ptr;
		int a;

		if ( ! crc_tabccitt_init ) init_crcccitt_tab();

		crc = start_value;
		ptr = input_str;

		if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

			short_c = 0x00ff & (unsigned short) *ptr;
			tmp     = (crc >> 8) ^ short_c;
			crc     = (crc << 8) ^ crc_tabccitt[tmp];

			ptr++;
		}

		return crc;
	}

	void init_crcccitt_tab( void )
	{
		unsigned short i;
		unsigned short j;
		unsigned short crc;
		unsigned short c;

		for (i=0; i<256; i++) {

			crc = 0;
			c   = i << 8;

			for (j=0; j<8; j++) {

				if ( (crc ^ c) & 0x8000 ) crc = ( crc << 1 ) ^ CRC_POLY_CCITT;
				else                      crc =   crc << 1;

				c = c << 1;
			}

			crc_tabccitt[i] = crc;
		}

		crc_tabccitt_init = true;
	}
};

#endif /* CRCCCITT_H_ */
