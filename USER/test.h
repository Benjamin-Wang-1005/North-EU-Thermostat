/******************************************************************************
// File: test.h (Updated for 32x32 monochrome icons)
// Description: Header file with 32x32 1-bit monochrome icon support
******************************************************************************/
#ifndef __TEST_H__
#define __TEST_H__

#include <stdint.h>

// Function for drawing variable-size RGB565 icons (legacy)
void GUI_DrawBmpIcon(u16 x, u16 y, u16 w, u16 h, const unsigned char *p);

// Function for drawing 32x32 1-bit monochrome icons
//void GUI_DrawMonoIcon32x32(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap);

// Function for drawing 32x32 1-bit monochrome icons with transparent background
void GUI_DrawMonoIcon32x32Transparent(u16 x, u16 y, u16 fc, const uint8_t *bitmap);

// Function for drawing 8x16 1-bit monochrome icons
void GUI_DrawMonoIcon8x16(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap);



// Function for drawing 24x24 1-bit monochrome icons
void GUI_DrawMonoIcon24x24(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap);

// Function for drawing scaled digits
void GUI_DrawBigDigit(u16 x, u16 y, u16 fc, u16 bc, u8 digit, u8 font_size);

// Function for drawing scaled digits (NO background)
void GUI_DrawBigDigitNoBg(u16 x, u16 y, u16 fc, u8 digit, u8 font_size);



// Function to draw Icon6 in normal or red color (32x32 monochrome version)
void Draw_Icon6_Red_32x32(u8 red);

#endif
