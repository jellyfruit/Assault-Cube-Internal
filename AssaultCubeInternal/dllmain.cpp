#include <Windows.h>
#include <iostream>
#include <thread>
#include <format>
#include "mem.h"
#include "offsets.h"

uintptr_t localPlayer{};
uintptr_t godModeJump{};
uintptr_t infAmmoJump{};
uintptr_t infArmorJump{};
uintptr_t noRecoilJump{};

void __declspec(naked) godMode() {
    __asm {
        sub ebx, 0xE8
        cmp ebx, localPlayer
        je equal

        notequal:
        add ebx, 0xE8
        sub [ebx+0x04], esi
        mov eax, esi

        equal:
        add ebx, 0xE8
        mov eax, esi

        jmp [godModeJump]
    }
}

void __declspec(naked) infAmmo() {
    __asm {
        mov eax, [esi+0x14]

        push esi
        mov esi, [esi+0x08]
        cmp esi, localPlayer
        pop esi
        je equal

        notequal:
        dec [eax]

        equal:
        jmp [infAmmoJump]
    }
}

void __declspec(naked) infArmor() {
    __asm {
        sub ebx, 0xE8
        cmp ebx, localPlayer
        je equal

        notequal:
        add ebx, 0xE8
        sub edx, eax
        mov [ebx+0x08], edx

        equal:
        add ebx, 0xE8
        sub edx, eax

        jmp [infArmorJump]
    }
}

void __declspec(naked) noRecoil() {
    __asm {
        cmp esi, localPlayer
        je equal
        
        movss[esi+38], xmm2

        equal:
        jmp [noRecoilJump]
    }
}

struct Vector3 {
    float x, y, z;
};

void displayUI(bool bGodMode, bool bInfAmmo, bool bInfArmor, bool bNoRecoil, Vector3 savedPos) {
    system("cls");
    std::cout << std::format("Jelly's Internal Trainer v1.0\n\n"
        "God Mode       [NUMPAD1] [{}]\n"
        "Infinite Ammo  [NUMPAD2] [{}]\n"
        "Infinite Armor [NUMPAD3] [{}]\n"
        "No Recoil      [NUMPAD4] [{}]\n\n"

        "Save Position  [NUMPAD5]\n"
        "Teleport       [NUMPAD6] [{}, {}, {}]\n\n"
        "Uninject       [INSERT]\n", 
        bGodMode, bInfAmmo, bInfArmor, bNoRecoil, 
        (int)savedPos.x, (int)savedPos.z, (int)savedPos.y);
}

uintptr_t WINAPI HackThread(HMODULE hModule) {
    AllocConsole();
    FILE* file;
    freopen_s(&file, "CONOUT$", "w", stdout);

    uintptr_t moduleBase{ (uintptr_t)GetModuleHandle(L"ac_client.exe") };
    localPlayer = *(uintptr_t*)(moduleBase + 0x18AC00);

    bool bGodMode{ false }, bInfAmmo{ false }, bInfArmor{ false }, bNoRecoil{ false };

    int hookSize = 5;
    uintptr_t godModeHook{ moduleBase + offsets::godMode }; godModeJump = godModeHook + hookSize;
    uintptr_t infAmmoHook{ moduleBase + offsets::infAmmo }; infAmmoJump = infAmmoHook + hookSize;
    uintptr_t infArmorHook{ moduleBase + offsets::infArmor }; infArmorJump = infArmorHook + hookSize;
    uintptr_t noRecoilHook{ moduleBase + offsets::noRecoil }; noRecoilJump = noRecoilHook + hookSize;

    uintptr_t positionPtr{ localPlayer + 0x28 };
    Vector3 savedPos{};
    uintptr_t armorPtr{ localPlayer + 0xF0 };
    int savedArmor{};

    std::cout << std::boolalpha;
    displayUI(bGodMode, bInfAmmo, bInfArmor, bNoRecoil, savedPos);
    while (true) {
        bool updateUI{ false };

        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            updateUI = true;
            bGodMode = !bGodMode;
            if (bGodMode)
                mem::Hook((void*)godModeHook, godMode, hookSize);
            else
                mem::Patch(godModeHook, (BYTE*)"\x29\x73\x04\x8B\xC6", hookSize);
        }
        if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
            updateUI = true;
            bInfAmmo = !bInfAmmo;
            if (bInfAmmo)
                mem::Hook((void*)infAmmoHook, infAmmo, hookSize);
            else
                mem::Patch(infAmmoHook, (BYTE*)"\x8B\x46\x14\xFF\x08", hookSize);
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
            updateUI = true;
            bInfArmor = !bInfArmor;
            if (bInfArmor) {
                mem::Hook((void*)infArmorHook, infArmor, hookSize);
                savedArmor = *reinterpret_cast<int*>(armorPtr);
                *reinterpret_cast<int*>(armorPtr) = 100;
            }
            else {
                mem::Patch(infArmorHook, (BYTE*)"\x2B\xD0\x89\x53\x08", hookSize);
                *reinterpret_cast<int*>(armorPtr) = savedArmor;
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
            updateUI = true;
            bNoRecoil = !bNoRecoil;
            if (bNoRecoil)
                mem::Hook((void*)noRecoilHook, noRecoil, hookSize);
            else
                mem::Patch(noRecoilHook, (BYTE*)"\xF3\x0F\x11\x56\x38", hookSize);
        }
        if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
            updateUI = true;
            savedPos = *reinterpret_cast<Vector3*>(positionPtr);
        }
        if (GetAsyncKeyState(VK_NUMPAD6) & 1) {
            *reinterpret_cast<Vector3*>(positionPtr) = savedPos;
        }
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            system("cls");

            *reinterpret_cast<int*>(armorPtr) = savedArmor;
            mem::Patch(godModeHook, (BYTE*)"\x29\x73\x04\x8B\xC6", hookSize);
            mem::Patch(infAmmoHook, (BYTE*)"\x8B\x46\x14\xFF\x08", hookSize);
            mem::Patch(infArmorHook, (BYTE*)"\x2B\xD0\x89\x53\x08", hookSize);
            mem::Patch(noRecoilHook, (BYTE*)"\xF3\x0F\x11\x56\x38", hookSize);

            std::cout << "Uninjected.";
            break;
        }


        if (updateUI)
            displayUI(bGodMode, bInfAmmo, bInfArmor, bNoRecoil, savedPos);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       uintptr_t  ul_reason_for_call,
                       LPVOID lpReserved )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr);

        if (thread)
            CloseHandle(thread);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

