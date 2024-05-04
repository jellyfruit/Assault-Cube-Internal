#include "mem.h"

bool mem::Hook(void* toHook, void* ourFunc, unsigned int size) {
	if (size < 5) return false;

	DWORD oldProtect;
	if (!VirtualProtect(toHook, size, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

	memset(toHook, 0x90, size);

	DWORD relativeAddress = ((DWORD)ourFunc - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress;

	return VirtualProtect(toHook, size, oldProtect, &oldProtect);
}

bool mem::Patch(uintptr_t address, BYTE* source, unsigned int size) {
	BYTE* destination = (BYTE*)address;

	DWORD oldProtect;
	if (!VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

	memcpy(destination, source, size);

	return VirtualProtect(destination, size, oldProtect, &oldProtect);
}

bool mem::Nop(uintptr_t address, unsigned int size) {
	BYTE* destination = (BYTE*)address;

	DWORD oldProtect;
	if (!VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

	memset(destination, 0x90, size);

	return VirtualProtect(destination, size, oldProtect, &oldProtect);
}