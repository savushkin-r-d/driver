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


class EasySrv : public CServiceBase
{
public:

    EasySrv(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);
    virtual ~EasySrv(void);

protected:

    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    virtual void OnStop();

    void ServiceWorkerThread(void);

    //Поток взаимодействия с сервером.
    void server_communication_thread();

private:

    BOOL m_fStopping;
    HANDLE m_hStoppedEvent;

    bool is_server_communication_thread_init_complete;
    HANDLE server_cmmctr_stopped_event;
    
    HANDLE server_pipe;
    static const wchar_t *server_pipe_name; 
    static const int BUFSIZE_PIPE = 1024;
    static TCHAR request_buff[ BUFSIZE_PIPE ];
    static TCHAR reply_buff[ BUFSIZE_PIPE ];
};