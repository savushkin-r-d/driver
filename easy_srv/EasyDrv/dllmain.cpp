// dllmain.cpp : Defines the entry point for the DLL application.
#include "windows.h"


#include "bug_log.h"
#include "exchange_data.h"
#include "PAC-driver\errors.h"

#include "drv_srv_communication.h"

#include <ctime>

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

//-----------------------------------------------------------------------------
int final();
int connect_to_srv();

bool g_connected = false;
int g_chbase_nodes_cont_count = 0;

all_alarm   g_all_alarms[ MAX_PROJECTS_CNT ];
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


            for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
            	{
                g_all_alarms[ i ].alarms = new alarm[ MAX_ALARMS_CNT ];
                for ( int j = 0; j < MAX_ALARMS_CNT; j++ )
                    {
                    g_all_alarms[ i ].alarms[ j ].description = 
                        new char[ MAX_DESCR_LEN ];
                    g_all_alarms[ i ].alarms[ j ].group = 
                        new char[ MAX_GROUP_LEN ];
                    }
            	}
            
            break;

        case DLL_THREAD_ATTACH:  
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:  
            for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
                {                
                for ( int j = 0; j < MAX_ALARMS_CNT; j++ )
                    {
                    delete [] g_all_alarms[ i ].alarms[ j ].description;
                    delete [] g_all_alarms[ i ].alarms[ j ].group;

                    g_all_alarms[ i ].alarms[ j ].description = 0;
                    g_all_alarms[ i ].alarms[ j ].group = 0;
                    }

                delete [] g_all_alarms[ i ].alarms;
                g_all_alarms[ i ].alarms = 0;
                }

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
    if ( !g_connected )
        {
        g_connected = connect_to_srv() == 0;
        }

    if ( !g_connected )
        {
        return 0;
        }

    BOOL fSuccess; 
    DWORD cbRead; 
    const int BUFSIZE = 512;
    static char chReadBuf[ BUFSIZE ];

    // Send a message to the pipe server and read the response. 
    fSuccess = TransactNamedPipe( 
        g_pipe,                 // pipe handle 
        buff,                   // message to server
        size,                   // message length 
        chReadBuf,              // buffer to receive reply
        sizeof( chReadBuf ),    // size of read buffer
        &cbRead,                // bytes read
        &g_overlap);            // overlapped 

    if ( fSuccess )
        {
        return chReadBuf;
        }
    else
        {
        int err = GetLastError();
        if ( err == ERROR_IO_PENDING )
            {       
            err = WaitForSingleObject( g_overlap.hEvent, 1000 );
            fSuccess = GetOverlappedResult( g_pipe, &g_overlap, &cbRead, false );

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
                    GET_TAG_RES &res_get_tag,
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

    static char cmd_str[ 500 ];
    int data_size = 1;

    cmd_str[ 0 ] = SRV_CMD::GET_TAG_VALUE;
    memcpy( &cmd_str[ data_size ], &tag, sizeof( tag ) );
    data_size += sizeof( tag );
    
    strcpy( &cmd_str[ data_size ], tag.PAC_address );
    data_size += strlen( tag.PAC_address ) + 1;
    strcpy( &cmd_str[ data_size ], tag.PAC_name );
    data_size += strlen( tag.PAC_name ) + 1;
    strcpy( &cmd_str[ data_size ], tag.tag_name );
    data_size += strlen( tag.tag_name ) + 1;

    cmd_str[ data_size++ ] = tag_type;
    cmd_str[ data_size++ ] = use_only_tag_id;

    void *res_buff = transact_pipe( cmd_str, data_size );    
    if ( res_buff!= 0 )
    	{
        res_get_tag = ( GET_TAG_RES ) ( ( char* ) res_buff )[ 0 ];

        switch ( tag_type )
            {
            case T_NUMBER:
                tag_val = *( ( double* )( ( char* ) res_buff + 1 ) ) ;
                res = &tag_val;
                break;

            case T_STRING:
                strcpy( str_tag_val, ( ( char* ) res_buff + 1 ) );                
                res = &str_tag_val;
                break;
            } 
    	}

    return res;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его id.
///
/// Внутренняя функция библиотеки.
///
/// @param [in] tag             - полное описание тега.
/// @param [in] tag_type        - тип значения тега.
/// @param [in] use_only_tag_id - использовать только номер тега.
///
/// @return Значение тега.
void* get_tag_value_by_id( UINT tag_id, UCHAR PAC_description_id,
                          TAG_VAL_TYPE tag_type, 
                          GET_TAG_RES &res_get_tag )
    {   
#ifdef DEBUG
    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    LARGE_INTEGER Frequency;
    if ( tag_id == 0xe1000000 )
        {        
        QueryPerformanceFrequency(&Frequency); 
        QueryPerformanceCounter(&StartingTime);
        }
#endif // DEBUG

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

    static char cmd_str[ 500 ];
    
    int data_size = 0;
    cmd_str[ data_size++ ] = SRV_CMD::GET_TAG_VALUE_BY_ID;
    cmd_str[ data_size++ ] = tag_type;

    cmd_str[ data_size++ ] = PAC_description_id;    
    memcpy( &cmd_str[ data_size ], &tag_id, sizeof( tag_id ) );    
    data_size += sizeof( tag_id );

    void *res_buff = transact_pipe( cmd_str, data_size );    

    if ( res_buff!= 0 )
        {
        res_get_tag = ( GET_TAG_RES ) ( ( char* ) res_buff )[ 0 ];

        switch ( tag_type )
            {
            case T_NUMBER:
                tag_val = *( ( double* )( ( char* ) res_buff + 1 ) ) ;
                res = &tag_val;
                break;

            case T_STRING:
                strcpy( str_tag_val, ( ( char* ) res_buff + 1 ) );                
                res = &str_tag_val;
                break;
            } 
        }

#ifdef DEBUG
    if ( tag_id == 0xe1000000 )
        {
        QueryPerformanceCounter(&EndingTime);
        ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - 
            StartingTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

        bug_log::msg.Format( _T( "get_tag_value_by_id time = %d!" ),
            ElapsedMicroseconds.QuadPart );
        BUG_LOG.add_msg( _T( "Driver" ), _T( "" ) );
        }
#endif // DEBUG

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
    static char cmd_str[ 500 ];

    int data_size = 0;
    cmd_str[ data_size++ ] = SRV_CMD::SET_TAG_VALUE;
    cmd_str[ data_size++ ] = tag_type;

    cmd_str[ data_size++ ] = PAC_description_id;   
    strcpy( &cmd_str[ data_size ], tag_name );    
    data_size += strlen( tag_name ) + 1;

    switch ( tag_type )
        {
        case T_NUMBER:
            memcpy( &cmd_str[ data_size ], value, sizeof( double ) );    
            data_size += sizeof( double );
            break;

        case T_STRING:            
            strcpy( &cmd_str[ data_size ], ( char* ) value );                
            data_size += strlen( ( char* ) value ) + 1;
            break;
        } 

    transact_pipe( cmd_str, data_size );    
    
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
EXPORT int __stdcall init_driver_thread( int prj_id )
    {    
    if ( !BUG_LOG.init_window_complete() )
        {
        Sleep( 100 );

        if ( BUG_LOG.init_window_complete() )
            {         
            bug_log::msg.Format( 
                _T( "Драйвер для узла базы каналов [ $%X ] загружен." ), 
                prj_id );

            BUG_LOG.add_msg( "Driver", "" );
            }
        }

    g_chbase_nodes_cont_count++;
    return 0;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall stop_driver_thread( int prj_id )
    {
    bug_log::msg.Format( 
        _T( "Драйвер для узла базы каналов [ $%X ] выгружен." ), 
        prj_id );

    BUG_LOG.add_msg( "Driver", "" );

    g_chbase_nodes_cont_count--;
    if ( g_chbase_nodes_cont_count <= 0 )
    	{
        final();
        bug_log::free_instance();   
    	}     

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
    GET_TAG_RES res_get_tag;
    void *res = get_tag_value( tag, T_NUMBER, res_get_tag );

    if ( res_get_tag == GT_OK )
        {
        return *( double* ) res;
        }
    else
        {
        if ( res_get_tag == GT_NO_TAG_FOUND )
            {
            wchar_t tmp[ 50 ];
            mbstowcs( tmp, tag.tag_name, sizeof( tmp ) );
            bug_log::msg.Format( _T( "Тег \"%s\" не найден!" ), tmp );                
            BUG_LOG.add_msg_once( tag.PAC_name, tag.PAC_address );
            }
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
#ifdef DEBUG
    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    LARGE_INTEGER Frequency;

    if ( tag_id == 0xe1000000 )
        {
        QueryPerformanceFrequency(&Frequency); 
        QueryPerformanceCounter(&StartingTime);
        }
#endif // DEBUG

    GET_TAG_RES res_get_tag;
    double tag_val = 0;        

    char cmd_str[ 10 ];
    int data_size = 0;

    cmd_str[ data_size++ ] = SRV_CMD::GET_TAG_VALUE_BY_ID;
    cmd_str[ data_size++ ] = T_NUMBER;

    cmd_str[ data_size++ ] = PAC_description_id;    
    memcpy( &cmd_str[ data_size ], &tag_id, sizeof( tag_id ) );    
    data_size += sizeof( tag_id );
        
    void* res_buff = 0;

    BOOL fSuccess; 
    DWORD cbRead; 
    const int BUFSIZE = 10;
    char chReadBuf[ BUFSIZE ];

    // Send a message to the pipe server and read the response. 
    fSuccess = TransactNamedPipe( 
        g_pipe,                 // pipe handle 
        cmd_str,                // message to server
        data_size,              // message length 
        chReadBuf,              // buffer to receive reply
        sizeof( chReadBuf ),    // size of read buffer
        &cbRead,                // bytes read
        &g_overlap);            // overlapped 

    if ( fSuccess )
        {      
        res_buff = chReadBuf;
        }
    else
        {
        int err = GetLastError();
        if ( err == ERROR_IO_PENDING )
            {       
            err = WaitForSingleObject( g_overlap.hEvent, 1000 );
            fSuccess = GetOverlappedResult( g_pipe, &g_overlap, &cbRead, false );

            if ( fSuccess )
                {
                res_buff = chReadBuf;
                }
            else
                {
                bug_log::msg.Format( _T( "Нет ответа от сервиса. %s" ), 
                    FormatErrorMessage( GetLastError() ) ); 
                BUG_LOG.add_warning_msg( "Driver", "" );

                CloseHandle( g_pipe );
                g_pipe = 0;
                g_connected = 0;
                }
            }
        }

    if ( res_buff!= 0 )
        {
        res_get_tag = ( GET_TAG_RES ) ( ( char* ) res_buff )[ 0 ];
        tag_val = *( ( double* )( ( char* ) res_buff + 1 ) ) ;

        if ( res_get_tag == GT_NEED_FUL_TAG_INFO )
            {
            result = 1;     
            }
        else
            {
            result = 0;        
            }
        }

#ifdef DEBUG
    if ( tag_id == 0xe1000000 )
        {
        QueryPerformanceCounter(&EndingTime);
        ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - 
            StartingTime.QuadPart;

        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

        bug_log::msg.Format( _T( "get_value2 = %d!" ), ElapsedMicroseconds.QuadPart );
        BUG_LOG.add_msg( _T( "Driver" ), _T( "" ) );
        }
#endif // DEBUG

    return tag_val;
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
    GET_TAG_RES res_get_tag;
    void *res = get_tag_value( tag, T_STRING, res_get_tag );

    if ( res_get_tag == GT_OK )
        {        
        }
    else
        {
        if ( res_get_tag == GT_NO_TAG_FOUND )
            {
            wchar_t tmp[ 50 ];
            mbstowcs( tmp, tag.tag_name, sizeof( tmp ) );
            bug_log::msg.Format( _T( "Тег \"%s\" не найден!" ), tmp );
            BUG_LOG.add_msg_once( tag.PAC_name, tag.PAC_address );
            }
        }

    return ( char* ) res;
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
    GET_TAG_RES res_get_tag;
    void *res = get_tag_value_by_id( tag_id, PAC_description_id, T_STRING, res_get_tag );

    if ( res_get_tag == GT_NEED_FUL_TAG_INFO )
        {
        result = 1;
        }
    else
        {
        result = 0;        
        }
    
    return ( char* ) res;
    
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
    Sleep( 1 );
    return 0;
    }
//-----------------------------------------------------------------------------
int load_from_stream( alarm &a, char *buff )
    {
    int len = 0;
    int str_len_0 = 0;

    memcpy( &a.params, buff + len, sizeof( a.params ) );
    len += sizeof( a.params );    
    memcpy( &a.type, buff + len, sizeof( a.type ) );
    len += sizeof( a.type );

    str_len_0 = strlen( buff + len ) + 1; 
    memcpy( a.description, buff + len, str_len_0 );
    len += str_len_0;

    memcpy( &a.enable, buff + len, sizeof( a.enable ) );
    len += sizeof( a.enable );

    str_len_0 = strlen( buff + len ) + 1; 
    memcpy( a.group, buff + len, str_len_0 );
    len += str_len_0;

    memcpy( &a.inhibit, buff + len, sizeof( a.inhibit ) );
    len += sizeof( a.inhibit );
    memcpy( &a.priority, buff + len, sizeof( a.priority ) );
    len += sizeof( a.priority );
    memcpy( &a.state, buff + len, sizeof( a.state ) );
    len += sizeof( a.state );
    memcpy( &a.suppress, buff + len, sizeof( a.suppress ) );
    len += sizeof( a.suppress );
    memcpy( &a.id, buff + len, sizeof( a.id ) );
    len += sizeof( a.id );
    memcpy( &a.driver_id, buff + len, sizeof( a.driver_id ) );
    len += sizeof( a.driver_id );

    return len;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall get_alarms( unsigned char PAC_id, all_alarm &alarms )
    {  
    char cmd_str[ 2 ] = {};

    int data_size = 0;
    cmd_str[ data_size++ ] = SRV_CMD::GET_ALARMS;
    cmd_str[ data_size++ ] = PAC_id;

    char *res = ( char* ) transact_pipe( cmd_str, data_size );
    int idx = 0;

    if ( res != 0 )
        {
        alarms.cnt = ( ( int* ) res )[ 0 ];
        idx += sizeof( int );
        alarms.id = ( ( int* ) ( res + idx ) )[ 0 ];
        idx += sizeof( int );

        if ( g_all_alarms[ PAC_id ].id != alarms.id )
            {
            g_all_alarms[ PAC_id ].id = alarms.id;
            g_all_alarms[ PAC_id ].cnt = alarms.cnt;

            for ( int i = 0; i < alarms.cnt; i++ )
                {
                idx += load_from_stream ( g_all_alarms[ PAC_id ].alarms[ i ], res + idx );
                }    
            }

        alarms.alarms = g_all_alarms[ PAC_id ].alarms;
        }

    return 0;        
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall set_alarm_cmd( unsigned char PAC_id, int count,
                                   error_cmd *errors )
    {       
    char* cmd_str = new char[ 1 + 1 + 4 + count * sizeof( error_cmd ) ];

    int data_size = 0;
    cmd_str[ data_size++ ] = SRV_CMD::SET_ALARMS;
    cmd_str[ data_size++ ] = PAC_id;

    memcpy( cmd_str + data_size, &count, sizeof( count ) );
    data_size += sizeof( count );

    for ( int i = 0; i < count; i++)
    	{
        memcpy( cmd_str + data_size, &errors[ i ], sizeof( error_cmd ) );
        data_size += sizeof( error_cmd );
    	}

    transact_pipe( cmd_str, data_size );    
    
    delete [] cmd_str;

    return 0;
    }
//-----------------------------------------------------------------------------
