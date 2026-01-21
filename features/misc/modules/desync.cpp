#include "../misc.h"
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <Psapi.h>
#include <thread>
#include <chrono>
#pragma comment(lib, "Psapi.lib")

using namespace roblox;

std::string findRobloxPath() {
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32;
    std::string exePath = "";

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return "";

    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return "";
    }

    do {
        std::wstring wExe = pe32.szExeFile;
        std::string exeName;
        exeName.assign(wExe.begin(), wExe.end());

        if (_stricmp(exeName.c_str(), "RobloxPlayerBeta.exe") == 0) {
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProc) {
                char buffer[MAX_PATH];
                if (GetModuleFileNameExA(hProc, NULL, buffer, MAX_PATH)) {
                    exePath = buffer;
                    CloseHandle(hProc);
                    break;
                }
                CloseHandle(hProc);
            }
        }
    } while (Process32NextW(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return exePath;
}

void blockRoblox() {
    if (!globals::misc::roblox_path.empty()) {
        std::string cmdOut = "netsh advfirewall firewall add rule name=\"" + globals::misc::firewall_rule_name + "_OUT\" dir=out program=\"" + globals::misc::roblox_path + "\" action=block enable=yes";
        system(cmdOut.c_str());
        std::cout << "[+] Started The Goon\n";
    }
}

void unblockRoblox() {
    std::string cmdDel = "netsh advfirewall firewall delete rule name=\"" + globals::misc::firewall_rule_name + "_OUT\"";
    system(cmdDel.c_str());
    std::cout << "[-] Stopped Gooning\n";
}

#include "../../../util/globals.h"
#include "../../../util/classes/classes.h"
#include "../../../util/driver/driver.h"
#include <chrono>
#include <thread>

namespace desync
{
    void Desync()
    {
        static bool bWasActive = false;
        static int32_t iOriginalBandwidth = -1;
        static auto tRestorationStart = std::chrono::steady_clock::now();
        static bool bIsRestoring = false;
        static bool bInitialized = false;
        while (true)
        {
            if (!globals::instances::localplayer.address)
            {
                if (bInitialized && iOriginalBandwidth != -1)
                {
                    write<int32_t>(base_address + 0x676BF14, iOriginalBandwidth);
                    iOriginalBandwidth = -1;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            globals::misc::desynckeybind.update();

            bool bCurrentlyActive = globals::misc::desync && globals::misc::desynckeybind.enabled;

            if (bCurrentlyActive && !bWasActive)
            {
                if (iOriginalBandwidth == -1)
                {
                    iOriginalBandwidth = read<int32_t>(base_address + 0x676BF14);
                    bInitialized = true;
                }
                write<int32_t>(base_address + 0x676BF14, 0);
                bIsRestoring = false;
                globals::misc::desync_active = true;
            }
            else if (!bCurrentlyActive && bWasActive)
            {
                if (iOriginalBandwidth != -1)
                {
                    tRestorationStart = std::chrono::steady_clock::now();
                    bIsRestoring = true;
                }
                globals::misc::desync_active = false;
            }
            else if (bCurrentlyActive)
            {
                write<int32_t>(base_address + 0x676BF14, 0);
            }

            if (bIsRestoring && iOriginalBandwidth != -1)
            {
                auto tCurrentTime = std::chrono::steady_clock::now();
                auto tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tCurrentTime - tRestorationStart);

                if (tElapsed.count() < 3000)
                {
                    write<int32_t>(base_address + 0x676BF14, iOriginalBandwidth);
                }
                else
                {
                    bIsRestoring = false;
                    iOriginalBandwidth = -1;
                }
            }

            bWasActive = bCurrentlyActive;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void hooks::desync() {
    desync::Desync();
}
