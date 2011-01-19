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
char   *g_is_terminated = 0; 

/// @brief Потоки обмена с PAC.
HANDLE *g_commctr_threads_array = new HANDLE[ PAC_cmmctr_group::get_max_PAC_number() ];

/// @brief Количество потоков обмена с PAC.
int    g_commctr_threads_count = 0;
//-Данные для потоков, работающие с контроллерами.-!>

//-----------------------------------------------------------------------------
EXPORT int __cdecl init();
EXPORT int __cdecl final();
double get_tag_value( in_tag_info &tag );
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
        in_tag_info tst;
        get_tag_value( tst );
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

    char synch_access_needs_done = 0;

    while ( !*g_is_terminated )
        {
        res = PAC_com->get_PAC_info();//Получение информации от PAC.
        if ( res <= 0 )
            {
            Sleep( 2 * sleep_time );
            continue;
            }

        //    int PAC_program_version = res;
        //    //Состояния устройств будут доступны после того, как мы получим 
        //    //всю необходимую информацию от контроллера. Поэтому устанавливаем блокировку
        //    //после очищения вектора тегов проекта.
        //    synch_access_needs_done = 1;                                            
        //    g_server_tags[ PAC_com->get_prj_id() ].synch_access->WaitToWrite();
        //    g_server_tags[ PAC_com->get_prj_id() ].clear();        

        //    sprintf_s( bug_log::msg, bug_log::msg_size, 
        //        "Устройства PAC%d изменились.", PAC_com->get_id() );
        //    bug_log::add_msg( PAC_com->get_name(), PAC_com->get_address() );

        //    while ( !*g_is_terminated )   //Пытаемся получить все устройства контроллера.
        //        {            
        //        res = PAC_com->get_PAC_devices();

        //        if ( PAC_cmmctr::LOAD_OK == res )
        //            {
        //            sprintf_s( bug_log::msg, bug_log::msg_size,
        //                "Получены устройства PAC%d.", PAC_com->get_id() );
        //            bug_log::add_msg( PAC_com->get_name(), PAC_com->get_address() );
        //            break;
        //            }     
        //        Sleep( sleep_time );
        //        }

        //    while ( !*g_is_terminated )   //Пытаемся получить состояния всех устройств контроллера.
        //        {
        //        Sleep( sleep_time );
        //        res = PAC_com->get_all_devices_states();

        //        if ( PAC_cmmctr::LOAD_OK == res )     
        //            { 
        //            if ( 1 == synch_access_needs_done )
        //                {
        //                g_server_tags[ PAC_com->get_prj_id() ].synch_access->Done(); 
        //                synch_access_needs_done = 0;
        //                }                

        //            if ( 1 == PAC_com->is_set_getting_changed_devices() ) break;
        //            } 
        //        if ( PAC_cmmctr::PAC_DEVICES_CHANGING == res )     
        //            {   
        //            break;
        //            }

        //        Sleep( 10 );
        //        if ( PAC_program_version >= VERSION_WITH_ERRORS ) 
        //            {
        //            PAC_com->get_PAC_errors();
        //            }                
        //        }

        //    if ( 1 == PAC_com->is_set_getting_changed_devices() )
        //        {            
        //        while ( !*g_is_terminated )   //Пытаемся получить состояния всех измененных устройств контроллера.
        //            {
        //            Sleep( sleep_time );
        //            res = PAC_com->get_all_devices_changed_state();

        //            if ( PAC_cmmctr::PAC_DEVICES_CHANGING == res )     
        //                {   
        //                break;
        //                }

        //            Sleep( 10 );
        //            if ( PAC_program_version >= VERSION_WITH_ERRORS ) 
        //                {
        //                PAC_com->get_PAC_errors();
        //                } 
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
double get_tag_value( in_tag_info &tag )
    {
    double res = 0;
    // Проверяется, не превышает ли номер описания PAC максимальный (1). Далее
    // есть ли описание данного PAC (2). Если нет, то тогда он добавляется в 
    // список контроллеров проекта (3) и создается поток, который
    // взаимодействует с контроллером (4).


    // Проверяется есть ли значение tag_id в интерпретаторе PAC_Lua_state (1). Если есть, тогда 
    // возвращается значение тега (2), иначе проверяется есть ли контроллер в списке 
    // контроллеров проекта (3). Если нет, то тогда он добавляется в список контроллеров 
    // проекта (4). Затем создается поток, который взаимодействует с контроллером (5).
    // Далее на основании tag.tag_name ищется соответствующий объект контроллера (6). 
    // После его нахождения добавляется новый тег в в интерпретатор (7). Возвращается 
    // значение тега (8). Если объект не найден, добавляется новый тег (9), который
    // всегда возвращает значение 0.

    if ( tag.PAC_descr_id > g_PAC_descriptions->get_max_PAC_number() ) //1
        {
        snprintf( bug_log::msg, bug_log::msg_size, 
            "Ошибка get_tag_value(...) - номер описания PAC %d превышает допустимый %d!",
            tag.PAC_descr_id, g_PAC_descriptions->get_max_PAC_number() );

        bug_log::add_msg_once( "Driver", "" );
        return 0;
        }

    if ( g_PAC_descriptions->get_PAC( tag.PAC_descr_id ) == 0 )         //2
        {
        PAC_cmmctr *new_PAC_cmmctr = g_PAC_descriptions->add_PAC(       //3
            tag.PAC_address,
            tag.PAC_name, tag.PAC_descr_id, 
            tag.PAC_port, tag.timeout );     

        if ( 0 == new_PAC_cmmctr )
            {
            snprintf( bug_log::msg, bug_log::msg_size, 
                "get_tag_value(...) - ошибка добавления new_PAC_cmmctr = 0!" );
            bug_log::add_msg_once( "Driver", "" );
            return 0;
            }

        g_commctr_threads_array[ g_commctr_threads_count ] = 
            chBEGINTHREADEX( 0, 0, PAC_communication_thread, 
            new_PAC_cmmctr, 0, 0 );						          //4
        }


    //    g_server_tags[ tag.PAC_id ].synch_access->WaitToRead();
    //    if ( g_server_tags[ tag.PAC_id ].exist_tag( tag.tag_id  ) )                     //1
    //        {
    //        res = g_server_tags[ tag.PAC_id ].get_tag_val( tag.tag_id  );               //2
    //        g_server_tags[ tag.PAC_id ].synch_access->Done();
    //        return res;
    //        }
    //    else
    //        {
    //        g_server_tags[ tag.PAC_id ].synch_access->Done();
    //        PAC_cmmctr *existed_PAC_cmmctr = 0;
    //        existed_PAC_cmmctr = g_server_PACs->get_PAC( tag.PAC_address ); 
    //
    //        if ( 0 == existed_PAC_cmmctr )                                   //3
    //            {
    //            existed_PAC_cmmctr = g_server_PACs->add_PAC( tag.PAC_address,
    //                tag.PAC_name, tag.PAC_id, 
    //                tag.PAC_baudrate_or_port, tag.timeout,
    //                tag.is_get_changed_devices );                            //4            
    //
    //            if ( 0 == existed_PAC_cmmctr )
    //                {
    //                sprintf_s( bug_log::msg, bug_log::msg_size, 
    //                    "get_tag_value(...) - ошибка existed_PAC_cmmctr = 0!" );
    //                bug_log::add_msg_once( "Driver", "" );
    //                return 0;
    //                }
    //
    //#ifdef MONITORING_DRIVER
    //            g_commctr_threads_array[ g_commctr_threads_count ] = 
    //                chBEGINTHREADEX( 0, 0, PAC_error_communication_thread, 
    //                existed_PAC_cmmctr, 0, 0 );						          //5
    //#else
    //            g_commctr_threads_array[ g_commctr_threads_count ] = 
    //                chBEGINTHREADEX( 0, 0, PAC_communication_thread, 
    //                existed_PAC_cmmctr, 0, 0 );						          //5
    //#endif // MONITORING_DRIVER
    //
    //            if ( 0 == g_commctr_threads_array[ g_commctr_threads_count ] )
    //                {
    //                sprintf_s( bug_log::msg, bug_log::msg_size, 
    //                    "get_tag_value(...) - не удалось создать поток PAC\"%s\"!", 
    //                    existed_PAC_cmmctr->get_address() );
    //                bug_log::add_msg_once( "Driver", "" );
    //                return 0;
    //                }
    //
    //            g_commctr_threads_count++;
    //            }        
    //
    //#ifdef MONITORING_DRIVER
    //        CString sys_tag_name = "PAC_STATE";
    //        if ( sys_tag_name.CompareNoCase( tag.tag_name ) == 0 )
    //            {
    //            return existed_PAC_cmmctr->get_connection_state();
    //            }
    //#endif // MONITORING_DRIVER
    //
    //        virtual_device *dev = g_server_PACs->get_device( tag.PAC_address, 
    //            tag.tag_name );										        //6  
    //        if ( dev != 0 ) 
    //            {
    //            g_server_tags[ tag.PAC_id ].synch_access->WaitToWrite();
    //            g_server_tags[ tag.PAC_id ].add_tag( tag.tag_id, tag.tag_name, dev );   //7
    //            res = g_server_tags[ tag.PAC_id ].get_tag_val( tag.tag_id );            //8
    //            g_server_tags[ tag.PAC_id ].synch_access->Done();
    //            }
    //        else
    //            {
    //            g_server_tags[ tag.PAC_id ].synch_access->WaitToWrite();
    //            g_server_tags[ tag.PAC_id ].add_tag( tag.tag_id, tag.tag_name, 0 );    //9
    //            g_server_tags[ tag.PAC_id ].synch_access->Done();
    //            //10
    //            }
    //        }

    return res;
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
            //g_is_terminated = new char;
            //*g_is_terminated = 0;

            //for ( int i = 0; i < G_MAX_PAC_COUNT; i++ )
            //	{
            //    g_PAC_Lua_state[ i ] = lua_open();  /* create state */
            //    if ( g_PAC_Lua_state[ i ] == NULL) 
            //        {
            //        MessageBox( 0, _T( "cannot create state: not enough memory" ),
            //            _T( "Error" ), 0 );

            //        for ( int j = 0; j < i; j++ ) 
            //            {
            //            lua_close( g_PAC_Lua_state[ j ] );
            //            }

            //        return EXIT_FAILURE;
            //        }

            //    lua_gc( g_PAC_Lua_state[ i ], LUA_GCSTOP, 0);  /* stop collector during initialization */
            //    luaL_openlibs( g_PAC_Lua_state[ i ] );          /* open libraries */
            //    lua_gc( g_PAC_Lua_state[ i ], LUA_GCRESTART, 0);
            //	}


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
    return 0;
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
    return 0;
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
/// @brief Типы значения тега.
enum SET_TAG_VAL_TYPE
    {
    T_ULONG, ///< Беззнаковое целое (unsigned int, 32 бита).
    T_FLOAT, ///< Вещественное (float, 32 бита).
    };
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
EXPORT int __cdecl set_value( in_tag_info &tag, double value, SET_TAG_VAL_TYPE type )
    {
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
