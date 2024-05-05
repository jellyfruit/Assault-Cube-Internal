#include <Windows.h>
#include <iostream>
#include <thread>
#include <format>
#include <array>
#include "mem.h"
#include "hook.h"
#include "offsets.h"

uintptr_t localPlayer{};
uintptr_t godModeJump{};
uintptr_t infAmmoJump{};
uintptr_t infArmorJump{};
uintptr_t fastReloadJump{};
uintptr_t rapidFireJump{};

constexpr std::array<uint8_t, 5> noRecoilBytes = { 0xF3, 0x0F, 0x11, 0x56, 0x38 };

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

void __declspec(naked) fastReload() {
    __asm {
        push edi
        mov edi, [edi+0x08]
        cmp edi, localPlayer
        pop edi
        je equal

        notequal:
        mov [ecx], eax

        equal:
        mov eax, [edi+0x10]

        jmp [fastReloadJump]
    }
}

void __declspec(naked) rapidFire() {
    __asm {
        push esi
        mov esi, [esi+0x08]
        cmp esi, localPlayer
        pop esi
        je equal

        notequal:
        mov eax, [esi+0x18]
        mov eax, [ecx]

        equal:
        mov eax, [esi+0x18]

        jmp [rapidFireJump]
    }
}

struct Vector3 {
    float x, y, z;
};

struct Vector2 {
    float x, y;
};

void displayUI(bool bGodMode, bool bInfAmmo, bool bInfArmor, bool bNoRecoil, bool bFastReload, bool bRapidFire, Vector3 savedPos) {
    system("cls");
    std::cout << std::format("Jelly's Internal Trainer v1.1\n\n"
        "God Mode       [NUMPAD1] [{}]\n"
        "Infinite Ammo  [NUMPAD2] [{}]\n"
        "Infinite Armor [NUMPAD3] [{}]\n"
        "No Recoil      [NUMPAD4] [{}]\n"
        "Fast Reload    [NUMPAD5] [{}]\n"
        "Rapid Fire     [NUMPAD6] [{}]\n"
        "Please note rapid fire only removes the firing delay; it does not make weapons automatic.\n\n"

        "Save Position  [NUMPAD7]\n"
        "Teleport       [NUMPAD8] [{}, {}, {}]\n\n"
        "Uninject       [INSERT]\n", 
        bGodMode, bInfAmmo, bInfArmor, bNoRecoil, bFastReload, bRapidFire,
        (int)savedPos.x, (int)savedPos.z, (int)savedPos.y);
}

uintptr_t WINAPI HackThread(HMODULE hModule) {
    AllocConsole();
    FILE* file;
    freopen_s(&file, "CONOUT$", "w", stdout);

    uintptr_t moduleBase{ (uintptr_t)GetModuleHandle(L"ac_client.exe") };
    localPlayer = *(uintptr_t*)(moduleBase + offsets::localPlayer);

    bool bGodMode{ false }, bInfAmmo{ false }, bInfArmor{ false }, bNoRecoil{ false }, bFastReload{ false }, bRapidFire{ false };

    Hook<5> godModeHook{ moduleBase + offsets::godMode, godMode };
    Hook<5> infAmmoHook{ moduleBase + offsets::infAmmo, infAmmo };
    Hook<5> infArmorHook{ moduleBase + offsets::infArmor, infArmor };
    Hook<5> fastReloadHook{ moduleBase + offsets::fastReload, fastReload };
    Hook<5> rapidFireHook{ moduleBase + offsets::rapidFire, rapidFire };
    uintptr_t noRecoilHook{ moduleBase + offsets::noRecoil };

    godModeJump = godModeHook.address + godModeHook.size;
    infAmmoJump = infAmmoHook.address + infAmmoHook.size;
    infArmorJump = infArmorHook.address + infArmorHook.size;
    fastReloadJump = fastReloadHook.address + fastReloadHook.size;
    rapidFireJump = rapidFireHook.address + rapidFireHook.size;

    uintptr_t positionPtr{ localPlayer + offsets::position };
    Vector3 savedPos{};
    uintptr_t viewAnglesPtr{ localPlayer + offsets::viewAngles };
    Vector2 savedViewAngles{};
    uintptr_t armorPtr{ localPlayer + offsets::armor };
    int savedArmor{};

    std::cout << std::boolalpha;
    displayUI(bGodMode, bInfAmmo, bInfArmor, bNoRecoil, bFastReload, bRapidFire, savedPos);
    while (!GetAsyncKeyState(VK_INSERT)) {
        bool updateUI{ false };

        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            updateUI = true;
            bGodMode = !bGodMode;
            if (bGodMode)
                godModeHook.hook();
            else
                godModeHook.unhook();
        }
        if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
            updateUI = true;
            bInfAmmo = !bInfAmmo;
            if (bInfAmmo)
                infAmmoHook.hook();
            else
                infAmmoHook.unhook();
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
            updateUI = true;
            bInfArmor = !bInfArmor;
            if (bInfArmor) {
                infArmorHook.hook();
                savedArmor = *reinterpret_cast<int*>(armorPtr);
                *reinterpret_cast<int*>(armorPtr) = 100;
            }
            else {
                infArmorHook.unhook();
                *reinterpret_cast<int*>(armorPtr) = savedArmor;
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
            updateUI = true;
            bNoRecoil = !bNoRecoil;
            if (bNoRecoil)
                mem::Nop(noRecoilHook, 5);
            else
                mem::Patch(noRecoilHook, noRecoilBytes);
        }
        if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
            updateUI = true;
            bFastReload = !bFastReload;
            if (bFastReload)
                fastReloadHook.hook();
            else
                fastReloadHook.unhook();
        }
        if (GetAsyncKeyState(VK_NUMPAD6) & 1) {
            updateUI = true;
            bRapidFire = !bRapidFire;
            if (bRapidFire)
                rapidFireHook.hook();
            else
                rapidFireHook.unhook();
        }
        if (GetAsyncKeyState(VK_NUMPAD7) & 1) {
            updateUI = true;
            savedPos = *reinterpret_cast<Vector3*>(positionPtr);
            savedViewAngles = *reinterpret_cast<Vector2*>(viewAnglesPtr);
        }
        if (GetAsyncKeyState(VK_NUMPAD8) & 1) {
            *reinterpret_cast<Vector3*>(positionPtr) = savedPos;
            *reinterpret_cast<Vector2*>(viewAnglesPtr) = savedViewAngles;
        }


        if (updateUI)
            displayUI(bGodMode, bInfAmmo, bInfArmor, bNoRecoil, bFastReload, bRapidFire, savedPos);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }

    system("cls");

    *reinterpret_cast<int*>(armorPtr) = savedArmor;
    godModeHook.unhook();
    infAmmoHook.unhook();
    infArmorHook.unhook();
    fastReloadHook.unhook();
    mem::Patch(noRecoilHook, noRecoilBytes);

    std::cout << "Uninjected.";
    
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
        std::thread thread(HackThread, hModule);
        thread.detach();

        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

