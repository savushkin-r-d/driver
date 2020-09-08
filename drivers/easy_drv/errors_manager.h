/// @file errors_manager.h
/// @brief Получение ошибок от контроллера, обработка и передача их на сервер.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// @c DEBUG - компиляция c дополнительной отладочной информацией.
/// 
/// @par Текущая версия:
/// @$Rev: 363 $.\n
/// @$Author: id $.\n
/// @$Date:: 2011-09-07 15:52:24#$.

#ifndef ERRORS_MANAGER_H
#define ERRORS_MANAGER_H

#include "exchange_data.h"
#include "PAC_cmmctr.h"

#define lua_c

extern "C" 
    {
#include  "lua.h"
#include  "lauxlib.h"
#include  "lualib.h"

#include "tolua++.h"
    };

extern PAC_cmmctr_group *g_PAC_descriptions;  
extern alarm   *g_alarms[ 256 ];
extern u_int_2  g_alarms_id[ 256 ];

extern int  tolua_PAC_dev_open ( lua_State* tolua_S );
//-----------------------------------------------------------------------------
/// @brief Представление информации обо всех ошибках. 
class alarm_manager
    {
    public:

        enum AM_CONST
            {
            AM_MAX_COUNT = 256,
            
            /// @brief Число циклов получения ошибок, после которого 
            /// происходит уборка мусора.
            AM_GARBAGE_CYCLE = 100,
            };

        alarm_manager();

        ~alarm_manager();

        /// @brief Добавление ошибки отсутствия соединения с PAC.
        ///
        /// @param PAC_name [ in ] - имя PAC.
        /// @param project_description_id [ in ] - номер описания в базе 
        /// каналов.
        ///
        /// @return 0 - ок.
        /// @return 1 - ошибка добавления.
        int add_no_PAC_connection_error( const char *PAC_name, 
            UINT project_description_id, const char* PAC_IP_address );
        
        /// @brief Удаление ошибки отсутствия соединения с PAC.
        ///
        /// @param project_description_id [ in ] - номер описания в базе 
        /// каналов.
        ///
        /// @return 0 - ок.
        int remove_no_PAC_connection_error( UINT project_description_id );

        /// @brief Получение ошибок для их передачи на сервер.
        ///
        /// @param project_description_id [ in ] - номер описания в базе 
        /// каналов.
        /// @param project_alarms [ out ] - ошибка для данного описания проекта.
        ///
        /// @return 0 - ок.
        int get_alarms( unsigned char project_description_id, 
            all_alarm &project_alarms );


        int add_PAC_errors( const char *LUA_str, 
            unsigned char project_description_id );

    private:
        lua_State *lua_state;        ///< Экземпляр Lua для работы с ошибками.
    };

#endif // ERRORS_MANAGER_H
