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

        int add_no_PAC_connection_error( const char *description, 
            UINT project_id )
            {
            std::string str = "";

            //--alarms[ object_type ][ object_number ][ object_alarm_number ]
            str += "alarms[ OBJECT_TYPE.OT_PAC ] = alarms[ G_PAC ] or {}\n";
            
            char tmp[ 10 ] = { 0 };
            itoa( project_id, tmp, 10 );

            str += "alarms[ OBJECT_TYPE.OT_PAC ][ " + tmp + 
                " ][ ALARM_CLASS.AC_NO_CONNECTION ]  =\n";




            //alarms[ G_PAC ][ PAC_id ] = {}
            //alarms[ G_PAC ][ PAC_id ][ EC_NO_CONNECTION ]  =
            //    {
            //    description = "Ќет св€зи с контроллером проекта '—ыворотка'!",
            //    type        = ALARM_TYPE.AT_SPECIAL,
            //    group       = 'ќшибка св€зи',
            //    priority    = 1,
            //    state       = ALARM_STATE.AS_ALARM,
            //    suppress    = false,
            //    }


            return 0;
            }
        
        int remove_no_PAC_connection_error( const char *description,
            UINT driver_id )
            {

            return 0;
            }

        int get_alarms( unsigned char driver_id, all_alarm &project_alarms )
            {

            return 0;
            }

    private:
        lua_State *lua_state;    ///< Ёкземпл€р Lua дл€ PAC.
    };

#endif // ERRORS_MANAGER_H