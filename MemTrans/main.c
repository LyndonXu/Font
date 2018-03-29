#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define WIDTH	128
#define HEIGHT	64

#define WIDTH_BYTE	(WIDTH / 8)
#define HEIGHT_BYTE	(HEIGHT / 8)

uint8_t u8MapMem[WIDTH_BYTE * HEIGHT];
uint8_t u8RealMem[(HEIGHT_BYTE + 1) * WIDTH] = {0};

void Trans(void)
{
	int32_t i;
	int32_t j;
	uint8_t *pRealMemTmp = u8RealMem;
	for (j = 0; j < HEIGHT_BYTE; j++)
	{
		uint8_t *pMapMemTmp = u8MapMem + j * WIDTH_BYTE * 8;
		for (i = 0; i < WIDTH_BYTE; i++)
		{
			uint8_t u8Map8Byte[8];
			int32_t k;
			uint8_t *pTmpSrc = pMapMemTmp + i;
			uint8_t u8Tmp = 0;
			for (k = 0; k < 8; k++)
			{
				u8Map8Byte[k] = pTmpSrc[k * WIDTH_BYTE];
			}

			for (k = 7; k >= 0; k--)
			{
				uint8_t n;
				for (n = 0; n < 8; n++)
				{
					u8Tmp <<= 1;
					u8Tmp |= (u8Map8Byte[n] >> k);
				}
				*pRealMemTmp++ = u8Tmp;
			}

		}
	}
}


void DrawPicToFile(void)
{
	FILE *pFile = fopen("f:\\PIC.txt", "wb+");
	if (pFile != NULL)
	{
		int i, j, k;
		for (i = 0; i < HEIGHT_BYTE; i++)
		{
			uint8_t *pTmp = u8RealMem + i * WIDTH;
			for (j = 0; j < 8; j++)
			{
				char u8Tmp;
				for (k = 0; k < WIDTH; k++)
				{
					if ((pTmp[k] & (0x01 << j)) == 0)
					{
						u8Tmp = ' ';
					}
					else
					{
						u8Tmp = '*';
					}
					fwrite(&u8Tmp, 1, 1, pFile);
				}
				u8Tmp = '\n';
				fwrite(&u8Tmp, 1, 1, pFile);
			}
		}
		fclose(pFile);
	}
}


const char u8ZhongCHS[] =
{
	0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x10, 0x04, 0x10, 0x04, 0x10, 0x04, 0x10, 0x04, 0xff, 0xff,
	0x10, 0x04, 0x10, 0x04, 0x10, 0x04, 0x10, 0x04, 0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};/* жа */

const char u8YingCHS[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x00, 0x00, 0x1c, 0x60, 0x00, 0x00, 0x7c, 0x40, 0x18,
	0x0c, 0x78, 0xe0, 0x3f, 0x38, 0x70, 0xf8, 0x3f, 0xf8, 0x20, 0xff, 0x3f, 0xf0, 0xe0, 0xff, 0x00,
	0xf0, 0xfc, 0x07, 0x20, 0x00, 0xff, 0x00, 0x30, 0x40, 0xff, 0x00, 0x38, 0x40, 0xfc, 0x10, 0x1c,
	0x40, 0x7c, 0x30, 0x0e, 0x40, 0x88, 0x31, 0x0f, 0x40, 0x88, 0xf9, 0x07, 0xfc, 0xcb, 0xf9, 0x13,
	0xfc, 0x6b, 0x9d, 0x11, 0xfc, 0xfb, 0x15, 0x31, 0x48, 0xb8, 0xf7, 0x7f, 0x40, 0x98, 0xf3, 0x7f,
	0x40, 0x98, 0xf3, 0x3f, 0xfc, 0x8b, 0xf1, 0x1f, 0xfc, 0xcb, 0x91, 0x00, 0xfc, 0xeb, 0x93, 0x01,
	0x48, 0xe8, 0x97, 0x03, 0x40, 0x48, 0x1e, 0x07, 0x60, 0x68, 0x7c, 0x1f, 0x60, 0x7c, 0x78, 0x3e,
	0x60, 0x3c, 0x70, 0x3c, 0x40, 0x1c, 0x00, 0x18, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

int32_t DrawFillRectWithBuf(uint16_t u16XPos, uint16_t u16YPos,
	uint16_t u16Width, uint16_t u16Height, void *pBuf)
{
	uint8_t *pSrc = (uint8_t *)pBuf;
	uint8_t u8Offset = u16YPos & 0x07;
	uint8_t u8Mask = 0xFF << u16YPos;



	if (u16XPos >= WIDTH || u16YPos >= HEIGHT)
	{
		return -1;
	}


	if ((u16Width > WIDTH) || (u16Height > HEIGHT))
	{
		return -1;
	}

	if ((u16XPos + u16Width) > WIDTH)
	{
		return -1;
	}

	if (u16Height + u16YPos > HEIGHT)
	{
		return -1;
	}

	do 
	{
		uint16_t i;
		uint8_t *pDest = u8RealMem + (u16YPos >> 3) * WIDTH + u16XPos;


		for (i = 0; i < (u16Height + 7) / 8; i++)
		{
			uint8_t u8Src = pSrc[0];
			pDest[0] &= ~u8Mask;
			pDest[0] |= u8Src << u8Offset;
			pDest += WIDTH;
			pDest[0] &= u8Mask;
			pDest[0] |= u8Src >> (8 - u8Offset);
			pSrc++;
		}
		u16XPos++;
		u16Width--;
	} while ((int16_t)u16Width > 0);

	return 0;


}

int32_t DrawFillRect(uint16_t u16XPos, uint16_t u16YPos,
	uint16_t u16Width, uint16_t u16Height, bool boIsClear)
{
	uint16_t u16HeightTmp;
	uint16_t u16BytesPosTmp;
	uint16_t u16Tmp;
	uint8_t u8Tmp;
	uint8_t *pTmp = u8RealMem;
	


	if (u16XPos >= WIDTH || u16YPos >= HEIGHT)
	{
		return -1;
	}


	if (u16Height > HEIGHT)
	{
		u16Height = HEIGHT;		/* redress the value of end height */
	}

	if (u16Width > WIDTH)
	{
		u16Width = WIDTH;				/* redress the value of end height */
	}

	if ((u16XPos + u16Width) > WIDTH)
	{
		u16Width = WIDTH - u16XPos;	/* redress the value of width */
	}

	if (u16Height + u16YPos > HEIGHT)
	{
		u16Height = HEIGHT - u16YPos;		/* redress the value of end height */
	}


	do
	{
		u8Tmp = 0xFF;
		u16BytesPosTmp = u16YPos >> 3;	/* get the byte beginning */
		u16Tmp = u16YPos & 0x07;	/* get the bit beginning */

		u16HeightTmp = 0x08 - u16Tmp;

		if (u16HeightTmp > u16Height)
		{
			u8Tmp >>= (8 - u16Height);	/* clear the overage bits */
			u16HeightTmp = u16Height;
		}
		u8Tmp <<= u16Tmp;
		if (boIsClear)
		{
			uint16_t i;
			u8Tmp = ~u8Tmp;
			for (i = u16XPos; i < u16Width; i++)
			{
				pTmp[i + u16BytesPosTmp * WIDTH] &= u8Tmp;
			}
		}
		else
		{
			uint16_t i;
			for (i = u16XPos; i < u16Width; i++)
			{
				pTmp[i + u16BytesPosTmp * WIDTH] |= u8Tmp;
			}
		}
		u16Height -= u16HeightTmp;
		u16YPos += u16HeightTmp;
	} while ((int16_t)u16Height > 0);

	return 0;
}



int main()
{
	//u8RealMem[0] = u8RealMem[128] = 0xFF;

	//DrawFillRect(3, 5, 100, 30, false);
	DrawFillRectWithBuf(3, 7, 32, 32, u8YingCHS);
	DrawPicToFile();
	return 0;
}