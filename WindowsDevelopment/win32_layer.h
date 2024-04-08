#include "types.h"
#pragma once

// ARGB
#define WIN32_WHITE 0xffffffff
#define WIN32_BLACK 0xff000000
#define WIN32_RED 0xffff0000
#define WIN32_BLUE 0xff0000ff
#define WIN32_GREEN 0xff00ff00
#define WIN32_DEFAULT 0xffe0e0e0
#define WIN32_RANDOM_COLOR 69

struct win32_grid_cell {
	uint32 x;
	uint32 y;
};

struct win32_grid {
	uint32 x;
	uint32 y;
	uint32 color[60][40];
};

void ClearScreenBuffer();
HWND CreateConsoleForDebugging();

void DrawTileMap(uint32 step);
void DrawRectangle(uint32);

void FillRectangleOnGrid(HWND, uint32, uint32);
void FillWindowWithFlatColor(uint32);
void FillBitmapMemoryWithFlatColor(uint32);


void InstantiateBitmapMemory(HWND);

void MakeAGrid(uint32, uchar8, uchar8, uchar8);
void MakeAPlayerCube(uint32, uint32, uint32, uint32);

void PathFinding(POINT*);

void RestoreSavedPattern();
void ResizeApplication(HWND);
void RunOnlyOnce();
LRESULT RootWindowProcedure(HWND, UINT, WPARAM, LPARAM);

void StretchPixels(HWND);
void AdjustClientArea(LPRECT rt);
uchar8 SaveBitPatternToFile(HWND, void*);

void UpdateClientAreaRECT(HWND);

//
void FillAdjasentRectangles();
void PaintRectangleWithCellCoords(win32_grid_cell*);
uchar8 CheckCell(win32_grid_cell* coords);
void PrintCellColor(POINT* cursor_pos);
