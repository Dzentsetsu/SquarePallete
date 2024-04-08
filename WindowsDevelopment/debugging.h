#pragma once

#include <stdio.h>

#define DEBUGG_MODE
#define debugg_msg(debuggMessage) MessageBoxExW(NULL, debuggMessage, L"Error message", 0x00000001L, 0);


