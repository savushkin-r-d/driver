/// @file stdafx.h
/// @brief  Include file for standard system include files,
/// or project specific include files that are used frequently, but
/// are changed infrequently. For using precompiled headers.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// 
/// @par Текущая версия:
/// @$Rev$.\n
/// @$Author$.\n
/// @$Date::                     $.
#pragma once

#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <shellapi.h> 

// Additional headers program requires.
#include <string>

#include "CmnHdr.h"

#include "exchange_data.h"
#include "bug_log.h"
#include "PAC_cmmctr.h"

#include "errors_manager.h"
