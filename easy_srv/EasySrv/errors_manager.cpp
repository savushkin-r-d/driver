#include "errors_manager.h"

#if _MSC_VER == 1700
#define snprintf _snprintf
#endif // _MSC_VER

alarm_manager *g_alarm_manager; ///< Работа с ошибками контроллеров.

alarm   g_alarms[ MAX_PROJECTS_CNT ][ MAX_ALARMS_CNT ];
int     g_active_alarms_cnt[ MAX_PROJECTS_CNT ];
int     g_active_alarms_id[ MAX_PROJECTS_CNT ];

extern PAC_cmmctr_group *g_PAC_descriptions;		///< Контроллеры сервера.

extern int  tolua_PAC_dev_open ( lua_State* tolua_S );
//-----------------------------------------------------------------------------
alarm_manager* G_ALARM_MANAGER()
    {
    return g_alarm_manager;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int save_to_stream( alarm &a, char *buff )
    {
    int len = 0;
    int str_len_0 = 0;

    memcpy( buff + len, &a.params, sizeof( a.params ) );
    len += sizeof( a.params );    
    memcpy( buff + len, &a.type, sizeof( a.type ) );
    len += sizeof( a.type );

    str_len_0 = strlen( a.description ) + 1; 
    memcpy( buff + len, a.description, str_len_0 );
    len += str_len_0;

    memcpy( buff + len, &a.enable, sizeof( a.enable ) );
    len += sizeof( a.enable );

    str_len_0 = strlen( a.group ) + 1; 
    memcpy( buff + len, a.group, str_len_0 );
    len += str_len_0;

    memcpy( buff + len, &a.inhibit, sizeof( a.inhibit ) );
    len += sizeof( a.inhibit );
    memcpy( buff + len, &a.priority, sizeof( a.priority ) );
    len += sizeof( a.priority );
    memcpy( buff + len, &a.state, sizeof( a.state ) );
    len += sizeof( a.state );
    memcpy( buff + len, &a.suppress, sizeof( a.suppress ) );
    len += sizeof( a.suppress );
    memcpy( buff + len, &a.id, sizeof( a.id ) );
    len += sizeof( a.id );
    memcpy( buff + len, &a.driver_id, sizeof( a.driver_id ) );
    len += sizeof( a.driver_id );

    return len;
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
//-----------------------------------------------------------------------------
alarm_manager::alarm_manager(): lua_synch_access( new CSWMRG )
    {
    lua_state = lua_open();  /* create state */
    if ( lua_state == NULL )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Cannot create Lua state: not enough memory!" );

        BUG_LOG.add_error_msg( "System", "" );

#ifdef DEBUG
        DebugBreak();
#endif // DEBUG
        }

    lua_gc( lua_state, LUA_GCSTOP, 0 );  /* stop collector during initialization */
    luaL_openlibs( lua_state );          /* open libraries */
    lua_gc( lua_state, LUA_GCRESTART, 0 );    

    tolua_PAC_dev_open( lua_state );

    memset( g_alarms, 0, sizeof( g_alarms ) );

    for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
        {
        for ( int j = 0; j < MAX_ALARMS_CNT; j++ )
            {
            g_alarms[ i ][ j ].description = new char[ MAX_DESCR_LEN ];
            g_alarms[ i ][ j ].group = new char[ MAX_GROUP_LEN ];
            }
        }
    }
//-----------------------------------------------------------------------------
alarm_manager::~alarm_manager()
    {
    lua_close( lua_state );
    lua_state = 0;

    for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
        {
        for ( int j = 0; j < MAX_ALARMS_CNT; j++ )
            {
            delete g_alarms[ i ][ j ].description;
            delete g_alarms[ i ][ j ].group;

            g_alarms[ i ][ j ].description = 0;
            g_alarms[ i ][ j ].group = 0;
            }
        }
    }
//-----------------------------------------------------------------------------
int alarm_manager::add_no_PAC_connection_error( const char *PAC_name, 
    UINT project_description_id )
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
    sprintf( str + strlen( str ), "%s%s%s\n", 
        "description = \"Нет связи с контроллером проекта '", PAC_name, "'!\"," );

    sprintf( str + strlen( str ), "%s\n", "type        = AT_SPECIAL," );
    sprintf( str + strlen( str ), "%s\n", "group       = 'Авария'," );
    sprintf( str + strlen( str ), "%s\n", "priority    = 1," );
    sprintf( str + strlen( str ), "%s\n", "state       = AS_ALARM," );
    sprintf( str + strlen( str ), "%s\n", "}" );

    lua_synch_access->WaitToWrite();

    int res = luaL_dostring( lua_state, str ); 

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Cannot create ""NO PAC RESPOND"", error - ""%s""!",
            lua_tostring( lua_state, -1 ) );

        BUG_LOG.add_error_msg( "System", 
            g_PAC_descriptions->get_PAC( project_description_id )->get_address() );
#ifdef DEBUG
        DebugBreak();
#endif // DEBUG

        lua_synch_access->Done();
        return 1;
        }

    lua_synch_access->Done();
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

    lua_synch_access->WaitToWrite();
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

        lua_synch_access->Done();
        return 1;
        }

    lua_synch_access->Done();
    return 0;
    }
//-----------------------------------------------------------------------------
int alarm_manager::sync_alarms( u_char PAC_id )
    {
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

    lua_synch_access->WaitToWrite();
    lua_getfield( lua_state, LUA_GLOBALSINDEX, "get_alarms" );   
    lua_pushnumber( lua_state, PAC_id );
    lua_pushnumber( lua_state, g_active_alarms_id[ PAC_id ] );
    int res = lua_pcall( lua_state, 2, 0, 0 );

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "get_alarms(...) error - '%s'!",
            lua_tostring( lua_state, -1 ) );

        BUG_LOG.add_error_msg( "System", "" );

#ifdef DEBUG
        DebugBreak();
#endif // DEBUG       
        }
    lua_synch_access->Done();

    return 0;
    }
//-----------------------------------------------------------------------------
int alarm_manager::add_PAC_errors( const char *LUA_str, 
    unsigned char project_description_id )
    {
    lua_synch_access->WaitToWrite();

    static unsigned long int gc_counter = 0;
    gc_counter++;
    if ( gc_counter > AM_GARBAGE_CYCLE )
        {
        // Полная уборка мусора каждые n итераций.
        lua_gc( lua_state, LUA_GCCOLLECT, 0 ); 
        gc_counter = 0;
        }

    int res = luaL_dostring( lua_state, LUA_str ); 

    if( res != 0 )
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Cannot process PAC errors, error - ""%s""!",
            lua_tostring( lua_state, -1 ) );

        BUG_LOG.add_error_msg( "System", 
            g_PAC_descriptions->get_PAC( project_description_id )->get_address() );

        BUG_LOG.add_msg_once( "System", 
            g_PAC_descriptions->get_PAC( project_description_id )->get_address(),
            LUA_str );
#ifdef DEBUG
        DebugBreak();
#endif // DEBUG

        lua_synch_access->Done();
        return 1;
        }

    lua_synch_access->Done();
    return 0;
    }
//-----------------------------------------------------------------------------
int alarm_manager::save_to_stream( unsigned char PAC_description_id, char *buff )
    {
    int idx = 0;
    memcpy( buff, &g_active_alarms_cnt[ PAC_description_id ], sizeof( int ) );
    idx += sizeof( int );
    memcpy( buff + idx, &g_active_alarms_id[ PAC_description_id ], sizeof( int ) );
    idx += sizeof( int );

    for ( int i = 0; i < g_active_alarms_cnt[ PAC_description_id ]; i++ )
    	{
        idx += ::save_to_stream( g_alarms[ PAC_description_id ][ i ], buff + idx );
    	}    

    return idx;
    }
//-----------------------------------------------------------------------------
int alarm_manager::set_alarm( unsigned char PAC_description_id, int n,
                             ALARM_TYPE a_type, char * a_description, 
                             char * a_group,
                             u_char a_enable,
                             bool a_suppress, 
                             u_char a_inhibit, 
                             int a_priority,
                             ALARM_STATE a_state,
                             u_char a_driver_id, 
                             int a_id_object_type,
                             int a_id_object_number,
                             int a_id_object_alarm_number )
    {
    if ( PAC_description_id < MAX_PROJECTS_CNT && n < MAX_ALARMS_CNT )
        {

        g_alarms[ PAC_description_id ][ n ].type = a_type;

        strncpy( g_alarms[ PAC_description_id ][ n ].description, a_description, MAX_DESCR_LEN );
        strncpy( g_alarms[ PAC_description_id ][ n ].group, a_group, MAX_GROUP_LEN );

        g_alarms[ PAC_description_id ][ n ].enable = a_enable;
        g_alarms[ PAC_description_id ][ n ].suppress = a_suppress;                    
        g_alarms[ PAC_description_id ][ n ].inhibit = a_inhibit;                        
        g_alarms[ PAC_description_id ][ n ].priority = a_priority;  
        g_alarms[ PAC_description_id ][ n ].state = a_state; 
        g_alarms[ PAC_description_id ][ n ].driver_id = a_driver_id; 

        g_alarms[ PAC_description_id ][ n ].id.object_type = 
            a_id_object_type;
        g_alarms[ PAC_description_id ][ n ].id.object_number = 
            a_id_object_number;
        g_alarms[ PAC_description_id ][ n ].id.object_alarm_number =
            a_id_object_alarm_number;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
int alarm_manager::set_alarms_id(unsigned char PAC_description_id, int id)
    {
    g_active_alarms_id[ PAC_description_id ] = id;

    return 0;
    }

int alarm_manager::set_alarms_cnt( unsigned char PAC_description_id, int cnt )
    {
    g_active_alarms_cnt[ PAC_description_id ] = cnt;

    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
