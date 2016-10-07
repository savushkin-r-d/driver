/****************************** Module Header ******************************\
* Module Name:  SampleService.h
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include "ServiceBase.h"

#include "common_src\exchange_data.h"
#include "drv_srv_communication.h"
#include "PAC-driver\errors.h"

class EasySrv : public CServiceBase
{
public:

    EasySrv(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);
    virtual ~EasySrv(void);
    
    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    virtual void OnStop();    

    //ѕоток взаимодействи€ с сервером.
    static uintptr_t WINAPI server_communication_thread( LPVOID lpParameter );

    protected:

    /// @brief ѕолучение значени€ тега на основе его полного описани€.
    ///
    /// ¬нутренн€€ функци€ библиотеки.
    ///
    /// @param [in]  tag             - полное описание тега.
    /// @param [in]  tag_type        - тип значени€ тега.
    /// @param [out] is_exist_tag    - найден ли тег.
    /// @param [in]  use_only_tag_id - использовать только номер тега.
    ///
    /// @return «начение тега.
    static void* get_tag_value( in_tag_info &tag, TAG_VAL_TYPE tag_type, 
        GET_TAG_RES &res, bool use_only_tag_id = false );

    /// @brief «апись в тег.
    ///
    /// @param [in] tag_name            - им€ тега.
    /// @param [in] PAC_description_id  - номер описани€ PAC.
    /// @param [in] value               - новое значение тега.
    /// @param [in] tag_type            - тип значени€ тега.
    ///
    /// @return 0 - ок.
    static int set_tag( const char *tag_name, u_char PAC_description_id,
        void *value,  TAG_VAL_TYPE tag_type );

    static int set_alarm_cmd( u_char PAC_id, int count, error_cmd *errors );

private:
    static BOOL m_fStopping;

    bool is_server_communication_thread_init_complete;
    static HANDLE server_cmmctr_stopped_event;
    
    static HANDLE server_pipe;
    static const wchar_t *server_pipe_name; 
    static const int BUFSIZE_PIPE = 1024;
    static char request_buff[ BUFSIZE_PIPE ];
    static char reply_buff[ BUFSIZE_PIPE ];

    static double tag_val;
    static char str_tag_val[ 500 ];
};