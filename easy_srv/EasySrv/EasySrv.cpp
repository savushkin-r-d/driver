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

#include "SWMRG.h"
#include "CmnHdr.h"

#if _MSC_VER == 1700
#define snprintf _snprintf
#endif // _MSC_VER

extern PAC_cmmctr_group *g_PAC_descriptions;

extern alarm_manager *g_alarm_manager; ///< Работа с ошибками контроллеров.

const wchar_t *EasySrv::server_pipe_name = TEXT( "\\\\.\\pipe\\EasySrvPipe" );
char EasySrv::request_buff[ EasySrv::BUFSIZE_PIPE ];
char EasySrv::reply_buff[ EasySrv::BUFSIZE_PIPE ];

double EasySrv::tag_val;    
char EasySrv::str_tag_val[ 500 ];

BOOL EasySrv::m_fStopping = FALSE;
HANDLE EasySrv::server_pipe;
HANDLE EasySrv::server_cmmctr_stopped_event;

//-Данные для потоков, работающие с контроллерами.
bool   g_thread_is_terminated[ MAX_PROJECTS_CNT ]       = { 0 };
HANDLE g_commctr_threads_array[ MAX_PROJECTS_CNT + 1 ]  = { 0 };
int    g_chbase_nodes_cont_count                        = 0;

/// @brief Синхронизатор доступа к PAC-ам.
CSWMRG g_sync_PAC;

/// @brief Количество потоков обмена с PAC.
int g_commctr_threads_count = 0;

PAC_cmmctr_group *g_PAC_descriptions = 0;		///< Контроллеры сервера.

///< Работа с ошибками контроллеров.
extern PAC_cmmctr_group *g_PAC_descriptions;  
extern alarm  g_alarms[ MAX_PROJECTS_CNT ][ MAX_ALARMS_CNT ];


//-----------------------------------------------------------------------------
uintptr_t WINAPI PAC_communication_thread( LPVOID lpParameter )
    {		        
    PAC_cmmctr *PAC_com = ( PAC_cmmctr* ) lpParameter;
    int res;

    // 1 - интервал опроса контроллера.
    int sleep_time = 210;                            
    if ( PAC_com->get_cmmctr()->get_timeout() > 2000 )
        {
        sleep_time *= 2;
        }
    if ( PAC_com->get_cmmctr()->get_timeout() > 4000 )
        {
        sleep_time *= 3;
        }
    bug_log::msg.Format( 
        _T( "Поток работы с описанием PAC [ $%X ] запущен. Интервал опроса - %d мсек." ),
        PAC_com->get_description_id(), sleep_time );
    BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );

    while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )
        {        
        res = PAC_com->get_PAC_info();//Получение информации от PAC.
        if ( res <= 0 )
            {
            Sleep( 2 * sleep_time );
            continue;
            }

        //Состояния устройств будут доступны после того, как мы получим 
        //всю необходимую информацию от контроллера.
        bug_log::msg.Format( 
            _T( "Устройства PAC изменились." ) );
        BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );

        PAC_com->get_dev_synch_access()->WaitToWrite();
        PAC_com->clear_tags(); // Очищаем все теги проекта.
        PAC_com->get_dev_synch_access()->Done();

        //Пытаемся получить все устройства контроллера.
        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   
            {            
            PAC_com->get_dev_synch_access()->WaitToWrite();
            res = PAC_com->get_PAC_devices();
            PAC_com->get_dev_synch_access()->Done();

            if ( PAC_cmmctr::LOAD_OK == res )
                {
                bug_log::msg.Format( 
                    _T( "Получены устройства PAC." ) );
                BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );                
                break;
                }  

            Sleep( 4 * sleep_time );
            }

        //Пытаемся получить состояния всех устройств контроллера.
        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   
            {            
            PAC_com->get_dev_synch_access()->WaitToWrite();
            res = PAC_com->get_PAC_all_devices_states();
            PAC_com->get_dev_synch_access()->Done();

            if ( PAC_cmmctr::PAC_DEVICES_CHANGING == res )     
                {   
                break;
                }

            ////Пытаемся получить параметры всех устройств контроллера.
            //int CRC = PAC_com->get_PAC_params_CRC();
            //if ( CRC >= 0 && CRC != PAC_com->get_saved_CRC() ) 
            //    {                
            //    PAC_com->backup_PAC_params();
            //    PAC_com->set_saved_CRC( CRC );
            //    }

            //Получаем ошибки устройств и объектов.
            PAC_com->get_dev_synch_access()->WaitToWrite();
            PAC_com->get_PAC_errors();
            PAC_com->get_dev_synch_access()->Done();

            Sleep( 4 * sleep_time );
            } //  while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )           

        } // !g_thread_is_terminated[ PAC_com->get_description_id() ]

    _endthreadex( 0 );
    return 0;
    }
//-----------------------------------------------------------------------------
uintptr_t WINAPI PAC_control_thread( LPVOID lpParameter )
    {
    char server_PACs_connection_state[ MAX_PROJECTS_CNT ];
    memset( server_PACs_connection_state, 1, MAX_PROJECTS_CNT );

    while ( !g_thread_is_terminated[ 0 ] )   
        {
        g_sync_PAC.WaitToRead();
        if ( g_thread_is_terminated[ 0 ] )
            {            
            g_sync_PAC.Done();
            break;
            }

        //-Проверяем состояния контроллеров.
        for ( unsigned int i = 0; i < PAC_cmmctr_group::MAX_PAC_DESCR_NUMBER; i++ )
            {   
            PAC_cmmctr *PAC = g_PAC_descriptions->get_PAC( i );
            if ( 0 == PAC )
                {
                continue;
                }

            if ( PAC->get_connection_state() != 
                server_PACs_connection_state[ PAC->get_description_id() ] )
                {
                if ( 0 == PAC->get_connection_state() )
                    {
                    g_alarm_manager->add_no_PAC_connection_error( PAC->get_name(), 
                        PAC->get_description_id() );
                    }
                else
                    {
                    g_alarm_manager->remove_no_PAC_connection_error(
                        PAC->get_description_id() );
                    }

                server_PACs_connection_state[ PAC->get_description_id() ] = 
                    PAC->get_connection_state();
                }
            }

        g_sync_PAC.Done();
        Sleep( 1000 );
        }

    _endthreadex( 0 );  
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
EasySrv::EasySrv(PWSTR pszServiceName, 
                 BOOL fCanStop, 
                 BOOL fCanShutdown, 
                 BOOL fCanPauseContinue)
                 : CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue),
                 is_server_communication_thread_init_complete( false )                 
    {
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    server_cmmctr_stopped_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (server_cmmctr_stopped_event == NULL)
        {
        throw GetLastError();
        }
    }
//-----------------------------------------------------------------------------
EasySrv::~EasySrv(void)
    {
    if (server_cmmctr_stopped_event)
        {
        CloseHandle(server_cmmctr_stopped_event);
        server_cmmctr_stopped_event = NULL;
        }
    }
//-----------------------------------------------------------------------------
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

    //Запускаем поток для работы с запросами от сервера.
    SECURITY_ATTRIBUTES g_sa = {0};
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

    //Создаем поток, который будет следить, есть ли связь с 
    // контроллерами. В случае ее пропадания\появления 
    // устанавливать\сбрасывать соответствующую ошибку.
    g_commctr_threads_array[ 0 ] = 
        chBEGINTHREADEX( 0, 0, PAC_control_thread, 0, 0, 0 ); 

#ifdef DEBUG
    if ( dwArgc > 0 && _wcsicmp( L"debug", lpszArgv[ 1 ] + 1 ) == 0 )
        {
        return;
        }
#endif // DEBUG

    chBEGINTHREADEX( 0, 0, &EasySrv::server_communication_thread, 
        0, 0, 0 );	
    BUG_LOG.add_msg(  _T( "System" ),  _T( "" ), _T( "Сервис запущен." ) );
    }
//-----------------------------------------------------------------------------
uintptr_t WINAPI EasySrv::server_communication_thread( LPVOID lpParameter )
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
                        Sleep( 1 );                
                        break;

                    default:
                        Sleep( 1 );
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
                res = WaitForSingleObject( overlap.hEvent, 1000 );
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
                {
#ifdef DEBUG
                LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
                LARGE_INTEGER Frequency;

                QueryPerformanceFrequency(&Frequency); 
                QueryPerformanceCounter(&StartingTime);
#endif // DEBUG

                in_tag_info tag;
                GET_TAG_RES res_get_tag;
                TAG_VAL_TYPE tag_type;
                bool use_only_tag_id;

                int idx;
                int str_len;

                use_only_tag_id = false;
                idx = 1;

                switch (request_buff[ 0 ])
                    {
                    case SRV_CMD::GET_TAG_VALUE: //Get tag value

                        memcpy( &tag, &request_buff[ idx ], sizeof( tag ) );
                        idx += sizeof( tag );

                        str_len = strlen( &request_buff[ idx ] ) + 1;
                        tag.PAC_address = &request_buff[ idx ];
                        idx += str_len;

                        str_len = strlen( &request_buff[ idx ] ) + 1;
                        tag.PAC_name = &request_buff[ idx ];
                        idx += str_len;

                        str_len = strlen( &request_buff[ idx ] ) + 1;
                        tag.tag_name = &request_buff[ idx ];
                        idx += str_len;

                        tag_type = ( TAG_VAL_TYPE ) request_buff[ idx++ ];
                        use_only_tag_id = request_buff[ idx ] != 0;
                        idx++; 
                        break;

                    case SRV_CMD::GET_TAG_VALUE_BY_ID: //Get tag value by ID                        
                        tag_type = ( TAG_VAL_TYPE ) request_buff[ idx++ ];
                        tag.PAC_descr_id = request_buff[ idx++ ];

                        tag.tag_id = *( ( u_int* ) ( request_buff + idx ) );

                        use_only_tag_id = true;
                        break;

                    case SRV_CMD::SET_TAG_VALUE:
                        {
                        tag_type = ( TAG_VAL_TYPE ) request_buff[ idx++ ];
                        u_char PAC_descr_id = request_buff[ idx++ ];

                        str_len = strlen( &request_buff[ idx ] ) + 1;
                        char *tag_name = &request_buff[ idx ];
                        idx += str_len;

                        set_tag( tag_name, PAC_descr_id, request_buff + idx, tag_type );

                        reply_buff[ 0 ] = 0;
                        fSuccess = WriteFile( server_pipe, 
                            reply_buff, 1, &cbWritten, &overlap );  

                        state = READING;
                        continue;
                        }     

                    case SRV_CMD::GET_ALARMS:
                        {                    
                        u_char PAC_descr_id = request_buff[ idx++ ];

                        g_alarm_manager->sync_alarms( PAC_descr_id );
                        int size = g_alarm_manager->save_to_stream(
                            PAC_descr_id, reply_buff );
                                                
                        fSuccess = WriteFile( server_pipe,     
                            reply_buff, size, &cbWritten, &overlap );  

                        state = READING;
                        continue;
                        }

                    case SRV_CMD::SET_ALARMS:
                        {                        
                        u_char PAC_descr_id = request_buff[ idx++ ];
                        int err_cnt = *( ( int* ) ( request_buff + idx ) );
                        idx += sizeof( int );                         
                        error_cmd *errors = ( error_cmd* ) ( request_buff + idx );
                       
                        set_alarm_cmd( PAC_descr_id, err_cnt, errors );

                        reply_buff[ 0 ] = 0;
                        fSuccess = WriteFile( server_pipe, 
                            reply_buff, 1, &cbWritten, &overlap );  

                        state = READING;
                        continue;
                        }

                    default:
                        {                        
                        reply_buff[ 0 ] = 0;
                        WriteFile( 
                            server_pipe,       // handle to pipe 
                            reply_buff,        // buffer to write from 
                            1,                 // number of bytes to write 
                            &cbWritten,        // number of bytes written 
                            &overlap );  

                        state = READING;
                        continue;
                        }
                    }

                void* tag_val = get_tag_value( tag, tag_type, res_get_tag,
                    use_only_tag_id );
                
                reply_buff[ 0 ] = ( char ) res_get_tag;
                int size_to_write = 1;

                switch (tag_type)
                    {
                    case T_NUMBER:
                        // Write the reply to the pipe. 
                        size_to_write += sizeof( double );
                        memcpy( reply_buff + 1,
                            ( double* )tag_val, sizeof( double ) );

                        break;

                    case T_STRING:
                        size_to_write += strlen( ( char* ) tag_val ) + 1;
                        memcpy( reply_buff + 1,
                            tag_val, strlen( ( char* ) tag_val ) );                                
                        break;

                    default:
                        break;
                    }

                fSuccess = WriteFile( 
                    server_pipe,       // handle to pipe 
                    reply_buff,        // buffer to write from 
                    size_to_write,     // number of bytes to write 
                    &cbWritten,        // number of bytes written 
                    &overlap );  

#ifdef DEBUG

                QueryPerformanceCounter(&EndingTime);
                ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - 
                    StartingTime.QuadPart;
                printf( "%u\n", ElapsedMicroseconds.QuadPart );
#endif // DEBUG

                //Успешно записали данные.
                if ( fSuccess && cbWritten != 0 )
                    {   
                    state = READING;
                    continue;
                    }

                res = GetLastError();

                //Операция еще не завершена.
                if ( !fSuccess && res == ERROR_IO_PENDING )
                    {   
                    state = READING;                    
                    continue;
                    }

                // An error occurred; disconnect from the client.
                state = DISCONNECT;

                break;
                }

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

    return 0;
    }

//-----------------------------------------------------------------------------
void* EasySrv::get_tag_value( in_tag_info &tag, TAG_VAL_TYPE tag_type,
                             GET_TAG_RES &res_get_tag,
                             bool use_only_tag_id )
    {
    // Проверяется, не превышает ли номер описания PAC максимальный (1). Далее
    // есть ли описание данного PAC (2). Если нет, то тогда он добавляется в 
    // список контроллеров проекта (3) и создается поток, который
    // взаимодействует с контроллером (4).
    // Проверяется есть ли значение tag.tag_id в интерпретаторе Lua (5),
    // Если есть, тогда возвращается значение тега (6), иначе проверяется 
    // есть ли переменная tag.tag_name в в интерпретаторе Lua (7). После ее
    // нахождения добавляется новый тег в в интерпретатор (8), если же она 
    // не найдена, добавляется новый тег (9), который всегда возвращает 
    // значение 0.
    void* res = 0;

    tag_val          = 0;
    str_tag_val[ 0 ] = 0;    
    
    switch ( tag_type )
        {
        case T_NUMBER:
            res = ( void* ) &tag_val;   
            break;

        case T_STRING:
            res = ( void* ) &str_tag_val;    
            break;
        }      

    if ( tag.PAC_descr_id > PAC_cmmctr_group::MAX_PAC_DESCR_NUMBER )       //1
        {
        bug_log::msg.Format( 
            L"Ошибка get_tag_value(...) - номер описания PAC %d превышает "
            L"допустимый %d!",
            tag.PAC_descr_id, PAC_cmmctr_group::MAX_PAC_DESCR_NUMBER );
        BUG_LOG.add_msg_once( "Driver", "" );

        res_get_tag = GET_TAG_RES::GT_ERR;
        return res;
        }

    PAC_cmmctr *current_PAC_cmmctr =
        g_PAC_descriptions->get_PAC( tag.PAC_descr_id );
    if ( 0 == current_PAC_cmmctr )                                         //2
        {
        if ( use_only_tag_id )
            {
            res_get_tag = GET_TAG_RES::GT_NEED_FUL_TAG_INFO;
            return res;
            }

        current_PAC_cmmctr = g_PAC_descriptions->add_PAC(                  //3
            tag.PAC_address,
            tag.PAC_name, tag.PAC_descr_id, 
            tag.PAC_port, tag.timeout );     

        if ( 0 == current_PAC_cmmctr )
            {
            bug_log::msg.Format( 
                _T( "get_tag_value(...) - ошибка добавления new_PAC_cmmctr = 0!" ) );
            BUG_LOG.add_msg_once( "Driver", "" );

            res_get_tag = GET_TAG_RES::GT_ERR;
            return res;
            }

        g_commctr_threads_array[ g_commctr_threads_count++ ] = 
            chBEGINTHREADEX( 0, 0, PAC_communication_thread, 
            current_PAC_cmmctr, 0, 0 );						               //4
        }

    //-Получены ли устройства контроллера.
    if ( current_PAC_cmmctr->is_got_PAC_devices() == 0 ) 
        {        
        res_get_tag = GET_TAG_RES::GT_OK;
        return res; //Не получены устройства PAC.
        }

    current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();              //5
    bool is_exist_tag = false;
    switch ( tag_type )                                                    
        {
        case T_NUMBER:
            tag_val = current_PAC_cmmctr->get_tag_value(                   
                tag.tag_id, is_exist_tag );
            break;

        case T_STRING:
            current_PAC_cmmctr->get_tag_str_value( tag.tag_id,
                is_exist_tag, str_tag_val, sizeof( str_tag_val ) );
            break;
        }    
    current_PAC_cmmctr->get_dev_synch_access()->Done();

    if ( false == is_exist_tag )                                           //7
        {
        if ( use_only_tag_id ) 
            {
            res_get_tag = GET_TAG_RES::GT_NEED_FUL_TAG_INFO;
            return res;
            }

        current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();
        switch ( tag_type )
            {
            case T_NUMBER:
                tag_val = current_PAC_cmmctr->get_tag_value(                
                    tag.tag_name, is_exist_tag );
                break;

            case T_STRING:
                current_PAC_cmmctr->get_tag_str_value( tag.tag_name,
                    is_exist_tag, str_tag_val, sizeof( str_tag_val ) );
                break;
            }
        current_PAC_cmmctr->get_dev_synch_access()->Done();

        if ( true == is_exist_tag )                                        //8
            {
            current_PAC_cmmctr->add_exist_tag( tag.tag_name, tag.tag_id );
            res_get_tag = GET_TAG_RES::GT_OK;
            }

        if ( false == is_exist_tag )                                       //9
            {
            wchar_t tmp[ 50 ];
            mbstowcs( tmp, tag.tag_name, sizeof( tmp ) );

            bug_log::msg.Format( _T( "Тег \"%s\" не найден!" ), tmp );
            BUG_LOG.add_msg_once( current_PAC_cmmctr->get_name(), 
                current_PAC_cmmctr->get_address() );

            current_PAC_cmmctr->add_nill_tag( tag.tag_id );
            res_get_tag = GET_TAG_RES::GT_NO_TAG_FOUND;
            }
        }
    else
        {
        res_get_tag = GET_TAG_RES::GT_OK;
        }

    return res;
    }

//-----------------------------------------------------------------------------
int EasySrv::set_tag( const char *tag_name, u_char PAC_description_id,
                     void *value, TAG_VAL_TYPE tag_type )
    {
    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( PAC_description_id );
    if ( current_PAC_cmmctr )
        {	
        char cmd[ 1000 ];

        switch ( tag_type )
            {
            case T_NUMBER:
                sprintf( cmd, "res = make_lua_str( \"%s\", %f )", 
                    tag_name, *( double* ) value  );
                break;

            case T_STRING:
                snprintf( cmd, sizeof( cmd ), "res = make_lua_str( \"%s\", \"%s\" )", 
                    tag_name, ( char* ) value );
                break;
            }      

        current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();
        current_PAC_cmmctr->set_tag_Lua_cmd( cmd );
        current_PAC_cmmctr->get_dev_synch_access()->Done();
        }

    return 0;
    }
//-----------------------------------------------------------------------------
int EasySrv::set_alarm_cmd( u_char PAC_id, int count, error_cmd *errors )
    {   
    if ( g_PAC_descriptions->get_PAC( PAC_id ) != 0 )
        {
        std::string Lua_str = " ";
        Lua_str[ 0 ] = 104;

        for ( int i = 0; i < count; i++ )
            {
            char tmp_str[ 200 ];

            snprintf( tmp_str, sizeof( tmp_str ),
                "errors_manager:get_instance():set_cmd( %d, %d, %d, %d )\n",
                errors[ i ].cmd, 
                errors[ i ].object_type,
                errors[ i ].object_number,
                errors[ i ].object_alarm_number );

            Lua_str += tmp_str;         
            }

        const int SERVICE_ID = 1;

        g_PAC_descriptions->get_PAC( PAC_id )->get_cmmctr()->send_2_PAC( SERVICE_ID, 
            Lua_str.c_str(), Lua_str.length() );

        g_PAC_descriptions->get_PAC( PAC_id )->get_dev_synch_access()->WaitToWrite();
        g_PAC_descriptions->get_PAC( PAC_id )->get_PAC_errors();
        g_PAC_descriptions->get_PAC( PAC_id )->get_dev_synch_access()->Done();
        }        

    return 0;
    }
//----------------------------------------------------------------------------
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

    if (WaitForSingleObject(server_cmmctr_stopped_event, INFINITE) != WAIT_OBJECT_0)
        {
        throw GetLastError();
        }

    //-Завершение всех потоков, работающих с контроллерами.
    memset( g_thread_is_terminated, 1, sizeof( g_thread_is_terminated ) );
    Sleep( 1 );

    const int MAX_THREAD_END_WAIT_TIME = 15000;
    for ( int i = 0; i < MAX_PROJECTS_CNT + 1; i++ )
        {
        if (  g_commctr_threads_array[ i ] )
            {
            WaitForSingleObject( g_commctr_threads_array[ i ],
                MAX_THREAD_END_WAIT_TIME );
            CloseHandle( g_commctr_threads_array[ i ] );
            g_commctr_threads_array[ i ] = 0;
            }
        }
    Sleep( 1 );

    delete g_PAC_descriptions;
    g_PAC_descriptions = 0;

    delete g_alarm_manager;
    g_alarm_manager = 0;
    }