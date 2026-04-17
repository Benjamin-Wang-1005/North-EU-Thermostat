/******************************************************************************
// File: test.c (Updated for 32x32 monochrome icons)
// Description: Simplified test.c with 32x32 1-bit monochrome icon support
******************************************************************************/
#include "lcd.h"
#include "delay.h"
#include "gui.h"
#include "test.h"
#include "icons.h"
#include "icons_32x32.h"  // New 32x32 monochrome icons
#include "icons_8x16.h"   // New 8x16 monochrome icons for Function Setting
#include "icons_24x24.h"  // New 24x24 monochrome icons for Active page
#include "arial_digits_flash.h"

/*****************************************************************************
 * @name       :void GUI_DrawBmpIcon(u16 x, u16 y, u16 w, u16 h, const unsigned char *p)
 * @date       :2025-03-24
 * @function   :Display a 16-bit RGB565 icon at specified position
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              w:width of the icon
 *              h:height of the icon
 *				p:the start address of image array (RGB565, little endian)
 * @retvalue   :None
******************************************************************************/
void GUI_DrawBmpIcon(u16 x, u16 y, u16 w, u16 h, const unsigned char *p)
{
	int i;
	u16 color;
	LCD_SetWindows(x, y, x+w-1, y+h-1);
	for(i=0; i<w*h; i++)
	{
		color = (p[i*2+1] << 8) | p[i*2];
		Lcd_WriteData_16Bit(color);
	}
	LCD_SetWindows(0, 0, lcddev.width-1, lcddev.height-1);
}

/*****************************************************************************
 * @name       :void GUI_DrawMonoIcon32x32(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
 * @date       :2025-03-31
 * @function   :Display a 32x32 1-bit monochrome icon at specified position
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              fc:foreground color (for '1' bits)
 *              bc:background color (for '0' bits)
 *				bitmap:the start address of 1-bit monochrome bitmap (128 bytes)
 * @retvalue   :None
 * @note       :Bitmap format: MSB first, 4 bytes per row (32 bits), 32 rows total
******************************************************************************/
/*void GUI_DrawMonoIcon32x32(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
{
	u16 row, col;
	u16 byteIndex;
	u8 bitMask;
	
	for(row = 0; row < 32; row++)
	{
		for(col = 0; col < 32; col++)
		{
			byteIndex = row * 4 + (col / 8);  // 4 bytes per row
			bitMask = 0x80 >> (col % 8);       // MSB first
			
			if(bitmap[byteIndex] & bitMask)
			{
				LCD_Fill(x + col, y + row, x + col, y + row, fc);  // Foreground pixel
			}
			else
			{
				LCD_Fill(x + col, y + row, x + col, y + row, bc);  // Background pixel
			}
		}
	}
}*/

/*****************************************************************************
 * @name       :void GUI_DrawMonoIcon32x32Transparent(u16 x, u16 y, u16 fc, const uint8_t *bitmap)
 * @date       :2025-03-31
 * @function   :Display a 32x32 1-bit monochrome icon with transparent background
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              fc:foreground color (for '1' bits)
 *				bitmap:the start address of 1-bit monochrome bitmap (128 bytes)
 * @retvalue   :None
 * @note       :Only draws foreground pixels, background is transparent
******************************************************************************/
void GUI_DrawMonoIcon32x32Transparent(u16 x, u16 y, u16 fc, const uint8_t *bitmap)
{
	u16 row, col;
	u16 byteIndex;
	u8 bitMask;
	
	for(row = 0; row < 32; row++)
	{
		for(col = 0; col < 32; col++)
		{
			byteIndex = row * 4 + (col / 8);  // 4 bytes per row
			bitMask = 0x80 >> (col % 8);       // MSB first
			
			if(bitmap[byteIndex] & bitMask)
			{
				LCD_Fill(x + col, y + row, x + col, y + row, fc);  // Foreground pixel only
			}
			// Note: No else branch - background is transparent
		}
	}
}










/*****************************************************************************
 * @name       :void Draw_Icon6_Red_32x32(u8 red)
 * @date       :2025-03-31 (Updated for 32x32 monochrome icons)
 * @function   :Draw Icon6 in normal or red color
 * @parameters :red: 1 = draw in RED, 0 = draw in BLACK
 * @retvalue   :None
******************************************************************************/
void Draw_Icon6_Red_32x32(u8 red)
{
	u16 x = 136;
	u16 y = 85;
	u16 color = red ? RED : BLACK;

	// Clear the icon area first (24x24 pixels)
	LCD_Fill(x, y, x + 23, y + 23, WHITE);

	// Draw Icon6 with specified color using 24x24 icon
	GUI_DrawMonoIcon24x24(x, y, color, WHITE, Icon06_24x24);
}


