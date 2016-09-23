// dllmain.cpp : Defines the entry point for the DLL application.
#include "windows.h"

#include "bug_log.h"
#include "exchange_data.h"

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
            }
        catch (...)
            {
            return false;
            }

        hRes = _Module.Init( 0, ( HINSTANCE ) hModule );            // Инициализируем модуль.
        ATLASSERT( SUCCEEDED( hRes ) );
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
const int BUFSIZE = 512;
TCHAR chReadBuf[BUFSIZE];
double res;
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
    LPTSTR lpszWrite = _T( "\1" );  
    BOOL fSuccess; 
    DWORD cbRead; 
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\EasySrvPipe");

    fSuccess = CallNamedPipe( 
        lpszPipename,        // pipe name 
        lpszWrite,           // message to server 
        (lstrlen(lpszWrite)+1)*sizeof(TCHAR), // message length 
        chReadBuf,              // buffer to receive reply 
        BUFSIZE*sizeof(TCHAR),  // size of read buffer 
        &cbRead,                // number of bytes read 
        20000);                 // waits for 20 seconds 

    if ( fSuccess )
    	{
        //sprintf( bug_log::msg, "Для тэга %d получено значение %d", 
        //    tag.tag_id, chReadBuf ); 
        //BUG_LOG.add_msg( "Driver", "" );

        res = *((int*) chReadBuf);
        return &res;
    	}
    else
        {
        sprintf( bug_log::msg, "Нет ответа от сервиса, %d", GetLastError() ); 
        BUG_LOG.add_msg( "Driver", "" );
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
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
EXPORT int __stdcall init_driver_thread( int prj_id )
    {    
    if ( BUG_LOG.init_window_complete() )
        {         
        sprintf( bug_log::msg, "Драйвер для узла базы каналов [ $%X ] загружен.", 
            prj_id ); 
        BUG_LOG.add_msg( "Driver", "" );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall stop_driver_thread( int prj_id )
    {
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
    return 0;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall get_alarms( unsigned char PAC_id, all_alarm &alarms )
    {   
    return 0;        
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall set_alarm_cmd( unsigned char PAC_id, int count,
    error_cmd *errors )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
