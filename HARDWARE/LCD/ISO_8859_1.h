#ifndef __ISO_8859_1_H
#define __ISO_8859_1_H

// ISO 8859-1 Font Table (Spleen 8x16)
// Encoding mapping:
//   Index = Code - 32   (for Code < 160)
//   Index = Code - 64   (for Code >= 160)
// Table size: 192 characters x 16 bytes each
extern const unsigned char ISO_8859_1_8x16[192][16];

// Get ISO 8859-1 character bitmap pointer
// Input: ISO 8859-1 code (0~255)
// Output: pointer to 16-byte bitmap (8x16)
// Mapping: Code < 160 -> Index = Code - 32
//          Code >= 160 -> Index = Code - 64
static __inline const unsigned char* Get_ISO_8859_1_Char(unsigned char code)
{
    unsigned int index;
    if (code < 32) {
        index = 0;  // Return space for control characters
    } else if (code < 160) {
        index = code - 32;
    } else {
        index = code - 64;
    }
    if (index >= 192) {
        index = 0;  // Return space for out-of-range
    }
    return ISO_8859_1_8x16[index];
}

#endif
