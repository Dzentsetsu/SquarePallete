#include "types.h"
#pragma once

// ARGB
#define WIN32_WHITE 0xffffffff
#define WIN32_BLACK 0xff000000
#define WIN32_RED 0xffff0000
#define WIN32_BLUE 0xff0000ff
#define WIN32_GREEN 0xff00ff00
#define WIN32_YELLOW 0xffffff00
#define WIN32_DEFAULT 0xffe0e0e0
#define WIN32_RANDOM_COLOR 69
#define WIN32_KELLY_GREEN 0xff4cbb17

struct win32_cell {
	uint32 x;
	uint32 y;
};

struct win32_grid {
	win32_cell cell;
	uint32 color[60][40];
};

struct node {
	win32_cell coords;
	double total_weight;
	double h_cost; // How far away from the end node
	double g_cost; // How far away fron the start node
};

struct path_finding_nodes {
	win32_cell start; 
	win32_cell end;
	node node[60][40];
};


void ClearScreenBuffer();
HWND CreateConsoleForDebugging();

void DrawTileMap(uint32 step);

void FillRectangleOnGrid(HWND, uint32, uint32);
void FillWindowWithFlatColor(uint32);
void FillBitmapMemoryWithFlatColor(uint32);
void FillRectangleByCellCoords(uint32 gridX, uint32 gridY, uint32 color);


void InstantiateBitmapMemory(HWND);

void MakeAGrid(uint32, uchar8, uchar8, uchar8);
void MakeAPlayerCube(uint32, uint32, uint32, uint32);

void PathFinding();

void RestoreSavedPattern();
void ResizeApplication(HWND);
void RunOnlyOnce();
LRESULT RootWindowProcedure(HWND, UINT, WPARAM, LPARAM);

void StretchPixels(HWND);
void AdjustClientArea(LPRECT rt);
uchar8 SaveBitPatternToFile(HWND, void*);

void UpdateClientAreaRECT(HWND);

//
void SearchAdjasentNodes(win32_cell * next_cell);
void PaintRectangleWithCellCoords(uint32 x, uint32 y);
uchar8 IsItABlockCell(uint32 x, uint32 y);
void PrintCellColor(POINT* cursor_pos);
void PrintCellCosts(POINT*);
void PaintRectangleByCellCoordsPathFinding(uint32 x, uint32 y);
void test();