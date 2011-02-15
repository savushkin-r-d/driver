/// @file dllmain.cpp
/// @brief Содержит реализацию экспортируемых функций библиотеки.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// @c DEBUG - компиляция с выводом отладочной информации в консоль.@n
/// 
/// @par Текущая версия:
/// @$Rev: 153 $.\n
/// @$Author: id $.\n
/// @$Date:: 2010-10-14 12:46:29#$.

#include "stdafx.h"

#ifdef  __cplusplus
extern "C" {
#endif

#include "snprintf.h"

#ifdef  __cplusplus
    };
#endif

/// @brief Объявление функции как экспортируемой.
#define EXPORT extern "C" __declspec (dllexport) 

/// @brief Критическая секция для выполнения единожды инициализации библиотеки.
CRITICAL_SECTION g_init_cs; 

/// @brief Описание всех контроллеров сервера.
PAC_cmmctr_group *g_PAC_descriptions = 0; 

//-Данные для потоков, работающие с контроллерами.
/// @brief Признак завершения работы потоков обмена с PAC.
char   *g_thread_is_terminated = new char[ PAC_cmmctr_group::get_max_PAC_number() ];

/// @brief Потоки обмена с PAC.
HANDLE *g_commctr_threads_array = new HANDLE[ PAC_cmmctr_group::get_max_PAC_number() ];

/// @brief Количество потоков обмена с PAC.
int    g_commctr_threads_count = 0;
//-Данные для потоков, работающие с контроллерами.-!>

/// @brief Типы значения тега.
enum TAG_VAL_TYPE
    {
    T_ULONG, ///< Беззнаковое целое (unsigned int, 32 бита).
    T_FLOAT, ///< Вещественное (float, 32 бита).

    T_NUMBER,///< Вещественное (float, 32 бита).
    T_STRING,///< Строка.
    };
//-----------------------------------------------------------------------------
/// @brief Главная функция библиотеки.
///
BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
    {
    switch (ul_reason_for_call)
        {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
        }
    return TRUE;
    }
//-----------------------------------------------------------------------------
/// @brief Потоки обмена с PAC.
/// 
/// @param [in] lpParameter - экземпляр @ref PAC_cmmctr, с которым будет 
/// работать поток.
///
/// @return - код завершения потока.
DWORD WINAPI PAC_communication_thread( LPVOID lpParameter )
    {		        
#pragma chMSG( Тестирование комментария! )

    PAC_cmmctr *PAC_com = ( PAC_cmmctr* ) lpParameter;
    int res;

    // 1 - интервал опроса контроллера.
    // 2 - пороговое значение таймаута получения ответа.
    int sleep_time = 510;                            //1
    if ( PAC_com->get_cmmctr()->get_timeout() > 2000 /*2*/ )
        {
        sleep_time *= 2;
        }
    if ( PAC_com->get_cmmctr()->get_timeout() > 4000 /*2*/ )
        {
        sleep_time *= 3;
        }

    while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )
        {
        PAC_com->get_dev_synch_access()->WaitToWrite();
        res = PAC_com->get_PAC_info();//Получение информации от PAC.
        PAC_com->get_dev_synch_access()->Done();

        if ( res <= 0 )
            {
            Sleep( 2 * sleep_time );
            continue;
            }

        //Состояния устройств будут доступны после того, как мы получим 
        //всю необходимую информацию от контроллера.

        snprintf( bug_log::msg, bug_log::msg_size, 
            "Устройства PAC изменились." );
        bug_log::add_msg( PAC_com->get_name(), PAC_com->get_address() );

        PAC_com->get_dev_synch_access()->WaitToWrite();
        PAC_com->clear_tags(); // Очищаем все теги проекта.
        PAC_com->get_dev_synch_access()->Done();

        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   //Пытаемся получить все устройства контроллера.
            {            
            PAC_com->get_dev_synch_access()->WaitToWrite();
            res = PAC_com->get_PAC_devices();
            PAC_com->get_dev_synch_access()->Done();

            if ( PAC_cmmctr::LOAD_OK == res )
                {
                snprintf( bug_log::msg, bug_log::msg_size,
                    "Получены устройства PAC." );
                bug_log::add_msg( PAC_com->get_name(), PAC_com->get_address() );                
                break;
                }  

            Sleep( sleep_time );
            }

        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   //Пытаемся получить состояния всех устройств контроллера.
            {
            Sleep( sleep_time );
            PAC_com->get_dev_synch_access()->WaitToWrite();
            res = PAC_com->get_all_devices_states();
            PAC_com->get_dev_synch_access()->Done();

            if ( PAC_cmmctr::PAC_DEVICES_CHANGING == res )     
                {   
                break;
                }
            }

        //        Sleep( 10 );
        //        if ( PAC_program_version >= VERSION_WITH_ERRORS ) 
        //            {
        //            PAC_com->get_PAC_errors();
        //            }                
        //        }              
        } // while ( !*g_is_terminated )

    //if ( 1 == synch_access_needs_done )
    //    {
    //    g_server_tags[ PAC_com->get_prj_id() ].synch_access->Done();         
    //    }

    _endthreadex( 0 );
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его полного описания.
///
/// Внутренняя функция библиотеки.
///
/// @param [in] tag - полное описание тега.
///
/// @return - значение тега.
void* get_tag_value( in_tag_info &tag, TAG_VAL_TYPE tag_type )
    {
    // Проверяется, не превышает ли номер описания PAC максимальный (1). Далее
    // есть ли описание данного PAC (2). Если нет, то тогда он добавляется в 
    // список контроллеров проекта (3) и создается поток, который
    // взаимодействует с контроллером (4).
    // Проверяется есть ли значение tag.tag_id в интерпретаторе Lua (5),
    // Если есть, тогда возвращается значение тега (6), иначе проверяется 
    // есть ли переменная tag.tag_name в в интерпретаторе Lua (7). После ее
    // нахождения добавляется новый тег в в интерпретатор (8), если же она 
    // не найдена, добавляется новый тег (9), который всегда возвращает 
    // значение 0.

    if ( tag.PAC_descr_id > g_PAC_descriptions->get_max_PAC_number() ) //1
        {
        snprintf( bug_log::msg, bug_log::msg_size, 
            "Ошибка get_tag_value(...) - номер описания PAC %d превышает допустимый %d!",
            tag.PAC_descr_id, g_PAC_descriptions->get_max_PAC_number() );

        bug_log::add_msg_once( "Driver", "" );
        return 0;
        }

    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( tag.PAC_descr_id );
    if ( 0 == current_PAC_cmmctr )                                      //2
        {
        current_PAC_cmmctr = g_PAC_descriptions->add_PAC(               //3
            tag.PAC_address,
            tag.PAC_name, tag.PAC_descr_id, 
            tag.PAC_port, tag.timeout );     

        if ( 0 == current_PAC_cmmctr )
            {
            snprintf( bug_log::msg, bug_log::msg_size, 
                "get_tag_value(...) - ошибка добавления new_PAC_cmmctr = 0!" );
            bug_log::add_msg_once( "Driver", "" );
            return 0;
            }

        g_commctr_threads_array[ g_commctr_threads_count ] = 
            chBEGINTHREADEX( 0, 0, PAC_communication_thread, 
            current_PAC_cmmctr, 0, 0 );						            //4
        }

    //-Получены ли устройства контроллера.
    if ( current_PAC_cmmctr->is_got_PAC_devices() == 0 ) 
        {        
        return 0; //Не получены устройства PAC.
        }

    current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();
    bool   is_exist_tag = false;

    static double tag_val;    
    static char   str_tag_val[ 500 ] = {};
    tag_val     = 0;
    str_tag_val[ 0 ] = 0;

    switch ( tag_type )
        {
    case T_NUMBER:
        tag_val = current_PAC_cmmctr->get_tag_value(                 //5
            tag.tag_id, is_exist_tag );
        break;

    case T_STRING:
        strncpy( str_tag_val, current_PAC_cmmctr->get_tag_str_value(          //5
            tag.tag_id, is_exist_tag ), 500 );
        break;
        }
    current_PAC_cmmctr->get_dev_synch_access()->Done();

    if ( false == is_exist_tag )                                        //7
        {
        current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();

        switch ( tag_type )
            {
        case T_NUMBER:
            tag_val = current_PAC_cmmctr->get_tag_value(                
                tag.tag_name, is_exist_tag );
            break;

        case T_STRING:
            strncpy( str_tag_val, current_PAC_cmmctr->get_tag_str_value(          
                tag.tag_name, is_exist_tag ), 500 );
            break;
            }

        current_PAC_cmmctr->get_dev_synch_access()->Done();

        if ( true == is_exist_tag )                                     //8
            {
            current_PAC_cmmctr->add_exist_tag( tag.tag_name, tag.tag_id );
            }

        if ( false == is_exist_tag )                                    //9
            {
            snprintf( bug_log::msg, bug_log::msg_size,
                "Тег \"%s\" не найден!", 
                tag.tag_name );
            bug_log::add_msg_once( current_PAC_cmmctr->get_name(), 
                current_PAC_cmmctr->get_address() );

            current_PAC_cmmctr->add_nill_tag( tag.tag_id );
            }
        }

    switch ( tag_type )
        {
    case T_NUMBER:
        return ( void* ) &tag_val;    

    case T_STRING:
        return ( void* ) &str_tag_val;    
        }   
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Инициализация библиотеки.
///
/// @return - 1   - ок.
/// @return - > 0 - ошибка.
EXPORT int __cdecl init()
    {    
    static UCHAR is_got_init = 0;
    int init_res = 0;

    is_got_init++;
    if ( is_got_init == 1 )
        {
        InitializeCriticalSection( &g_init_cs );
        EnterCriticalSection( &g_init_cs );        

        try
            {
            g_PAC_descriptions = new PAC_cmmctr_group;
            for ( int i = 0; i < PAC_cmmctr_group::get_max_PAC_number(); i++ )
                {
                g_thread_is_terminated[ i ] = 0;
                }            

            const int MAX_PATH_LENGTH = 500;
            WCHAR path[ MAX_PATH_LENGTH ];
            GetCurrentDirectory( MAX_PATH_LENGTH, path );

            std::wstring full_path = path; 
            full_path += _T( "\\drv_buglog.log" );

            bug_log::init( full_path.c_str() );
            }
        catch (...)
            {
            init_res = 1;
            }

        LeaveCriticalSection( &g_init_cs );
        }
    else
        {
        EnterCriticalSection( &g_init_cs );
        LeaveCriticalSection( &g_init_cs );
        }

    return init_res;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
///
/// @return - значение тега.
EXPORT double __cdecl get_value( in_tag_info &tag )
    {
    return *( double* ) get_tag_value( tag, T_NUMBER );
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
/// @return - значение тега.
EXPORT double __cdecl get_value2( UINT tag_id, UCHAR PAC_description_id,
    UCHAR &result )
    {
    if ( PAC_description_id > g_PAC_descriptions->get_max_PAC_number() ) //1
        {
        snprintf( bug_log::msg, bug_log::msg_size, 
            "Ошибка get_tag_value(...) - номер описания PAC %d превышает допустимый %d!",
            PAC_description_id, g_PAC_descriptions->get_max_PAC_number() );

        bug_log::add_msg_once( "Driver", "" );
        return 0;
        }

    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( PAC_description_id );
    if ( current_PAC_cmmctr )                                      //2
        {
        //-Получены ли устройства контроллера.
        if ( current_PAC_cmmctr->is_got_PAC_devices() == 0 ) 
            {       
            result = 0;
            return 0; //Не получены устройства PAC.
            }

        current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();

        bool   is_exist_tag = false;
        double tag_val = current_PAC_cmmctr->get_tag_value(                 //5
            tag_id, is_exist_tag );

        current_PAC_cmmctr->get_dev_synch_access()->Done();

        if ( false == is_exist_tag )                                        //7
            {
            result = 1;
            return 0;
            }

        result = 0;
        return tag_val;
        }

    result = 1;
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение строкового значения тега на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
///
/// @return - значение тега.
EXPORT char* __cdecl get_str_value( in_tag_info &tag )
    {
    return ( char* ) get_tag_value( tag, T_STRING );
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
/// @return - значение тега.
EXPORT char* __cdecl get_str_value2( UINT tag_id, UCHAR PAC_description_id,
    UCHAR &result )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/// @brief Запись в тег на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
/// @param [in] value - записываемое в тег значение.
/// @param [in] type - тип значения тега.
///
/// @return - новое значение тега.
EXPORT int __cdecl set_value( in_tag_info &tag, double value, TAG_VAL_TYPE type )
    {
	PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( tag.PAC_descr_id );
    if ( current_PAC_cmmctr )
        {	
        char cmd[ 1000 ];
        sprintf( cmd, "res = make_lua_str( \"%s\", %f )", tag.tag_name, value);
        current_PAC_cmmctr->exec_Lua_str( cmd, "set_value(...)" );

        bool is_exist_tag = false;
        const char* str_res = current_PAC_cmmctr->get_str_param_from_Lua( "res", "set_value(...)" );

        char buff[ 100 ] = { 0 };
        buff[ 0 ] = device_communicator::CMD_EXEC_DEVICE_COMMAND;
        memcpy( buff + 1, str_res, strlen( str_res ) );

        current_PAC_cmmctr->send_PAC_cmd( buff, 1 + strlen( str_res ) );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Запись в строковый тег на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
/// @param [in] str_value - записываемое в тег значение.
///
/// @return - признак успешной записи: 0 - ок, 1 - ошибка.
EXPORT int __cdecl set_str_value( in_tag_info &tag, char *str_value )
    {
    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( tag.PAC_descr_id );
    if ( current_PAC_cmmctr )
        {	
        char cmd[ 1000 ];
        sprintf( cmd, "res = make_lua_str( \"%s\", \"%s\" )", 
            tag.tag_name, str_value);
        current_PAC_cmmctr->exec_Lua_str( cmd, "set_value(...)" );

        bool is_exist_tag = false;
        const char* str_res = current_PAC_cmmctr->get_str_param_from_Lua( "res", "set_value(...)" );

        char buff[ 100 ] = { 0 };
        buff[ 0 ] = device_communicator::CMD_EXEC_DEVICE_COMMAND;
        memcpy( buff + 1, str_res, strlen( str_res ) );

        current_PAC_cmmctr->send_PAC_cmd( buff, 1 + strlen( str_res ) );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Завершение работы с библиотекой.
///
/// @return - 1   - ок.
/// @return - > 0 - ошибка.
EXPORT int __cdecl final()
    {    
    static UCHAR is_got_final = 0;
    is_got_final++;

    if ( 1 == is_got_final )
        {     
        //Останавливаем потоки PAC.
        for ( int i = 0; i < PAC_cmmctr_group::get_max_PAC_number(); i++ )
            {
            g_thread_is_terminated[ i ] = 1;
            }
        Sleep( 100 );

        DeleteCriticalSection( &g_init_cs );

        delete [] g_commctr_threads_array;
        g_commctr_threads_array = 0;

        bug_log::close();
        }

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение аварий.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in]  PAC_description_id - номер описания контроллера 
/// (узла базы каналов).
/// @param [out]  alarms - аварии PAC.
///
/// @return - успешность операции: 0 - ок, 1 - ошибка.
EXPORT int __cdecl get_alarms( UCHAR PAC_description_id, all_alarm &alarms )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Обработка аварий PAC.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in]  PAC_description_id - номер описания контроллера 
/// (узла базы каналов).
/// @param [in]  count - количество обрабатываемых аварий PAC.
/// @param [in]  errors - аварии PAC.
///
/// @return успешность операции: 0 - ок, 1 - ошибка.
EXPORT int __cdecl set_alarm_cmd( UCHAR PAC_description_id, int count, 
    error_cmd *errors )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
