#ifndef __ISO_8859_1_12X24_H
#define __ISO_8859_1_12X24_H

// ISO 8859-1 Font Table (Spleen 12x24)
// Source: https://github.com/fcambus/spleen (v2.2.0)
// License: BSD 2-Clause
//
// 192 characters, each 48 bytes (24 rows x 2 bytes/row = 12 pixels wide)
// Row format: [byte0: pixels 7..0] [byte1: pixels 11..8 (low nibble)]

extern const unsigned char ISO_8859_1_12x24[192][48];

// Get ISO 8859-1 12x24 character bitmap pointer
// Input: ISO 8859-1 code (0~255)
// Output: pointer to 48-byte bitmap
//
// Mapping: Code < 160  -> index = code - 32
//          Code >= 160 -> index = code - 64
static __inline const unsigned char* Get_ISO_8859_1_12x24_Char(unsigned char code)
{
    unsigned int index;
    if (code < 32) {
        index = 0;
    } else if (code < 160) {
        index = code - 32;
    } else {
        index = code - 64;
    }
    if (index >= 192) {
        index = 0;
    }
    return ISO_8859_1_12x24[index];
}

#endif
