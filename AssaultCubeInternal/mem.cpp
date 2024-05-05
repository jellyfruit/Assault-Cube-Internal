#include "mem.h"

bool mem::Nop(uintptr_t address, unsigned int size) {
	BYTE* destination = (BYTE*)address;

	DWORD oldProtect;
	if (!VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

	memset(destination, 0x90, size);

	return VirtualProtect(destination, size, oldProtect, &oldProtect);
}