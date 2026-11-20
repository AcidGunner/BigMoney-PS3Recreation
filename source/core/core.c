#include "core.h"

Cell board[MAX_W][MAX_H];

int cursorX = 0;
int cursorY = 0;

void GenerateBoard()
{
	int xx, yy;
    memset(board, 0, sizeof(board));
	for (xx = 0; xx < MAX_W; xx++)
    {
        for (yy = 0; yy < MAX_H; yy++)
        {
            board[xx][yy].id = 0;
            board[xx][yy].active = false;
        }
    }
}

void MoveCursor(int dx, int dy)
{
    int nx = cursorX;
    int ny = cursorY;

    do
    {
        nx += dx; ny += dy;

        if(nx < 0 || ny < 0 || nx >= MAX_W || ny >= MAX_H) return;
    }
    while (!board[nx][ny].active);

    cursorX = nx;
    cursorY = ny;
}