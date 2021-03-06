#include "types.h"
#include "fceu.h"
#include "drawing.h"
#include "video.h"
#include "driver.h"

static uint8 Font6x7[792] =
{
	6,  0,  0,  0,  0,  0,  0,  0,	// 0x20 - Spacebar
	3, 64, 64, 64, 64, 64,  0, 64,
	5, 80, 80, 80,  0,  0,  0,  0,
	6, 80, 80,248, 80,248, 80, 80,
	6, 32,120,160,112, 40,240, 32,
	6, 64,168, 80, 32, 80,168, 16,
	6, 96,144,160, 64,168,144,104,
	3, 64, 64,  0,  0,  0,  0,  0,
	4, 32, 64, 64, 64, 64, 64, 32,
	4, 64, 32, 32, 32, 32, 32, 64,
	6,  0, 80, 32,248, 32, 80,  0,
	6,  0, 32, 32,248, 32, 32,  0,
	3,  0,  0,  0,  0,  0, 64,128,
	5,  0,  0,  0,240,  0,  0,  0,
	3,  0,  0,  0,  0,  0,  0, 64,
	5, 16, 16, 32, 32, 32, 64, 64,
	6,112,136,136,136,136,136,112,	// 0x30 - 0
	6, 32, 96, 32, 32, 32, 32, 32,
	6,112,136,  8, 48, 64,128,248,
	6,112,136,  8, 48,  8,136,112,
	6, 16, 48, 80,144,248, 16, 16,
	6,248,128,128,240,  8,  8,240,
	6, 48, 64,128,240,136,136,112,
	6,248,  8, 16, 16, 32, 32, 32,
	6,112,136,136,112,136,136,112,
	6,112,136,136,120,  8, 16, 96,
	3,  0,  0, 64,  0,  0, 64,  0,
	3,  0,  0, 64,  0,  0, 64,128,
	4,  0, 32, 64,128, 64, 32,  0,
	5,  0,  0,240,  0,240,  0,  0,
	4,  0,128, 64, 32, 64,128,  0,
	6,112,136,  8, 16, 32,  0, 32,	// 0x3F - ?
	6,112,136,136,184,176,128,112,	// 0x40 - @
	6,112,136,136,248,136,136,136,	// 0x41 - A
	6,240,136,136,240,136,136,240,
	6,112,136,128,128,128,136,112,
	6,224,144,136,136,136,144,224,
	6,248,128,128,240,128,128,248,
	6,248,128,128,240,128,128,128,
	6,112,136,128,184,136,136,120,
	6,136,136,136,248,136,136,136,
	4,224, 64, 64, 64, 64, 64,224,
	6,  8,  8,  8,  8,  8,136,112,
	6,136,144,160,192,160,144,136,
	6,128,128,128,128,128,128,248,
	6,136,216,168,168,136,136,136,
	6,136,136,200,168,152,136,136,
	7, 48, 72,132,132,132, 72, 48,
	6,240,136,136,240,128,128,128,
	6,112,136,136,136,168,144,104,
	6,240,136,136,240,144,136,136,
	6,112,136,128,112,  8,136,112,
	6,248, 32, 32, 32, 32, 32, 32,
	6,136,136,136,136,136,136,112,
	6,136,136,136, 80, 80, 32, 32,
	6,136,136,136,136,168,168, 80,
	6,136,136, 80, 32, 80,136,136,
	6,136,136, 80, 32, 32, 32, 32,
	6,248,  8, 16, 32, 64,128,248,
	3,192,128,128,128,128,128,192,
	5, 64, 64, 32, 32, 32, 16, 16,
	3,192, 64, 64, 64, 64, 64,192,
	4, 64,160,  0,  0,  0,  0,  0,
	6,  0,  0,  0,  0,  0,  0,248,
	3,128, 64,  0,  0,  0,  0,  0,
	5,  0,  0, 96, 16,112,144,112,	// 0x61 - a
	5,128,128,224,144,144,144,224,
	5,  0,  0,112,128,128,128,112,
	5, 16, 16,112,144,144,144,112,
	5,  0,  0, 96,144,240,128,112,
	5, 48, 64,224, 64, 64, 64, 64,
	5,  0,112,144,144,112, 16,224,
	5,128,128,224,144,144,144,144,
	2,128,  0,128,128,128,128,128,
	4, 32,  0, 32, 32, 32, 32,192,
	5,128,128,144,160,192,160,144,
	2,128,128,128,128,128,128,128,
	6,  0,  0,208,168,168,168,168,
	5,  0,  0,224,144,144,144,144,
	5,  0,  0, 96,144,144,144, 96,
	5,  0,  0,224,144,144,224,128,
	5,  0,  0,112,144,144,112, 16,
	5,  0,  0,176,192,128,128,128,
	5,  0,  0,112,128, 96, 16,224,
	4, 64, 64,224, 64, 64, 64, 32,
	5,  0,  0,144,144,144,144,112,
	5,  0,  0,144,144,144,160,192,
	6,  0,  0,136,136,168,168, 80,
	5,  0,  0,144,144, 96,144,144,
	5,  0,144,144,144,112, 16, 96,
	5,  0,  0,240, 32, 64,128,240,
	4, 32, 64, 64,128, 64, 64, 32,
	3, 64, 64, 64, 64, 64, 64, 64,
	4,128, 64, 64, 32, 64, 64,128,
	6,  0,104,176,  0,  0,  0,  0
};

static int FixJoedChar(uint8 ch)
{
	int c = ch - 32;
	return (c < 0 || c > 98) ? 0 : c;
}

char target[64][256];

void DrawTextTransWH(uint8 *dest, int width, uint8 *textmsg, uint8 fgcolor, int max_w, int max_h, int border)
{
	int beginx=2, x=beginx;
	int y=2;

	memset(target, 0, 64 * 256);

	assert(width==256);
	if (max_w > 256) max_w = 256;
	if (max_h >  64) max_h =  64;

	int ch = 0, wid = 0, nx = 0, ny = 0, max_x = x, offs = 0;
	int pixel_color;
	for(; *textmsg; ++textmsg)
	{
		if(*textmsg == '\n')
		{
			// new line
			x = beginx;
			y += 8;
			continue;
		}
		ch  = FixJoedChar(*textmsg);
		wid = Font6x7[ch * 8];

		if ((x + wid) >= (int)width)
		{
			// wrap to new line
			x = beginx;
			y += 8;
		}

		for(ny = 0; ny < 7; ++ny)
		{
			uint8 d = Font6x7[ch * 8 + 1 + ny];
			for(nx = 0; nx < wid; ++nx)
			{
				pixel_color = (d >> (7 - nx)) & 1;
				if (pixel_color)
				{
					if (y + ny >= 62)
					{
						// Max border is 2, so the max safe y is 62 (since 64 is the max for the target array
						goto textoverflow;
					}
					target[y + ny][x + nx] = 2;
				} else
				{
					target[y + ny][x + nx] = 1;
				}
			}
		}
		// proceed to next char
		x += wid;
		if (max_x < x)
			max_x = x;

	}
textoverflow:

	max_x += 2;
	if (max_x > width)
		max_x = width;
	int max_y = y + ny + 2;
	if (max_y > 62)
		max_y = 62;

	// draw target buffer to screen buffer
	for (y = 0; y < max_y; ++y)
	{
		for (x = 0; x < max_x; ++x)
		{
			offs = y * width + x;
			pixel_color = target[y][x] * 100;

			if(border>=1)
			{
				x>=(     1) && (pixel_color += target[y][x-1]);
				x<(width-1) && (pixel_color += target[y][x+1]);
				y>=(     1) && (pixel_color += target[y-1][x]);
				y<(16   -1) && (pixel_color += target[y+1][x]);
			}
			if(border>=2)
			{
				x>=(     1) && (pixel_color += target[y][x-1]*10);
				x<(width-1) && (pixel_color += target[y][x+1]*10);
				y>=(     1) && (pixel_color += target[y-1][x]*10);
				y<(16   -1) && (pixel_color += target[y+1][x]*10);

				x>=(     1) && y>=(  1) && (pixel_color += target[y-1][x-1]);
				x<(width-1) && y>=(  1) && (pixel_color += target[y-1][x+1]);
				x>=(     1) && y<(16-1) && (pixel_color += target[y+1][x-1]);
				x<(width-1) && y<(16-1) && (pixel_color += target[y+1][x+1]);

				x>=(     2) && (pixel_color += target[y][x-2]);
				x<(width-2) && (pixel_color += target[y][x+2]);
				y>=(     2) && (pixel_color += target[y-2][x]);
				y<(16   -2) && (pixel_color += target[y+2][x]);
			}

			if(pixel_color >= 200)
				dest[offs] = fgcolor;
			else if(pixel_color >= 10)
			{
				if(dest[offs] < 0xA0)
					dest[offs] = 0xC1;
				else
					dest[offs] = 0xD1;
			}
			else if(pixel_color > 0)
			{
				dest[offs] = 0xCF;
			}
		}
	}
}

void DrawTextTrans(uint8 *dest, uint32 width, uint8 *textmsg, uint8 fgcolor)
{
	DrawTextTransWH(dest, width, textmsg, fgcolor, 256, 16, 2);
}
