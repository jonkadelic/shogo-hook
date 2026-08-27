#include <stdio.h>

#include <windows.h>
#include <MinHook.h>

#include "dsys_interface.h"
#include "hook.h"

typedef struct CommandLineArgs {
    char** argv;
    UINT32 argc;
} CommandLineArgs;
static_assert(sizeof(CommandLineArgs) == 0x8);

typedef UINT32 DRESULT;
#define DE_OK 0

static void InitHooks(void);
static void ShutdownHooks(void);

typedef WPARAM (__cdecl *RunClientApp_t)(HINSTANCE hInstance, CommandLineArgs* pArgs);
static uintptr_t const addr_RunClientApp = 0x000031b0;
static WPARAM __cdecl hook_RunClientApp(HINSTANCE hInstance, CommandLineArgs* pArgs);
static RunClientApp_t orig_RunClientApp = NULL;

typedef char* (__cdecl *FindArgDash_t)(char* name, int argc, char** argv);
static FindArgDash_t p_FindArgDash = (FindArgDash_t) 0x004030e0UL;

typedef int (__stdcall *dsi_Init_t)(void);
static dsi_Init_t p_dsi_Init = (dsi_Init_t) 0x004203f0;

typedef void (__stdcall *dsi_Term_t)(void);
static dsi_Term_t p_dsi_Term = (dsi_Term_t) 0x00420460;

typedef void* (__stdcall *cm_Init_t)(void); // real return is CClientMgr*
static cm_Init_t p_cm_Init = (cm_Init_t) 0x0040ec40;

typedef LRESULT (__stdcall *MainWndProc_t)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static MainWndProc_t p_MainWndProc = (MainWndProc_t) 0x004036a0;

typedef BOOL (__stdcall *StartClient_t)(ClientGlob* pGlob, CommandLineArgs* pArgs);
static StartClient_t p_StartClient = (StartClient_t) 0x00403530;

typedef DRESULT (__thiscall *CClientMgr__Update_t)(void* this);
static CClientMgr__Update_t p_CClientMgr__Update = (CClientMgr__Update_t) 0x0040e790;

typedef void (__cdecl *dsi_PrintToConsole_t)(char* format, ...);
static dsi_PrintToConsole_t p_dsi_PrintToConsole = (dsi_PrintToConsole_t) 0x00421240;

typedef DRESULT (__cdecl *r_TermRender_t)(void* pClientMgr, int surfaceHandling);
static r_TermRender_t p_r_TermRender = (r_TermRender_t) 0x00442910;

typedef void (__cdecl *dsi_MessageBox_t)(char const* pMessage, char const* pTitle);
static dsi_MessageBox_t p_dsi_MessageBox = (dsi_MessageBox_t) 0x004212e0;

typedef DRESULT (__thiscall *CClientMgr__Term_t)(void* this);
static CClientMgr__Term_t p_CClientMgr__Term = (CClientMgr__Term_t) 0x0040efe0;

static ClientGlob* g_ClientGlob = (ClientGlob*) 0x00489d20;
static INT32* g_bDebugStructBanks = (INT32*) 0x0048cb38;
static BOOL* g_CV_HighPriority = (BOOL*) 0x0048b220;
static BOOL* g_bShowRunningTime = (BOOL*) 0x0048b218;
static UINT32* g_EngineStartMS = (UINT32*) 0x0048a644;
static INT32* g_CV_CursorCenter = (INT32*) 0x0048b21c;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            AllocConsole();
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
            printf("Patch DLL loaded into Shogo, PID=%lu\n", GetCurrentProcessId());
            InitHooks();
        } break;
        case DLL_PROCESS_DETACH: {
            ShutdownHooks();
        } break;
    }

    return TRUE;
}

static void InitHooks(void) {
    char buffer[1024];
    GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    HMODULE hMod = GetModuleHandleA(NULL); // or the specific module name
    printf("%s base address: 0x%p\n", buffer, hMod);
    HOOK_OFFSET = (uintptr_t) hMod;

    if (MH_Initialize() != MH_OK) {
        printf("MinHook init failed\n");
        return;
    }

    if (!install_hook(addr_RunClientApp, hook_RunClientApp, (void**) &orig_RunClientApp, "RunClientApp")) {
        return;
    }

    printf("MinHook initialized.\n");
}

static void ShutdownHooks(void) {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

static WPARAM __cdecl hook_RunClientApp(HINSTANCE hInstance, CommandLineArgs* pArgs) {
    MSG msg;
    WNDCLASS wndclass;
    ClientGlob* pGlob;
    char const* pArg;
    RECT screenRect, wndRect;
    int status, nExitValue;
    BOOL bOutOfMemory, bPrevHighPriority;

    nExitValue = 0;

    pGlob = g_ClientGlob;

    // Init the globals.
    memset(g_ClientGlob, 0, sizeof(ClientGlob));

    if (p_FindArgDash("DebugStructBanks", pArgs->argc, pArgs->argv) != NULL) {
        *g_bDebugStructBanks = 1;
    }

    // Set the jump-to position for memory errors...
    bOutOfMemory = FALSE;
    if (setjmp(g_ClientGlob->m_MemoryJump) != 0) {
        bOutOfMemory = TRUE;
        goto END_MAINLOOP;
    }

    // Set the working directory.
    pArg = p_FindArgDash("workingdir", pArgs->argc, pArgs->argv);
    if (pArg) {
        SetCurrentDirectory(pArg);
    }

    pGlob->m_bInputEnabled = TRUE;

    // Initialize the system-dependent modules.
    status = p_dsi_Init();
    if (status != 0) {
        if (status == 1) {
            MessageBox(NULL, "Unable to load de_msg.dll", "Error", 0);
            p_dsi_Term();
            return -1;
        } else {
            MessageBox(NULL, "Unknown error initializing engine.", "Error", 0);
            p_dsi_Term();
            return 1;
        }
    }

    // Initialize the client.
    pGlob->m_hInstance = hInstance;
    pGlob->m_WndClassName = "LithTech";
    pGlob->m_pClientMgr = p_cm_Init();
    pGlob->m_bClientActive = 1;
    pArg = p_FindArgDash("windowtitle", pArgs->argc, pArgs->argv);
    pGlob->m_WndCaption = pArg;
    if (!pGlob->m_WndCaption) {
        pGlob->m_WndCaption = "LithTech";
    }
    pGlob->m_bBreakOnError = p_FindArgDash("breakonerror", pArgs->argc, pArgs->argv) != NULL;

    // Create the main window;
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC; //temp
    wndclass.lpfnWndProc = p_MainWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = pGlob->m_hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = pGlob->m_WndClassName;

    RegisterClass(&wndclass);

    GetWindowRect(GetDesktopWindow(), &screenRect);

    pGlob->m_hMainWnd = CreateWindow(
        pGlob->m_WndClassName,
        pGlob->m_WndCaption,
        WS_CAPTION,
        ((screenRect.right - screenRect.left) - 320) / 2,
        ((screenRect.bottom - screenRect.top) - 200) / 2,
        320,
        200,
        NULL,
        NULL,
        pGlob->m_hInstance,
        NULL
    );

    printf("Random printf in the middle of it all\n");

    if (p_StartClient(pGlob, pArgs)) {
        pGlob->m_bProcessWindowMessages = TRUE;

        ShowWindow(pGlob->m_hMainWnd, SW_SHOWNORMAL);
        UpdateWindow(pGlob->m_hMainWnd);

        for (;;) {
            if (p_CClientMgr__Update(pGlob->m_pClientMgr) != DE_OK) {
                goto END_MAINLOOP;
            }

            // Give our process high priority?
            if (*g_CV_HighPriority != bPrevHighPriority) {
                if (g_CV_HighPriority) {
                    p_dsi_PrintToConsole("Setting process to high priority");
                    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
                }
            }

            if (*g_bShowRunningTime != 0) {
                p_dsi_PrintToConsole("Running for %.1f seconds", (float)(timeGetTime() - *g_EngineStartMS) / 1000.0f);
            }

            // Center the mouse in the window.
            if (*g_CV_CursorCenter && !g_ClientGlob->m_bLostFocus) {
                GetWindowRect(pGlob->m_hMainWnd, &screenRect);
                SetCursorPos((screenRect.right - screenRect.left) / 2, (screenRect.bottom - screenRect.top) / 2);
            }

            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    nExitValue = msg.wParam;
                    goto END_MAINLOOP;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

END_MAINLOOP:

    // Check for error message to post...
    if (pGlob != NULL) {
        if (pGlob->m_ExitMessage[0] != '\0') {
            p_r_TermRender(pGlob->m_pClientMgr, 2);
            p_dsi_MessageBox(pGlob->m_ExitMessage, pGlob->m_WndCaption);
        }

        pGlob->m_bProcessWindowMessages = FALSE;

        if (pGlob->m_pClientMgr != NULL) {
            p_CClientMgr__Term(pGlob->m_pClientMgr);
            free(pGlob->m_pClientMgr);
        }
    }

    p_dsi_Term();

    if (bOutOfMemory) {
        ShowWindow(pGlob->m_hMainWnd, SW_HIDE);
        MessageBox(GetDesktopWindow(), "Out of memory", pGlob->m_WndCaption, MB_OK);
    }

    return nExitValue;
}