#ifndef CORE_H
#define CORE_H

#include "libraries.h"

#define MAX_W 72
#define MAX_H 48

typedef struct
{
	bool active;
	int id;
	float vx, vy;
} Cell; //heh, funny

extern Cell board[MAX_W][MAX_H];

extern int cursorX;
extern int cursorY;

void GenerateBoard();
void MoveCursor(int dx, int dy);

#endif