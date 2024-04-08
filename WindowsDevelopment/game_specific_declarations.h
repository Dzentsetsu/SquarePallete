#pragma once
#include "types.h"

// If any C programmer will ever read it, I am sorry for what you see below this comment... I can't be helped
#define private static 
#define UNSIGNED_CHAR_MAX_VALUE 255
#define UNSIGNED_CHAR_MIN_VALUE 0
#define BYTES_PER_PIXEL 4

struct global_applicatiton_state
{
	uchar8 gameIsRunning;
	void* Bitmapmemory;
	uint32* last_pixel;
	int32 clientWidth;
	int32 clientHeight;
	uint32 grid_step;
};