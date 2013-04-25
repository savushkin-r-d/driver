// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

extern "C" 
    {
#include "snprintf.h"
    };

#pragma comment(linker, "/export:get_alarms=_get_alarms@8")
#pragma comment(linker, "/export:set_alarm_cmd=_set_alarm_cmd@12")

#pragma comment(linker, "/export:get_str_value=_get_str_value@4")
#pragma comment(linker, "/export:get_str_value2=_get_str_value2@12")
#pragma comment(linker, "/export:set_str_value=_set_str_value@8")

#pragma comment(linker, "/export:get_value=_get_value@4")
#pragma comment(linker, "/export:get_value2=_get_value2@12")
#pragma comment(linker, "/export:set_value=_set_value@16")

#pragma comment(linker, "/export:init_driver_thread=_init_driver_thread@4")
#pragma comment(linker, "/export:stop_driver_thread=_stop_driver_thread@4")

#define EXPORT extern "C" __declspec (dllexport)

/// @brief Типы значения тега.
enum TAG_VAL_TYPE
    {
    T_NUMBER,///< Вещественное (float, 32 бита).
    T_STRING,///< Строка.
    };
//-----------------------------------------------------------------------------
int final();

uintptr_t WINAPI PAC_control_thread( LPVOID lpParameter );
//-----------------------------------------------------------------------------
const int        MAX_PROJECTS_CNT    = 256;
PAC_cmmctr_group *g_PAC_descriptions = 0;   ///< Контроллеры сервера.

alarm_manager    *g_alarm_manager = 0;            ///< Работа с ошибками контроллеров.
alarm            *g_alarms[ MAX_PROJECTS_CNT ];   ///< Ошибки контроллеров.
u_int_2           g_alarms_id[ MAX_PROJECTS_CNT ];///< Ошибки контроллеров.

//-Данные для потоков, работающие с контроллерами.
bool   g_thread_is_terminated[ MAX_PROJECTS_CNT ]       = { 0 };
HANDLE g_commctr_threads_array[ MAX_PROJECTS_CNT + 1 ]  = { 0 };
int    g_chbase_nodes_cont_count                        = 0;

/// @brief Синхронизатор доступа к PAC-ам.
CSWMRG g_sync_PAC;

/// @brief Количество потоков обмена с PAC.
int    g_commctr_threads_count = 0;
//-----------------------------------------------------------------------------
// Используется для проверки соответствия DLL и версии в PAC.
extern u_int_2 G_CURRENT_PROTOCOL_VERSION;

//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved )
    {
    HRESULT hRes;

    switch ( ul_reason_for_call )
        {
    case DLL_PROCESS_ATTACH:          
        try
            {
            BUG_LOG.get_instance();

            g_PAC_descriptions = new PAC_cmmctr_group(); //Контроллеры сервера.
            g_alarm_manager    = new alarm_manager();    //Работа с ошибками контроллеров.

            memset( g_alarms_id, 0, sizeof( g_alarms_id ) );
            }
        catch (...)
            {
            delete g_PAC_descriptions;
            g_PAC_descriptions = 0;

            delete g_alarm_manager;
            g_alarm_manager = 0;

            return false;
            }

        hRes = _Module.Init( 0, ( HINSTANCE ) hModule );            // Инициализируем модуль.
        ATLASSERT( SUCCEEDED( hRes ) );

        //Создаем поток, который будет следить, есть ли связь с 
        // контроллерами. В случае ее пропадания\появления 
        // устанавливать\сбрасывать соответствующую ошибку.
        g_commctr_threads_array[ MAX_PROJECTS_CNT ] = 
            chBEGINTHREADEX( 0, 0, PAC_control_thread, 0, 0, 0 ); //
        break;

    case DLL_THREAD_ATTACH:  
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:        
        final();

        bug_log::free_instance();

        _Module.Term(); // Завершаем программу.
        //MessageBox( 0 , "Final", "Ok", 0 );
        break;
        }
    return TRUE;
    }
//-----------------------------------------------------------------------------
uintptr_t WINAPI PAC_control_thread( LPVOID lpParameter )
    {
    char server_PACs_connection_state[ MAX_PROJECTS_CNT ];
    memset( server_PACs_connection_state, 1, MAX_PROJECTS_CNT );

    while ( !g_thread_is_terminated[ 0 ] )   
        {
        g_sync_PAC.WaitToRead();
        if ( g_thread_is_terminated[ 0 ] )
            {            
            g_sync_PAC.Done();
            break;
            }

        //-Проверяем состояния всех контроллеров.
        for ( unsigned int i = 0; i < g_PAC_descriptions->get_PAC_count(); i++ )
            {   
            PAC_cmmctr *PAC = g_PAC_descriptions->get_PAC( i );
            if ( 0 == PAC )
                {
                continue;
                }

            if ( PAC->get_connection_state() != 
                server_PACs_connection_state[ PAC->get_description_id() ] )
                {
                if ( 0 == PAC->get_connection_state() )
                    {
                    g_alarm_manager->add_no_PAC_connection_error( PAC->get_name(), 
                        PAC->get_description_id() );
                    }
                else
                    {
                    g_alarm_manager->remove_no_PAC_connection_error(
                        PAC->get_description_id() );
                    }

                server_PACs_connection_state[ PAC->get_description_id() ] = 
                    PAC->get_connection_state();
                }
            }

        g_sync_PAC.Done();
        Sleep( 1000 );
        }

    //_endthreadex( 0 );  
    return 0;
    }
//-----------------------------------------------------------------------------
uintptr_t WINAPI PAC_communication_thread( LPVOID lpParameter )
    {		        
//#pragma chMSG( Тестирование комментария! )

    PAC_cmmctr *PAC_com = ( PAC_cmmctr* ) lpParameter;
    int res;

    // 1 - интервал опроса контроллера.
    int sleep_time = 510;                            //1
    if ( PAC_com->get_cmmctr()->get_timeout() > 2000 )
        {
        sleep_time *= 2;
        }
    if ( PAC_com->get_cmmctr()->get_timeout() > 4000 )
        {
        sleep_time *= 3;
        }

    sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
        "Поток работы с описанием PAC [ $%X ] запущен. Интервал опроса - %d мсек.",
        PAC_com->get_description_id(), sleep_time );
    BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );

    
    while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )
        {
        //Состояния устройств будут доступны после того, как мы получим 
        //всю необходимую информацию от контроллера.
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Устройства PAC изменились." );
        BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );

        PAC_com->get_dev_synch_access()->WaitToWrite();
        PAC_com->clear_tags(); // Очищаем все теги проекта.
        PAC_com->get_dev_synch_access()->Done();

        //Пытаемся получить все устройства контроллера.
        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   
            {            
            PAC_com->get_dev_synch_access()->WaitToWrite();
            res = PAC_com->get_PAC_devices();
            PAC_com->get_dev_synch_access()->Done();

            if ( PAC_cmmctr::LOAD_OK == res )
                {
                snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
                    "Получены устройства PAC." );
                BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );                
                break;
                }  

            Sleep( sleep_time );
            }

        //Пытаемся получить состояния всех устройств контроллера.
        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   
            {
            Sleep( sleep_time );
            PAC_com->get_dev_synch_access()->WaitToWrite();
            res = PAC_com->get_PAC_all_devices_states();
            PAC_com->get_dev_synch_access()->Done();

            if ( PAC_cmmctr::PAC_DEVICES_CHANGING == res )     
                {   
                break;
                }

            //Пытаемся получить параметры всех устройств контроллера.
            int CRC = PAC_com->get_PAC_params_CRC();
            if ( CRC >= 0 && CRC != PAC_com->get_saved_CRC() ) 
                {                
                PAC_com->backup_PAC_params();
                PAC_com->set_saved_CRC( CRC );
                }

            //Получаем ошибки устройств и объектов.
            PAC_com->get_dev_synch_access()->WaitToWrite();
            PAC_com->get_PAC_errors();
            PAC_com->get_dev_synch_access()->Done();

            } //  while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )           
        } // !g_thread_is_terminated[ PAC_com->get_description_id() ]

    //_endthreadex( 0 );
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его полного описания.
///
/// Внутренняя функция библиотеки.
///
/// @param [in] tag             - полное описание тега.
/// @param [in] tag_type        - тип значения тега.
/// @param [in] use_only_tag_id - использовать только номер тега.
///
/// @return Значение тега.
void* get_tag_value( in_tag_info &tag, TAG_VAL_TYPE tag_type, 
    bool use_only_tag_id = false )
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
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "Ошибка get_tag_value(...) - номер описания PAC %d превышает допустимый %d!",
            tag.PAC_descr_id, g_PAC_descriptions->get_max_PAC_number() );

        BUG_LOG.add_msg_once( "Driver", "" );
        return 0;
        }

    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( tag.PAC_descr_id );
    if ( 0 == current_PAC_cmmctr )                                     //2
        {
        if ( use_only_tag_id ) return 0;

        current_PAC_cmmctr = g_PAC_descriptions->add_PAC(              //3
            tag.PAC_address,
            tag.PAC_name, tag.PAC_descr_id, 
            tag.PAC_port, tag.timeout );     

        if ( 0 == current_PAC_cmmctr )
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                "get_tag_value(...) - ошибка добавления new_PAC_cmmctr = 0!" );
            BUG_LOG.add_msg_once( "Driver", "" );
            return 0;
            }

        g_commctr_threads_array[ g_commctr_threads_count ] = 
            chBEGINTHREADEX( 0, 0, PAC_communication_thread, 
            current_PAC_cmmctr, 0, 0 );						           //4
        }

    //-Получены ли устройства контроллера.
    if ( current_PAC_cmmctr->is_got_PAC_devices() == 0 ) 
        {        
        return 0; //Не получены устройства PAC.
        }

    current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();
    bool   is_exist_tag = false;

    static double tag_val            = 0;    
    static char   str_tag_val[ 500 ] = { 0 };    
    tag_val          = 0;
    str_tag_val[ 0 ] = 0;

    switch ( tag_type )
        {
    case T_NUMBER:
        tag_val = current_PAC_cmmctr->get_tag_value(                   //5
            tag.tag_id, is_exist_tag );
        break;

    case T_STRING:
        current_PAC_cmmctr->get_tag_str_value(                         //5
            tag.tag_id, is_exist_tag, str_tag_val, sizeof( str_tag_val ) );
        break;
        }
    current_PAC_cmmctr->get_dev_synch_access()->Done();

    if ( false == is_exist_tag )                                       //7
        {
        if ( use_only_tag_id ) return 0;

        current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();

        switch ( tag_type )
            {
        case T_NUMBER:
            tag_val = current_PAC_cmmctr->get_tag_value(                
                tag.tag_name, is_exist_tag );
            break;

        case T_STRING:
            current_PAC_cmmctr->get_tag_str_value(          
                tag.tag_name, is_exist_tag, str_tag_val, sizeof( str_tag_val ) );
            break;
            }

        current_PAC_cmmctr->get_dev_synch_access()->Done();

        if ( true == is_exist_tag )                                    //8
            {
            current_PAC_cmmctr->add_exist_tag( tag.tag_name, tag.tag_id );
            }

        if ( false == is_exist_tag )                                   //9
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
                "Тег \"%s\" не найден!", 
                tag.tag_name );
            BUG_LOG.add_msg_once( current_PAC_cmmctr->get_name(), 
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
enum SET_TAG_VAL_TYPE
    {
    T_ULONG,
    T_FLOAT,
    };
//-----------------------------------------------------------------------------
/// @brief Запись в тег.
///
/// Внутренняя функция библиотеки.
///
/// @param [in] tag_name            - имя тега.
/// @param [in] PAC_description_id  - номер описания PAC.
/// @param [in] value               - новое значение тега.
/// @param [in] tag_type            - тип значения тега.
///
/// @return 0 - ок.
int set_tag( const char *tag_name, UCHAR PAC_description_id, void *value, 
    TAG_VAL_TYPE tag_type )
    {
    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( PAC_description_id );
    if ( current_PAC_cmmctr )
        {	
        char cmd[ 1000 ];

        switch ( tag_type )
            {
        case T_NUMBER:
            sprintf( cmd, "res = make_lua_str( \"%s\", %f )", 
                tag_name, *( double* ) value  );
            break;

        case T_STRING:
            snprintf( cmd, sizeof( cmd ), "res = make_lua_str( \"%s\", \"%s\" )", 
                tag_name, ( char* ) value );
            break;
            }      

        current_PAC_cmmctr->get_dev_synch_access()->WaitToRead();
        current_PAC_cmmctr->set_tag_Lua_cmd( cmd );
        current_PAC_cmmctr->get_dev_synch_access()->Done();
        }

    return 0;
    }
//-----------------------------------------------------------------------------
const int MAX_THREAD_END_WAIT_TIME = 10000;

EXPORT int __stdcall stop_driver_thread( int prj_id )
    {
    if ( prj_id > MAX_PROJECTS_CNT )
        {
        return 1;
        }

    g_thread_is_terminated[ prj_id ] = 1;
    Sleep( 1 );
        
    if (  g_commctr_threads_array[ prj_id ] )
        {
        WaitForSingleObject( g_commctr_threads_array[ prj_id ],
            MAX_THREAD_END_WAIT_TIME );
        CloseHandle( g_commctr_threads_array[ prj_id ] );
        g_commctr_threads_array[ prj_id ] = 0;
        }

    g_thread_is_terminated[ prj_id ] = 0;
    g_chbase_nodes_cont_count--;

    sprintf( bug_log::msg, "Драйвер для узла базы каналов [ $%X ] выгружен.", 
        prj_id ); 
    BUG_LOG.add_msg( "Driver", "" );

    if ( g_chbase_nodes_cont_count <= 0 )
        {
        final();
        bug_log::free_instance();        
        }

    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
EXPORT int __stdcall init_driver_thread( int prj_id )
    {    
    g_thread_is_terminated[ prj_id ] = 0;
    g_chbase_nodes_cont_count++;

    Sleep( 100 );

    if ( BUG_LOG.init_window_complete() )
        {         
        sprintf( bug_log::msg, "Драйвер для узла базы каналов [ $%X ] загружен.", 
            prj_id ); 
        BUG_LOG.add_msg( "Driver", "" );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Получение значения тега на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
///
/// @return Значение тега.
EXPORT double __stdcall get_value( in_tag_info &tag )
    {
    void *res = get_tag_value( tag, T_NUMBER );

    if ( res )
        {
        return *( double* ) res;
        }

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
/// @return Значение тега.
EXPORT double __stdcall get_value2( UINT tag_id, UCHAR PAC_description_id,
    UCHAR &result )
    {
    in_tag_info tag;
    tag.tag_id = tag_id;
    tag.PAC_descr_id = PAC_description_id;

    void *res = get_tag_value( tag, T_NUMBER, true );
    if ( res )
        {
        result = 0;
        return *( double* ) res;
        }

    result = 1;
    return 1;
    }
//-----------------------------------------------------------------------------
/// @brief Получение строкового значения тега на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
///
/// @return Значение тега.
EXPORT char* __stdcall get_str_value( in_tag_info &tag )
    {
    void *res = get_tag_value( tag, T_NUMBER );

    if ( res )
        {
        return ( char* ) res;
        }

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
/// @return Значение тега.
EXPORT char* __stdcall get_str_value2( UINT tag_id, UCHAR PAC_description_id,
    UCHAR &result )
    {
    in_tag_info tag;
    tag.tag_id = tag_id;
    tag.PAC_descr_id = PAC_description_id;

    void *res = get_tag_value( tag, T_STRING, true );
    if ( res )
        {
        result = 0;
        return ( char* ) res;
        }

    result = 1;
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief Запись в тег на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
/// @param [in] value - записываемое в тег значение.
/// @param [in] type - тип значения тега.
///
/// @return Новое значение тега.
EXPORT int __stdcall set_value( in_tag_info &tag, double value, TAG_VAL_TYPE type )
    {
    return set_tag( tag.tag_name, tag.PAC_descr_id, &value, T_NUMBER );    
    }
//-----------------------------------------------------------------------------
/// @brief Запись в строковый тег на основе его полного описания.
///
/// Экспортируемая функция библиотеки.
///
/// @param [in] tag - полное описание тега.
/// @param [in] str_value - записываемое в тег значение.
///
/// @return Признак успешной записи: 0 - ок, 1 - ошибка.
EXPORT int __stdcall set_str_value( in_tag_info &tag, char *str_value )
    {
    return set_tag( tag.tag_name, tag.PAC_descr_id, str_value, T_STRING );
    }
//-----------------------------------------------------------------------------
int final()
    {
    //-Завершение всех потоков, работающих с контроллерами.
    memset( g_thread_is_terminated, 1, sizeof( g_thread_is_terminated ) );
    Sleep( 1 );

    const int MAX_THREAD_END_WAIT_TIME = 15000;
    for ( int i = 0; i < MAX_PROJECTS_CNT + 1; i++ )
        {
        if (  g_commctr_threads_array[ i ] )
            {
            WaitForSingleObject( g_commctr_threads_array[ i ],
                MAX_THREAD_END_WAIT_TIME );
            CloseHandle( g_commctr_threads_array[ i ] );
            g_commctr_threads_array[ i ] = 0;
            }
        }
    Sleep( 1 );

    g_sync_PAC.WaitToWrite();
    delete g_PAC_descriptions;
    g_PAC_descriptions = 0;
    g_sync_PAC.Done();

    delete g_alarm_manager;
    g_alarm_manager = 0;

    for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
        {
        delete [] g_alarms[ i ];
        g_alarms[ i ] = 0;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall get_alarms( unsigned char PAC_id, all_alarm &alarms )
    {   
    if ( g_PAC_descriptions->get_PAC( PAC_id ) == 0 ) 
        {
        return 0;
        }

    g_PAC_descriptions->get_PAC( PAC_id )->get_dev_synch_access()->WaitToRead();
    g_alarm_manager->get_alarms( PAC_id, alarms );
    g_PAC_descriptions->get_PAC( PAC_id )->get_dev_synch_access()->Done();

    return 0;        
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall set_alarm_cmd( unsigned char PAC_id, int count,
    error_cmd *errors )
    {   
    if ( g_PAC_descriptions->get_PAC( PAC_id ) != 0 )
        {
        std::string Lua_str = " ";
        Lua_str[ 0 ] = 104;

        for ( int i = 0; i < count; i++ )
            {
            char tmp_str[ 200 ];

            snprintf( tmp_str, sizeof( tmp_str ),
                "dev_errors_manager:get_instance():set_cmd( %d, %d, %d, %d )\n",
                errors[ i ].cmd, 
                errors[ i ].object_type, errors[ i ].object_number,
                errors[ i ].object_alarm_number );

            Lua_str += tmp_str;         
            }

        const int SERVICE_ID = 1;
        
        g_PAC_descriptions->get_PAC( PAC_id )->get_cmmctr()->send_2_PAC( SERVICE_ID, 
            Lua_str.c_str(), Lua_str.length() );
        
        g_PAC_descriptions->get_PAC( PAC_id )->get_dev_synch_access()->WaitToWrite();
        g_PAC_descriptions->get_PAC( PAC_id )->get_PAC_errors();
        g_PAC_descriptions->get_PAC( PAC_id )->get_dev_synch_access()->Done();
        }        

    return 0;
    }
//-----------------------------------------------------------------------------
