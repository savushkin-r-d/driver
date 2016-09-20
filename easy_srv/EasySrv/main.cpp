/****************************** Module Header ******************************\
* Module Name:  CppWindowsService.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
*
* The file defines the entry point of the application. According to the
* arguments in the command line, the function installs or uninstalls or
* starts the service by calling into different routines.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region Includes
#include <stdio.h>
#include <windows.h>
#include "ServiceInstaller.h"
#include "ServiceBase.h"
#include "EasySrv.h"
#pragma endregion

#include "PAC_cmmctr.h"
#include "errors_manager.h"

// 
// Settings of the service
// 

// Internal name of the service
#define SERVICE_NAME             L"CppWindowsService"

// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"CppWindowsService Sample Service"

// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START

// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""

// The name of the account under which the service should run
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"

// The password to the service account name
#define SERVICE_PASSWORD         NULL

const int        MAX_PROJECTS_CNT = 256;
PAC_cmmctr_group *g_PAC_descriptions = 0;		///< Контроллеры сервера.

alarm_manager    *g_alarm_manager = 0;          ///< Работа с ошибками контроллеров.
alarm            *g_alarms[MAX_PROJECTS_CNT];   ///< Ошибки контроллеров.
u_int_2           g_alarms_id[MAX_PROJECTS_CNT];///< Ошибки контроллеров.

//-Данные для потоков, работающие с контроллерами.
bool   g_thread_is_terminated[MAX_PROJECTS_CNT] = { 0 };
HANDLE g_commctr_threads_array[MAX_PROJECTS_CNT + 1] = { 0 };
int    g_chbase_nodes_cont_count = 0;

/// @brief Синхронизатор доступа к PAC-ам.
CSWMRG g_sync_PAC;

/// @brief Количество потоков обмена с PAC.
int    g_commctr_threads_count = 0;
//-----------------------------------------------------------------------------
// Используется для проверки соответствия DLL и версии в PAC.
extern u_int_2 G_CURRENT_PROTOCOL_VERSION;

//
//  FUNCTION: wmain(int, wchar_t *[])
//
//  PURPOSE: entry point for the application.
// 
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    wmain() either performs the command line task, or run the service.
//
int wmain(int argc, wchar_t *argv[])
{
	if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
	{
		if (_wcsicmp(L"install", argv[1] + 1) == 0)
		{
			// Install the service when the command is 
			// "-install" or "/install".
			InstallService(
				SERVICE_NAME,               // Name of service
				SERVICE_DISPLAY_NAME,       // Name to display
				SERVICE_START_TYPE,         // Service start type
				SERVICE_DEPENDENCIES,       // Dependencies
				SERVICE_ACCOUNT,            // Service running account
				SERVICE_PASSWORD            // Password of the account
				);
		}
		else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
		{
			// Uninstall the service when the command is 
			// "-remove" or "/remove".
			UninstallService(SERVICE_NAME);
		}
	}
	else
	{
		wprintf(L"Parameters:\n");
		wprintf(L" -install  to install the service.\n");
		wprintf(L" -remove   to remove the service.\n");

		EasySrv service(SERVICE_NAME);
		if (!CServiceBase::Run(service))
		{
			wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
		}
	}

	return 0;
}