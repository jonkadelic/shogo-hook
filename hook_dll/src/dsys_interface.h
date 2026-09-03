#pragma once

#include <setjmp.h>
#include <windows.h>

typedef struct ClientGlob {
    BOOL m_bProcessWindowMessages;
    jmp_buf m_MemoryJump;
    HWND m_hMainWnd;
    HINSTANCE m_hInstance;
    char* m_WndClassName;
    char const* m_WndCaption;
    void* m_pClientMgr; /* CClientMgr* */
    BOOL m_bInitializingRenderer;
    BOOL m_bBreakOnError;
    BOOL m_bClientActive;
    BOOL m_bLostFocus;
    BOOL m_bAppClosing;
    BOOL m_bDialogUp;
    BOOL m_bRendererShutdown;
    BOOL m_bHost;
    char* m_pGameResources;
    char* m_pWorldName;
    char m_CachePath[500];
    DWORD m_KeyDowns[100];
    DWORD m_KeyUps[100];
    BOOL m_KeyDownReps[100];
    WORD m_nKeyDowns;
    WORD m_nKeyUps;
    BOOL m_bIsConsoleUp;
    BOOL m_bInputEnabled;
    char m_ExitMessage[500];
} ClientGlob;

static_assert(sizeof(ClientGlob) == 0x924);