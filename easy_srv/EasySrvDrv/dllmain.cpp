// dllmain.cpp : Defines the entry point for the DLL application.
#include "windows.h"


#include "bug_log.h"
#include "exchange_data.h"

#pragma comment(linker, "/export:get_alarms=_get_alarms@8")
#pragma comment(linker, "/export:set_alarm_cmd=_set_alarm_cmd@12")

#pragma comment(linker, "/export:get_str_value=_get_str_value@4")
#pragma comment(linker, "/export:get_str_value2=_get_str_value2@12")
#pragma comment(linker, "/export:set_str_value=_set_str_value@8")

#pragma comment(linker, "/export:get_value=_get_value@4")
#pragma comment(linker, "/export:get_value2=_get_value2@12")
#pragma comment(linker, "/export:set_value=_set_value@16")

#pragma comment(linker, "/export:init_driver_thread=_init_driver_thread@4")
#pragma comment(linker, "/export:stop_driver_thread=_stop_driver_thread@4")

#define EXPORT extern "C" __declspec (dllexport)

/// @brief Типы значения тега.
enum TAG_VAL_TYPE
    {
    T_NUMBER,///< Вещественное (float, 32 бита).
    T_STRING,///< Строка.
    };
//-----------------------------------------------------------------------------
int final();
int connect_to_srv();

bool g_connected = false;
//-----------------------------------------------------------------------------
HANDLE g_pipe;
OVERLAPPED g_overlap;
HANDLE g_event;

BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved )
    {
    HRESULT hRes;

    switch ( ul_reason_for_call )
        {
        case DLL_PROCESS_ATTACH:          
            try
                {
                BUG_LOG.get_instance();
                }
            catch (...)
                {
                return false;
                }

            hRes = _Module.Init( 0, ( HINSTANCE ) hModule );            // Инициализируем модуль.
            ATLASSERT( SUCCEEDED( hRes ) );
            
            SecureZeroMemory( &g_overlap, sizeof( g_overlap ) );
            g_event = CreateEvent( 
                NULL,    // default security attribute 
                TRUE,    // manual-reset event 
                FALSE,   // initial state = non signaled 
                NULL);   // unnamed event object 
            g_overlap.hEvent = g_event;

            break;

        case DLL_THREAD_ATTACH:  
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:  
            final();

            bug_log::free_instance();

            _Module.Term(); // Завершаем программу.
            //MessageBox( 0 , "Final", "Ok", 0 );
            break;
        }
    return TRUE;
    }
//-----------------------------------------------------------------------------
CString FormatErrorMessage(DWORD ErrorCode)
    {
    TCHAR   *pMsgBuf = NULL;
    DWORD   nMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&pMsgBuf), 0, NULL);
    if (!nMsgLen)
        return _T("FormatMessage fail");
    CString sMsg(pMsgBuf, nMsgLen);
    LocalFree(pMsgBuf);
    return sMsg;
    }
//-----------------------------------------------------------------------------
int connect_to_srv()
    {
    static CString err_str;
    static char is_srv_connect_err = 0;

    BOOL fSuccess; 
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\EasySrvPipe");

    g_pipe = CreateFile( 
        lpszPipename,                 // pipe name 
        GENERIC_READ | GENERIC_WRITE, // read and write access              
        0,                            // no sharing 
        NULL,                         // default security attributes
        OPEN_EXISTING,                // opens existing pipe 
        FILE_FLAG_OVERLAPPED,         // default attributes 
        NULL);                        // no template file 

    // Exit if the pipe handle is invalid. 
    if (g_pipe == INVALID_HANDLE_VALUE) 
        {
        if ( is_srv_connect_err == 0 )
            {
            err_str.Format( _T( "Нет подключения к сервису. %s" ),
                FormatErrorMessage( GetLastError() ) );     
            BUG_LOG.set_error( is_srv_connect_err, "Driver", "", err_str );
            }
        return 1; 
        }

    // The pipe connected; change to message-read mode. 
    DWORD dwMode = PIPE_READMODE_MESSAGE; 
    fSuccess = SetNamedPipeHandleState( 
        g_pipe,    // pipe handle 
        &dwMode,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL);    // don't set maximum time 
    
    // Exit if can't set the pipe state. 
    if (!fSuccess) 
        {
        if ( is_srv_connect_err == 0 )
            {
            err_str.Format( _T( "Нет подключения к сервису. %s" ),
                FormatErrorMessage( GetLastError() ) );      
            BUG_LOG.set_error( is_srv_connect_err, "Driver", "", err_str );
            }
        CloseHandle( g_pipe );
        g_pipe = 0;
        return 1;
        }

    if ( is_srv_connect_err == 1 )
        {
        BUG_LOG.reset_error( is_srv_connect_err, "Driver", "", err_str );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
void* transact_pipe( void* buff, int size )
    {
    BOOL fSuccess; 
    DWORD cbRead; 
    const int BUFSIZE = 512;
    static char chReadBuf[ BUFSIZE ];
    ZeroMemory( chReadBuf, sizeof( chReadBuf ) );

    // Send a message to the pipe server and read the response. 
    fSuccess = TransactNamedPipe( 
        g_pipe,                  // pipe handle 
        buff,                   // message to server
        size,                   // message length 
        chReadBuf,              // buffer to receive reply
        BUFSIZE*sizeof(TCHAR),  // size of read buffer
        &cbRead,                // bytes read
        &g_overlap);              // overlapped 

    if ( fSuccess )
        {
        return chReadBuf;
        }
    else
        {
        int err = GetLastError();
        if ( err == ERROR_IO_PENDING )
            {
            Sleep( 1 );
            fSuccess = GetOverlappedResult( g_pipe, &g_overlap, &cbRead, true );

            if ( fSuccess )
                {
                return chReadBuf;
                }
            }
        }

    bug_log::msg.Format( _T( "Нет ответа от сервиса. %s" ), 
        FormatErrorMessage( GetLastError() ) ); 
    BUG_LOG.add_warning_msg( "Driver", "" );

    CloseHandle( g_pipe );
    g_pipe = 0;
    g_connected = 0;

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его полного описания.
///
/// Внутренняя функция библиотеки.
///
/// @param [in] tag             - полное описание тега.
/// @param [in] tag_type        - тип значения тега.
/// @param [in] use_only_tag_id - использовать только номер тега.
///
/// @return Значение тега.
void* get_tag_value( in_tag_info &tag, TAG_VAL_TYPE tag_type, 
                    bool use_only_tag_id = false )
    {   
    static double tag_val            = 0;    
    static char   str_tag_val[ 500 ] = { 0 };    
    tag_val          = 0;
    str_tag_val[ 0 ] = 0;
    void* res = 0;

    switch ( tag_type )
        {
        case T_NUMBER:
            tag_val = 0;
            res = &tag_val;
            break;

        case T_STRING:
            sprintf( str_tag_val, "" );
            res = &str_tag_val;
            break;
        }

    if ( !g_connected )
    	{
        g_connected = connect_to_srv() == 0;
    	}

    if ( !g_connected )
        {
        return res;
        }

    TCHAR cmd_str[] = _T( "\1" );  
    void *res_buff = transact_pipe( cmd_str, sizeof( cmd_str ) );    
    if ( res_buff!= 0 )
    	{
        tag_val = *( ( int* )res_buff );
        res = &tag_val;
        return res;
    	}

    return res;
    }
//-----------------------------------------------------------------------------
enum SET_TAG_VAL_TYPE
    {
    T_ULONG,
    T_FLOAT,
    };
//-----------------------------------------------------------------------------
/// @brief Запись в тег.
///
/// Внутренняя функция библиотеки.
///
/// @param [in] tag_name            - имя тега.
/// @param [in] PAC_description_id  - номер описания PAC.
/// @param [in] value               - новое значение тега.
/// @param [in] tag_type            - тип значения тега.
///
/// @return 0 - ок.
int set_tag( const char *tag_name, UCHAR PAC_description_id, void *value, 
            TAG_VAL_TYPE tag_type )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
EXPORT int __stdcall init_driver_thread( int prj_id )
    {    
    if ( BUG_LOG.init_window_complete() )
        {         
        bug_log::msg.Format( 
            _T( "Драйвер для узла базы каналов [ $%X ] загружен." ), 
            prj_id );

        BUG_LOG.add_msg( "Driver", "" );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall stop_driver_thread( int prj_id )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
///
/// @return Значение тега.
EXPORT double __stdcall get_value( in_tag_info &tag )
    {
    void *res = get_tag_value( tag, T_NUMBER );

    if ( res )
        {
        return *( double* ) res;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его частичного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in]  tag_id - номер тега.
/// @param [in]  PAC_description_id - номер описания контроллера 
/// (узла базы каналов).
/// @param [out] result - признак удачной записи значения тега:
/// 1 - неудачно, 0 - ок.
///
/// @return Значение тега.
EXPORT double __stdcall get_value2( UINT tag_id, UCHAR PAC_description_id,
                                   UCHAR &result )
    {
    in_tag_info tag;
    tag.tag_id = tag_id;
    tag.PAC_descr_id = PAC_description_id;

    void *res = get_tag_value( tag, T_NUMBER, true );
    if ( res )
        {
        result = 0;
        return *( double* ) res;
        }

    result = 1;
    return 1;
    }
//-----------------------------------------------------------------------------
/// @brief Получение строкового значения тега на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
///
/// @return Значение тега.
EXPORT char* __stdcall get_str_value( in_tag_info &tag )
    {
    void *res = get_tag_value( tag, T_NUMBER );

    if ( res )
        {
        return ( char* ) res;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение строкового значения тега на основе его частичного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in]  tag_id - номер тега.
/// @param [in]  PAC_description_id - номер описания контроллера 
/// (узла базы каналов).
/// @param [out] result - признак удачной записи значения тега:
/// 1 - неудачно, 0 - ок.
///
/// @return Значение тега.
EXPORT char* __stdcall get_str_value2( UINT tag_id, UCHAR PAC_description_id,
                                      UCHAR &result )
    {
    in_tag_info tag;
    tag.tag_id = tag_id;
    tag.PAC_descr_id = PAC_description_id;

    void *res = get_tag_value( tag, T_STRING, true );
    if ( res )
        {
        result = 0;
        return ( char* ) res;
        }

    result = 1;
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Запись в тег на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
/// @param [in] value - записываемое в тег значение.
/// @param [in] type - тип значения тега.
///
/// @return Новое значение тега.
EXPORT int __stdcall set_value( in_tag_info &tag, double value, TAG_VAL_TYPE type )
    {
    return set_tag( tag.tag_name, tag.PAC_descr_id, &value, T_NUMBER );    
    }
//-----------------------------------------------------------------------------
/// @brief Запись в строковый тег на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
/// @param [in] str_value - записываемое в тег значение.
///
/// @return Признак успешной записи: 0 - ок, 1 - ошибка.
EXPORT int __stdcall set_str_value( in_tag_info &tag, char *str_value )
    {
    return set_tag( tag.tag_name, tag.PAC_descr_id, str_value, T_STRING );
    }
//-----------------------------------------------------------------------------
int final()
    {
    return 0;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall get_alarms( unsigned char PAC_id, all_alarm &alarms )
    {   
    return 0;        
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall set_alarm_cmd( unsigned char PAC_id, int count,
                                   error_cmd *errors )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
