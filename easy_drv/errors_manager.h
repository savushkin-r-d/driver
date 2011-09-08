#ifndef ERRORS_MANAGER_H
#define ERRORS_MANAGER_H

#include "exchange_data.h"

#define lua_c

extern "C" 
    {
#include "snprintf.h"

#include  "lua.h"
#include  "lauxlib.h"
#include  "lualib.h"
    };

extern PAC_cmmctr_group *g_PAC_descriptions;     //Контроллеры сервера.

extern int  tolua_PAC_dev_open ( lua_State* tolua_S );

class alarm_manager
    {
    public:

        enum AM_CONST
            {
            AM_MAX_COUNT = 256,
            };

        alarm_manager()
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
                        
            tolua_PAC_dev_open ( lua_state );

            const char* Lua_F =
                "alarms = {}\n"
                "--alarms[ object_type ][ object_number ][ alarm_class ][ alarm_subclass ]\n"
                "\n";

            int res = luaL_dostring( lua_state, Lua_F ); 

            if( res != 0 )
                {
                snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                    "Cannot init Lua state (PAC control thread)!" );

                BUG_LOG.add_error_msg( "System", "" );

#ifdef DEBUG
                DebugBreak();
#endif // DEBUG
                }
            }

        ~alarm_manager()
            {
            lua_close( lua_state );
            lua_state = 0;
            }

        int add_no_PAC_connection_error( const char *PAC_name, 
            UINT project_description_id )
            {
			const int MAX_SIZE   = 2000;
			char str[ MAX_SIZE ] = { 0 };

            //--alarms[ project_description_id ][ object_type ][ object_number ][ alarm_class ]
			sprintf( str, "%s %d %s\n",
				"alarms[", project_description_id, "] = {}" );

            sprintf( str + strlen( str ), "%s %d %s\n",
				"alarms[", project_description_id, "][ OBJECT_TYPE.OT_PAC ] = {}" );
			//sprintf( str + strlen( str ), "%s %d %s\n",
			//	"alarms[", project_description_id, "][ OBJECT_TYPE.OT_PAC ][ 1 ] = {}" );
   //         
			//sprintf( str + strlen( str ), "%s %d %s\n",
			//	"alarms[", project_description_id,
			//	"][ OBJECT_TYPE.OT_PAC ][ 1 ][ ALARM_CLASS.AC_NO_CONNECTION ]  =" );

			//sprintf( str + strlen( str ), "%s\n", "{" );
			//sprintf( str + strlen( str ), "%s\n", 
			//	"description = ""Нет связи с контроллером проекта '", PAC_name, "'!""," );
			//
			//sprintf( str + strlen( str ), "%s\n", "type        = ALARM_TYPE.AT_SPECIAL," );
			//sprintf( str + strlen( str ), "%s\n", "group       = 'Ошибка связи'," );
			//sprintf( str + strlen( str ), "%s\n", "priority    = 1," );
			//sprintf( str + strlen( str ), "%s\n", "state       = ALARM_STATE.AS_ALARM," );
			//sprintf( str + strlen( str ), "%s\n", "suppress    = false," );
			//
			//sprintf( str + strlen( str ), "%s\n", "}" );

			int res = luaL_dostring( lua_state, str ); 

			if( res != 0 )
			    {
				snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
					"Cannot create ""NO PAC RESPOND"" error!" );
				BUG_LOG.add_error_msg( "System", 
					g_PAC_descriptions->get_PAC( project_description_id )->get_address() );
#ifdef DEBUG
				DebugBreak();
#endif // DEBUG
			    }

            return 0;
            }
        
        int remove_no_PAC_connection_error( UINT project_description_id )
            {
            const int MAX_SIZE   = 200;
            char str[ MAX_SIZE ] = { 0 };

            //--alarms[ project_description_id ][ object_type ][ object_number ][ alarm_class ]
            sprintf( str, "%s %d %s\n",
                "alarms[", project_description_id, "] = {}" );

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
                }

            return 0;
            }

        int get_alarms( unsigned char driver_id, all_alarm &project_alarms )
            {

            return 0;
            }

    private:
        lua_State *lua_state;    ///< Экземпляр Lua для PAC.
    };

#endif // ERRORS_MANAGER_H