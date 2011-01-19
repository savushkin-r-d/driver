#include "stdafx.h"

#include "PAC_cmmctr.h"

#ifdef  __cplusplus
extern "C" {
#endif

#include "snprintf.h"

#include    "lua.h"
#include    "lauxlib.h"
#include    "lualib.h"

#ifdef  __cplusplus
    };
#endif

int tcp_cmmctr::instancesCount = 0;
int tcp_cmmctr::isInitialized = 0;

extern u_int_2 G_PROTOCOL_VERSION;

int abstract_cmmctr::count = 0;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PAC_cmmctr::PAC_cmmctr( const char* PAC_address, char *PAC_name, 
    UCHAR PAC_descr_id, int port /*= 10000*/, 
    int timeout /*= 1500 */ ): PAC_name( PAC_name ),
    PAC_descr_id( PAC_descr_id ), PAC_address( PAC_address )
    {
    PAC_Lua_state = lua_open();  /* create state */
    if ( PAC_Lua_state == NULL )
        {
        snprintf( bug_log::msg, bug_log::msg_size, 
            "Cannot create Lua state: not enough memory!" );

        bug_log::add_error_msg( PAC_name, PAC_address );

#ifdef DEBUG
        DebugBreak();
#endif // DEBUG
        }

    lua_gc( PAC_Lua_state, LUA_GCSTOP, 0 );  /* stop collector during initialization */
    luaL_openlibs( PAC_Lua_state );          /* open libraries */
    lua_gc( PAC_Lua_state, LUA_GCRESTART, 0 );    
    }
//-----------------------------------------------------------------------------
const char* PAC_cmmctr::get_address() const
    {
    return 0;
    }
//-----------------------------------------------------------------------------
const char*  PAC_cmmctr::get_name() const
    {
    return 0;
    }
//-----------------------------------------------------------------------------
char PAC_cmmctr::get_description_id() const
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::get_PAC_info()
    {
    if ( !cmmctr ) return -1;

    char buff[ 1 ];
    buff[ 0 ] = CMD_GET_INFO_ON_CONNECT;
    const int cmd_length = 1;
    int res = cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, buff, cmd_length );
    if ( res != 0 ) // Не успешная операция обмена данными с контроллером.
        {
        if ( err_retr_count >= PM_MAX_ERRORS_COUNT )
            {
            *is_connected = 0;
            }
        else
            {
            err_retr_count++;
            }        
        }
    else            // Успешная операция обмена данными с контроллером.
        {
        *is_connected = 1;
        err_retr_count = 0;
        }

    unsigned int answer_size;
    char *answer = cmmctr->get_out_data( answer_size );
    if ( answer_size > 0 )
        { 
        PAC_protocol_version = ( ( u_int_2* ) answer )[ 0 ]; 
        if ( ( PAC_protocol_version != G_PROTOCOL_VERSION ) &&
            !( 1 == PAC_protocol_version ||
            2 == PAC_protocol_version ) ) //Совместимость с версиями 1 и 2.
            {
            sprintf_s( bug_log::msg, bug_log::msg_size, 
                "Протокол PAC имеет более раннюю версию %d - должна быть %d!",
                PAC_protocol_version, G_PROTOCOL_VERSION );
            bug_log::add_msg_once( PAC_name, address );

            *is_connected = 0;
            return -2;
            }

        char *in_name = answer + 2;
        if ( strcmp( in_name, PAC_name ) != 0 )
            {
            sprintf_s( bug_log::msg, bug_log::msg_size, 
                "Имя PAC [ %s ] - \"%s\", в базе каналов - \"%s\"!",
                address, in_name, PAC_name );
            bug_log::add_msg_once( PAC_name, address );
            *is_connected = 0;
            return -3;
            }

        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Версия драйвера - %d.", PAC_protocol_version );
        bug_log::add_msg( PAC_name, address );

        return PAC_protocol_version;
        }    

    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PAC_cmmctr_group::PAC_cmmctr_group()
    {
    PAC_descriptions.reserve( MAX_PAC_DESCR_NUMBER );
    for ( int i = 0; i < MAX_PAC_DESCR_NUMBER; i++ )
    	{
        PAC_descriptions.push_back( 0 );
    	}
    }
//-----------------------------------------------------------------------------
PAC_cmmctr* PAC_cmmctr_group::add_PAC( char* const PAC_address, 
    char* const PAC_name, UCHAR PAC_descr_id, int PAC_port, int timeout )
    {
    //Создаем новый коммуникатор.
    PAC_cmmctr *res = new PAC_cmmctr( PAC_address, PAC_name, PAC_descr_id,
        PAC_port, timeout );  

    snprintf( bug_log::msg, bug_log::msg_size, 
        "Новый PAC был добавлен. Таймаут опроса - %d мсек.",
        timeout );
    bug_log::add_msg( res->get_name(), res->get_address() );

    PAC_descriptions.push_back( res );    

    return get_PAC( PAC_descr_id );  //Возвращаем добавленный контроллер.
    }
//-----------------------------------------------------------------------------
PAC_cmmctr* PAC_cmmctr_group::get_PAC( int descr_id )
    {
    PAC_cmmctr* res = 0;

    try
    	{
        res = PAC_descriptions.at( descr_id );
    	}
    catch (...)
    	{
#ifdef DEBUG
        DebugBreak();
#endif // DEBUG
        res = 0;    	
    	}

    return res;
   }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
char* abstract_cmmctr::get_out_data( unsigned int &cnt ) 
    {    
    cnt = answer_size;
    return in_buff;
    }
//-----------------------------------------------------------------------------
abstract_cmmctr* PAC_cmmctr::get_cmmctr()
    {
    return cmmctr; 
    }
//-----------------------------------------------------------------------------
abstract_cmmctr::abstract_cmmctr( char* PAC_name, int timeout ): id( id ),
    timeout( timeout )
    {
    strcpy_s( this->PAC_name, 20, PAC_name );

    count++;
    id = count;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int tcp_cmmctr::InitLib()
    {
    WSAData tmpWSAData;
    if (!isInitialized)
        {
        if ( WSAStartup( 0x202, &tmpWSAData ) )
            {
            sprintf_s( bug_log::msg, bug_log::msg_size,
                "Ошибка инициализации сетевой библиотеки." );
            bug_log::add_error_msg( "Driver", "" );            
            return 0;
            }
        }
    isInitialized = 1;
    return 1;
    }
//-----------------------------------------------------------------------------
tcp_cmmctr::tcp_cmmctr( char *PAC_name, char* sIP,
    int iSocket /*= 10000*/, 
    int timeout /*=1500*/ ): abstract_cmmctr( PAC_name, timeout ),
    isConnected( 0 ),                       
    pidx( 0 ),
    port( iSocket )
    {
    InitializeCriticalSection( &m_cs );

    memset( is_errors, 0, C_ERRORS_SIZE );

    instancesCount++;
    sprintf_s( ip_address, 20, "%s", sIP );

    InitLib();
    //Connect( 1 );
    }
//-----------------------------------------------------------------------------
tcp_cmmctr::~tcp_cmmctr()
    {
    DeleteCriticalSection( &m_cs );

    Disconnect();
    if ( instancesCount < 2 )
        {
        DeinitLib();
        }
    instancesCount--;
    }
//-----------------------------------------------------------------------------
int tcp_cmmctr::send_2_PAC( UCHAR Service_ID, char *data, UINT length )
    {
    EnterCriticalSection( &m_cs );

    answer_size = 0;
    if ( !isInitialized )
        {
        if (!InitLib() )
            {
            sprintf_s( bug_log::msg, bug_log::msg_size, 
                "Фатальная ошибка. Сетевая библиотека не инициализирована." );
            bug_log::add_error_msg( "Driver", "" );

            LeaveCriticalSection( &m_cs );
            return 1;
            }
        }

    if ( !isConnected )
        {
        if ( !Connect() )
            {
            bug_log::set_error( is_errors[ EF_NO_CONNECTION ], PAC_name, 
                ip_address, "Нет связи!" );

            LeaveCriticalSection( &m_cs );
            return 1;
            }    

        //Сбрасываем ошибку.
        bug_log::reset_error( is_errors[ EF_NO_CONNECTION ], PAC_name, 
            ip_address, "Нет связи!" );
        }

    buff[0] = 's';
    buff[1] = Service_ID;
    buff[2] = 1;                 // FrameSingle.
    buff[3] = ++pidx;            // Идентификатор пакета.
    buff[4] = ( char ) ( length >> 8 );
    buff[5] = length & 0xFF;
    memcpy(buff+6,data,length);


    if ( send( sock, buff, length + 6, 0 ) == SOCKET_ERROR )
        {
        bug_log::set_error( is_errors[ EF_SEND_ERROR ], PAC_name, 
            ip_address, "Ошибка отсылки сообщения!" );

        Disconnect();

        LeaveCriticalSection( &m_cs );
        return 1;
        }
    bug_log::reset_error( is_errors[ EF_SEND_ERROR ], PAC_name, 
        ip_address, "Ошибка отсылки сообщения!" );

    int res = recv( sock, in_buff, 8000, 0 );

    if ( 0 == res )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size,
            "PAC закрыл соединение." );
        bug_log::add_msg( PAC_name, ip_address );
        Disconnect();

        LeaveCriticalSection( &m_cs );
        return 1;
        }

    if ( res < 0 /*res == SOCKET_ERROR*/ )
        {
        bug_log::set_error( is_errors[ EF_ANSWER_ERROR ], PAC_name, 
            ip_address, "Ошибка получения ответа!" );

        Disconnect();

        LeaveCriticalSection( &m_cs );
        return 1;
        }
    bug_log::reset_error( is_errors[ EF_ANSWER_ERROR ], PAC_name, 
        ip_address, "Ошибка получения ответа!" );


    if ( in_buff[ 1 ] == 7 )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Возвращена ошибка!" );
        bug_log::add_warning_msg( PAC_name, ip_address );

        LeaveCriticalSection( &m_cs );
        return 1;
        }    
    unsigned char *work_buff = ( unsigned char* ) in_buff;

    //-Проверка на правильность заголовка блока данных от PAC.
    if ( !( work_buff[ 0 ] == 's'                   //NetId 
        && pidx == work_buff[ 2 ] ) )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Возвращен неверный ответ!" );        
        bug_log::add_warning_msg( PAC_name, ip_address );
        Disconnect();
#ifdef DEBUG
        //        _DebugBreak();
#endif

        LeaveCriticalSection( &m_cs );
        return 1;
        }
    //-!>

    answer_size = work_buff[ 3 ] * 256 + work_buff[ 4 ];
    if ( 0 == answer_size )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Длина ответа - 0!" );
        bug_log::add_warning_msg( PAC_name, ip_address );
#ifdef DEBUG
        //_DebugBreak();
#endif
        LeaveCriticalSection( &m_cs );
        return 1;
        }

    if ( answer_size > P_MAX_BUFFER_SIZE )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Длина ответа[ %d ] > максимальной длины[ %d ]!", 
            answer_size, P_MAX_BUFFER_SIZE );        
        bug_log::add_warning_msg( PAC_name, ip_address );
        answer_size = 0;

#ifdef DEBUG
        _DebugBreak();
#endif
        LeaveCriticalSection( &m_cs );
        return 1;
        }

    int  tmp_answer_size = answer_size + 5 - res; 
    char *tmp_buff = in_buff + res; 
    while ( tmp_answer_size > 0 )
        {        
        res = recv( sock, tmp_buff, 8000, 0 );         

        if ( res <= 0 /*res == SOCKET_ERROR*/ )
            {
            sprintf_s( bug_log::msg, bug_log::msg_size, 
                "Получена часть ответа от PAC!" );
            bug_log::add_warning_msg( PAC_name, ip_address );

            answer_size = 0;

            Disconnect();
            LeaveCriticalSection( &m_cs );
            return 1;
            }

        tmp_buff += res;      
        tmp_answer_size -= res;   
        }     

    LeaveCriticalSection( &m_cs );
    return 0;
    }
//-----------------------------------------------------------------------------
void tcp_cmmctr::DeinitLib()
    {
    if (isInitialized)
        WSACleanup();
    isInitialized = 0;
    }
//-----------------------------------------------------------------------------
int tcp_cmmctr::Connect()
    {
    if ( isConnected )
        {
#ifdef _DEBUG
        _DebugBreak();
#endif // _DEBUG
        return 1;
        }
    sock = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
    if (sock == INVALID_SOCKET)
        {
        sprintf_s( bug_log::msg, bug_log::msg_size,
            "Ошибка создания сокета!" );        
        bug_log::add_error_msg( PAC_name, ip_address );
        return 0;
        }

    int vlen = sizeof( timeout );

    if ( setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, ( char* )&timeout, vlen) == SOCKET_ERROR ||
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout, vlen) == SOCKET_ERROR )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size,
            "Ошибка установления параметров сокета!" );        
        bug_log::add_error_msg( PAC_name, ip_address );
        return 0;
        }

    //Переводим сокет в неблокирующий режим.
    u_long mode = 1;
    int res = ioctlsocket( sock, FIONBIO, &mode );
    if ( res == SOCKET_ERROR )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Ошибка перевода сокета в неблокирующий режим!" );        
        bug_log::add_error_msg( PAC_name, ip_address );

        closesocket( sock );
        return 0;
        }

    sockaddr_in sock_address;
    memset(&sock_address,0,sizeof(sockaddr_in));
    sock_address.sin_family  = AF_INET;
    sock_address.sin_port = htons( ( u_short ) port);
    sock_address.sin_addr.s_addr = inet_addr(ip_address);

    connect( sock, ( SOCKADDR* ) &sock_address, sizeof( sockaddr_in ) );

    fd_set rdevents;
    struct timeval tv;
    FD_ZERO( &rdevents );
    FD_SET( sock, &rdevents );

    tv.tv_sec = 2;
    tv.tv_usec = 0;
    int rc = select( ( int ) sock, &rdevents, 0, 0, &tv );

    //Переводим сокет в блокирующий режим.
    mode = 0;
    res = ioctlsocket( sock, FIONBIO, &mode );
    if ( res == SOCKET_ERROR )
        {
        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Ошибка перевода сокета в блокирующий режим!" );
        bug_log::add_error_msg( PAC_name, ip_address );
        }   

    if ( SOCKET_ERROR == rc || 0 == rc )
        {
        bug_log::set_error( is_errors[ EF_CONNECT_ERROR ], PAC_name, 
            ip_address, "Ошибка установления соединения!" );

        closesocket( sock );
        return 0;
        }
    bug_log::reset_error( is_errors[ EF_CONNECT_ERROR ], PAC_name, 
        ip_address, "Ошибка установления соединения!" );

    res = recv( sock, in_buff, 255, 0 );
    if (SOCKET_ERROR == res)
        {
        sprintf_s( bug_log::msg, bug_log::msg_size, 
            "Ошибка получения ответа при подключении!" );
        bug_log::add_warning_msg( PAC_name, ip_address );

        closesocket( sock );

        return 0;
        }
    isConnected = 1;
    return 1;
    }
//-----------------------------------------------------------------------------
int tcp_cmmctr::recvtimeout( UINT s, char *buf, int len, int timeout, int usec ) 
    { 
    fd_set fds; 
    int n; 
    struct timeval tv; 

    // настраиваем  file descriptor set 
    FD_ZERO( &fds ); 
    FD_SET( s, &fds ); 

    // настраиваем время на таймаут 
    tv.tv_sec = timeout; 
    tv.tv_usec = usec; 

    // ждем таймаута или полученных данных
    n = select( s + 1, &fds, NULL, NULL, &tv ); 
    if ( 0 == n ) return -2;  // timeout! 
    if ( -1 == n ) return -1; // error 

    // данные должны быть здесь, поэтому делаем обычный recv()
    return recv( s, buf, len, 0 ); 
    } 
//-----------------------------------------------------------------------------
void tcp_cmmctr::Disconnect()
    {
    if ( isConnected )
        {
        closesocket( sock );
        }
    isConnected = 0;
    }
//-----------------------------------------------------------------------------
char* tcp_cmmctr::get_out_data( unsigned int &cnt )
    {
    cnt = answer_size;
    return in_buff + 5;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


