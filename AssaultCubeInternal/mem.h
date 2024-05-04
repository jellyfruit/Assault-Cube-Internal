#pragma once
#include <Windows.h>

namespace mem {
	bool Hook(void* toHook, void* ourFunc, unsigned int size);
	bool Patch(uintptr_t address, BYTE* source, unsigned int size);
	bool Nop(uintptr_t address, unsigned int size);
}