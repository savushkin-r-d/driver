// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

//#pragma comment(linker, "/export:Init=_Init@0")

#define EXPORT extern "C" __declspec (dllexport)
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
    {
    switch (ul_reason_for_call)
        {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
        }
    return TRUE;
    }
//-----------------------------------------------------------------------------
UCHAR   g_is_got_init = 0;
int     g_init_res = 0;
CRITICAL_SECTION init_cs;

//-ƒанные дл€ потоков, работающих с контроллерами.
char    *g_is_terminated = 0;
//-----------------------------------------------------------------------------
EXPORT int __cdecl init()
    {    
    g_is_got_init++;
    if ( g_is_got_init == 1 )
        {
        InitializeCriticalSection( &init_cs );
        EnterCriticalSection( &init_cs );

        try
            {
            g_is_terminated = new char;
            *g_is_terminated = 0;

            }
        catch (...)
            {
            g_init_res = 1;
            }

        LeaveCriticalSection( &init_cs );
        }
    else
        {
        EnterCriticalSection( &init_cs );
        LeaveCriticalSection( &init_cs );
        }

    return g_init_res;
    }
//-----------------------------------------------------------------------------
EXPORT double __cdecl get_value( in_tag_info &tag )
    {

    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
