#pragma once
#include <array>
#include <Windows.h>

namespace mem {
	template<size_t N>
	bool Patch(uintptr_t address, const std::array<uint8_t, N> &bytes) {
		BYTE* destination = (BYTE*)address;

		DWORD oldProtect;
		if (!VirtualProtect(destination, N, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;

		memcpy(destination, bytes.data(), N);

		return VirtualProtect(destination, N, oldProtect, &oldProtect);
	}

	bool Nop(uintptr_t address, unsigned int size);
}