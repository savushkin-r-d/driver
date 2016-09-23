/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
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

#pragma region Includes
#include "EasySrv.h"
#include "ThreadPool.h"
#pragma endregion

#include "exchange_data.h"
#include "PAC_cmmctr.h"
#include "errors_manager.h"

extern PAC_cmmctr_group *g_PAC_descriptions;
extern alarm   *g_alarms[256];
extern u_int_2  g_alarms_id[256];

extern alarm_manager *g_alarm_manager; ///< Работа с ошибками контроллеров.

const wchar_t *EasySrv::server_pipe_name = TEXT("\\\\.\\pipe\\EasySrvPipe");
TCHAR EasySrv::request_buff[ EasySrv::BUFSIZE_PIPE ];
TCHAR EasySrv::reply_buff[ EasySrv::BUFSIZE_PIPE ];

EasySrv::EasySrv(PWSTR pszServiceName, 
                 BOOL fCanStop, 
                 BOOL fCanShutdown, 
                 BOOL fCanPauseContinue)
                 : CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue),
                 is_server_communication_thread_init_complete( false ),
                 server_pipe( INVALID_HANDLE_VALUE )
    {
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
        {
        throw GetLastError();
        }

    server_cmmctr_stopped_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (server_cmmctr_stopped_event == NULL)
        {
        throw GetLastError();
        }
    }


EasySrv::~EasySrv(void)
    {
    if (m_hStoppedEvent)
        {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
        }

    if (server_cmmctr_stopped_event)
        {
        CloseHandle(server_cmmctr_stopped_event);
        server_cmmctr_stopped_event = NULL;
        }
    }


//
//   FUNCTION: EasySrv::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//
void EasySrv::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
    {
    // Log a service start message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);

    g_PAC_descriptions = new PAC_cmmctr_group(); //Контроллеры сервера.
    g_alarm_manager = new alarm_manager();       //Работа с ошибками контроллеров.

    memset(g_alarms_id, 0, sizeof(g_alarms_id));
        
    //Запускаем поток для работы с запросами от сервера.
    static SECURITY_ATTRIBUTES g_sa = {0};
    g_sa.nLength = sizeof(g_sa);
    HGLOBAL g_hsa = GlobalAlloc (GHND,SECURITY_DESCRIPTOR_MIN_LENGTH);
    g_sa.lpSecurityDescriptor = GlobalLock(g_hsa);
    g_sa.bInheritHandle = TRUE;

    InitializeSecurityDescriptor (g_sa.lpSecurityDescriptor, 1);
    SetSecurityDescriptorDacl (g_sa.lpSecurityDescriptor, TRUE,NULL,FALSE);

    server_pipe = CreateNamedPipe( 
        server_pipe_name,         // pipe name 
        PIPE_ACCESS_DUPLEX |      // read/write access 
        FILE_FLAG_OVERLAPPED,     // overlapped mode 
        PIPE_TYPE_MESSAGE |       // message type pipe 
        PIPE_READMODE_MESSAGE |   // message-read mode 
        PIPE_WAIT,                // blocking mode 
        PIPE_UNLIMITED_INSTANCES, // max. instances  
        BUFSIZE_PIPE,             // output buffer size 
        BUFSIZE_PIPE,             // input buffer size 
        0,                        // client time-out 
        &g_sa);                   // default security attribute 

    if (server_pipe == INVALID_HANDLE_VALUE) 
        {
        throw GetLastError();        
        }

    // Queue the pipe service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&EasySrv::server_communication_thread, this);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&EasySrv::ServiceWorkerThread, this);
    }


//
//   FUNCTION: EasySrv::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void EasySrv::ServiceWorkerThread(void)
    {
    // Periodically check if the service is stopping.
    while (!m_fStopping)
        {
        // Perform main service function here...

        ::Sleep(2000);  // Simulate some lengthy operations.
        }

    // Signal the stopped event.
    SetEvent(m_hStoppedEvent);
    }

void EasySrv::server_communication_thread()
    {
    OVERLAPPED overlap;
    SecureZeroMemory(&overlap, sizeof(overlap));
    HANDLE event;
    event = CreateEvent( 
        NULL,    // default security attribute 
        TRUE,    // manual-reset event 
        FALSE,   // initial state = unsignaled 
        NULL);   // unnamed event object 
    overlap.hEvent = event;

    bool connected = false;
    DWORD cbBytesRead = 0, cbWritten = 0; 
    int res = 0;
    int fSuccess = 0;

    enum WORK_STATES 
        {
        DEBUG_ = 0,
        WAITING_SERVER = 1,
        READING,
        READING_WAITING,
        WRITING,
        DISCONNECT,
        };
    WORK_STATES state = WAITING_SERVER;

    while (!m_fStopping) 
        {
        switch ( state )
            {
            case WAITING_SERVER: //Ожидание подключения сервера.

                ConnectNamedPipe( server_pipe, &overlap);
                res = GetLastError();

                switch ( res )
                    {
                    case ERROR_PIPE_CONNECTED:
                        SecureZeroMemory(&overlap, sizeof(overlap));                
                        overlap.hEvent = event;

                        //Сервер подключился, работаем с запросами сервера.
                        state = READING;
                        break;

                    case ERROR_IO_PENDING:
                        Sleep(100);                
                        break;

                    default:
                        Sleep(100);
                        break;
                    }
                break;

            case READING: //Чтение запроса от сервера.
                fSuccess = ReadFile( 
                    server_pipe,                // handle to pipe 
                    request_buff,               // buffer to receive data 
                    BUFSIZE_PIPE*sizeof(TCHAR), // size of buffer 
                    &cbBytesRead,               // number of bytes read 
                    &overlap);                  // not overlapped I/O 

                //Успешно считали данные.
                if (fSuccess && cbBytesRead != 0)
                    {   
                    SecureZeroMemory(&overlap, sizeof(overlap));                
                    overlap.hEvent = event;

                    state = WRITING;
                    continue;
                    }

                res = GetLastError();
                //Операция еще не завершена.
                if (!fSuccess && res == ERROR_IO_PENDING)
                    {   
                    state = READING_WAITING;
                    continue;
                    }

                // An error occurred; disconnect from the client.
                state = DISCONNECT;
                break;

            case READING_WAITING:
                res = WaitForSingleObject( overlap.hEvent, 10000 );
                if ( res == WAIT_OBJECT_0 )
                	{
                    //SecureZeroMemory(&overlap, sizeof(overlap));                
                    //overlap.hEvent = event;

                    state = WRITING;
                	}
                else
                    {
                    res = GetLastError();
                    // An error occurred; disconnect from the client.
                    state = DISCONNECT;
                    }

                break;

            case WRITING:
                switch (request_buff[ 0 ])
                    {
                    case 1: //Get tag value
                        // Write the reply to the pipe. 
                        *((int*) reply_buff) = 10;

                        fSuccess = WriteFile( 
                            server_pipe,       // handle to pipe 
                            reply_buff,        // buffer to write from 
                            sizeof(int),       // number of bytes to write 
                            &cbWritten,        // number of bytes written 
                            &overlap );  

                        //Успешно считали данные.
                        if (fSuccess && cbWritten != 0)
                            {   
                            state = READING;
                            continue;
                            }

                        res = GetLastError();
                        //Операция еще не завершена.
                        if (!fSuccess && res == ERROR_IO_PENDING)
                            {   
                            Sleep(10);
                            continue;
                            }

                        // An error occurred; disconnect from the client.
                        state = DISCONNECT;
                        break;
                    }
                break;

            case DISCONNECT:
                FlushFileBuffers(server_pipe); 
                DisconnectNamedPipe(server_pipe);
                state = WAITING_SERVER;                
                break;
            }        
        }

    ::Sleep(1);

    FlushFileBuffers(server_pipe); 
    DisconnectNamedPipe(server_pipe);
    CloseHandle( server_pipe );
    server_pipe = 0;

    // Signal the stopped event.
    SetEvent(server_cmmctr_stopped_event);
    }

//
//   FUNCTION: EasySrv::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//
void EasySrv::OnStop()
    {
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
        {
        throw GetLastError();
        }

    if (WaitForSingleObject(server_cmmctr_stopped_event, INFINITE) != WAIT_OBJECT_0)
        {
        throw GetLastError();
        }

    delete g_PAC_descriptions;
    g_PAC_descriptions = 0;

    delete g_alarm_manager;
    g_alarm_manager = 0;
    }