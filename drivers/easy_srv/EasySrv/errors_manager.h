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

#include "PAC-driver\errors.h"
#include "exchange_data.h"
#include "PAC_cmmctr.h"

#define lua_c

extern "C" 
    {
#ifdef USE_STDAFX
#include "stdafx.h" //Стандартный заголовочный файл для использования precompiled headers.
#endif //USE_STDAFX

#include  "lua.h"
#include  "lauxlib.h"
#include  "lualib.h"

#include "tolua++.h"
    };

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
            UINT project_description_id );
        
        /// @brief Удаление ошибки отсутствия соединения с PAC.
        ///
        /// @param project_description_id [ in ] - номер описания в базе 
        /// каналов.
        ///
        /// @return 0 - ок.
        int remove_no_PAC_connection_error( UINT project_description_id );

        int add_PAC_errors( const char *LUA_str, 
            unsigned char project_description_id );

        int save_to_stream( unsigned char PAC_description_id, char *buff );

        int sync_alarms( u_char PAC_id );

        int set_alarm( unsigned char PAC_description_id, int n,            
            ALARM_TYPE a_type,
            char * a_description,
            char * a_group,
            u_char a_enable,
            bool   a_suppress,

            u_char a_inhibit,
            int    a_priority,
            ALARM_STATE a_state,

            u_char a_driver_id,

            int a_id_object_type,
            int a_id_object_number,
            int a_id_object_alarm_number );

        int set_alarms_id( unsigned char PAC_description_id, int id );
        int set_alarms_cnt( unsigned char PAC_description_id, int cnt );

    private:
        lua_State *lua_state;        ///< Экземпляр Lua для работы с ошибками.
        CSWMRG    *lua_synch_access; ///< Синхронизация обращений к Lua.
    };


alarm_manager* G_ALARM_MANAGER(); 

#endif // ERRORS_MANAGER_H
