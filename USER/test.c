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
void GUI_DrawMonoIcon32x32(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
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
}

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
 * @name       :void GUI_DrawMonoIcon8x16(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
 * @date       :2025-03-31
 * @function   :Display a 8x16 1-bit monochrome icon at specified position
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              fc:foreground color (for '1' bits)
 *              bc:background color (for '0' bits)
 *				bitmap:the start address of 1-bit monochrome bitmap (16 bytes)
 * @retvalue   :None
 * @note       :Bitmap format: MSB first, 1 byte per row (8 bits), 16 rows total
******************************************************************************/
void GUI_DrawMonoIcon8x16(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
{
	u16 row, col;
	u8 bitMask;
	
	for(row = 0; row < 16; row++)
	{
		for(col = 0; col < 8; col++)
		{
			bitMask = 0x80 >> (col % 8);  // MSB first
			
			if(bitmap[row] & bitMask)
			{
				LCD_Fill(x + col, y + row, x + col, y + row, fc);  // Foreground pixel
			}
			else
			{
				LCD_Fill(x + col, y + row, x + col, y + row, bc);  // Background pixel
			}
		}
	}
}

/*****************************************************************************
 * @name       :void GUI_DrawMonoIcon16x16(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
 * @date       :2025-04-01
 * @function   :Display a 16x16 1-bit monochrome icon at specified position
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              fc:foreground color (for '1' bits)
 *              bc:background color (for '0' bits)
 *				bitmap:the start address of 1-bit monochrome bitmap (32 bytes)
 * @retvalue   :None
 * @note       :Bitmap format: MSB first, 2 bytes per row (16 bits), 16 rows total
******************************************************************************/
void GUI_DrawMonoIcon16x16(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
{
	u16 row, col;
	u16 byteIndex;
	u8 bitMask;

	for(row = 0; row < 16; row++)
	{
		for(col = 0; col < 16; col++)
		{
			byteIndex = row * 2 + (col / 8);  // 2 bytes per row
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
}

/*****************************************************************************
 * @name       :void GUI_DrawMonoIcon24x24(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
 * @date       :2025-04-01
 * @function   :Display a 24x24 1-bit monochrome icon at specified position
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              fc:foreground color (for '1' bits)
 *              bc:background color (for '0' bits)
 *				bitmap:the start address of 1-bit monochrome bitmap (72 bytes)
 * @retvalue   :None
 * @note       :Bitmap format: MSB first, 3 bytes per row (24 bits), 24 rows total
******************************************************************************/
void GUI_DrawMonoIcon24x24(u16 x, u16 y, u16 fc, u16 bc, const uint8_t *bitmap)
{
	u16 row, col;
	u16 byteIndex;
	u8 bitMask;

	for(row = 0; row < 24; row++)
	{
		for(col = 0; col < 24; col++)
		{
			byteIndex = row * 3 + (col / 8);  // 3 bytes per row
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
}

/*****************************************************************************
 * @name       :void GUI_DrawBigDigit(u16 x, u16 y, u16 fc, u16 bc, u8 digit, u8 font_size)
 * @date       :2025-03-24
 * @function   :Display a scaled digit using Arial-style font
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              fc:foreground color
 *              bc:background color
 *              digit:ASCII digit ('0'-'9' or '.' for decimal point)
 *              font_size:0=32x64, 1=16x32, 2=12x24
 * @retvalue   :None
******************************************************************************/
void GUI_DrawBigDigit(u16 x, u16 y, u16 fc, u16 bc, u8 digit, u8 font_size)
{
	const unsigned char* pattern;
	u16 row, col;
	u8 pixelW, pixelH;
	u16 byteIndex;
	u8 bitMask;
	u16 width, height, bytes_per_row;

	/* Select pattern and size based on font_size */
	if(font_size == 0)  /* 32x64 */
	{
		width = ARIAL_32X64_WIDTH;
		height = ARIAL_32X64_HEIGHT;
		bytes_per_row = ARIAL_32X64_BYTES_PER_ROW;
		if(digit >= '0' && digit <= '9')
		{
			pattern = arial_32x64_digits[digit - '0'];
		}
		else
		{
			return;
		}
	}
	else if(font_size == 1)  /* 16x32 */
	{
		width = ARIAL_16X32_WIDTH;
		height = ARIAL_16X32_HEIGHT;
		bytes_per_row = ARIAL_16X32_BYTES_PER_ROW;
		if(digit >= '0' && digit <= '9')
		{
			pattern = arial_16x32_digits[digit - '0'];
		}
		else
		{
			return;
		}
	}
	else  /* 12x24 for decimal point */
	{
		width = ARIAL_12X24_WIDTH;
		height = ARIAL_12X24_HEIGHT;
		bytes_per_row = ARIAL_12X24_BYTES_PER_ROW;
		if(digit == '.')
		{
			LCD_Fill(x + 3, y + 16, x + 9, y + 22, fc);
			return;
		}
		else if(digit >= '0' && digit <= '9')
		{
			pattern = arial_12x24_digits[digit - '0'];
		}
		else
		{
			return;
		}
	}

	pixelW = 1;
	pixelH = 1;

	for(row = 0; row < height; row++)
	{
		for(col = 0; col < width; col++)
		{
			byteIndex = row * bytes_per_row + (col / 8);
			bitMask = 0x80 >> (col % 8);

			if(pattern[byteIndex] & bitMask)
			{
				LCD_Fill(x + col*pixelW, y + row*pixelH,
				         x + (col+1)*pixelW - 1, y + (row+1)*pixelH - 1, fc);
			}
			else
			{
				LCD_Fill(x + col*pixelW, y + row*pixelH,
				         x + (col+1)*pixelW - 1, y + (row+1)*pixelH - 1, bc);
			}
		}
	}
}

/*****************************************************************************
 * @name       :void Draw_Static_Icons(void)
 * @date       :2025-04-01 (Updated for 24x24 monochrome icons)
 * @function   :Draw static icons on the screen using 24x24 monochrome bitmaps
 * @parameters :None
 * @retvalue   :None
 * @note       :Icon mapping:
 *              - Icon1 (24x24) at (120, 5)
 *              - Icon2 (24x24) at (120, 5) - same position, different icon
 *              - Icon3 (24x24) at (144, 5) - rightmost
 *              - Icon8 (24x24) at (144, 45)
 *              - Icon6 (24x24) at (144, 85)
 *              Using 24x24 instead of 32x32 to avoid overlapping with time display
******************************************************************************/
void Draw_Static_Icons(void)
{
	// Icons on the RIGHT side - now using 24x24 monochrome icons
	// Smaller size (24x24) to avoid overlapping with time display at left
	// Screen width is 160px, icons positioned to stay within bounds
	
	// Icon3 at rightmost (136, 5) - rightmost, RED foreground (136+24=160)
	GUI_DrawMonoIcon24x24(136, 5, RED, WHITE, Icon03_24x24);
	
	// Icon2 at (112, 5) - to the left of Icon3
	GUI_DrawMonoIcon24x24(112, 5, BLACK, WHITE, Icon02_24x24);
	
	// Icon1 at (88, 5) - to the left of Icon2
	GUI_DrawMonoIcon24x24(88, 5, BLACK, WHITE, Icon01_24x24);
	
	// Icon8 at (136, 45) - below Icon3
	GUI_DrawMonoIcon24x24(136, 45, BLACK, WHITE, Icon08_24x24);
	
	// Icon6 at (136, 85) - below Icon8
	GUI_DrawMonoIcon24x24(136, 85, BLACK, WHITE, Icon06_24x24);
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

/*****************************************************************************
 * @name       :void GUI_DrawBigDigitNoBg(u16 x, u16 y, u16 fc, u8 digit, u8 font_size)
 * @date       :2025-03-24
 * @function   :Display a scaled digit using Arial-style font (NO background)
 * @parameters :x:the beginning x coordinate
 *              y:the beginning y coordinate
 *              fc:foreground color
 *              digit:ASCII digit ('0'-'9' or '.' for decimal point)
 *              font_size:0=32x64, 1=16x32, 2=12x24
 * @retvalue   :None
 * @note       :This version does NOT draw background, only foreground pixels
******************************************************************************/
void GUI_DrawBigDigitNoBg(u16 x, u16 y, u16 fc, u8 digit, u8 font_size)
{
	const unsigned char* pattern;
	u16 row, col;
	u8 pixelW, pixelH;
	u16 byteIndex;
	u8 bitMask;
	u16 width, height, bytes_per_row;

	/* Select pattern and size based on font_size */
	if(font_size == 0)  /* 32x64 */
	{
		width = ARIAL_32X64_WIDTH;
		height = ARIAL_32X64_HEIGHT;
		bytes_per_row = ARIAL_32X64_BYTES_PER_ROW;
		if(digit >= '0' && digit <= '9')
		{
			pattern = arial_32x64_digits[digit - '0'];
		}
		else
		{
			return;
		}
	}
	else if(font_size == 1)  /* 16x32 */
	{
		width = ARIAL_16X32_WIDTH;
		height = ARIAL_16X32_HEIGHT;
		bytes_per_row = ARIAL_16X32_BYTES_PER_ROW;
		if(digit >= '0' && digit <= '9')
		{
			pattern = arial_16x32_digits[digit - '0'];
		}
		else
		{
			return;
		}
	}
	else  /* 12x24 for decimal point */
	{
		width = ARIAL_12X24_WIDTH;
		height = ARIAL_12X24_HEIGHT;
		bytes_per_row = ARIAL_12X24_BYTES_PER_ROW;
		if(digit == '.')
		{
			LCD_Fill(x + 3, y + 16, x + 9, y + 22, fc);
			return;
		}
		else if(digit >= '0' && digit <= '9')
		{
			pattern = arial_12x24_digits[digit - '0'];
		}
		else
		{
			return;
		}
	}

	pixelW = 1;
	pixelH = 1;

	for(row = 0; row < height; row++)
	{
		for(col = 0; col < width; col++)
		{
			byteIndex = row * bytes_per_row + (col / 8);
			bitMask = 0x80 >> (col % 8);

			if(pattern[byteIndex] & bitMask)
			{
				LCD_Fill(x + col*pixelW, y + row*pixelH,
				         x + (col+1)*pixelW - 1, y + (row+1)*pixelH - 1, fc);
			}
			/* Note: No else branch - we don't draw background */
		}
	}
}
