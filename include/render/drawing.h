#ifndef DRAWING_H
#define DRAWING_H

#include "libraries.h"

typedef struct
{
    float x, y, z;
    u32 color;
    float u, v;
} RSXVertex;

#define PAD_TRIANGLE 0
#define PAD_CIRCLE 1
#define PAD_CROSS 2
#define PAD_SQUARE 3
#define PAD_START 4
#define PAD_SELECT 5
#define PAD_L1 6
#define PAD_R1 7
#define PAD_L2 8
#define PAD_R2 9
#define PAD_L3 10
#define PAD_R3 11

typedef enum GameScreen
{
	LOGO,
	TITLE,
	TEST
}
GameScreen;

extern GameScreen currentScreen;

extern u32 *texture_pointer;

void drawScene();
void LoadTexture();
int RandRange();
void DrawFString(int wrap, float x, float y, u32 textColor, int BW, int size, const char *fmt, ...);
void DrawTextAligned(int wrap, float x1, float x2, float y, int align, u32 tc, int bw, int fsize, int size, const char *text, ...);

void FadeInOut(GameScreen *variable, GameScreen value);
void UpdateFade(float dt);

#endif