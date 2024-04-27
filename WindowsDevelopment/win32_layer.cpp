#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <WinUser.h>
#include <math.h>
#include <shellscalingapi.h>

#include "win32_layer.h"
#include "debugging.h"
#include "game_specific_declarations.h"
#include "linked_list.cpp"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


#define DEBUGG_MODE

private global_applicatiton_state game_state = {0};
private uchar8 initial_run = 0;
private win32_grid grid = {0};
private path_finding_nodes path = {0};

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int32 nCmdShow)
{
#ifdef DEBUGG_MODE
	if (SetProcessDPIAware() == 0)
		debugg_msg(L"Application is not scaled properly, check DPI awareness")
#endif  

	RECT client_area_initial_dimensions;
	AdjustClientArea(&client_area_initial_dimensions);

	game_state.gameIsRunning = true;
	game_state.Bitmapmemory = NULL;
	game_state.clientWidth = client_area_initial_dimensions.right - client_area_initial_dimensions.left;
	game_state.clientHeight = client_area_initial_dimensions.bottom - client_area_initial_dimensions.top;

	game_state.last_pixel = NULL;
	game_state.grid_step = 20;


	const wchar_t* class_name = L"Explore_and_enjoy";

	WNDCLASSW wc = { 0 };

	wc.lpfnWndProc = RootWindowProcedure;
	wc.lpszClassName = class_name;
	wc.hInstance = hInstance;
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpszMenuName = L"PathFinding_Algo";
	wc.hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1);
	wc.hIcon = (HICON)LoadImageW(NULL, L"small.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED | LR_LOADTRANSPARENT);

	if (RegisterClassW(&wc))
	{
		HWND root_window_handle = CreateWindowW(
			class_name,
			L"Path_Finding_Algo",
			WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			50,
			100,
			game_state.clientWidth,
			game_state.clientHeight,
			0,
			0,
			hInstance,
			0);
#ifdef DEBUGG_MODE
		CreateConsoleForDebugging();
#endif
		while (game_state.gameIsRunning)
		{
			MSG msg = {};
			while (PeekMessageW(&msg, root_window_handle, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			HDC hdc = GetDC(root_window_handle);

			RunOnlyOnce();


			// VISIT:
			//test();
			//int debugStop = 22;



			MakeAGrid(game_state.grid_step, 100, 100, 100);
			StretchPixels(root_window_handle);

			Sleep(1);
			ReleaseDC(root_window_handle, hdc);
		}
	}
	else
	{
		MessageBoxExW(NULL, L"WNDCLASSW returned NULL", L"Abort program execution", 0x00000001L, 0);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

LRESULT RootWindowProcedure(HWND window_handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		InstantiateBitmapMemory(window_handle);
	}
	break;
	case WM_CLOSE:
	{
		DestroyWindow(window_handle);
	}
	break;
	case WM_DESTROY:
	{
		game_state.gameIsRunning = false;
		PostQuitMessage(0);
	}
	break;
	case WM_SIZE:
	{
		ResizeApplication(window_handle);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(window_handle, &ps);
		EndPaint(window_handle, &ps);
	}
	break;
	case WM_MOUSEMOVE:
	{

		SetCursor(LoadCursor(NULL, IDC_CROSS));

		
		POINT cursor_pos;
		if (GetCursorPos(&cursor_pos))
		{
			ScreenToClient(window_handle, &cursor_pos);
		}

		if (wParam & MK_LBUTTON) {
#if 0
			if (IsItABlockCell(cursor_pos.x / game_state.grid_step, cursor_pos.y / game_state.grid_step) == NOT_BLOCK)
			FillRectangleOnGrid(window_handle, WIN32_RANDOM_COLOR, game_state.grid_step);
#endif
		}
		if ((GetKeyState(VK_RBUTTON) & 0x80) != 0) {
			FillRectangleOnGrid(window_handle, WIN32_BLACK, game_state.grid_step);
		}

	}
	break;
	case WM_LBUTTONDOWN:
	{
		POINT cursor_pos;
		if (GetCursorPos(&cursor_pos))
		{
			ScreenToClient(window_handle, &cursor_pos);
		}

		if (path.start.x == 0 && path.start.y == 0) {
			FillRectangleOnGrid(window_handle, WIN32_KELLY_GREEN, game_state.grid_step);
			path.start.x = cursor_pos.x / game_state.grid_step;
			path.start.y = cursor_pos.y / game_state.grid_step;
		}

		if (GetKeyState(VK_CONTROL) >> 15 & 0x1) { PrintCellCosts(&cursor_pos); }
#if 0
		if (GetKeyState(VK_LCONTROL) >> 15 & 0x1) {
			PrintCellColor(&cursor_pos);
			break;
		}
		
		if (IsItABlockCell(cursor_pos.x / game_state.grid_step, cursor_pos.y / game_state.grid_step) == NOT_BLOCK) {
			FillRectangleOnGrid(window_handle, WIN32_RANDOM_COLOR, game_state.grid_step);
		}

		if (GetKeyState(VK_MENU) >> 15 & 0x1)
			if (IsItABlockCell(cursor_pos.x / game_state.grid_step, cursor_pos.y / game_state.grid_step) != IS_BLOCK) 
				PathFinding();
#endif		
	}
	break;
	case WM_RBUTTONDOWN:
	{
		POINT cursor_pos;
		if (GetCursorPos(&cursor_pos))
		{
			ScreenToClient(window_handle, &cursor_pos);
		}

		if (GetKeyState(VK_CONTROL) >> 15 & 0x1) {
			if (path.end.x == 0 && path.end.y == 0) {
				FillRectangleOnGrid(window_handle, WIN32_BLUE, game_state.grid_step);
				path.end.x = cursor_pos.x / game_state.grid_step;
				path.end.y = cursor_pos.y / game_state.grid_step;
			}
		}
		else {
			FillRectangleOnGrid(window_handle, WIN32_BLACK, game_state.grid_step);
		}
	}
	break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 0x43: {
			ClearScreenBuffer();
		} break;
		case 0x45: // Key "E"
		{
			_CrtDumpMemoryLeaks();
			//PathFinding();
		}break;
		case 0x47: // Key "G"
		{
			MakeAGrid(20, 100, 100, 100);
		} break;
		case 0x51: // Key "Q"
		{

		}break;
		case 0x52: {
			RestoreSavedPattern();
		} break;
		case 0x53: {
			SaveBitPatternToFile(window_handle, game_state.Bitmapmemory);
		} break;
		case 0x5A: // Key "Z"
		{

		} break;
		default:
			break;
		}
	}
	break;
	default:
		return DefWindowProcW(window_handle, msg, wParam, lParam);
	}
	return 0;
}

void RunOnlyOnce()
{
	if (initial_run != 0)
		return;

	else {
		FillBitmapMemoryWithFlatColor(WIN32_DEFAULT);
		MakeAGrid(game_state.grid_step, 100, 100, 100);

		initial_run = 1;
	}
}

void InstantiateBitmapMemory(HWND window_handle)
{
	if (game_state.Bitmapmemory == NULL) {
		game_state.Bitmapmemory = VirtualAlloc(0, game_state.clientWidth * game_state.clientHeight * BYTES_PER_PIXEL, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		game_state.last_pixel = (uint32*)game_state.Bitmapmemory + (game_state.clientHeight * game_state.clientWidth) - 1;
	}
}

void UpdateClientAreaRECT(HWND window_handle)
{
	RECT rect;
	GetClientRect(window_handle, &rect);
	game_state.clientWidth = rect.right - rect.left;
	game_state.clientHeight = rect.bottom - rect.top;
}

void ResizeApplication(HWND window_handle)
{
	UpdateClientAreaRECT(window_handle);

	if (game_state.Bitmapmemory != NULL)
	{
		VirtualFree(game_state.Bitmapmemory, 0, MEM_RELEASE);
		game_state.last_pixel = NULL;
	}
	game_state.Bitmapmemory = VirtualAlloc(0, game_state.clientWidth * game_state.clientHeight * BYTES_PER_PIXEL, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	game_state.last_pixel = ((uint32*)game_state.Bitmapmemory + game_state.clientHeight * game_state.clientWidth) - 1;
}

void FillBitmapMemoryWithFlatColor(uint32 color)
{
	if (game_state.Bitmapmemory != NULL)
	{
		uint32* Pixel = (uint32*)game_state.Bitmapmemory;
		for (uint32 x = 0; x < game_state.clientHeight; x++) {
			for (uint32 y = 0; y < game_state.clientWidth; y++) {
				*Pixel++ = color;
			}
		}
	}
}

void StretchPixels(HWND window_handle)
{
	HDC hdc = GetDC(window_handle);

	BITMAPINFO btmpINFO;
	btmpINFO.bmiHeader.biSize = sizeof(btmpINFO.bmiHeader);
	btmpINFO.bmiHeader.biWidth = game_state.clientWidth;
	btmpINFO.bmiHeader.biHeight = -game_state.clientHeight;
	btmpINFO.bmiHeader.biPlanes = 1;
	btmpINFO.bmiHeader.biBitCount = 32;
	btmpINFO.bmiHeader.biCompression = BI_RGB;

	StretchDIBits(
		hdc,
		0, 0,
		game_state.clientWidth,
		game_state.clientHeight,
		0, 0,
		game_state.clientWidth,
		game_state.clientHeight,
		game_state.Bitmapmemory,
		&btmpINFO,
		DIB_RGB_COLORS,
		SRCCOPY);
}

void MakeAGrid(uint32 step, uchar8 red, uchar8 green, uchar8 blue)
{
	uint32 cells = 0;
	uchar8* Pixel = (uchar8*)game_state.Bitmapmemory;
	for (int32 x = 0; x < game_state.clientHeight; x++) {
		for (int32 y = 0; y < game_state.clientWidth; y++) {
			if (y % step == 0 || x % step == 0) {
				*Pixel++ = blue;

				*Pixel++ = green;

				*Pixel++ = red;

				*Pixel++ = UNSIGNED_CHAR_MAX_VALUE;
				
			}
			else if (initial_run != 1 &&  x % 10 == 0 && y % 10 == 0) {
				grid.color[y / game_state.grid_step][x / game_state.grid_step] = *Pixel;
				cells++;
				Pixel += BYTES_PER_PIXEL;
			}
			else {
				Pixel += BYTES_PER_PIXEL;
			}
		}
	}
#if 0
	printf("Amount of cells - %d\n", cells);
#endif
}

void MakeAPlayerCube(uint32 width, uint32 height, uint32 coordX, uint32 coordY) {
	uchar8* Pixel = (uchar8*)game_state.Bitmapmemory;
	for (int x = 0; x < game_state.clientHeight; ++x) {
		for (int y = 0; y < game_state.clientWidth; ++y) {

			if (((x > (coordX - width)) && (x < (coordX + width))) && ((y > (coordY - height)) && (y < (coordY + height)))) {
				*Pixel = 60;
				++Pixel;
				*Pixel = 0;
				++Pixel;
				*Pixel = 100;
				++Pixel;
				*Pixel = 0;
				++Pixel;
			}

			else {
				Pixel += BYTES_PER_PIXEL;
			}
		}
	}
}

void FillRectangleOnGrid(HWND window_handle, uint32 rect_color, uint32 step)
{
	POINT cursor_position;
	if (GetCursorPos(&cursor_position))
	{
		uint32 tile_width = step;
		uint32 tile_index_x, tile_index_y = 0;
		int tile_width_x = step;

		ScreenToClient(window_handle, &cursor_position);

		tile_index_x = cursor_position.x / tile_width;
		tile_index_y = cursor_position.y / tile_width;

		grid.cell.x = tile_index_x;
		grid.cell.y = tile_index_y;

		if (game_state.clientWidth - cursor_position.x < step) {
			tile_width_x = game_state.clientWidth - tile_index_x * tile_width;
		}

		uchar8 r, g, b = 0;

		if (rect_color == WIN32_RANDOM_COLOR) {
			rect_color = 0;
			struct timespec ts;

			timespec_get(&ts, TIME_UTC);
			srand(ts.tv_nsec);
			r = rand() & 255;

			timespec_get(&ts, TIME_UTC);
			srand(ts.tv_nsec);
			g = rand() & 255;

			timespec_get(&ts, TIME_UTC);
			srand(ts.tv_nsec);
			b = rand() & 255;

			rect_color |= 0xff << 24 | b << 16 | g << 8 | r;
		}
		// !!!! Updating array with grid colors !!!!
		grid.color[tile_index_x][tile_index_y] = rect_color;


		uint32* Pixel = (uint32*)game_state.Bitmapmemory;
		Pixel += (tile_index_y * tile_width) * game_state.clientWidth + (tile_index_x * tile_width);

		for (int x = 0; x < tile_width; x++) {
			for (int y = 0; y < tile_width_x; y++) {
				if (Pixel > game_state.last_pixel) {
					break;
				}
				*Pixel++ = rect_color;
			}
			Pixel += game_state.clientWidth - tile_width_x;
		}
	}
}

void AdjustClientArea(LPRECT lprect)
{
	lprect->left = 0;
	lprect->top = 0;
	lprect->right = 1200;
	lprect->bottom = 800;

	AdjustWindowRectEx(lprect, WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 0, 0);
}

uchar8 SaveBitPatternToFile(HWND, void*)
{
	HANDLE file_handle = NULL;
	uint32 total_bytes_size = game_state.clientHeight * game_state.clientWidth * BYTES_PER_PIXEL;
	DWORD bytes_written = 0;

	file_handle = CreateFileW(L"save_file", FILE_GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);


	if (GetLastError() == ERROR_FILE_EXISTS) {
		file_handle = CreateFileW(L"save_file", FILE_GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		WriteFile(file_handle, game_state.Bitmapmemory, total_bytes_size, &bytes_written, NULL);
	}
	else
	{
		WriteFile(file_handle, game_state.Bitmapmemory, total_bytes_size, &bytes_written, NULL);
	}
	CloseHandle(file_handle);
	return 0;
}

void RestoreSavedPattern()
{
	HANDLE file_handle = NULL;
	DWORD amount_of_bytes_was_read = 0;
	uint32 total_bytes_size = game_state.clientHeight * game_state.clientWidth * BYTES_PER_PIXEL;

	void* bytes_to_read = malloc(total_bytes_size);


	file_handle = CreateFile(L"save_file", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file_handle == INVALID_HANDLE_VALUE) {
		debugg_msg(L"You have 0 saves");
		free(bytes_to_read);
		return;
	}

	ReadFile(file_handle, bytes_to_read, total_bytes_size, &amount_of_bytes_was_read, NULL);
	CloseHandle(file_handle);

	if (amount_of_bytes_was_read <= total_bytes_size) {
		memcpy(game_state.Bitmapmemory, bytes_to_read, total_bytes_size);
	}
	else {
		debugg_msg(L"SMTH went wrong during memcpy");
	}

	free(bytes_to_read);
}

void ClearScreenBuffer()
{
	uint32* Pixel = (uint32*)game_state.Bitmapmemory;
	for (uint32 i = 0; i < game_state.clientWidth * game_state.clientHeight; ++i) {
		*Pixel++ = WIN32_DEFAULT;
	}
	return;
}

void DrawTileMap(uint32 step)
{
	// Iterate over a tile map
	// Draw every tile

}

HWND CreateConsoleForDebugging()
{
	FILE* fp;

	AllocConsole();
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);

	HWND console_handle = GetConsoleWindow();
	MoveWindow(console_handle, 1500, 100, 800, 1200, true);
#ifdef DEBUGG_MODE
	unsigned int dpix;
	unsigned int dpiy;
	const POINT ptZero = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	HRESULT isOK = GetDpiForMonitor(monitor, MDT_RAW_DPI, &dpix, &dpiy);
	printf("is DPI function returned correctly? 0 is only correct answer - %i\n", isOK);
	printf("DPI on my laptop is - %i\n", dpix);
#endif
	return console_handle;
}

void PathFinding()
{
#ifdef DEBUGG_MODE
	printf("Current cell is %d,%d\n", grid.cell.x, grid.cell.y);
#endif
	//TODO:	FIX hardcoded values!!!!
		printf("Start Path Finding");
		
		SearchAdjasentNodes(&path.start);
}

void SearchAdjasentNodes(win32_cell* middle_cell)
{
	win32_cell top_cell, right_cell, bottom_cell, left_cell = {};

	top_cell.x = middle_cell->x;
	top_cell.y = middle_cell->y - 1;

	right_cell.x = middle_cell->x + 1;
	right_cell.y = middle_cell->y;

	bottom_cell.x = middle_cell->x;
	bottom_cell.y = middle_cell->y + 1;

	left_cell.x = middle_cell->x - 1;
	left_cell.y = middle_cell->y;

	if (top_cell.y >=0 && top_cell.x>=0 && IsItABlockCell(top_cell.x, top_cell.y) == NOT_BLOCK) {
		
		path.node[top_cell.x][top_cell.y].coords.x = top_cell.x;
		path.node[top_cell.x][top_cell.y].coords.y = top_cell.y;

		path.node[top_cell.x][top_cell.y].g_cost = 10;
		path.node[top_cell.x][top_cell.y].h_cost = hypot(abs((int32)top_cell.x - (int32)path.end.x), abs((int32)top_cell.y - (int32)path.end.y));
		path.node[top_cell.x][top_cell.y].total_weight = path.node[top_cell.x][top_cell.y].g_cost + path.node[top_cell.x][top_cell.y].h_cost;

	}

	if (right_cell.y >= 0 && right_cell.x>= 0 && right_cell.x <= 59 && right_cell.y <= 39 && IsItABlockCell(right_cell.x, right_cell.y) == NOT_BLOCK) {

		path.node[right_cell.x][right_cell.y].coords.x = right_cell.x;
		path.node[right_cell.x][right_cell.y].coords.y = right_cell.y;

		path.node[right_cell.x][right_cell.y].g_cost = 10;
		path.node[right_cell.x][right_cell.y].h_cost = hypot(abs((int32)right_cell.x - (int32)path.end.x), abs((int32)right_cell.y - (int32)path.end.y));
		path.node[right_cell.x][right_cell.y].total_weight = path.node[right_cell.x][right_cell.y].g_cost + path.node[right_cell.x][right_cell.y].h_cost;

	}

	if (bottom_cell.y >= 0 && bottom_cell.x >= 0 && bottom_cell.x <= 59 && bottom_cell.y <= 39 && IsItABlockCell(bottom_cell.x, bottom_cell.y) == NOT_BLOCK) {

		path.node[bottom_cell.x][bottom_cell.y].coords.x = bottom_cell.x;
		path.node[bottom_cell.x][bottom_cell.y].coords.y = bottom_cell.y;

		path.node[bottom_cell.x][bottom_cell.y].g_cost = 10;
		path.node[bottom_cell.x][bottom_cell.y].h_cost = hypot(abs((int32)bottom_cell.x - (int32)path.end.x), abs((int32)bottom_cell.y - (int32)path.end.y));
		path.node[bottom_cell.x][bottom_cell.y].total_weight = path.node[bottom_cell.x][bottom_cell.y].g_cost + path.node[bottom_cell.x][bottom_cell.y].h_cost;
	}

	if (left_cell.y >= 0 && left_cell.x >= 0 && left_cell.x <= 59 && left_cell.y <= 39 && IsItABlockCell(left_cell.x, left_cell.y) == NOT_BLOCK) {

		path.node[left_cell.x][left_cell.y].coords.x = left_cell.x;
		path.node[left_cell.x][left_cell.y].coords.y = left_cell.y;

		path.node[left_cell.x][left_cell.y].g_cost = 10;
		path.node[left_cell.x][left_cell.y].h_cost = hypot(abs((int32)left_cell.x - (int32)path.end.x), abs((int32)left_cell.y - (int32)path.end.y));
		path.node[left_cell.x][left_cell.y].total_weight = path.node[left_cell.x][left_cell.y].g_cost + path.node[left_cell.x][left_cell.y].h_cost;

	}

	// Know we can check the most promise node if any is present
	node most_promising_cell, most_p_cell_vertical, most_p_cell_horizontal;
	
	path.node[left_cell.x][left_cell.y].total_weight < path.node[right_cell.x][right_cell.y].total_weight ? most_p_cell_horizontal = path.node[left_cell.x][left_cell.y] :
		most_p_cell_horizontal = path.node[right_cell.x][right_cell.y];

	path.node[top_cell.x][top_cell.y].total_weight < path.node[bottom_cell.x][bottom_cell.y].total_weight ? most_p_cell_vertical = path.node[top_cell.x][top_cell.y] :
		most_p_cell_vertical = path.node[bottom_cell.x][bottom_cell.y];

	most_p_cell_horizontal.total_weight < most_p_cell_vertical.total_weight ? most_promising_cell = most_p_cell_horizontal : most_promising_cell = most_p_cell_vertical;

	PaintRectangleByCellCoordsPathFinding(most_promising_cell.coords.x, most_promising_cell.coords.y);

	if(most_promising_cell.coords.x != path.end.x && most_promising_cell.coords.y != path.end.y)
		SearchAdjasentNodes(&most_promising_cell.coords);
}
#if 0
void PaintRectangleWithCellCoords(uint32 x, uint32 y)
{
	uint32* Pixel = (uint32*)game_state.Bitmapmemory;
	Pixel += (y * game_state.grid_step) * game_state.clientWidth + (x * game_state.grid_step);

	for (int x = 0; x < game_state.grid_step; x++) {
		for (int y = 0; y < game_state.grid_step; y++) {
			if (Pixel > game_state.last_pixel) {
				break;
			}
			*Pixel++ = WIN32_YELLOW;
		}
		Pixel += game_state.clientWidth - game_state.grid_step;
	}
}
#endif

void PaintRectangleByCellCoordsPathFinding(uint32 x, uint32 y)
{
	uint32* Pixel = (uint32*)game_state.Bitmapmemory;
	Pixel += (y * game_state.grid_step) * game_state.clientWidth + (x * game_state.grid_step);

	for (int x = 0; x < game_state.grid_step; x++) {
		for (int y = 0; y < game_state.grid_step; y++) {
			if (Pixel > game_state.last_pixel) {
				break;
			}
			*Pixel++ = WIN32_GREEN;
		}
		Pixel += game_state.clientWidth - game_state.grid_step;
	}
}

uchar8 IsItABlockCell(uint32 x, uint32 y)
{
	if (grid.color[x][y] == WIN32_BLACK) return IS_BLOCK;
	else return NOT_BLOCK;
}
void PrintCellColor(POINT* cursor_pos)
{
	printf("Cell [%d][%d] color is - %x\n", cursor_pos->x / game_state.grid_step, cursor_pos->y / game_state.grid_step,
		grid.color[cursor_pos->x / game_state.grid_step][cursor_pos->y / game_state.grid_step]);
}

void FillRectangleByCellCoords(uint32 gridX, uint32 gridY, uint32 color) 
{
		uint32* Pixel = (uint32*)game_state.Bitmapmemory;
		Pixel += (gridY * game_state.grid_step) * game_state.clientWidth + (gridX * game_state.grid_step);

		for (int x = 0; x < game_state.grid_step; x++) {
			for (int y = 0; y < game_state.grid_step; y++) {
				if (Pixel > game_state.last_pixel) {
					debugg_msg(L"Tried to reference Pixel that is out of memory")
					break;
				}
				*Pixel++ = color;
			}
			Pixel += game_state.clientWidth - game_state.grid_step;
		}
}


void PrintCellCosts(POINT* cp)
{
	node* node = &(path.node[cp->x / game_state.grid_step][cp->y / game_state.grid_step]);
	printf("\n*******************\nG cost - %f\nH cost - %f\nTotal - %f\n*******************\n", node->g_cost, node->h_cost, node->total_weight);
}


void test() {
	//linked_list my_list;
	//my_list.next = NULL;
	//printf("NEXT - %p", my_list.next);
}