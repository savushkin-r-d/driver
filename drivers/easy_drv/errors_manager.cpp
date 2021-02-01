#include "stdafx.h"
//-----------------------------------------------------------------------------
alarm_manager::alarm_manager()
    {
    for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
        {
        ///< Экземпляр Lua для работы с ошибками.)
        lua_states[ i ] = lua_open();  /* create state */
        if ( lua_states[ i ] == NULL )
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
                "Cannot create Lua state: not enough memory!" );

            BUG_LOG.add_error_msg( "System", "" );

#ifdef DEBUG
            DebugBreak();
#endif // DEBUG
            }

        lua_gc( lua_states[ i ], LUA_GCSTOP, 0 );  /* stop collector during initialization */
        luaL_openlibs( lua_states[ i ] );          /* open libraries */
        lua_gc( lua_states[ i ], LUA_GCRESTART, 0 );

        tolua_PAC_dev_open( lua_states[ i ] );
        }

    memset( g_alarms, 0, sizeof( g_alarms ) );
    }
//-----------------------------------------------------------------------------
alarm_manager::~alarm_manager()
    {
    for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
        {
        lua_close( lua_states[ i ] );
        lua_states[ i ] = 0;
        }
    }
//-----------------------------------------------------------------------------
int alarm_manager::add_no_PAC_connection_error( const char *PAC_name, 
    UINT project_description_id, const char* PAC_IP_address,
    int PAC_protocol_version )
    {
    const int MAX_SIZE   = 2000;
    char str[ MAX_SIZE ] = { 0 };

    //--alarms[ project_description_id ]
    sprintf( str, "%s %d %s\n",
        "alarms[", project_description_id, "] = {}" );
    sprintf( str + strlen( str ), "%s %d %s\n",
        "alarms[", project_description_id, "].id = 1" );

    sprintf( str + strlen( str ), "%s %d %s\n",
        "alarms[", project_description_id, "][ 1 ] = " );

    sprintf( str + strlen( str ), "%s\n", "{" );
    if ( PAC_protocol_version > G_NON_UNICODE_VERSION )
        {
        const int MAX_STR_SIZE = 500;
        char PAC_name_utf8[ MAX_STR_SIZE ] = "";

        convert_windows1251_to_utf8(PAC_name_utf8, PAC_name);
        sprintf(str + strlen(str), "%s%s%s%s%s\n",
            u8"description = \"Нет связи с контроллером проекта '", PAC_name_utf8,
            "' (", PAC_IP_address, ")!\",");
        sprintf(str + strlen(str), "%s\n", u8"group       = 'Авария',");
        }
    else
        {
        sprintf(str + strlen(str), "%s%s%s%s%s\n",
            "description = \"Нет связи с контроллером проекта '", PAC_name,
            "' (", PAC_IP_address, ")!\",");

        sprintf(str + strlen(str), "%s\n", "group       = 'Авария',");
        }

    sprintf( str + strlen( str ), "%s\n", "type        = AT_SPECIAL," );   
    sprintf( str + strlen( str ), "%s\n", "priority    = 1," );
    sprintf( str + strlen( str ), "%s\n", "state       = AS_ALARM," );
    sprintf( str + strlen( str ), "%s\n", "}" );

    lua_State* lua_state = lua_states[ project_description_id ];
    int res = luaL_dostring( lua_state, str ); 

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Cannot create ""NO PAC RESPOND"", error - ""%s""!",
            lua_tostring( lua_state, -1 ) );

        BUG_LOG.add_error_msg( "System", 
            g_PAC_descriptions->get_PAC( project_description_id )->get_address() );
#ifdef DEBUG
        //DebugBreak();
#endif // DEBUG

        return 1;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
int alarm_manager::remove_no_PAC_connection_error( UINT project_description_id )
    {
    const int MAX_SIZE   = 200;
    char str[ MAX_SIZE ] = { 0 };

    //--alarms[ project_description_id ][ object_type ][ object_number ][ alarm_class ]
    sprintf( str, "%s %d %s\n",
        "alarms[", project_description_id, "] = {}" );
    sprintf( str + strlen( str ), "%s %d %s\n",
        "alarms[", project_description_id, "].id = 2" );

    lua_State* lua_state = lua_states[ project_description_id ];
    int res = luaL_dostring( lua_state, str ); 

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Cannot remove ""NO PAC RESPOND"" error!" );
        BUG_LOG.add_error_msg( "System", 
            g_PAC_descriptions->get_PAC( project_description_id )->get_address() );
#ifdef DEBUG
        DebugBreak();
#endif // DEBUG

        return 1;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
int alarm_manager::get_alarms( unsigned char project_description_id, 
    all_alarm &project_alarms, int PAC_protocol_version)
    {
    lua_State* lua_state = lua_states[ project_description_id ];

#ifdef DEBUG_LUA_MEM
    static int counter = 0;
    counter++;
    if ( counter > 1000 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, "Lua memory (alarms) = %d", 
            lua_gc( lua_state, LUA_GCCOUNT, 0 ) * 1024 +
            lua_gc( lua_state, LUA_GCCOUNTB, 0 ) );
        BUG_LOG.add_msg( "System", "Control thread" );

        counter = 0;
        }
#endif // DEBUG_LUA_MEM

    u_int_2 id = 0;

    lua_getfield( lua_state, LUA_GLOBALSINDEX, "get_alarms_id" );  
    lua_pushnumber( lua_state, project_description_id );
    int res = lua_pcall( lua_state, 1, 1, 0 );

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "get_alarms(...) error - '%s'!",
            lua_tostring( lua_state, -1 ) );

        BUG_LOG.add_error_msg( "System", 
            g_PAC_descriptions->get_PAC( project_description_id )->get_address() );
#ifdef DEBUG
        DebugBreak();
#endif // DEBUG       
        }
    else
        {
        id = ( u_int_2 ) tolua_tonumber( lua_state, -1, 0 );
        lua_remove( lua_state, -1 );
        }

    unsigned int alarms_cnt = 0;

    lua_getfield( lua_state, LUA_GLOBALSINDEX, "get_alarms_cnt" );  
    lua_pushnumber( lua_state, project_description_id );
    res = lua_pcall( lua_state, 1, 1, 0 );

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "get_alarms(...) error - '%s'!",
            lua_tostring( lua_state, -1 ) );

        BUG_LOG.add_error_msg( "System", 
            g_PAC_descriptions->get_PAC( project_description_id )->get_address() );
#ifdef DEBUG
        DebugBreak();
#endif // DEBUG
        }
    else
        {
        alarms_cnt = ( unsigned int ) tolua_tonumber( lua_state, -1, 0 );
        lua_remove( lua_state, -1 );
        }

    //Проверка на равенство уже полученных ошибок.
    if ( g_alarms[ project_description_id ] != 0 )
        {
        if ( g_alarms_id[ project_description_id ] == id )
            {
            project_alarms.alarms = g_alarms[ project_description_id ];
            project_alarms.cnt    = alarms_cnt;            
            project_alarms.id     = id;
            
            return 0;
            }
        }

    g_alarms_id[ project_description_id ] = id;

    delete [] g_alarms[ project_description_id ];
    g_alarms[ project_description_id ] = 0;
    if ( alarms_cnt )
        {                
        g_alarms[ project_description_id ] = new alarm[ alarms_cnt ];

        for ( unsigned int i = 0; i < alarms_cnt; i++ )
            {
            lua_getfield( lua_state, LUA_GLOBALSINDEX, "get_alarm" );  
            lua_pushnumber( lua_state, project_description_id );
            lua_pushnumber( lua_state, i + 1 );
            res = lua_pcall( lua_state, 2, 1, 0 );

            if( res != 0 )
                {                    
                snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                    "get_alarms(...) error - '%s'!",
                    lua_tostring( lua_state, -1 ) );

                BUG_LOG.add_error_msg( "System", 
                    g_PAC_descriptions->get_PAC( project_description_id )->get_address() );
#ifdef DEBUG
                DebugBreak();
#endif // DEBUG
                break;
                }
            else
                {
                alarm* new_alarm = (alarm*)tolua_tousertype(lua_state, -1, 0);
                lua_remove(lua_state, -1);

                g_alarms[project_description_id][i] = *new_alarm;

                if (PAC_protocol_version > G_NON_UNICODE_VERSION)
                    {
                    static char tmp_str[MAX_DESCR_LEN];
                    memset(tmp_str, 0, MAX_DESCR_LEN);
                    int result =
                        convert_utf8_to_windows1251(new_alarm->description, tmp_str, MAX_DESCR_LEN);
                    if ( result == 1 )
                        {
                        strcpy(new_alarm->description, tmp_str);
                        }
                    }
                }
            }
        }

    project_alarms.alarms = g_alarms[ project_description_id ];
    project_alarms.cnt    = alarms_cnt;
    project_alarms.id     = id;

    return 0;
    }
//-----------------------------------------------------------------------------
int alarm_manager::add_PAC_errors( const char *LUA_str, 
    unsigned char project_description_id )
    {
    lua_State* lua_state = lua_states[ project_description_id ];

    static unsigned long int gc_counter[ MAX_PROJECTS_CNT ] = { 0 };
    gc_counter[ project_description_id ]++;
    if ( gc_counter[ project_description_id ] > AM_GARBAGE_CYCLE )
        {
        // Полная уборка мусора каждые n итераций.
        lua_gc( lua_state, LUA_GCCOLLECT, 0 ); 
        gc_counter[ project_description_id ] = 0;
        }

    int res = luaL_dostring( lua_state, LUA_str ); 

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Cannot process PAC errors, error - ""%s""!",
            lua_tostring( lua_state, -1 ) );

        BUG_LOG.add_error_msg( 
            g_PAC_descriptions->get_PAC( project_description_id )->get_name(),
            g_PAC_descriptions->get_PAC( project_description_id )->get_address() );

        BUG_LOG.add_msg_once(
            g_PAC_descriptions->get_PAC( project_description_id )->get_name(),
            g_PAC_descriptions->get_PAC( project_description_id )->get_address(),
            LUA_str );
#ifdef DEBUG
        DebugBreak();
#endif // DEBUG

        return 1;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------