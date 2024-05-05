#pragma once
#include <Windows.h>
#include "mem.h"

template<size_t N>
class Hook {
private:
	const std::array<uint8_t, N> originalBytes;
	const void* ourFunc;
public:
	const uintptr_t address;
	const size_t size;

	Hook(uintptr_t address, void* ourFunc) : originalBytes(*reinterpret_cast<std::array<uint8_t, N>*>(address)),
		ourFunc(ourFunc), address(address), size(N) {}

	bool hook() {
		BYTE* destination{ (BYTE*)address };
		if (size < 5) return false;

		DWORD oldProtect{};
		if (!VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

		memset(destination, 0x90, size);

		DWORD relativeAddress = ((DWORD)ourFunc - (DWORD)address) - 5;

		*(BYTE*)destination = 0xE9;
		*(DWORD*)((DWORD)destination + 1) = relativeAddress;

		return VirtualProtect(destination, size, oldProtect, &oldProtect);
	}

	bool unhook() {
		return mem::Patch(address, originalBytes);
	}
};

