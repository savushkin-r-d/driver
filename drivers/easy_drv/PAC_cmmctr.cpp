#include "stdafx.h"

#include "PAC_cmmctr.h"

#include "errors_manager.h"

#ifdef  __cplusplus
extern "C" {
#endif

#include    "lua.h"
#include    "lauxlib.h"
#include    "lualib.h"

#ifdef  __cplusplus
    };
#endif
const u_int_2 G_QLZ_VERSION = 102;
const u_int_2 G_NON_UNICODE_VERSION = 103;
const u_int_2 G_CURRENT_PROTOCOL_VERSION = 104;
const u_int_2 G_UNKNOWN_PROTOCOL_VERSION = 1;

int tcp_cmmctr::instancesCount = 0;
int tcp_cmmctr::isInitialized = 0;

//История версий:
//    1 - базовая версия.
//    2 - добавлен механизм сохранения параметров и их автоматического
//    восстановления после сбоя (замены PAC).

int abstract_cmmctr::count = 0;

extern alarm_manager *g_alarm_manager; ///< Работа с ошибками контроллеров.
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
    PAC_params_CRC( 0 ),
    is_process_PAC_params( false )
    {
    *is_connected         = false;
    *has_got_PAC_devices  = false;

    PAC_Lua_state = lua_open();  /* create state */
    if ( PAC_Lua_state == NULL )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Cannot create Lua state: not enough memory!" );

        BUG_LOG.add_error_msg( PAC_name, PAC_address );

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
        "    cmd = string.format( '__%s:set_cmd( \"%s\", %s, \"%s\" )',\n" 
        "        obj_name, prop, n, val )\n"
        "else\n"
        "    cmd = string.format( '__%s:set_cmd( \"%s\", %s, %s )',\n" 
        "        obj_name, prop, n, val )\n"
        "end\n"
        "\n"
        "	if DEB then print( 'cmd = '..cmd ) end\n"
        "	if DEB then print( '' ) end\n"
        "	return cmd\n"
        "end\n";
    exec_Lua_str( Lua_F, "PAC_cmmctr(...)" );

    cmmctr = new tcp_cmmctr( PAC_name, 
        this->PAC_address.c_str() + 2, /*Пропускаем IP в адресе - IP10.0.1.23*/
        port, timeout );

    clear_tags();
        
    tags_str.reserve( 10000 );
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
int PAC_cmmctr::get_int_param_from_Lua( const char *param_name,
    const char *c_function_name ) const
    {    
    lua_getfield( PAC_Lua_state, LUA_GLOBALSINDEX, param_name );
    if ( lua_isnil( PAC_Lua_state, -1 ) )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Ошибка вызова (%s) - переменная \"%s\" не найдена в Lua!", 
            c_function_name, param_name );
        BUG_LOG.add_msg_once( PAC_name.c_str(), PAC_address.c_str() );
        }

    int res = lua_tointeger( PAC_Lua_state, -1 );

    lua_remove( PAC_Lua_state, -1 );
    return res;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::exec_Lua_str( const char *Lua_str, 
    const char *error_str, bool is_print_error_msg ) const
    {
    int res = luaL_dostring( PAC_Lua_state, Lua_str );

    if( res != 0 )
        {
        if ( is_print_error_msg )
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                "%s -> %s!",
                error_str, lua_tostring( PAC_Lua_state, -1 ) );
            BUG_LOG.add_msg_once( PAC_name.c_str(), PAC_address.c_str() );
            }

        lua_pop( PAC_Lua_state, 1 );
        return 1;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
const int MAX_STR_SIZE = 500;
char tmp_str[MAX_STR_SIZE];

const char* PAC_cmmctr::get_str_param_from_Lua( const char *param_name, 
    const char *c_function_name ) const
    {
    lua_getfield( PAC_Lua_state, LUA_GLOBALSINDEX, param_name );
    if ( lua_isnil( PAC_Lua_state, -1 ) )
        {
        //snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
        //    "Ошибка вызова (%s) - переменная \"%s\" не найдена в Lua!", 
        //    c_function_name, param_name );
        //BUG_LOG.add_msg_once( PAC_name.c_str(), PAC_address.c_str() ); 
        }

    const char *res = lua_tostring( PAC_Lua_state, -1 );       

    if (PAC_protocol_version > G_NON_UNICODE_VERSION)
        {
        memset(tmp_str, 0, MAX_DESCR_LEN);
        convert_utf8_to_windows1251(res, tmp_str, MAX_STR_SIZE);
        res = tmp_str;
        }    

    lua_remove( PAC_Lua_state, -1 );
    return res;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::get_PAC_info()
    {
    if ( !cmmctr ) return -1;

    const char buff[ 1 ] = { device_communicator::CMD_GET_INFO_ON_CONNECT };
    
    const int cmd_length = 1;
    int res = cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, buff, cmd_length );
    if ( res != 0 ) // Не успешная операция обмена данными с контроллером.
        {
        if ( err_retr_count >= PM_MAX_ERRORS_COUNT )
            {
            *is_connected = false;
            }
        else
            {
            err_retr_count++;
            }        
        }
    else            // Успешная операция обмена данными с контроллером.
        {
        *is_connected = true;
        err_retr_count = 0;
        }

    unsigned int answer_size;
    char *answer = cmmctr->get_out_data( answer_size );

    if ( answer_size > 0 )
        { 
        res = exec_Lua_str( answer,
            "Ошибка получения данных о PAC" );

        if( res != 0 )
            {
            return -1;
            }

        PAC_protocol_version = get_int_param_from_Lua( "protocol_version", 
            "int PAC_cmmctr::get_PAC_info()" );

        if ( PAC_protocol_version != G_CURRENT_PROTOCOL_VERSION && 
            !(PAC_protocol_version == G_NON_UNICODE_VERSION) &&
            !( PAC_protocol_version == G_QLZ_VERSION ) )
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                "Протокол PAC версии %d - должна быть %u!",
                PAC_protocol_version, G_CURRENT_PROTOCOL_VERSION );
            BUG_LOG.add_msg_once( PAC_name.c_str(), PAC_address.c_str() );

            *is_connected = false;
            return -2;
            }

        const char *in_name = get_str_param_from_Lua( "PAC_name", 
            "int PAC_cmmctr::get_PAC_info()" );
        
        if ( strcmp( in_name, PAC_name.c_str() ) != 0 )
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                "Имя PAC [ %s ] - \"%s\", в базе каналов - \"%s\"!",
                PAC_address.c_str(), in_name, PAC_name.c_str() );
            BUG_LOG.add_msg_once( PAC_name.c_str(), PAC_address.c_str() );
            *is_connected = false;
            return -3;
            }

        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Версия драйвера: PAC - %d, сервер - %u; имя PAC - \"%s\".",
            PAC_protocol_version, G_CURRENT_PROTOCOL_VERSION,
            PAC_name.c_str() );
        BUG_LOG.add_msg( PAC_name.c_str(), PAC_address.c_str() );
        
        //Проверка на сброс параметров в PAC.
        PAC_params_CRC = get_int_param_from_Lua( "params_CRC", 
            "int PAC_cmmctr::get_PAC_info()" );

        check_PAC_params();

        cmmctr->set_protocol_version( PAC_protocol_version );

        return PAC_protocol_version;
        }    

    return 0;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::get_PAC_devices()
    {
    if ( !cmmctr ) return -1;

    int res = 0;
    dev_synch_access->WaitToWrite();

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

        res = exec_Lua_str( answer + LUA_STRING_START_POSITION,
            "Ошибка получения объектов PAC" );

        if( res != 0 )
            {
            //Сохраняем строку, вызвавшую ошибку.
            BUG_LOG.bug_log_file.save_msg( answer + LUA_STRING_START_POSITION );
            }
        else
            {
            *has_got_PAC_devices = 1;
            }
        }   

    dev_synch_access->Done();
    return res;
    }
//-----------------------------------------------------------------------------
PAC_cmmctr::LOAD_RESULTS PAC_cmmctr::get_PAC_all_devices_states()
    {
    if ( !cmmctr ) return OTHER_ERROR;

    dev_synch_access->WaitToWrite();

    static unsigned long int gc_counter = 0;
    gc_counter++;
    if ( gc_counter % PM_GARBAGE_CYCLE == 0 )
        {
        // Полная уборка мусора каждые n итераций.
        lua_gc( PAC_Lua_state, LUA_GCCOLLECT, 0 ); 
        }

#ifdef DEBUG_LUA_MEM
    static int counter = 0;
    counter++;
    if ( counter > 100 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, "Lua memory (devices data) = %d", 
            lua_gc( PAC_Lua_state, LUA_GCCOUNT, 0 ) * 1024 +
            lua_gc( PAC_Lua_state, LUA_GCCOUNTB, 0 ) );
        BUG_LOG.add_msg( get_name(), get_address() );

        counter = 0;
        }
#endif // DEBUG_LUA_MEM


    char buff[ 1 ] = { device_communicator::CMD_GET_DEVICES_STATES };            
    int res = cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, buff, sizeof( buff ) );
    if ( res != 0 ) // Неуспешная операция обмена данными с контроллером.
        {
        if ( err_retr_count >= PM_MAX_ERRORS_COUNT )
            {
            *is_connected = false;
            }
        else
            {
            err_retr_count++;
            }        
        }
    else            // Успешная операция обмена данными с контроллером.
        {
        *is_connected = true;
        err_retr_count = 0;
        }

    unsigned int answer_size;
    char *answer = cmmctr->get_out_data( answer_size );
    if ( answer_size > 0 )
        { 
        if ( ( ( u_int_2* ) answer )[ 0 ] != devices_request_id )
            {
            dev_synch_access->Done();
            return PAC_DEVICES_CHANGING;
            }

        res = exec_Lua_str( answer + LUA_STRING_START_POSITION,
            "Ошибка получения состояния объектов PAC" );

        if ( res != 0 )
            {
            BUG_LOG.add_msg_once( PAC_name.c_str(), PAC_address.c_str(),
                answer + LUA_STRING_START_POSITION );

            dev_synch_access->Done();
            return OTHER_ERROR;
            }

        res = exec_Lua_str( tags_str.c_str(), 
            "Ошибка обновления тегов состояния объектов PAC" );
       
        if ( res != 0 )
            {
            dev_synch_access->Done();
            return OTHER_ERROR;
            }
       
        dev_synch_access->Done();
        return LOAD_OK;
        }   

    dev_synch_access->Done();
    return OTHER_ERROR;
    }
//-----------------------------------------------------------------------------
void PAC_cmmctr::get_tag_str_value( int tag_id, bool &is_exist_tag,
    char *str_value, int max_length )
    {
    if ( str_value == 0 )
        {
        return;
        }

    dev_synch_access->WaitToRead();

    memset(str_value, 0, strlen(str_value));
    is_exist_tag = false;

    lua_getfield( PAC_Lua_state, LUA_GLOBALSINDEX, "tags" );
    if ( !lua_isnil( PAC_Lua_state, -1 ) )
        {
        lua_pushinteger( PAC_Lua_state, tag_id );
        lua_gettable( PAC_Lua_state, -2 );
      
        if ( lua_isstring( PAC_Lua_state, -1 ) )
            {
            is_exist_tag = true;
            const char* val = lua_tostring( PAC_Lua_state, -1 );
            if (PAC_protocol_version > G_NON_UNICODE_VERSION)
                {
                convert_utf8_to_windows1251(val, str_value, max_length);
                }
            else
                {
                strncpy(str_value, val, max_length);
                }
            }
        lua_remove( PAC_Lua_state, -1 );
        }    
    lua_remove( PAC_Lua_state, -1 );

    dev_synch_access->Done();
    }
//-----------------------------------------------------------------------------
void PAC_cmmctr::get_tag_str_value( const char *tag_name, bool &is_exist_tag,
    char *str_value, int max_length )
    {
    if ( str_value == 0 )
        {
        return;
        }

    dev_synch_access->WaitToRead();

    is_exist_tag = false;    
    str_value[ 0 ] = 0;

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, MAX_CMD_SIZE, "res = t.%s", tag_name );
    if ( exec_Lua_str( cmd, "char* PAC_cmmctr::get_tag_str_value", false ) == 0 )
        {
        const char *res = get_str_param_from_Lua( "res", 
            "PAC_cmmctr::get_tag_str_value" );
                
        if ( res )
            {
            is_exist_tag = true;
            strncpy(str_value, res, max_length);
            }
        }

    dev_synch_access->Done();
    }
//-----------------------------------------------------------------------------
double PAC_cmmctr::get_tag_value( int tag_id, bool &is_exist_tag )
    {
    dev_synch_access->WaitToRead();

    is_exist_tag = false;
    double res = 0;

    lua_getfield( PAC_Lua_state, LUA_GLOBALSINDEX, "tags" );
    if ( !lua_isnil( PAC_Lua_state, -1 ) )
        {
        lua_pushinteger( PAC_Lua_state, tag_id );
        lua_gettable( PAC_Lua_state, -2 );
        if ( lua_isnumber( PAC_Lua_state, -1 ) )
            {
            res = lua_tonumber( PAC_Lua_state, -1 );
            is_exist_tag = true;
            }

        lua_remove( PAC_Lua_state, -1 );
        }
    lua_remove( PAC_Lua_state, -1 );

    dev_synch_access->Done();
    return res;
    }
//-----------------------------------------------------------------------------
double PAC_cmmctr::get_tag_value( const char *tag_name, bool &is_exist_tag )
    {
    dev_synch_access->WaitToRead();

    is_exist_tag = false;
    double res = 0;

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, MAX_CMD_SIZE, "res = t.%s", tag_name );    
    
    if ( exec_Lua_str( cmd, "double PAC_cmmctr::get_tag_value", false ) == 0 )
        {
        res = get_double_param_from_Lua( "res",
            "double PAC_cmmctr::get_tag_value", is_exist_tag );
        }

    dev_synch_access->Done();

    return res;
    }
//-----------------------------------------------------------------------------
double PAC_cmmctr::get_double_param_from_Lua( const char *param_name,
    const char *c_function_name, bool &is_exist ) const
    {    
    is_exist = true;
    lua_getfield( PAC_Lua_state, LUA_GLOBALSINDEX, param_name );
    if ( lua_isnil( PAC_Lua_state, -1 ) )
        {
        is_exist = false;
        }

    double res = lua_tonumber( PAC_Lua_state, -1 );

    lua_remove( PAC_Lua_state, -1 );
    return res;
    }
//-----------------------------------------------------------------------------
void PAC_cmmctr::add_nill_tag( int tag_id )
    { 
    dev_synch_access->WaitToRead();
    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];
    snprintf( cmd, MAX_CMD_SIZE, "tags[%d]=0\n", tag_id );

    exec_Lua_str( cmd, "PAC_cmmctr::add_nill_tag" );  

    dev_synch_access->Done();
    }
//-----------------------------------------------------------------------------
bool PAC_cmmctr::is_got_PAC_devices()
    {
    return *has_got_PAC_devices;
    }
//-----------------------------------------------------------------------------
void PAC_cmmctr::add_exist_tag( const char *tag_name, int tag_id )
    {
    dev_synch_access->WaitToWrite();

    const int MAX_CMD_SIZE = 200;
    char cmd[ MAX_CMD_SIZE ];

    snprintf( cmd, MAX_CMD_SIZE, "tags[%d]=t.%s\n", tag_id, tag_name );
    exec_Lua_str( cmd, "Ошибка обновления добавленного тега" );

    if ( tags_str.find( cmd ) != std::string::npos )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
            _T( "Тег \'%s\' уже добавлен в список тегов (возможно имеет неверный тип)!" ),
            tag_name );
        BUG_LOG.add_msg( PAC_name.c_str(), PAC_address.c_str() );
        }
    else
        {
        tags_str += cmd;
        }

    dev_synch_access->Done();
    }
//-----------------------------------------------------------------------------
abstract_cmmctr* PAC_cmmctr::get_cmmctr()
    {
    return cmmctr; 
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::clear_tags()
    {
    dev_synch_access->WaitToWrite();
    // Уборка мусора.
    lua_gc( PAC_Lua_state, LUA_GCSTOP, 0 );
    
    //-Инициализация таблицы тегов.
    int res = exec_Lua_str( "tags = {}", "clear_tags" );

    tags_str.clear();

    // Уборка мусора.
    lua_gc( PAC_Lua_state, LUA_GCRESTART, 0 );
    lua_gc( PAC_Lua_state, LUA_GCCOLLECT, 0 );

    dev_synch_access->Done();
    return res;
    }
//-----------------------------------------------------------------------------
void PAC_cmmctr::set_tag_Lua_cmd( const char *cmd )
    {
    dev_synch_access->WaitToWrite();

    exec_Lua_str( cmd, "set_value(...)" );

    const char* str_res = get_str_param_from_Lua(
        "res", "set_value(...)" );

    const int BUFF_SIZE = 1000;
    if ( str_res != 0 && strlen( str_res ) < BUFF_SIZE - 1 ) //Корректность строки скрипта.
        {
        char buff[ BUFF_SIZE ] = { 0 };
        buff[ 0 ] = device_communicator::CMD_EXEC_DEVICE_COMMAND;
        memcpy( buff + 1, str_res, strlen( str_res ) );

        cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, buff, 
            1 + strlen( str_res ) );
        }

    dev_synch_access->Done();
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::backup_PAC_params()
    {
    if ( !cmmctr ) return -1;

    char buff[ 1 ];
    buff[ 0 ] = device_communicator::CMD_GET_PARAMS;
    const int cmd_length = 1;
    cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, buff, cmd_length );

    unsigned int answer_size;
    char *answer = cmmctr->get_out_data( answer_size );

    char file_name[ 100 ];
    get_param_file_name( file_name, sizeof( file_name ) );

    save_to_file( file_name, answer );
    return 0;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::check_PAC_params()
    {
    char file_name[ 100 ];
    get_param_file_name( file_name, sizeof( file_name ) );

    FILE *f = fopen( file_name, "r" );
    if ( f == NULL ) 
        {
        // Не восстанавливаем параметры.
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Файл параметров \"%s\" отсутствует - не восстанавливаем их.",
            file_name );
        BUG_LOG.add_msg( get_name(), get_address() );  

        is_process_PAC_params = false;
        return 0;
        }
    else
        {
        is_process_PAC_params = true;
        }

    bool is_reset_params = get_int_param_from_Lua( "is_reset_params", 
        "int PAC_cmmctr::get_PAC_info()" ) > 0 ? true : false;
          
    if ( is_reset_params )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Параметры в PAC были сброшены к значениям по умолчанию." );

        BUG_LOG.add_warning_msg( PAC_name.c_str(), PAC_address.c_str() );

        //Передача в PAC сохраненных ранее параметров.
        fseek( f, 0, SEEK_END );
        int str_size = ftell( f );
        fseek( f, 0, SEEK_SET ) ; 

        char *str = new char[ str_size + 2 ];       
        memset( str, 0, str_size + 2 );

        fread( str + 1, sizeof( char ), str_size, f );

        bool params_restore_flag = false;

        str[ 0 ] = device_communicator::CMD_RESTORE_PARAMS;
        const int CMD_LENGTH = str_size + 2;
        int res = cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, str, CMD_LENGTH );                
        if ( 0 == res )
            {
            u_int answer_size = 0;
            char *answer = cmmctr->get_out_data( answer_size );

            if ( answer_size > 0 )
                { 
                if ( 0 == answer[ 0 ] && 0 == answer[ 1 ] )
                    {                    
                    get_param_file_name( file_name, sizeof( file_name ) );

                    snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                        "Параметры в PAC были успешно восстановлены из \"%s\".",
                        file_name );
                    BUG_LOG.add_msg( PAC_name.c_str(), PAC_address.c_str() );
                    params_restore_flag = true;

                    set_saved_CRC( get_PAC_params_CRC() );
                    }
                }
            }

        if ( false == params_restore_flag )
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                "Ошибка восстановления параметров в PAC." );
            BUG_LOG.add_error_msg( PAC_name.c_str(), PAC_address.c_str() );
            }
        }

    fclose( f );  

    return 0;
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::get_PAC_params_CRC()
    {
    if ( is_process_PAC_params )
        {
        const char CMD[] = { device_communicator::CMD_GET_PARAMS_CRC };
        cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, CMD, sizeof( CMD ) );         

        u_int answer_size = 0;
        char *answer = cmmctr->get_out_data( answer_size );

        if ( answer_size > 0 )
            { 
            int res = exec_Lua_str( answer,
                "Ошибка получения данных о контрольной сумме параметров PAC" );

            if( res != 0 )
                {
                return -2;
                }

            int crc = get_int_param_from_Lua( "params_CRC", 
                "int PAC_cmmctr::get_PAC_params_CRC()" );

            return crc;
            }
        }

    return -1;
    }
//-----------------------------------------------------------------------------
void PAC_cmmctr::get_param_file_name( char * file_name, int max_len )
    {

#ifdef DEBUG
    snprintf( file_name, max_len, "./Параметры PAC/%s.txt", 
        get_name() );
#else
    snprintf( file_name, max_len, "./drivers/Параметры PAC/%s.txt", 
        get_name() );
#endif // DEBUG    
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr::save_to_file( const char* file_name, const char * str )
    {
    FILE *f = fopen( file_name, "w+t" );
    if ( f == NULL ) 
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Не удалось сохранить параметры в файл \"%s\" - %s.",
            file_name, strerror( GetLastError() ) );
        BUG_LOG.add_error_msg( get_name(), get_address() );

        return -1;
        }

    fprintf( f, "%s", str );
    fclose( f );

    snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
        "Файл параметров \"%s\" обновлен.",
        file_name );
    BUG_LOG.add_msg( get_name(), get_address() );

    return 0;
    }

int PAC_cmmctr::get_PAC_errors()
    {
    dev_synch_access->WaitToWrite();

    char cmd[ 2 ];
    cmd[ 0 ] = device_communicator::CMD_GET_PAC_ERRORS;
    char descr_id = ( char ) get_description_id();
    cmd[ 1 ] = descr_id;

    cmmctr->send_2_PAC( PAC_CMMCTR_SERVICE_ID, cmd, sizeof( cmd ) );         

    u_int answer_size = 0;
    char *answer = cmmctr->get_out_data( answer_size );

    if ( answer_size > 0 )
        { 
        int res = g_alarm_manager->add_PAC_errors( answer, ( u_char ) descr_id );
        dev_synch_access->Done();
        return res;
        }

    dev_synch_access->Done();
    return 0;
    }

int PAC_cmmctr::set_alarm_cmd(int count, error_cmd* errors)
    {
    dev_synch_access->WaitToWrite();

    std::string Lua_str = { device_communicator::CMD_GET_PAC_ERRORS };

    for (int i = 0; i < count; i++)
        {
        size_t size = snprintf(nullptr, 0,
            "errors_manager:get_instance():set_cmd( %d, %d, %d, %d )\n",
            errors[i].cmd,
            errors[i].object_type, errors[i].object_number,
            errors[i].object_alarm_number);
        if (size > 0)
            {
            char* cmd_str = new char[size + 1];
            snprintf(cmd_str, size + 1,
                "errors_manager:get_instance():set_cmd( %d, %d, %d, %d )\n",
                errors[i].cmd,
                errors[i].object_type, errors[i].object_number,
                errors[i].object_alarm_number);
            Lua_str += cmd_str;
            delete [] cmd_str;
            }
        }

    const int SERVICE_ID = 1;

    cmmctr->send_2_PAC(SERVICE_ID, Lua_str.c_str(), Lua_str.length());

    dev_synch_access->Done();

    get_PAC_errors();
    return 0;
    }

int PAC_cmmctr::get_alarms(all_alarm& alarms)
    {
    dev_synch_access->WaitToRead();
    g_alarm_manager->get_alarms((unsigned char) this->PAC_descr_id, alarms);
    dev_synch_access->Done();

    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PAC_cmmctr_group::PAC_cmmctr_group()
    {    
    for ( int i = 0; i < MAX_PAC_DESCR_NUMBER; i++ )
        {
        PAC_descriptions[ i ] = 0;
        }
    }
//-----------------------------------------------------------------------------
PAC_cmmctr* PAC_cmmctr_group::add_PAC( char* const PAC_address, 
    char* const PAC_name, UCHAR PAC_descr_id, int PAC_port, int timeout )
    {
    //Создаем новый коммуникатор.
    PAC_cmmctr *res = new PAC_cmmctr( PAC_address, PAC_name, PAC_descr_id,
        PAC_port, timeout );  

    snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
        "Новое описание PAC (№%d) было добавлено. Таймаут опроса - %d мсек.",
        res->get_description_id(), timeout );
    BUG_LOG.add_msg( res->get_name(), res->get_address() );

    PAC_descriptions[ PAC_descr_id ] = res;    

    return res;  //Возвращаем добавленный контроллер.
    }
//-----------------------------------------------------------------------------
PAC_cmmctr* PAC_cmmctr_group::get_PAC( int descr_id )
    {
    return PAC_descriptions[ descr_id ];    
    }
//-----------------------------------------------------------------------------
int PAC_cmmctr_group::remove_PAC( UCHAR PAC_descr_id )
    {
    delete PAC_descriptions[PAC_descr_id];
    PAC_descriptions[PAC_descr_id] = 0;
    
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
char* abstract_cmmctr::get_out_data( unsigned int &cnt ) 
    {    
    cnt = answer_size;
    return in_buff;
    }
//-----------------------------------------------------------------------------
abstract_cmmctr::abstract_cmmctr( const char* PAC_name, int timeout ): id( id ),
    timeout( timeout )
    {
    this->PAC_name = 0;
    int name_length = strlen( PAC_name );
    this->PAC_name = new char[ name_length + 1 ];

    strcpy( this->PAC_name, "?" );
    strcpy( this->PAC_name, PAC_name );

    count++;
    id = count;
    }
//-----------------------------------------------------------------------------
int abstract_cmmctr::get_timeout() const
    {
    return timeout;
    }
//-----------------------------------------------------------------------------
void abstract_cmmctr::set_protocol_version( int version )
    {
    PAC_protocol_version = version;
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
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
                "Ошибка инициализации сетевой библиотеки: %s\n",
                WSA_Err_Decode( WSAGetLastError() ) );

            BUG_LOG.add_error_msg( "Driver", "" );            
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

    state_decompress =
        (qlz_state_decompress*)malloc( sizeof( qlz_state_decompress ) );

    InitLib();
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
int tcp_cmmctr::send_2_PAC( UCHAR Service_ID, const char *data, UINT length )
    {
    EnterCriticalSection( &m_cs );

    answer_size = 0;
    if ( !isInitialized )
        {
        if (!InitLib() )
            {
            sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
                "Фатальная ошибка. Сетевая библиотека не инициализирована." );
            BUG_LOG.add_error_msg( "Driver", "" );

            LeaveCriticalSection( &m_cs );
            return 1;
            }
        }

    if ( !isConnected )
        {
        if ( !Connect() )
            {
            BUG_LOG.set_error( is_errors[ EF_NO_CONNECTION ], PAC_name, 
                ip_address, "Нет связи!" );

            LeaveCriticalSection( &m_cs );
            return 1;
            }    

        //Сбрасываем ошибку.
        BUG_LOG.reset_error( is_errors[ EF_NO_CONNECTION ], PAC_name, 
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
        BUG_LOG.set_error( is_errors[ EF_SEND_ERROR ], PAC_name, 
            ip_address, "Ошибка отсылки сообщения!" );

        Disconnect();

        LeaveCriticalSection( &m_cs );
        return 1;
        }
    BUG_LOG.reset_error( is_errors[ EF_SEND_ERROR ], PAC_name, 
        ip_address, "Ошибка отсылки сообщения!" );

    static int recv_err_cnt = 0;
    int RECV_MAX_ERR_CNT = 3;
    memset( in_buff, 0, sizeof( in_buff ) );
    int res = recv( sock, in_buff, 8000, 0 );

    if ( 0 == res )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE,
            "PAC закрыл соединение." );
        BUG_LOG.add_msg( PAC_name, ip_address );
        Disconnect();

        LeaveCriticalSection( &m_cs );
        return 1;
        }

    if ( res < 0 /*res == SOCKET_ERROR*/ )
        {
        BUG_LOG.set_error( is_errors[ EF_ANSWER_ERROR ], PAC_name, 
            ip_address, "Ошибка получения ответа!" );
        recv_err_cnt++;

        if ( recv_err_cnt >= RECV_MAX_ERR_CNT )
            {
            BUG_LOG.add_warning_msg( PAC_name, 
                ip_address, "Соединение разорвано." );            
            Disconnect();            
            }    

        LeaveCriticalSection( &m_cs );
        return 1;
        }
    recv_err_cnt = 0;
    BUG_LOG.reset_error( is_errors[ EF_ANSWER_ERROR ], PAC_name, 
        ip_address, "Ошибка получения ответа!" );


    if ( in_buff[ 1 ] == 7 )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Возвращена ошибка!" );
        BUG_LOG.add_warning_msg( PAC_name, ip_address );

        LeaveCriticalSection( &m_cs );
        return 1;
        }    
    unsigned char *work_buff = ( unsigned char* ) in_buff;

    //-Проверка на правильность заголовка блока данных от PAC.
    if ( work_buff[ 0 ] != 's' )                  //NetId         
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Возвращен ответ с неверным заголовком!" );        
        BUG_LOG.add_warning_msg( PAC_name, ip_address );
        Disconnect();
#ifdef DEBUG
        //        _DebugBreak();
#endif

        LeaveCriticalSection( &m_cs );
        return 1;
        }

    if ( pidx != work_buff[ 2 ] )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Возвращен ответ номер %d, а ожидался %d!",
            work_buff[ 2 ], pidx );        
        BUG_LOG.add_warning_msg( PAC_name, ip_address );
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
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Длина ответа - 0!" );
        BUG_LOG.add_warning_msg( PAC_name, ip_address );
#ifdef DEBUG
        //_DebugBreak();
#endif
        LeaveCriticalSection( &m_cs );
        return 1;
        }

    if ( answer_size > P_MAX_BUFFER_SIZE )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Длина ответа[ %d ] > максимальной длины[ %d ]!", 
            answer_size, P_MAX_BUFFER_SIZE );        
        BUG_LOG.add_warning_msg( PAC_name, ip_address );
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
            sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
                "Получена часть ответа от PAC!" );
            BUG_LOG.add_warning_msg( PAC_name, ip_address );

            answer_size = 0;

            LeaveCriticalSection( &m_cs );
            return 1;
            }

        tmp_buff += res;      
        tmp_answer_size -= res;   
        }   

    int buff_req_size = 0;
    unsigned long r = 0;

    switch ( PAC_protocol_version )
        {
        case G_QLZ_VERSION:
            buff_req_size = qlz_size_decompressed( in_buff + 5 );
            if ( buff_req_size >= P_MAX_BUFFER_SIZE )
                {
                CString tmp;
                tmp.Format( "Размер после декомпрессии превышает размер буфера (%d>%d)! "
                    "Данные от PAC потеряны.",
                    buff_req_size, P_MAX_BUFFER_SIZE );
                BUG_LOG.add_warning_msg( PAC_name, ip_address, tmp );

                answer_size = 0;
                }
            else
                {
                r = qlz_decompress( in_buff + 5, buff, state_decompress );

                if ( 0 == r )
                    {
                    sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE,
                        "Ошибка декомпрессии!" );
                    BUG_LOG.add_warning_msg( PAC_name, ip_address );

                    answer_size = 0;
                    }
                }
            break;

        case  G_CURRENT_PROTOCOL_VERSION:
            {
            r = sizeof( buff );
            res = uncompress( (u_char*)buff, &r, (u_char*)in_buff + 5, tmp_answer_size - 5 );

            if ( res != Z_OK )
                {
                char err_str[ 20 ] = "";
                switch ( res )
                    {
                    case Z_ERRNO:
                        sprintf( err_str, "%s", "Z_ERRNO" );
                        break;
                    case Z_STREAM_ERROR:
                        sprintf( err_str, "%s", "Z_STREAM_ERROR" );
                        break;
                    case Z_DATA_ERROR:
                        sprintf( err_str, "%s", "Z_DATA_ERROR" );
                        break;
                    case Z_MEM_ERROR:
                        sprintf( err_str, "%s", "Z_MEM_ERROR" );
                        break;
                    case Z_BUF_ERROR:
                        sprintf( err_str, "%s", "Z_BUF_ERROR" );
                        break;
                    case Z_VERSION_ERROR:
                        sprintf( err_str, "%s", "Z_VERSION_ERROR" );
                        break;
                    default:
                        sprintf( err_str, "%d", res );
                        break;
                    }
                sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE,
                    "Ошибка декомпрессии (%s)!", err_str );
                BUG_LOG.add_warning_msg( PAC_name, ip_address );

                answer_size = 0;
                }
                break;

        case G_UNKNOWN_PROTOCOL_VERSION:
            r = sizeof( buff );
            res = uncompress( (u_char*)buff, &r, (u_char*)in_buff + 5, tmp_answer_size - 5 );

            if ( res != Z_OK )
                {
                buff_req_size = qlz_size_decompressed( in_buff + 5 );
                if ( buff_req_size >= P_MAX_BUFFER_SIZE )
                    {
                    CString tmp;
                    tmp.Format( "Размер после декомпрессии превышает размер буфера (%d>%d)! "
                        "Данные от PAC потеряны.",
                        buff_req_size, P_MAX_BUFFER_SIZE );
                    BUG_LOG.add_warning_msg( PAC_name, ip_address, tmp );

                    answer_size = 0;
                    }
                else
                    {
                    r = qlz_decompress( in_buff + 5, buff, state_decompress );

                    if ( 0 == r )
                        {
                        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE,
                            "Ошибка декомпрессии!" );
                        BUG_LOG.add_warning_msg( PAC_name, ip_address );

                        answer_size = 0;
                        }
                    }
                }
            break;
            }
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
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE,
            "Ошибка создания сокета!" );        
        BUG_LOG.add_error_msg( PAC_name, ip_address );
        return 0;
        }

    int vlen = sizeof( timeout );

    if ( setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, ( char* )&timeout, vlen) == SOCKET_ERROR ||
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout, vlen) == SOCKET_ERROR )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE,
            "Ошибка установления параметров сокета!" );        
        BUG_LOG.add_error_msg( PAC_name, ip_address );
        return 0;
        }

    //Переводим сокет в неблокирующий режим.
    u_long mode = 1;
    int res = ioctlsocket( sock, FIONBIO, &mode );
    if ( res == SOCKET_ERROR )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Ошибка перевода сокета в неблокирующий режим!" );        
        BUG_LOG.add_error_msg( PAC_name, ip_address );

        closesocket( sock );
        return 0;
        }

    sockaddr_in sock_address;
    memset(&sock_address,0,sizeof(sockaddr_in));
    sock_address.sin_family  = AF_INET;
    sock_address.sin_port = htons( ( u_short ) port);
    sock_address.sin_addr.s_addr = inet_addr(ip_address);

    connect( sock, ( SOCKADDR* ) &sock_address, sizeof( sockaddr_in ) );
    //res = connect( sock, ( SOCKADDR* ) &sock_address, sizeof( sockaddr_in ) );
    //if ( res == SOCKET_ERROR )
    //    {
    //    snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
    //        "Ошибка подключения - %s", WSA_Err_Decode( WSAGetLastError() ) );        
    //    BUG_LOG.add_error_msg( PAC_name, ip_address );
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
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Ошибка перевода сокета в блокирующий режим!" );
        BUG_LOG.add_error_msg( PAC_name, ip_address );
        }   

    if ( SOCKET_ERROR == rc || 0 == rc )
        {
        BUG_LOG.set_error( is_errors[ EF_CONNECT_ERROR ], PAC_name, 
            ip_address, "Ошибка установления соединения!" );

        closesocket( sock );
        return 0;
        }
    BUG_LOG.reset_error( is_errors[ EF_CONNECT_ERROR ], PAC_name, 
        ip_address, "Ошибка установления соединения!" );

    res = recv( sock, in_buff, 255, 0 );
    if (SOCKET_ERROR == res)
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Ошибка получения ответа при подключении!" );
        BUG_LOG.add_warning_msg( PAC_name, ip_address );

        closesocket( sock );

        return 0;
        }
    isConnected = 1;
    return 1;
    }
//-----------------------------------------------------------------------------
int tcp_cmmctr::recvtimeout( UINT s, char *buf, int len, int _timeout, int usec ) 
    { 
    fd_set fds; 
    int n; 
    struct timeval tv; 

    // настраиваем  file descriptor set 
    FD_ZERO( &fds ); 
    FD_SET( s, &fds ); 

    // настраиваем время на таймаут 
    tv.tv_sec = _timeout; 
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
    return buff;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


