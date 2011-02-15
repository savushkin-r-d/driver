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

u_int_2 G_PROTOCOL_VERSION = 1;

int abstract_cmmctr::count = 0;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PAC_cmmctr::PAC_cmmctr( const char* PAC_address, char *PAC_name, 
    UCHAR PAC_descr_id, int port /*= 10000*/, 
    int timeout /*= 1500 */ ): PAC_name( PAC_name ),
    PAC_descr_id( PAC_descr_id ), PAC_address( PAC_address ),
    dev_synch_access( new CSWMRG ),
    has_got_PAC_devices( new bool ),
    devices_request_id( 1000 ),
    is_connected( new bool ),
    prev_connected_state( new bool )
    {
    *is_connected         = false;
    *prev_connected_state = true;
    *has_got_PAC_devices  = false;

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

    const char* Lua_F =
        "function make_lua_str( str, val )\n"
        "	local DEB = 0\n"
        "\n"
        "	str =  str or ''\n"
        "	val =  val or 0\n"
        "\n"
        "	if DEB then print( '\"'..str..'\"' ) end\n"
        "	str = string.gsub( str, '%s*%-%-.*', '' ) --Удаляем комментарий.\n"
        "	str = string.gsub( str, '%s*', '' )		  --Удаляем пробелы.\n"
        "	if DEB then print( '\"'..str..'\"' ) end\n"
        "\n"
        "	--Выделяем объект, свойство и номер.\n"
        "	local obj_name, prop, n = string.match( str,\n"
        "		'(.*)%.([%a_]+)%[*(%d*)%]*$' );	 \n"
        "	obj_name = obj_name or ''\n"
        "	prop 	 =  prop or ''\n"
        "	n    	 =  n or ''\n"
        "	if n == '' then n = '0' end\n"
        "\n"
        "	if 	obj_name == '' or prop == '' then return '' end\n"
        "\n"
        "	if DEB then \n"
        "		print( string.format('\"%s\" -> \"%s\", n = \"%s\"', obj_name, prop, n ) )\n"
        "	end\n"
        "\n"
        "local cmd\n"
        "if type( val ) == 'string' then\n"
        "    cmd = string.format( 'sys.%s:set_cmd( \"%s\", %s, \"%s\" )',\n" 
        "        obj_name, prop, n, val )\n"
        "else\n"
        "    cmd = string.format( 'sys.%s:set_cmd( \"%s\", %s, %s )',\n" 
        "        obj_name, prop, n, val )\n"
        "end\n"
        "\n"
        "	if DEB then print( 'cmd = '..cmd ) end\n"
        "	if DEB then print( '' ) end\n"
        "	return cmd\n"
        "end\n";
    exec_Lua_str( PAC_Lua_state, Lua_F, "PAC_cmmctr(...)" );

    cmmctr = new tcp_cmmctr( PAC_name, 
        this->PAC_address.c_str() + 2 /*Пропускаем IP в адресе - IP10.0.1.23*/ );

    clear_tags();
    }
//-----------------------------------------------------------------------------
const char* PAC_cmmctr::get_address() const
    {
    return PAC_address.c_str();
    }
//-----------------------------------------------------------------------------
const char*  PAC_cmmctr::get_name() const
    {
    return PAC_name.c_str();
    }
//-----------------------------------------------------------------------------
UINT PAC_cmmctr::get_description_id() const
    {
    return PAC_descr_id;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::get_PAC_info()
    {
    if ( !cmmctr ) return -1;

    char buff[ 1 ];
    buff[ 0 ] = device_communicator::CMD_GET_INFO_ON_CONNECT;
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
        int res = exec_Lua_str( PAC_Lua_state, answer,
            "Ошибка получения данных о PAC" );
            
        if( res != 0 )
            {
            return -1;
            }

        PAC_protocol_version = get_int_param_from_Lua( PAC_Lua_state, 
            "protocol_version", "int PAC_cmmctr::get_PAC_info()" );

        if ( PAC_protocol_version != G_PROTOCOL_VERSION )
            {
            snprintf( bug_log::msg, bug_log::msg_size, 
                "Протокол PAC имеет более раннюю версию %d - должна быть %d!",
                PAC_protocol_version, G_PROTOCOL_VERSION );
            bug_log::add_msg_once( PAC_name.c_str(), PAC_address.c_str() );

            *is_connected = 0;
            return -2;
            }

        const char *in_name = get_str_param_from_Lua( PAC_Lua_state, 
            "PAC_name", "int PAC_cmmctr::get_PAC_info()" );

        if ( strcmp( in_name, PAC_name.c_str() ) != 0 )
            {
            snprintf( bug_log::msg, bug_log::msg_size, 
                "Имя PAC [ %s ] - \"%s\", в базе каналов - \"%s\"!",
                PAC_address.c_str(), in_name, PAC_name );
            bug_log::add_msg_once( PAC_name.c_str(), PAC_address.c_str() );
            *is_connected = 0;
            return -3;
            }

        snprintf( bug_log::msg, bug_log::msg_size, 
            "Версия драйвера PAC - %d, имя - \"%s\".",
            PAC_protocol_version, PAC_name.c_str() );
        bug_log::add_msg( PAC_name.c_str(), PAC_address.c_str() );

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
        "Новое описание PAC (№%d) было добавлено. Таймаут опроса - %d мсек.",
        res->get_description_id(), timeout );
    bug_log::add_msg( res->get_name(), res->get_address() );

    PAC_descriptions.at( PAC_descr_id ) = res;    

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
int PAC_cmmctr::get_int_param_from_Lua( lua_State *L, const char *param_name,
    const char *c_function_name ) const
    {    
    lua_getfield( L, LUA_GLOBALSINDEX, param_name );
    if ( lua_isnil( PAC_Lua_state, -1 ) )
        {
        snprintf( bug_log::msg, bug_log::msg_size, 
            "Ошибка вызова (%s) - переменная \"%s\" не найдена в Lua!", 
            c_function_name, param_name );
        bug_log::add_msg_once( PAC_name.c_str(), PAC_address.c_str() );
        }

    int res = lua_tointeger( PAC_Lua_state, -1 );

    lua_remove( PAC_Lua_state, -1 );
    return res;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::exec_Lua_str( lua_State* L, const char *Lua_str, 
    const char *error_str, bool is_print_error_msg ) const
    {
    int res = luaL_dostring( L, Lua_str );

    if( res != 0 && is_print_error_msg )
        {
        snprintf( bug_log::msg, bug_log::msg_size, 
            "%s -> %s!",
            error_str, lua_tostring( L, -1 ) );
        bug_log::add_msg_once( PAC_name.c_str(), PAC_address.c_str() );
        
        lua_pop( L, 1 );
        return 1;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::exec_Lua_str( const char *Lua_str, const char *error_str, 
    bool is_print_error_msg /*= true */ ) const
    {
    int res = luaL_dostring( PAC_Lua_state, Lua_str );

    if( res != 0 && is_print_error_msg )
        {
        snprintf( bug_log::msg, bug_log::msg_size, 
            "%s -> %s!",
            error_str, lua_tostring( PAC_Lua_state, -1 ) );
        bug_log::add_msg_once( PAC_name.c_str(), PAC_address.c_str() );

        lua_pop( PAC_Lua_state, 1 );
        return 1;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
const char* PAC_cmmctr::get_str_param_from_Lua( lua_State *L, 
    const char *param_name, const char *c_function_name ) const
    {    
    lua_getfield( L, LUA_GLOBALSINDEX, param_name );
    if ( lua_isnil( L, -1 ) )
        {
        //snprintf( bug_log::msg, bug_log::msg_size, 
        //    "Ошибка вызова (%s) - переменная \"%s\" не найдена в Lua!", 
        //    c_function_name, param_name );
        //bug_log::add_msg_once( PAC_name.c_str(), PAC_address.c_str() ); 
        }

    const char *res = lua_tostring( L, -1 );

    lua_remove( L, -1 );
    return res;
    }
//-----------------------------------------------------------------------------
const char* PAC_cmmctr::get_str_param_from_Lua( const char *param_name, 
    const char *c_function_name ) const
    {
    lua_getfield( PAC_Lua_state, LUA_GLOBALSINDEX, param_name );
    if ( lua_isnil( PAC_Lua_state, -1 ) )
        {
        //snprintf( bug_log::msg, bug_log::msg_size, 
        //    "Ошибка вызова (%s) - переменная \"%s\" не найдена в Lua!", 
        //    c_function_name, param_name );
        //bug_log::add_msg_once( PAC_name.c_str(), PAC_address.c_str() ); 
        }

    const char *res = lua_tostring( PAC_Lua_state, -1 );

    lua_remove( PAC_Lua_state, -1 );
    return res;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::get_PAC_devices()
    {
    if ( !cmmctr ) return -1;

    *has_got_PAC_devices = 0;

    char buff[ 3 ];
    buff[ 0 ] = device_communicator::CMD_GET_DEVICES;
    devices_request_id++;
    ( ( u_int_2* ) ( buff + 1 ) )[ 0 ] = devices_request_id;
    const int cmd_length = 3;
    cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, buff, cmd_length );

    unsigned int answer_size;
    char *answer = cmmctr->get_out_data( answer_size );
    if ( answer_size > 0 )
        { 
        devices_request_id = ( ( u_int_2* ) answer )[ 0 ]; 

        int res = exec_Lua_str( PAC_Lua_state, answer + 2,
            "Ошибка получения объектов PAC" );

        if( res != 0 )
            {
            return -1;
            }

        *has_got_PAC_devices = 1;
        return res;
        }   

    return -1;
    }
//-----------------------------------------------------------------------------
char* PAC_cmmctr::get_tag_str_value( int tag_id, bool &is_exist_tag )
    {
    is_exist_tag = false;
    const char *res = 0;
    static char res_str[ 1000 ] = {};

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, sizeof( cmd ), "res = nil; assert( loadstring( tags[ %d ] ) )()", tag_id );    
    exec_Lua_str( PAC_Lua_state, cmd, "char* PAC_cmmctr::get_tag_str_value", false );
    
    res = get_str_param_from_Lua( PAC_Lua_state, "res",
        "char* PAC_cmmctr::get_tag_str_value" );
        
    if ( res )
    	{
        is_exist_tag = true;

        strncpy( res_str, res, 1000 );        
    	}

    return res_str;    
    }
//-----------------------------------------------------------------------------
char* PAC_cmmctr::get_tag_str_value( const char *tag_name, bool &is_exist_tag )
    {
    is_exist_tag = false;
    const char *res = 0;

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, MAX_CMD_SIZE, "res = t.%s", tag_name );
    exec_Lua_str( PAC_Lua_state, cmd, "char* PAC_cmmctr::get_tag_str_value", false );

    res = get_str_param_from_Lua( PAC_Lua_state, "res",
        "PAC_cmmctr::get_tag_str_value" );
    static char res_str[ 1000 ] = {};
    if ( res )
        {
        is_exist_tag = true;
        strncpy( res_str, res, sizeof( res_str ) );
        }

    return res_str;
    }
//-----------------------------------------------------------------------------
double PAC_cmmctr::get_tag_value( int tag_id, bool &is_exist_tag )
    {
    is_exist_tag = false;
    double res = 0;

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, sizeof( cmd ), "res = nil; assert( loadstring( tags[ %d ] ) )()", tag_id );

    exec_Lua_str( PAC_Lua_state, cmd, "double PAC_cmmctr::get_tag_value", false );

    res = get_double_param_from_Lua( PAC_Lua_state, "res",
        "double PAC_cmmctr::get_tag_value", is_exist_tag );

    return res;
    }
//-----------------------------------------------------------------------------
double PAC_cmmctr::get_tag_value( const char *tag_name, bool &is_exist_tag )
    {
    is_exist_tag = false;
    double res = 0;

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, MAX_CMD_SIZE, "res = t.%s", tag_name );
    exec_Lua_str( PAC_Lua_state, cmd, "double PAC_cmmctr::get_tag_value", false );

    res = get_double_param_from_Lua( PAC_Lua_state, "res",
        "double PAC_cmmctr::get_tag_value", is_exist_tag );
    return res;
    }
//-----------------------------------------------------------------------------
double PAC_cmmctr::get_double_param_from_Lua( lua_State *L, 
    const char *param_name, const char *c_function_name, bool &is_exist ) const
    {
    is_exist = true;
    lua_getfield( L, LUA_GLOBALSINDEX, param_name );
    if ( lua_isnil( PAC_Lua_state, -1 ) )
        {
        is_exist = false;
        }

    double res = lua_tonumber( PAC_Lua_state, -1 );

    lua_remove( PAC_Lua_state, -1 );
    return res;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::add_nill_tag( int tag_id )
    {
    int res = 0;

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, MAX_CMD_SIZE, "tags[ %d ] = 0", tag_id );
    exec_Lua_str( PAC_Lua_state, cmd, "PAC_cmmctr::add_nill_tag" );

    return res;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::get_all_devices_states()
    {
    if ( !cmmctr ) return -1;

    char buff[ 1 ] = { device_communicator::CMD_GET_DEVICES_STATES };            
    int res = cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, buff, sizeof( buff ) );
    if ( res != 0 ) // Неуспешная операция обмена данными с контроллером.
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
        if ( ( ( u_int_2* ) answer )[ 0 ] != devices_request_id )
            {
            return PAC_DEVICES_CHANGING;
            }

        int res = exec_Lua_str( PAC_Lua_state, answer + 2,
            "Ошибка получения объектов PAC" );

        if( res != 0 )
            {
            return OTHER_ERROR;
            }

        return LOAD_OK;
        }   

    return OTHER_ERROR;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::is_got_PAC_devices()
    {
    return *has_got_PAC_devices;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::add_exist_tag( const char *tag_name, int tag_id )
    {
    int res = 0;

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, MAX_CMD_SIZE, "tags[ %d ] = \"t.%s\"", tag_id, tag_name );
    exec_Lua_str( PAC_Lua_state, cmd, "PAC_cmmctr::add_exist_tag" );

    return res;
    }
//-----------------------------------------------------------------------------
void PAC_cmmctr::send_PAC_cmd( char *cmd, int count )
	{
	cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, cmd, count );
	}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
abstract_cmmctr::abstract_cmmctr( const char* PAC_name, int timeout ): id( id ),
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
            snprintf( bug_log::msg, bug_log::msg_size,
                "Ошибка инициализации сетевой библиотеки: %s\n",
                WSA_Err_Decode( WSAGetLastError() ) );

            bug_log::add_error_msg( "Driver", "" );            
            return 0;
            }
        }
    isInitialized = 1;
    return 1;
    }
//-----------------------------------------------------------------------------
tcp_cmmctr::tcp_cmmctr( const char *PAC_name, const char* sIP,
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

    res = connect( sock, ( SOCKADDR* ) &sock_address, sizeof( sockaddr_in ) );
    //if ( res == SOCKET_ERROR )
    //    {
    //    snprintf( bug_log::msg, bug_log::msg_size,
    //        "Ошибка подключения - %s", WSA_Err_Decode( WSAGetLastError() ) );        
    //    bug_log::add_error_msg( PAC_name, ip_address );
    //    return 0;
    //    }

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


