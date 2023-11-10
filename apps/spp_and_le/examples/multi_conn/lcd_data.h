#ifndef _LCD_DATA_H_
#define _LCD_DATA_H_

const unsigned char DotTbl12Ascii[][16] = 
{
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // - -

     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00, // -.-

 	0x00,0x00,0x7C,0xC6,0xC6,0xCE,0xD6,0xD6,0xE6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00, // -0-

	0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00, // -1-

	0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,0x60,0xC0,0xC6,0xFE,0x00,0x00,0x00,0x00, // -2-

	0x00,0x00,0x7C,0xC6,0x06,0x06,0x3C,0x06,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00, // -3-

	0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00, // -4-

	0x00,0x00,0xFE,0xC0,0xC0,0xC0,0xFC,0x0E,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00, // -5-

	0x00,0x00,0x38,0x60,0xC0,0xC0,0xFC,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00, // -6-

	0x00,0x00,0xFE,0xC6,0x06,0x06,0x0C,0x18,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00, // -7-

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00, // -8-

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,0x06,0x06,0x0C,0x78,0x00,0x00,0x00,0x00, // -9-
      
      0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00, // -A-

	0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0x66,0xFC,0x00,0x00,0x00,0x00, // -B-

	0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xC0,0xC0,0xC2,0x66,0x3C,0x00,0x00,0x00,0x00, // -C-

	0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00, // -D-

	0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00, // -E-

	0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00, // -F-

     0x00,0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00, // -a-

	0x00,0x00,0xE0,0x60,0x60,0x78,0x6C,0x66,0x66,0x66,0x66,0xDC,0x00,0x00,0x00,0x00, // -b-

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00, // -c-

	0x00,0x00,0x1C,0x0C,0x0C,0x3C,0x6C,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00, // -d-

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00, // -e-

	0x00,0x00,0x38,0x6C,0x64,0x60,0xF0,0x60,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00, // -f-

	0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,0x00, // -g-

	0x00,0x00,0xE0,0x60,0x60,0x6C,0x76,0x66,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00, // -h-

	0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00, // -i-

	0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00, // -j-

	0x00,0x00,0xE0,0x60,0x60,0x66,0x6C,0x78,0x78,0x6C,0x66,0xE6,0x00,0x00,0x00,0x00, // -k-

	0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00, // -l-

	0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xD6,0xD6,0xD6,0x00,0x00,0x00,0x00, // -m-

	0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00, // -n-

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00, // -o-

	0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00, // -p-

	0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,0x00, // -q-

	0x00,0x00,0x00,0x00,0x00,0xDC,0x76,0x62,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00, // -r-

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00,0x00,0x00,0x00, // -s-

	0x00,0x00,0x10,0x30,0x30,0xFC,0x30,0x30,0x30,0x30,0x36,0x1C,0x00,0x00,0x00,0x00, // -t-

	0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00, // -u-

	0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00, // -v-

	0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xD6,0xD6,0xFE,0x6C,0x00,0x00,0x00,0x00, // -w-

	0x00,0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00, // -x-

	0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,0x00, // -y-

	0x00,0x00,0x00,0x00,0x00,0xFE,0xCC,0x18,0x30,0x60,0xC6,0xFE,0x00,0x00,0x00,0x00, // -z-


     

      0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00, // ->-

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ---

      0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00, // -:-

      0x00,0x00,0x00,0x00,0xC2,0xC6,0x0C,0x18,0x30,0x60,0xC6,0x86,0x00,0x00,0x00,0x00, // -%-
};
const char DotTbl24AsciiString[] =" .0123456789ABCDEFabcdefghijklmnopqrstuvwxyz>-:%";

/*
const unsigned char DotTbl12Ascii[][48] = 
{
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // - -
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -.-
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x1E,0x00,
      0x1E,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x03,0x80,0x06,0xC0,0x0C,0x60,0x0C,0x60,0x18,0x30,0x18,0x30,0x30,0x18,0x30,0x18,  // -0-
      0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x18,0x30,0x18,0x30,0x0C,0x60,0x0C,0x60,
      0x06,0xC0,0x03,0x80,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x01,0x80,0x07,0x80,0x1F,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,  // -1-
      0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
      0x01,0x80,0x0F,0xF0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x07,0xC0,0x0C,0x60,0x18,0x30,0x38,0x18,0x38,0x18,0x18,0x18,0x00,0x18,0x00,0x30,  // -2-
      0x00,0x30,0x00,0x60,0x00,0xC0,0x01,0x80,0x03,0x00,0x06,0x00,0x0C,0x00,0x1C,0x00,0x18,0x08,0x38,0x08,
      0x3F,0xF8,0x3F,0xF8,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x03,0xC0,0x0E,0x70,0x1C,0x38,0x1C,0x18,0x0C,0x18,0x00,0x18,0x00,0x30,0x00,0x60,  // -3-
      0x00,0xC0,0x03,0x80,0x00,0xE0,0x00,0x30,0x00,0x18,0x00,0x18,0x00,0x18,0x18,0x18,0x38,0x18,0x38,0x38,
      0x1C,0x70,0x0F,0xC0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x60,0x00,0xE0,0x00,0xE0,0x01,0xE0,0x01,0x60,0x03,0x60,0x02,0x60,0x06,0x60,  // -4-
      0x04,0x60,0x0C,0x60,0x08,0x60,0x18,0x60,0x10,0x60,0x30,0x60,0x3F,0xFC,0x00,0x60,0x00,0x60,0x00,0x60,
      0x00,0x60,0x01,0xF8,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x1F,0xF0,0x1F,0xF0,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x19,0xC0,  // -5-
      0x1A,0x60,0x1C,0x30,0x10,0x30,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x18,0x18,0x38,0x30,0x38,0x30,
      0x1C,0x60,0x0F,0xC0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x07,0xC0,0x0C,0xE0,0x18,0x70,0x18,0x70,0x30,0x60,0x30,0x00,0x30,0x00,0x33,0xC0,  // -6-
      0x34,0x60,0x38,0x30,0x30,0x30,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x18,0x30,0x18,0x30,
      0x0C,0x60,0x07,0xC0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x1F,0xF8,0x3F,0xF8,0x30,0x18,0x20,0x10,0x20,0x30,0x00,0x20,0x00,0x60,0x00,0x40,  // -7-
      0x00,0xC0,0x00,0xC0,0x01,0x80,0x01,0x80,0x01,0x80,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,
      0x03,0x00,0x03,0x00,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x07,0xC0,0x0C,0x60,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x0C,0x60,0x06,0xC0,  // -8-
      0x03,0x80,0x07,0xC0,0x0C,0x60,0x18,0x30,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x18,0x30,
      0x0C,0x60,0x07,0xC0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x07,0xC0,0x0C,0x60,0x18,0x30,0x18,0x30,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,  // -9-
      0x30,0x18,0x18,0x18,0x18,0x38,0x0C,0x58,0x07,0x98,0x00,0x18,0x00,0x18,0x0C,0x18,0x1C,0x30,0x1C,0x30,
      0x0E,0x60,0x07,0xC0,0x00,0x00,0x00,0x00,

      	0x00,0x00,0x00,0x00,0x06,0x00,0x06,0x00,0x0B,0x00,0x0B,0x00,0x09,0x00,0x09,0x00,0x19,0x80,0x19,0x80,0x10,0x80,0x10,0x80,  // -A-
	0x30,0xC0,0x30,0xC0,0x3F,0xC0,0x30,0xC0,0x20,0x40,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0xF0,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0xFE,0x00,0x63,0x00,0x61,0x80,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x61,0x80,0x63,0x00,0x7E,0x00,  // -B-
	0x63,0x80,0x60,0xC0,0x60,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0xC0,0x60,0xC0,0x61,0x80,0xFF,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x1F,0x20,0x31,0xE0,0x60,0xE0,0x60,0x60,0xC0,0x20,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,  // -C-
	0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x60,0x60,0x60,0x60,0xC0,0x31,0x80,0x1F,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0xFE,0x00,0x63,0x00,0x61,0x80,0x60,0xC0,0x60,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,  // -D-
	0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0xC0,0x60,0xC0,0x61,0x80,0x63,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0xFF,0xE0,0x60,0xE0,0x60,0x60,0x60,0x20,0x60,0x20,0x60,0x00,0x61,0x00,0x61,0x00,0x63,0x00,0x7F,0x00,  // -E-
	0x63,0x00,0x61,0x00,0x61,0x00,0x60,0x00,0x60,0x00,0x60,0x20,0x60,0x20,0x60,0x60,0x60,0xE0,0xFF,0xE0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0xFF,0xE0,0x60,0xE0,0x60,0x60,0x60,0x20,0x60,0x20,0x60,0x00,0x61,0x00,0x61,0x00,0x63,0x00,0x7F,0x00,  // -F-
	0x63,0x00,0x61,0x00,0x61,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xC0,  // -a-
      0x18,0x60,0x3C,0x30,0x18,0x30,0x00,0x30,0x03,0xF0,0x0E,0x30,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
      0x38,0xF0,0x0F,0x98,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x38,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x1B,0xC0,  // -b-
      0x1E,0x60,0x1C,0x30,0x18,0x30,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x30,0x1C,0x30,
      0x1E,0x60,0x33,0xC0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xC0,  // -c-
      0x0C,0x70,0x18,0x38,0x18,0x78,0x30,0x30,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x18,0x00,0x18,0x18,
      0x0C,0x30,0x07,0xE0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x07,0xD8,  // -d-
      0x0C,0x78,0x18,0x38,0x18,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x18,0x18,0x18,0x38,
      0x0C,0x78,0x07,0xC8,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xC0,  // -e-
      0x0C,0x60,0x18,0x30,0x18,0x30,0x30,0x18,0x30,0x18,0x3F,0xF8,0x30,0x00,0x30,0x00,0x18,0x00,0x18,0x18,
      0x0C,0x30,0x07,0xE0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x01,0xF0,0x03,0x38,0x06,0x38,0x06,0x10,0x06,0x00,0x06,0x00,0x06,0x00,0x3F,0xC0,  // -f-
      0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,
      0x06,0x00,0x1F,0x80,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x98,  // -g-
      0x18,0xF8,0x30,0x60,0x30,0x60,0x30,0x60,0x18,0xC0,0x0F,0x80,0x18,0x00,0x1E,0x00,0x33,0xE0,0x30,0x70,
      0x30,0x18,0x30,0x18,0x18,0x30,0x0F,0xE0,

      0x00,0x00,0x00,0x00,0x38,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x19,0xC0,  // -h-
      0x1B,0x60,0x1E,0x30,0x1C,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,
      0x18,0x30,0x3C,0x78,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x01,0x80,0x03,0xC0,0x03,0xC0,0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x80,  // -i-
      0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
      0x01,0x80,0x07,0xE0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0xC0,0x01,0xE0,0x01,0xE0,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xE0,  // -j-
      0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x18,0x60,
      0x3C,0x60,0x38,0x60,0x18,0xC0,0x0F,0x80,

      0x00,0x00,0x00,0x00,0x38,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0xF8,  // -k-
      0x18,0x60,0x18,0xC0,0x19,0x80,0x1B,0x00,0x1F,0x80,0x1D,0x80,0x18,0xC0,0x18,0xC0,0x18,0x60,0x18,0x60,
      0x18,0x30,0x3C,0x78,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x07,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,  // -l-
      0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
      0x01,0x80,0x07,0xE0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3B,0x70,  // -m-
      0x1F,0xF8,0x19,0x98,0x19,0x98,0x19,0x98,0x19,0x98,0x19,0x98,0x19,0x98,0x19,0x98,0x19,0x98,0x19,0x98,
      0x19,0x98,0x3D,0xDC,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x39,0xC0,  // -n-
      0x1B,0x60,0x1E,0x30,0x1C,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,
      0x18,0x30,0x3C,0x78,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xC0,  // -o-
      0x0C,0x60,0x18,0x30,0x18,0x30,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x18,0x30,0x18,0x30,
      0x0C,0x60,0x07,0xC0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3B,0xC0,  // -p-
      0x1E,0x60,0x1C,0x30,0x18,0x30,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x30,0x1C,0x30,0x1E,0x60,0x1B,0xC0,
      0x18,0x00,0x18,0x00,0x18,0x00,0x3C,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x90,  // -q-
      0x0C,0xF0,0x18,0x70,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x18,0x30,0x18,0x70,0x0C,0xF0,0x07,0xB0,
      0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x78,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x39,0xE0,  // -r-
      0x1B,0x30,0x1E,0x38,0x1C,0x30,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,
      0x18,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xC0,  // -s-
      0x18,0x70,0x30,0x30,0x30,0x10,0x38,0x00,0x1E,0x00,0x07,0xC0,0x00,0xF0,0x00,0x38,0x20,0x18,0x30,0x18,
      0x38,0x30,0x0F,0xE0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x3F,0xF0,  // -t-
      0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x18,0x06,0x18,
      0x03,0x30,0x01,0xE0,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x70,  // -u-
      0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x70,0x18,0xF0,
      0x0D,0xB0,0x07,0x38,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x78,  // -v-
      0x18,0x30,0x18,0x30,0x0C,0x60,0x0C,0x60,0x0C,0x60,0x06,0xC0,0x06,0xC0,0x06,0xC0,0x03,0x80,0x03,0x80,
      0x03,0x80,0x01,0x00,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x3C,  // -w-
      0x18,0x18,0x18,0x18,0x19,0x98,0x19,0x98,0x0D,0xB0,0x0D,0xB0,0x0D,0xB0,0x0F,0xF0,0x06,0x60,0x06,0x60,
      0x06,0x60,0x06,0x60,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x78,  // -x-
      0x18,0x30,0x18,0x30,0x0C,0x60,0x06,0xC0,0x03,0x80,0x03,0x80,0x03,0x80,0x06,0xC0,0x0C,0x60,0x18,0x30,
      0x18,0x30,0x3C,0x78,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x78,  // -y-
      0x18,0x30,0x0C,0x30,0x0C,0x60,0x06,0x60,0x06,0x60,0x03,0xC0,0x03,0xC0,0x01,0xC0,0x01,0x80,0x01,0x80,
      0x01,0x80,0x33,0x00,0x33,0x00,0x1E,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xF8,  // -z-
      0x10,0x18,0x10,0x30,0x00,0x60,0x00,0xC0,0x01,0x80,0x03,0x00,0x06,0x00,0x0C,0x00,0x18,0x08,0x30,0x08,
      0x30,0x18,0x3F,0xF8,0x00,0x00,0x00,0x00,

      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x12,0x08,0x33,0x18,0x33,0x10,0x33,0x30,0x33,0x20,  // -%-
      0x33,0x60,0x12,0xC0,0x0C,0x80,0x01,0x80,0x01,0x00,0x03,0x00,0x02,0x60,0x06,0x90,0x0D,0x98,0x09,0x98,
      0x19,0x98,0x11,0x98,0x31,0x98,0x20,0x90,

};
const char DotTbl24AsciiString[] =" .0123456789ABCDEFabcdefghijklmnopqrstuvwxyz%";*/


static const unsigned char DotTbl24[][72/3]=        // 数据表
{   
//-- 太 --
      0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x3F,0xFF,0xFE,0x3F,0xFF,0xFE,0x3F,0xFF,0xFE,0x00,0x3C,0x00,
      0x00,0x3C,0x00,0x00,0x3E,0x00,0x00,0x36,0x00,0x00,0x77,0x00,0x00,0x67,0x00,0x00,0xE3,0x80,0x01,0xC3,0xC0,0x03,0xF1,0xE0,0x07,0xB8,0xF0,0x1F,0x38,0xFC,
      0x7E,0x18,0x7E,0x78,0x1C,0x1E,0x30,0x00,0x0C,0x00,0x00,0x00,
//-- 阳 --
      0x00,0x00,0x00,0x7F,0xC0,0x00,0x7F,0xDF,0xFC,0x71,0xDF,0xFC,0x71,0x9C,0x1C,0x71,0x9C,0x1C,0x73,0x9C,0x1C,0x73,0x1C,0x1C,0x73,0x1C,0x1C,0x77,0x1C,0x1C,
      0x73,0x9F,0xFC,0x73,0x9F,0xFC,0x71,0xDC,0x1C,0x71,0xDC,0x1C,0x71,0xDC,0x1C,0x71,0xDC,0x1C,0x7F,0xDC,0x1C,0x77,0x9C,0x1C,0x77,0x1F,0xFC,0x70,0x1F,0xFC,
      0x70,0x1C,0x1C,0x70,0x1C,0x1C,0x70,0x00,0x00,0x00,0x00,0x00,
//-- 能 --
      0x06,0x07,0x00,0x07,0x07,0x00,0x0E,0x07,0x18,0x0C,0xC7,0x7C,0x1C,0xE7,0xF0,0x38,0x77,0xC0,0x7F,0xF7,0x08,0x7F,0xFF,0x0E,0x38,0x17,0xFE,0x00,0x03,0xFC,
      0x3F,0xF1,0xF8,0x3F,0xF7,0x00,0x38,0x77,0x00,0x38,0x77,0x18,0x3F,0xF7,0x3C,0x3F,0xF7,0xFC,0x38,0x77,0xE0,0x38,0x77,0x00,0x3F,0xF7,0x00,0x3F,0xF7,0x0E,
      0x38,0x77,0x0E,0x38,0x77,0xFE,0x39,0xE3,0xFC,0x39,0xC0,0x00,
//-- 控 --
      0x0C,0x03,0x80,0x0C,0x03,0x80,0x0C,0x01,0x80,0x0C,0x7F,0xFE,0x0C,0x7F,0xFE,0x7F,0x70,0x0E,0x7F,0x70,0x0E,0x7F,0x76,0x6E,0x0C,0x0E,0x70,0x0C,0x1C,0x38,
      0x0D,0xB8,0x3C,0x0F,0xF0,0x1C,0x3F,0x00,0x08,0x7C,0x7F,0xFC,0x7C,0x7F,0xFC,0x4C,0x7F,0xFC,0x0C,0x03,0x80,0x0C,0x03,0x80,0x0C,0x03,0x80,0x0C,0x03,0x80,
      0x0C,0xFF,0xFE,0x3C,0xFF,0xFE,0x3C,0xFF,0xFE,0x00,0x00,0x00,
//-- 制 --
      0x01,0xC0,0x0E,0x01,0xC0,0x0E,0x0D,0xC0,0x0E,0x1D,0xC0,0xEE,0x1F,0xFC,0xEE,0x3F,0xFC,0xEE,0x31,0xC0,0xEE,0x01,0xC0,0xEE,0x7F,0xFE,0xEE,0x7F,0xFE,0xEE,
      0x01,0xC0,0xEE,0x01,0xC0,0xEE,0x01,0xC0,0xEE,0x3F,0xFE,0xEE,0x3F,0xFE,0xEE,0x39,0xCE,0xEE,0x39,0xCE,0xEE,0x39,0xCE,0xEE,0x39,0xCE,0x0E,0x39,0xFE,0x0E,
      0x39,0xDC,0x7E,0x01,0xC0,0x3E,0x01,0xC0,0x1C,0x00,0x00,0x00,
//-- 器 --
      0x1F,0xE7,0xF8,0x1F,0xE7,0xF8,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1F,0xE7,0xF8,0x1F,0xE7,0xF8,0x00,0x18,0xE0,0x00,0x38,0xE0,0x7F,0xFF,0xFE,
      0x7F,0xFF,0xFE,0x00,0x7E,0x00,0x03,0xE7,0xE0,0x7F,0xC3,0xFE,0x7E,0x00,0x7E,0x3F,0xE7,0xF8,0x1F,0xE7,0xF8,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1C,0xE7,0x38,
      0x1F,0xE7,0xF8,0x1F,0xE7,0xF8,0x1C,0xE7,0x38,0x00,0x00,0x00,
//-- 红 --
      0x06,0x00,0x00,0x07,0x00,0x00,0x0E,0x1F,0xFC,0x0C,0x1F,0xFC,0x1C,0x1F,0xFC,0x18,0xC1,0xC0,0x38,0xE1,0xC0,0x31,0xC1,0xC0,0x7F,0xC1,0xC0,0x7F,0x81,0xC0,
      0x73,0x01,0xC0,0x06,0x01,0xC0,0x0E,0x01,0xC0,0x1C,0x01,0xC0,0x3B,0xE1,0xC0,0x7F,0xE1,0xC0,0x3F,0x81,0xC0,0x00,0x01,0xC0,0x00,0xE1,0xC0,0x1F,0xFF,0xFE,
      0x7F,0xFF,0xFE,0x7C,0x3F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,
//-- 蓝 --
      0x00,0xE3,0x80,0x00,0xE3,0x80,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x00,0xE3,0x80,0x00,0xE3,0x00,0x00,0x63,0x00,0x1C,0xE7,0x00,0x1C,0xE7,0xFC,0x1C,0xEF,0xFC,
      0x1C,0xEE,0xC0,0x1C,0xFC,0xC0,0x1C,0xF8,0xE0,0x1C,0xE0,0x60,0x00,0x40,0x40,0x1F,0xFF,0xF8,0x1F,0xFF,0xF8,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1C,0xE7,0x38,
      0x1C,0xE7,0x38,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x00,0x00,0x00,
//-- 牙 --
      0x00,0x00,0x00,0x1F,0xFF,0xFC,0x1F,0xFF,0xFC,0x1F,0xFF,0xFC,0x00,0x03,0x80,0x03,0x83,0x80,0x03,0x03,0x80,0x07,0x03,0x80,0x06,0x03,0x80,0x1F,0xFF,0xFE,
      0x1F,0xFF,0xFE,0x1F,0xFF,0xFE,0x00,0x0F,0x80,0x00,0x1B,0x80,0x00,0x3B,0x80,0x00,0x73,0x80,0x00,0xE3,0x80,0x03,0xC3,0x80,0x07,0x83,0x80,0x1F,0x03,0x80,
      0x7C,0x03,0x80,0x38,0x1F,0x80,0x00,0x0F,0x00,0x00,0x00,0x00,
//-- 设 --
      0x00,0x00,0x00,0x08,0x1F,0xF0,0x1C,0x1F,0xF0,0x0E,0x1C,0x70,0x06,0x1C,0x70,0x06,0x1C,0x70,0x00,0x18,0x70,0x00,0x38,0x7E,0x7E,0xF0,0x7E,0x7E,0x60,0x00,
      0x0E,0x00,0x00,0x0E,0xFF,0xF8,0x0E,0xFF,0xF8,0x0E,0x30,0x30,0x0E,0x38,0x70,0x0E,0x18,0x60,0x0F,0x9C,0xE0,0x0F,0x8F,0xC0,0x0F,0x87,0x80,0x1F,0x0F,0xE0,
      0x1E,0x3F,0xFE,0x1D,0xF8,0x7E,0x09,0xE0,0x0E,0x00,0x00,0x00,
//-- 置 --
      0x1F,0xFF,0xF8,0x1F,0xFF,0xF8,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1F,0xFF,0xF8,0x1F,0xFF,0xF8,0x00,0x1C,0x00,0x3F,0xFF,0xFC,0x3F,0xFF,0xFC,0x00,0x1C,0x00,
      0x07,0xFF,0xE0,0x07,0xFF,0xE0,0x07,0x00,0xE0,0x07,0xFF,0xE0,0x07,0xFF,0xE0,0x07,0x00,0xE0,0x07,0xFF,0xE0,0x07,0xFF,0xE0,0x07,0x00,0xE0,0x07,0xFF,0xE0,
      0x07,0xFF,0xE0,0x07,0x00,0xE0,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,
//-- 曲 --
      0x00,0xE7,0x00,0x00,0xE7,0x00,0x00,0xE7,0x00,0x00,0xE7,0x00,0x00,0xE7,0x00,0x1F,0xFF,0xF8,0x1F,0xFF,0xF8,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1C,0xE7,0x38,
      0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1F,0xFF,0xF8,0x1F,0xFF,0xF8,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1C,0xE7,0x38,0x1F,0xFF,0xF8,
      0x1F,0xFF,0xF8,0x1C,0x00,0x38,0x1C,0x00,0x38,0x00,0x00,0x00,
//-- 线 --
      0x06,0x07,0x20,0x07,0x07,0x60,0x0E,0x07,0x70,0x0C,0x07,0x38,0x1C,0x87,0x20,0x19,0xC7,0x7C,0x39,0xCF,0xFC,0x73,0xFF,0xE0,0x7F,0xFF,0x00,0x7F,0x07,0x04,
      0x06,0x07,0x7E,0x0E,0x1F,0xFE,0x1C,0x7F,0xE0,0x18,0xFF,0x10,0x7F,0x83,0xBC,0x3F,0x83,0xF8,0x30,0x03,0xF0,0x00,0x41,0xE0,0x07,0xE3,0xC0,0x7F,0xC7,0xE6,
      0x7F,0x1E,0xFE,0x60,0xFC,0x7E,0x00,0x78,0x3E,0x00,0x00,0x00,
//-- 充 --
      0x00,0x08,0x00,0x00,0x38,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x3F,0xFF,0xFE,0x3F,0xFF,0xFE,0x00,0x70,0x00,0x00,0xE1,0x80,0x01,0xC1,0xC0,0x03,0x80,0xE0,
      0x0F,0x00,0xF0,0x0F,0xFF,0xF8,0x0F,0xFF,0xF8,0x00,0xE7,0x10,0x00,0xE7,0x00,0x00,0xE7,0x00,0x00,0xE7,0x00,0x00,0xC7,0x00,0x01,0xC7,0x0C,0x03,0x87,0x0E,
      0x07,0x87,0x0E,0x3F,0x07,0xFE,0x3C,0x03,0xFC,0x18,0x00,0x00,
//-- 电 --
      0x00,0x38,0x00,0x00,0x38,0x00,0x00,0x38,0x00,0x00,0x38,0x00,0x1F,0xFF,0xF0,0x1F,0xFF,0xF0,0x1F,0xFF,0xF0,0x1C,0x38,0x70,0x1C,0x38,0x70,0x1C,0x38,0x70,
      0x1F,0xFF,0xF0,0x1F,0xFF,0xF0,0x1C,0x38,0x70,0x1C,0x38,0x70,0x1C,0x38,0x70,0x1F,0xFF,0xF0,0x1F,0xFF,0xF0,0x1F,0xFF,0xF0,0x1C,0x38,0x08,0x1C,0x38,0x0E,
      0x00,0x38,0x0E,0x00,0x3F,0xFE,0x00,0x3F,0xFE,0x00,0x1F,0xF8,
//-- 放 --
      0x02,0x01,0x80,0x07,0x03,0xC0,0x03,0x03,0x80,0x03,0x83,0x80,0x02,0x03,0x00,0x7F,0xE7,0xFE,0x7F,0xE7,0xFE,0x0E,0x0E,0x18,0x0E,0x0E,0x18,0x0E,0x0E,0x18,
      0x0F,0xFF,0x38,0x0F,0xFF,0x38,0x0C,0x63,0x30,0x0C,0x63,0x30,0x0C,0x63,0x30,0x1C,0x63,0xF0,0x1C,0xE1,0xE0,0x18,0xE1,0xE0,0x18,0xE1,0xE0,0x38,0xE3,0xF0,
      0x77,0xC7,0xFC,0x73,0xDF,0x3E,0x63,0xBC,0x0E,0x00,0x00,0x00,
//-- 外 --
      0x02,0x03,0x80,0x07,0x03,0x80,0x07,0x03,0x80,0x07,0x03,0x80,0x06,0x03,0x80,0x0F,0xFB,0x80,0x0F,0xF3,0xE0,0x1C,0x33,0xF0,0x1C,0x33,0xF8,0x38,0x73,0xB8,
      0x3A,0x73,0x9C,0x77,0x63,0x9C,0x67,0xE3,0x8E,0x03,0xE3,0x8E,0x01,0xE3,0x86,0x01,0xC3,0x80,0x03,0xC3,0x80,0x07,0x83,0x80,0x0F,0x03,0x80,0x1E,0x03,0x80,
      0x7C,0x03,0x80,0x30,0x03,0x80,0x00,0x03,0x80,0x00,0x00,0x00,
//-- 指 --
      0x0E,0x1C,0x00,0x0E,0x1C,0x0C,0x0E,0x1C,0x3C,0x0E,0x1F,0xFE,0x0E,0x1F,0xF0,0x7F,0xDC,0x00,0x7F,0xDC,0x06,0x0E,0x1C,0x06,0x0E,0x1F,0xFE,0x0E,0x1F,0xFE,
      0x0E,0x0F,0xFC,0x0F,0xC0,0x00,0x1F,0xDF,0xFC,0x7E,0x1F,0xFC,0x7E,0x1C,0x1C,0x0E,0x1C,0x1C,0x0E,0x1F,0xFC,0x0E,0x1F,0xFC,0x0E,0x1C,0x1C,0x0E,0x1C,0x1C,
      0x0E,0x1F,0xFC,0x3E,0x1F,0xFC,0x1E,0x1C,0x1C,0x00,0x00,0x00,
//-- 定 --
      0x00,0x18,0x00,0x00,0x1C,0x00,0x00,0x0C,0x00,0x3F,0xFF,0xFE,0x3F,0xFF,0xFE,0x38,0x00,0x0E,0x38,0x00,0x0E,0x38,0x00,0x0E,0x07,0xFF,0xF0,0x07,0xFF,0xF0,
      0x00,0x1C,0x00,0x00,0x1C,0x00,0x07,0x1C,0x00,0x07,0x1F,0xF8,0x07,0x1F,0xF8,0x0E,0x1F,0xF8,0x0E,0x1C,0x00,0x0F,0x1C,0x00,0x1F,0x9C,0x00,0x39,0xDC,0x00,
      0x38,0xFC,0x06,0x70,0x7F,0xFE,0x70,0x1F,0xFE,0x00,0x00,0x00,
//-- 连 --
      0x00,0x06,0x00,0x08,0x07,0x00,0x1C,0x0E,0x00,0x1D,0xFF,0xFC,0x1D,0xFF,0xFC,0x0F,0xFF,0xFC,0x0C,0x3B,0x80,0x00,0x33,0x80,0x00,0x73,0x80,0x7C,0xFF,0xFC,
      0x7C,0xFF,0xFC,0x7C,0xFF,0xFC,0x0C,0x03,0x80,0x0C,0x03,0x80,0x0D,0xFF,0xFE,0x0D,0xFF,0xFE,0x0D,0xFF,0xFE,0x0C,0x03,0x80,0x0E,0x03,0x80,0x3F,0x83,0x80,
      0x7F,0xF0,0x00,0x71,0xFF,0xFE,0x20,0x3F,0xFE,0x00,0x00,0x00,
//-- 接 --
      0x0C,0x03,0x80,0x0C,0x03,0x80,0x0C,0x01,0x80,0x0C,0x3F,0xFC,0x0C,0x3F,0xFC,0x7F,0x8C,0x70,0x7F,0x9C,0x70,0x0C,0x0E,0x60,0x0C,0x04,0xE0,0x0C,0x7F,0xFE,
      0x0F,0xFF,0xFE,0x0F,0x87,0x00,0x1F,0x07,0x00,0x7C,0xFF,0xFE,0x7C,0xFF,0xFE,0x4C,0x0E,0x70,0x0C,0x1C,0x70,0x0C,0x3C,0xE0,0x0C,0x1F,0xC0,0x0C,0x03,0xE0,
      0x0C,0x0F,0xF8,0x3C,0xFE,0x3E,0x3C,0xF8,0x0E,0x00,0x00,0x00,
//-- 自 --
      0x00,0x70,0x00,0x00,0x70,0x00,0x00,0xE0,0x00,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,0x0E,0x00,0x70,0x0E,0x00,0x70,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,
      0x0F,0xFF,0xF0,0x0E,0x00,0x70,0x0E,0x00,0x70,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,0x0E,0x00,0x70,0x0E,0x00,0x70,0x0E,0x00,0x70,0x0F,0xFF,0xF0,
      0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,0x0E,0x00,0x70,0x00,0x00,0x00,
//-- 动 --
      0x00,0x01,0xC0,0x00,0x01,0xC0,0x3F,0xE1,0x80,0x3F,0xE1,0x80,0x3F,0xE1,0x80,0x00,0x01,0x80,0x00,0x1F,0xFE,0x00,0x1F,0xFE,0x7F,0xF3,0x8C,0x7F,0xF3,0x8C,
      0x07,0x03,0x8C,0x06,0x03,0x8C,0x0E,0x03,0x8C,0x0E,0xC3,0x0C,0x0C,0xE3,0x0C,0x1C,0x67,0x1C,0x18,0xE7,0x1C,0x3F,0xF6,0x1C,0x7F,0xFE,0x1C,0x7C,0x3C,0x1C,
      0x20,0x3C,0xFC,0x00,0xF8,0xF8,0x00,0xF0,0x70,0x00,0x60,0x00,
//-- Ｍ --
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x7C,0x1E,0x00,0x78,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x81,0xF0,0x0B,0xC3,0xF0,
      0x09,0xC3,0x70,0x09,0xC3,0x70,0x09,0xE6,0x70,0x08,0xE4,0x70,0x08,0xE4,0x70,0x08,0x7C,0x70,0x08,0x78,0x70,0x08,0x38,0x70,0x1C,0x30,0xF8,0x3E,0x01,0xFC,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//-- Ｐ --
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0x00,0x01,0xFF,0xC0,0x01,0xC1,0xE0,0x01,0xC0,0xE0,0x01,0xC0,0xE0,0x01,0xC0,0xE0,0x01,0xC1,0xE0,
      0x01,0xC3,0xC0,0x01,0xFF,0x80,0x01,0xFE,0x00,0x01,0xC0,0x00,0x01,0xC0,0x00,0x01,0xC0,0x00,0x01,0xC0,0x00,0x01,0xC0,0x00,0x03,0xE0,0x00,0x07,0xF0,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//-- Ｔ --
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xF0,0x07,0xFF,0xF0,0x06,0x1C,0x30,0x04,0x1C,0x10,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,
      0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x1C,0x00,0x00,0x3E,0x00,0x00,0x7F,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//-- 版 --
      0x21,0x80,0x08,0x39,0x8F,0xFC,0x31,0x9F,0xFE,0x31,0x9E,0x00,0x31,0x9C,0x00,0x31,0x9C,0x00,0x31,0x9C,0x00,0x3F,0xFF,0xFC,0x3F,0xFF,0xFC,0x30,0x1C,0x1C,
      0x30,0x1F,0x1C,0x30,0x1F,0x18,0x3F,0x9F,0x18,0x3F,0x9F,0xB8,0x31,0x9D,0xB0,0x31,0x99,0xB0,0x31,0x99,0xF0,0x31,0x98,0xE0,0x31,0xB8,0xE0,0x71,0xB9,0xF0,
      0x61,0xB7,0xBC,0x61,0xFF,0x1F,0x60,0xEE,0x0E,0x40,0x64,0x00,
//-- 本 --
      0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x3F,0xFF,0xFE,0x3F,0xFF,0xFE,0x00,0x7E,0x00,0x00,0x7F,0x00,0x00,0xFF,0x00,
      0x00,0xDB,0x00,0x01,0xDB,0x80,0x01,0x99,0xC0,0x03,0x99,0xC0,0x07,0x18,0xE0,0x0F,0x18,0x78,0x1E,0x18,0x7C,0x3F,0xFF,0xFF,0x7B,0xFF,0xDE,0x70,0x18,0x04,
      0x20,0x18,0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x18,0x00,
//-- 查 --
      0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x3F,0xFF,0xFE,0x3F,0xFF,0xFE,0x00,0xFF,0x00,0x01,0xDB,0x80,0x03,0x99,0xC0,0x0F,0x18,0xF0,0x1E,0x18,0x7E,
      0x7F,0xFF,0xFE,0x77,0xFF,0xEC,0x07,0x00,0xE0,0x07,0x00,0xE0,0x07,0xFF,0xE0,0x07,0xFF,0xE0,0x07,0x00,0xE0,0x07,0x00,0xE0,0x07,0xFF,0xE0,0x07,0xFF,0xE0,
      0x00,0x00,0x00,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x00,0x00,0x00,
//-- 询 --
      0x00,0x0C,0x00,0x18,0x0E,0x00,0x1C,0x1C,0x00,0x0E,0x1C,0x00,0x0E,0x3F,0xFE,0x06,0x3F,0xFE,0x00,0x70,0x0E,0x00,0x60,0x0E,0x7E,0xFF,0xCE,0x7F,0xFF,0xCE,
      0x0F,0xF9,0xCE,0x0E,0x39,0xCE,0x0E,0x39,0xCE,0x0E,0x3F,0xCE,0x0E,0x3F,0xCE,0x0E,0x39,0xCE,0x0E,0xB9,0xCE,0x0F,0xF9,0xDC,0x0F,0xBF,0xDC,0x0F,0x3F,0xDC,
      0x1E,0x00,0x1C,0x1C,0x00,0x7C,0x08,0x00,0x7C,0x00,0x00,0x00,
//-- ＞ --
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x0F,0x80,0x00,0x01,0xE0,0x00,
      0x00,0x78,0x00,0x00,0x1F,0x00,0x00,0x03,0xC0,0x00,0x00,0xE0,0x00,0x03,0xC0,0x00,0x1F,0x00,0x00,0x78,0x00,0x01,0xE0,0x00,0x0F,0x80,0x00,0x0C,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//-- 参 --
      0x00,0x70,0x00,0x00,0x71,0x00,0x00,0xE3,0x80,0x01,0xC1,0xC0,0x0F,0x81,0xE0,0x0F,0xFF,0xF8,0x0F,0xFF,0xF0,0x00,0x60,0x00,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,
      0x7F,0xFF,0xFE,0x03,0x89,0xC0,0x07,0x1C,0xE0,0x1E,0x38,0x78,0x7C,0xF3,0xBE,0x7F,0xE7,0x9E,0x27,0x8F,0x64,0x02,0x3C,0x70,0x01,0xF9,0xE0,0x07,0xE3,0xC0,
      0x03,0x8F,0x80,0x00,0x7F,0x00,0x1F,0xFC,0x00,0x0F,0xE0,0x00,
//-- 数 --
      0x03,0x01,0x80,0x3B,0x31,0xC0,0x3B,0x31,0xC0,0x1B,0x31,0x80,0x1B,0x61,0x80,0x7F,0xFB,0xFE,0x7F,0xFB,0xFE,0x0F,0xC3,0x18,0x0F,0x67,0x18,0x1B,0x76,0x18,
      0x7B,0x3F,0x98,0x73,0x0F,0xB8,0x07,0x01,0xB8,0x7F,0xFD,0xB8,0x7F,0xFD,0xB8,0x0E,0x71,0xF0,0x0C,0x60,0xF0,0x1E,0xE0,0xF0,0x1F,0xE0,0xE0,0x03,0xF9,0xF0,
      0x0F,0xF7,0xFC,0x7F,0x1F,0x1E,0x7C,0x1E,0x0E,0x00,0x00,0x00,
//-- 选 --
      0x00,0x03,0x80,0x00,0x23,0x80,0x18,0x73,0x80,0x18,0x73,0x80,0x1C,0x73,0x80,0x1C,0x7F,0xF8,0x0E,0xFF,0xF8,0x08,0xC3,0x80,0x00,0xC3,0x80,0x00,0x03,0x80,
      0x7D,0xFF,0xFE,0x7D,0xFF,0xFE,0x0C,0x1D,0xC0,0x0C,0x1D,0xC0,0x0C,0x1D,0xC0,0x0C,0x19,0xC0,0x0C,0x39,0xCC,0x0C,0x39,0xCE,0x0C,0xF1,0xFE,0x0D,0xE1,0xFE,
      0x1E,0x40,0xFC,0x3F,0x80,0x00,0x73,0xFF,0xFE,0x60,0xFF,0xFE,
//-- 择 --
      0x0E,0x00,0x00,0x0E,0x00,0x00,0x0E,0x7F,0xFC,0x0E,0x7F,0xF8,0x0E,0x18,0x38,0x0E,0x18,0x70,0x7F,0xCC,0x70,0x7F,0xCE,0xE0,0x0E,0x07,0xC0,0x0E,0x07,0xE0,
      0x0E,0x3F,0xFE,0x0F,0xFF,0xBE,0x0F,0xF3,0x86,0x7E,0x03,0x80,0x7E,0x3F,0xFC,0x4E,0x3F,0xFC,0x0E,0x03,0x80,0x0E,0x03,0x80,0x0E,0xFF,0xFE,0x0E,0xFF,0xFE,
      0x0E,0x03,0x80,0x0E,0x03,0x80,0x3E,0x03,0x80,0x3C,0x03,0x80,
//-- 号 --
      0x00,0x00,0x00,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,0x0E,0x00,0x70,0x0E,0x00,0x70,0x0E,0x00,0x70,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
      0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x03,0x80,0x00,0x03,0x80,0x00,0x07,0xFF,0xF0,0x07,0xFF,0xF0,0x00,0x00,0x70,0x00,0x00,0x70,0x00,0x00,0xE0,
      0x00,0x00,0xE0,0x00,0x0F,0xE0,0x00,0x0F,0xC0,0x00,0x00,0x00,
//-- 池 --
      0x00,0x07,0x00,0x18,0x07,0x00,0x3E,0x07,0x00,0x0F,0x47,0x00,0x06,0x67,0x00,0x02,0x67,0x18,0x00,0x67,0xF8,0x70,0x67,0xF8,0x7C,0x7F,0x98,0x3C,0xFF,0x18,
      0x1B,0xF7,0x18,0x03,0xE7,0x18,0x08,0x67,0x18,0x0E,0x67,0x38,0x0E,0x67,0x78,0x1C,0x67,0x78,0x1C,0x67,0x60,0x1C,0x67,0x08,0x38,0x63,0x0E,0x38,0x60,0x0E,
      0x38,0x78,0x7E,0x70,0x7F,0xFC,0x30,0x3F,0xF8,0x00,0x00,0x00,
//-- 规 --
      0x07,0x00,0x00,0x07,0x00,0x00,0x07,0x1F,0xFC,0x07,0x1F,0xFC,0x07,0x1C,0x1C,0x3F,0xFC,0x1C,0x3F,0xFD,0xDC,0x07,0x1D,0xDC,0x07,0x1D,0xDC,0x07,0x1D,0xDC,
      0x7F,0xFD,0xDC,0x7F,0xFD,0xDC,0x07,0x1D,0xDC,0x07,0x1D,0xDC,0x07,0x1D,0xDC,0x07,0x81,0xC0,0x0F,0xC3,0xE0,0x0C,0xE7,0x60,0x1C,0x66,0x64,0x38,0x1E,0x66,
      0x78,0x3C,0x66,0x70,0xF8,0x7E,0x00,0xE0,0x3C,0x00,0x00,0x00,
//-- 格 --
      0x06,0x07,0x00,0x06,0x07,0x00,0x06,0x0E,0x00,0x06,0x0F,0xFC,0x06,0x1F,0xFC,0x7F,0xDE,0x38,0x7F,0xFE,0x30,0x0E,0x37,0x70,0x0E,0x73,0xE0,0x0F,0x83,0xE0,
      0x1F,0x83,0xC0,0x1F,0xCF,0xF8,0x3E,0xBE,0x7E,0x36,0xFC,0x1E,0x76,0x7F,0xFE,0x66,0x1F,0xFC,0x66,0x1C,0x1C,0x06,0x1C,0x1C,0x06,0x1C,0x1C,0x06,0x1C,0x1C,
      0x06,0x1F,0xFC,0x06,0x1F,0xFC,0x06,0x1C,0x1C,0x00,0x00,0x00,
//-- 恒 --
      0x0E,0x00,0x00,0x0E,0x00,0x00,0x0C,0x7F,0xFE,0x0C,0x7F,0xFE,0x0C,0x00,0x00,0x0C,0x00,0x00,0x7F,0x80,0x00,0x7D,0xBF,0xFC,0x7D,0xBF,0xFC,0x6D,0xF8,0x1C,
      0x6C,0xF8,0x1C,0x6C,0x3F,0xFC,0x6C,0x3F,0xFC,0x6C,0x38,0x1C,0x0C,0x38,0x1C,0x0C,0x38,0x1C,0x0C,0x3F,0xFC,0x0C,0x3F,0xFC,0x0C,0x38,0x1C,0x0C,0x00,0x00,
      0x0C,0x00,0x00,0x0C,0xFF,0xFE,0x0C,0xFF,0xFE,0x0C,0x00,0x00,
//-- 压 --
      0x00,0x00,0x00,0x0F,0xFF,0xFE,0x0F,0xFF,0xFE,0x0F,0xFF,0xFE,0x0E,0x07,0x00,0x0E,0x07,0x00,0x0E,0x07,0x00,0x0E,0x07,0x00,0x0E,0x07,0x00,0x0E,0xFF,0xFC,
      0x0E,0xFF,0xFC,0x0E,0xFF,0xFC,0x0E,0x07,0x00,0x0E,0x07,0x30,0x0E,0x07,0x70,0x0E,0x07,0x38,0x1C,0x07,0x1C,0x1C,0x07,0x18,0x1C,0x07,0x00,0x38,0x07,0x00,
      0x3B,0xFF,0xFE,0x73,0xFF,0xFE,0x73,0xFF,0xFE,0x00,0x00,0x00,
//-- 流 --
      0x00,0x03,0x00,0x10,0x07,0x00,0x3C,0x07,0x00,0x1F,0xFF,0xFE,0x0F,0xFF,0xFE,0x04,0x0C,0x00,0x00,0x1E,0x60,0x70,0x3C,0xE0,0x7C,0x70,0x70,0x3D,0xFF,0xF8,
      0x0D,0xFF,0xFC,0x00,0xFC,0x0C,0x00,0x46,0x60,0x1C,0x76,0x60,0x1C,0x66,0x60,0x1C,0x66,0x60,0x18,0x66,0x60,0x18,0xE6,0x60,0x38,0xE6,0x60,0x38,0xC6,0x64,
      0x71,0xC6,0x66,0x77,0x86,0x7E,0x37,0x06,0x3E,0x00,0x00,0x00,
//-- 低 --
      0x06,0x00,0x00,0x07,0x00,0xF8,0x07,0x7F,0xFC,0x0E,0x7F,0xF0,0x0E,0x71,0x80,0x0C,0x71,0x80,0x1C,0x71,0x80,0x1C,0x71,0x80,0x3C,0x71,0x80,0x7C,0x7F,0xFE,
      0x7C,0x7F,0xFE,0x7C,0x71,0xC0,0x5C,0x71,0xC0,0x1C,0x71,0xC0,0x1C,0x70,0xC0,0x1C,0x70,0xC0,0x1C,0x74,0xC0,0x1C,0x7E,0xE0,0x1C,0x7E,0x64,0x1C,0x7F,0x76,
      0x1C,0x7B,0xBF,0x1C,0xF3,0xBE,0x1C,0x61,0x9E,0x00,0x00,0x00,
//-- 保 --
      0x00,0x00,0x00,0x03,0x00,0x00,0x03,0xFF,0xF8,0x07,0x7F,0xF8,0x07,0x60,0x18,0x0E,0x60,0x18,0x0E,0x60,0x18,0x0C,0x60,0x18,0x1C,0x7F,0xF8,0x3C,0x7F,0xF8,
      0x3C,0x03,0x00,0x7C,0x03,0x00,0x6C,0x03,0x00,0x6D,0xFF,0xFE,0x0D,0xFF,0xFE,0x0C,0x0F,0xC0,0x0C,0x1F,0xC0,0x0C,0x1B,0x60,0x0C,0x3B,0x70,0x0C,0x73,0x38,
      0x0D,0xE3,0x3E,0x0F,0xC3,0x1F,0x0D,0x83,0x0E,0x0C,0x03,0x00,
//-- 护 --
      0x00,0x00,0x80,0x0E,0x01,0x80,0x0E,0x01,0xC0,0x0E,0x01,0xC0,0x0E,0x00,0xC0,0x0E,0x1F,0xFE,0x7F,0xDF,0xFE,0x7F,0xDC,0x0E,0x7F,0xDC,0x0E,0x0E,0x1C,0x0E,
      0x0E,0x1C,0x0E,0x0E,0xDC,0x0E,0x0F,0xDF,0xFE,0x7F,0xDF,0xFE,0x7E,0x1C,0x0E,0x7E,0x1C,0x00,0x0E,0x1C,0x00,0x0E,0x1C,0x00,0x0E,0x1C,0x00,0x0E,0x38,0x00,
      0x0E,0x38,0x00,0x0E,0x70,0x00,0x3E,0x70,0x00,0x1E,0xE0,0x00,
//-- 通 --
      0x08,0x00,0x00,0x38,0xFF,0xFC,0x1C,0xFF,0xF8,0x1C,0x04,0xF0,0x0E,0x0F,0xE0,0x0C,0x07,0x80,0x00,0xFF,0xFE,0x00,0xFF,0xFE,0x7C,0xE3,0x8E,0x7C,0xE3,0x8E,
      0x7C,0xFF,0xFE,0x1C,0xFF,0xFE,0x1C,0xE3,0x8E,0x1C,0xE3,0x8E,0x1C,0xFF,0xFE,0x1C,0xFF,0xFE,0x1C,0xE3,0x8E,0x1C,0xE3,0x8E,0x1C,0xE3,0xBE,0x3F,0xE0,0x3E,
      0x7F,0xC0,0x00,0x71,0xFF,0xFE,0x20,0x7F,0xFE,0x00,0x00,0x00,
//-- 信 --
      0x03,0x03,0x80,0x07,0x83,0x80,0x07,0x03,0x80,0x06,0x03,0x00,0x0E,0xFF,0xFE,0x0E,0xFF,0xFE,0x1E,0x00,0x00,0x1E,0x7F,0xFC,0x3E,0x7F,0xFC,0x3E,0x00,0x00,
      0x7E,0x00,0x00,0x6E,0x7F,0xFC,0x6E,0x7F,0xFC,0x0E,0x00,0x00,0x0E,0x00,0x00,0x0E,0x7F,0xFC,0x0E,0x7F,0xFC,0x0E,0x70,0x1C,0x0E,0x70,0x1C,0x0E,0x70,0x1C,
      0x0E,0x7F,0xFC,0x0E,0x7F,0xFC,0x0E,0x70,0x1C,0x00,0x00,0x00,
//-- 开 --
      0x00,0x00,0x00,0x1F,0xFF,0xFC,0x1F,0xFF,0xFC,0x1F,0xFF,0xFC,0x00,0xE3,0x80,0x00,0xE3,0x80,0x00,0xE3,0x80,0x00,0xE3,0x80,0x00,0xE3,0x80,0x00,0xE3,0x80,
      0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x00,0xE3,0x80,0x01,0xC3,0x80,0x01,0xC3,0x80,0x01,0x83,0x80,0x03,0x83,0x80,0x03,0x83,0x80,0x07,0x03,0x80,
      0x1E,0x03,0x80,0x3C,0x03,0x80,0x18,0x03,0x80,0x00,0x00,0x00,
//-- 关 --
      0x03,0x81,0xC0,0x03,0x81,0xC0,0x01,0xC3,0x80,0x01,0xC3,0x00,0x00,0x87,0x00,0x3F,0xFF,0xFC,0x3F,0xFF,0xFC,0x3F,0xFF,0xFC,0x00,0x38,0x00,0x00,0x38,0x00,
      0x00,0x38,0x00,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x00,0x3C,0x00,0x00,0x7C,0x00,0x00,0x7E,0x00,0x00,0xE7,0x00,0x01,0xE3,0xC0,0x07,0xC3,0xF0,
      0x1F,0x80,0xFE,0x7E,0x00,0x7C,0x3C,0x00,0x1C,0x00,0x00,0x00,
//-- ： --
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,
      0x01,0xE0,0x00,0x01,0xE0,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x01,0xE0,0x00,0x01,0xE0,0x00,0x00,0xC0,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
      
};         
const char DotTbl24String[] ="太阳能控制器红蓝牙设置曲线充电放外指定连接自动ＭＰＴ版本查询＞参数选择号池规格恒压流低保护通信开关：";

#endif
