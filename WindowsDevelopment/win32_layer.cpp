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

#ifdef DEBUGG_MODE
//#undef DEBUGG_MODE
#endif

private global_applicatiton_state game_state;
private uchar8 initial_run = 0;
private win32_grid_cell cell = { 0 };
private win32_grid grid = {};

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int32 nCmdShow)
{
	RECT client_area_initial_dimensions;
	AdjustClientArea(&client_area_initial_dimensions);

	game_state.gameIsRunning = true;
	game_state.Bitmapmemory = NULL;
	game_state.clientWidth = client_area_initial_dimensions.right - client_area_initial_dimensions.left;
	game_state.clientHeight = client_area_initial_dimensions.bottom - client_area_initial_dimensions.top;

	game_state.last_pixel = NULL;
	game_state.grid_step = 20;

#ifdef DEBUGG_MODE
	if (SetProcessDPIAware() == 0)
		debugg_msg(L"Application is not scaled properly, check DPI awareness")
#endif  
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
#ifndef DEBUGG_MODE
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
			FillRectangleOnGrid(window_handle, WIN32_RANDOM_COLOR, game_state.grid_step);
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

		if (GetKeyState(VK_LCONTROL) >> 15 & 0x1) {
			PrintCellColor(&cursor_pos);
		}
		else {
		FillRectangleOnGrid(window_handle, WIN32_RANDOM_COLOR, game_state.grid_step);
		}

		//PrintCellColor(&cursor_pos);
		//PathFinding(&cursor_pos);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		FillRectangleOnGrid(window_handle, WIN32_DEFAULT, game_state.grid_step);
	}
	break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 0x43: {
			ClearScreenBuffer();
		} break;
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
				Pixel += BYTES_PER_PIXEL;
			}
			else {
				Pixel += BYTES_PER_PIXEL;
			}
		}
	}
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

		cell.x = tile_index_x;
		cell.y = tile_index_y;

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

void PathFinding(POINT* cursor_pos)
{
	printf("Current cell is %d,%d\n", cell.x, cell.y);
	// Check adjacent cells

	if (cell.x >= 1 && cell.y >= 1 && cell.x < 59 && cell.y < 39) {
		FillAdjasentRectangles();
	}

}

void FillAdjasentRectangles()
{
	win32_grid_cell cell_to_paint = { 0 };
	cell_to_paint.x = cell.x;
	cell_to_paint.y = cell.y - 1;

	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}

	cell_to_paint.x = cell_to_paint.x + 1;
	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}

	cell_to_paint.y = cell_to_paint.y + 1;
	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}

	cell_to_paint.y = cell_to_paint.y + 1;
	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}

	cell_to_paint.x = cell_to_paint.x - 1;
	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}

	cell_to_paint.x = cell_to_paint.x - 1;
	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}

	cell_to_paint.y = cell_to_paint.y - 1;
	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}

	cell_to_paint.y = cell_to_paint.y - 1;
	if (CheckCell(&cell_to_paint)) {
		PaintRectangleWithCellCoords(&cell_to_paint);
	}
}

void PaintRectangleWithCellCoords(win32_grid_cell* coords)
{
	uint32* Pixel = (uint32*)game_state.Bitmapmemory;
	Pixel += (coords->y * game_state.grid_step) * game_state.clientWidth + (coords->x * game_state.grid_step);

	for (int x = 0; x < game_state.grid_step; x++) {
		for (int y = 0; y < game_state.grid_step; y++) {
			if (Pixel > game_state.last_pixel) {
				break;
			}

			*Pixel++ = WIN32_BLUE;
		}
		Pixel += game_state.clientWidth - game_state.grid_step;
	}
}
uchar8 CheckCell(win32_grid_cell* coords)
{

	uint32* Pixel = (uint32*)game_state.Bitmapmemory;
	Pixel += (coords->y * game_state.grid_step) * game_state.clientWidth + (coords->x * game_state.grid_step);

	for (int x = 0; x < game_state.grid_step / 2; x++) {
		for (int y = 0; y < game_state.grid_step / 2; y++) {
			if (Pixel > game_state.last_pixel) {
				break;
			}
			if (x == game_state.grid_step / 2 - 1 && y == game_state.grid_step / 2 - 1) {
				if (*Pixel == WIN32_BLACK) return 0;
			}
			Pixel++;
		}
		Pixel += game_state.clientWidth - game_state.grid_step;
	}

	return 1;
}
void PrintCellColor(POINT* cursor_pos)
{
	printf("Cell [%d][%d] color is - %x\n", cursor_pos->x / game_state.grid_step, cursor_pos->y / game_state.grid_step,
		grid.color[cursor_pos->x / game_state.grid_step][cursor_pos->y / game_state.grid_step]);
}