#include <stdio.h>

#include <windows.h>
#include <tlhelp32.h>

bool inject_dll(HANDLE hProcess, const char* dll_path) {
    size_t path_len = strlen(dll_path) + 1;

    LPVOID remote_path = VirtualAllocEx(hProcess, nullptr, path_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_path) {
        printf("VirtualAllocEx failed: %lu\n", GetLastError());
        return false;
    }

    if (!WriteProcessMemory(hProcess, remote_path, dll_path, path_len, nullptr)) {
        printf("WriteProcessMemory failed: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, remote_path, 0, MEM_RELEASE);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPTHREAD_START_ROUTINE load_library_addr = (LPTHREAD_START_ROUTINE) GetProcAddress(hKernel32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, load_library_addr, remote_path, 0, nullptr);
    if (!hThread) {
        printf("CreateRemoteThread failed: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, remote_path, 0, MEM_RELEASE);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeThread(hThread, &exit_code);
    if (exit_code == 0) {
        printf("Remote LoadLibraryA returned NULL - DLL load failed\n");
    }

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remote_path, 0, MEM_RELEASE);

    return exit_code != 0;
}

int main(int argc, char** argv) {
    char const* client_exe_path = "C:\\Games\\Shogo\\Client.exe";
    char const* dll_path = "C:\\jonkwin\\shogo_hook.dll";
    char const* workingdir = "C:\\Games\\Shogo";

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "shogo_exe_path=", 15) == 0) {
            client_exe_path = &argv[i][15];
        }
        if (strncmp(argv[i], "dll_path=", 9) == 0) {
            dll_path = &argv[i][9];
        }
        if (strncmp(argv[i], "workingdir=", 11) == 0) {
            workingdir = &argv[i][11];
        }
    }

    printf("client_exe_path = %s\ndll_path = %s\nworkingdir = %s\n", client_exe_path, dll_path, workingdir);

    char cmdline[4096];
    sprintf_s(
        cmdline,
        sizeof(cmdline),
        "\"%s\" "
        "-workingdir %s "
        "-rez shogo.rez "
        "-rez shogop.rez "
        "-rez shogop2.rez "
        "-rez shogop3.rez "
        "-rez shogop4.rez "
        "-rez sound.rez ",
        client_exe_path,
        workingdir
    );

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    if (!CreateProcessA(client_exe_path, cmdline, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, "C:\\Games\\Shogo", &si, &pi)) {
        printf("CreateProcess failed: %lu\n", GetLastError());
        return 1;
    }

    if (!inject_dll(pi.hProcess, dll_path)) {
        printf("Injection failed; terminating suspended process\n");
        TerminateProcess(pi.hProcess, 1);
        return 1;
    }

    printf("DLL injected successfully, resuming Shogo\n");
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return 0;
}