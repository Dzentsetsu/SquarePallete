#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <WinUser.h>
#include <math.h>
#include <shellscalingapi.h>

#if 0
#define DEBUGG_MODE
#endif

#define debugg_msg(debuggMessage) MessageBoxExW(NULL, debuggMessage, L"Ошибочка вышла", 0x00000001L, 0);
#define UNSIGNED_CHAR_MAX_VALUE 255
#define UNSIGNED_CHAR_MIN_VALUE 0

typedef unsigned char uchar8;
typedef int int32;
typedef long long int int64;
typedef unsigned int uint32;
typedef unsigned long long int uint64;

static const int BytesPerPixel = 4;
static  int xDisplacement = 350;
static  int yDisplacement = 350;
static unsigned int displacementMang = 100;
static uchar8 initial_run = 0;
static uint32 displaying_grid_on_top;

struct global_applicatiton_state
{
	boolean gameIsRunning;
	void* Bitmapmemory;
	uint32* last_pixel;
	int clientWidth;
	int clientHeight;
	uint32 default_color;
} game_state;

static HINSTANCE hInstance_w;
HWND child;

void ClearScreenBuffer();
void RestoreSavedPattern();
void SetDPI_Awareness(LPRECT rt);
void RunOnlyOnce();
void FillRectangleOnGrid(HWND, uint32);
void UpdateClientAreaRECT(HWND);
void InstantiateBitmapMemory(HWND);
void ResizeApplication(HWND);
void FillWindowWithFlatColor(uint32);
void StretchPixels(HWND);
void MakeAGrid(uint32, uchar8, uchar8, uchar8);
void MakeAPlayerCube(uint32, uint32, uint32, uint32);
void DrawRectangle(uint32);
void FillBitmapMemoryWithFlatColor(uint32);
uchar8 SaveBitPatternToFile(HWND, void*);
LRESULT RootWindowProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT LittleWindowProc(HWND, UINT, WPARAM, LPARAM);
HWND CreateChildWindow(HINSTANCE hInstance, HWND parent_window);

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	hInstance_w = hInstance;
	RECT client_area_initial_dimensions;

	SetDPI_Awareness(&client_area_initial_dimensions);

	game_state.gameIsRunning = true;
	game_state.Bitmapmemory = NULL;
	game_state.clientWidth = client_area_initial_dimensions.right - client_area_initial_dimensions.left;
	game_state.clientHeight = client_area_initial_dimensions.bottom - client_area_initial_dimensions.top;
	game_state.last_pixel = NULL;
	game_state.default_color = 0b111000001110000011100000;

#ifdef DEBUGG_MODE
	if (SetProcessDPIAware() == 0)
		debugg_msg(L"Application is not scaled properly, check DPI awareness")
#endif  
		const wchar_t* class_name = L"AttemptToSpinAppWin32App";

	WNDCLASSW wc = { 0 };

	wc.lpfnWndProc = RootWindowProcedure;
	wc.lpszClassName = class_name;
	wc.hInstance = hInstance;
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpszMenuName = L"SquarePallete";
	wc.hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1);
	wc.hIcon = (HICON)LoadImageW(NULL, L"small.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED | LR_LOADTRANSPARENT);

	if (RegisterClassW(&wc))
	{
		HWND root_window_handle = CreateWindowW(
			class_name,
			L"Squares Pallete",
			WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
			50,
			50,
			game_state.clientWidth,
			game_state.clientHeight,
			0,
			0,
			hInstance,
			0);
		child = CreateWindow(L"STATIC", L"Child Window", WS_CHILD | WS_VISIBLE | WS_BORDER, 50, 50, 200, 100, root_window_handle, NULL, hInstance_w, NULL);
#ifdef DEBUGG_MODE
		FILE* fp;

		AllocConsole();
		freopen_s(&fp, "CONIN$", "r", stdin);
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);

		HWND console_handle = GetConsoleWindow();
		MoveWindow(console_handle, 1500, 100, 800, 1200, true);
#if 0
		unsigned int dpix;
		unsigned int dpiy;
		HMONITOR monitor = GetPrimaryMonitorHandle();
		HRESULT isOK = GetDpiForMonitor(monitor, MDT_RAW_DPI, &dpiX, &dpiY);
		printf("is DPI function returned correctly? 0 is only correct answer - %i\n", isOK);
		printf("DPI on my laptop is - %i\n", dpiX);
#endif

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

			if (displaying_grid_on_top == 1)  MakeAGrid(100, 200, 200, 200);

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
#ifdef DEBUGG_MODE
		static int count = 1;
		printf("%d times WM_SIZE was called\n", count++);
#endif
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
	}
	break;
	case WM_LBUTTONDOWN:
	{
		FillRectangleOnGrid(window_handle, 0);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		FillRectangleOnGrid(window_handle, game_state.default_color);
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
			displaying_grid_on_top == 0 ? displaying_grid_on_top = 1 : displaying_grid_on_top = 0;
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
		FillBitmapMemoryWithFlatColor(game_state.default_color);
		initial_run = 1;
	}
}

void InstantiateBitmapMemory(HWND window_handle)
{
	if (game_state.Bitmapmemory == NULL) {
		game_state.Bitmapmemory = VirtualAlloc(0, abs(game_state.clientWidth * game_state.clientHeight * BytesPerPixel), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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
	game_state.Bitmapmemory = VirtualAlloc(0, abs(game_state.clientWidth * game_state.clientHeight * BytesPerPixel), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	game_state.last_pixel = (uint32*)game_state.Bitmapmemory + abs(game_state.clientHeight * game_state.clientWidth) - 1;
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
			else {
				Pixel += BytesPerPixel;
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
				Pixel += BytesPerPixel;
			}
		}
	}
}

void DrawRectangle(uint32 step) {

	unsigned int* currentRow = (unsigned int*)game_state.Bitmapmemory;

	for (int i = 0; i < step; ++i) {
		unsigned int* Pixel = currentRow;
		for (int j = 0; j < step; ++j) {
			*Pixel++ = 40000;
		}
		currentRow = currentRow + game_state.clientWidth;
	}
}

void FillRectangleOnGrid(HWND window_handle, uint32 rect_color)
{
	POINT cursor_position;
	if (GetCursorPos(&cursor_position))
	{
		uint32 tile_width = 100;
		uint32 tile_index_x, tile_index_y = 0;
		int tile_width_x = 100;

		ScreenToClient(window_handle, &cursor_position);

		tile_index_x = cursor_position.x / tile_width;
		tile_index_y = cursor_position.y / tile_width;

		if (game_state.clientWidth - cursor_position.x <= 96) {
			tile_width_x = game_state.clientWidth - tile_index_x * tile_width;
		}

		uchar8 r, g, b = 0;
		uint32 color = 0;

		if (rect_color != game_state.default_color) {
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

			color |= b << 16 | g << 8 | r;
		}
		else color = game_state.default_color;

		uint32* Pixel = (uint32*)game_state.Bitmapmemory;
		Pixel += (tile_index_y * tile_width) * game_state.clientWidth + (tile_index_x * tile_width);

		for (int x = 0; x < tile_width; x++) {
			for (int y = 0; y < tile_width_x; y++) {
				if (Pixel > game_state.last_pixel) {
					break;
				}

				*Pixel++ = color;
			}
			Pixel += game_state.clientWidth - tile_width_x;
		}
	}
}

void SetDPI_Awareness(LPRECT lprect)
{
	lprect->left = 0;
	lprect->top = 0;
	lprect->right = 1400;
	lprect->bottom = 800;

	uint32 dpiY = 96, dpiX = 96;
	HRESULT res = GetDpiForMonitor(MonitorFromWindow(0, MONITOR_DEFAULTTOPRIMARY), MDT_RAW_DPI, &dpiY, &dpiX);

	if (dpiY > 96 || dpiX > 96)
	{
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
		AdjustWindowRectExForDpi(lprect, WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 0, dpiY > dpiX ? dpiY : dpiX);
	}
	else
	{
		AdjustWindowRectEx(lprect, WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 0);
	}
}

uchar8 SaveBitPatternToFile(HWND, void*)
{
	HANDLE file_handle = NULL;
	uint32 total_bytes_size = game_state.clientHeight * game_state.clientWidth * BytesPerPixel;
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
	uint32 total_bytes_size = game_state.clientHeight * game_state.clientWidth * BytesPerPixel;

	void* bytes_to_read = malloc(total_bytes_size);


	file_handle = CreateFile(L"save_file", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file_handle == INVALID_HANDLE_VALUE) {
		debugg_msg(L"У тебя пока нет сейвов");
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
		*Pixel++ = game_state.default_color;
	}
	return;
}
