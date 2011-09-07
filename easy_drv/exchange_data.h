/// @file exchange_data.h
/// @brief Содержит описания данных, которые используются для обмена данными
/// драйвера с сервером.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// 
/// @par Текущая версия:
/// @$Rev$.\n
/// @$Author$.\n
/// @$Date::                     $.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/// @brief Представление информации о запрашиваемом теге.
///
///  Порядок полей в структуре важен для передачи данных в библиотеку драйвера
///  EasyDriver!

#ifndef EXCHANGE_DATA_H
#define EXCHANGE_DATA_H

#include "errors.h"

struct in_tag_info    
    {      
    UCHAR   PAC_descr_id;///< Номер описания PAC в базе каналов (номер узла).
    UCHAR   PAC_number;  ///< Номер контроллера	- 1..255. Для COM-порта. Устарело, не используется.
    UINT    tag_id;	     ///< Уникальный номер тега.

    int     PAC_port;	 ///< Номер порта.
    char    *PAC_address;///< Адрес контроллера - 'IP192.200.0.0'.
    char    *tag_name;   ///< Строка с именем тега. 

    int     timeout;    		    ///< Значение таймаута при приеме ответа от PAC.
    char    *PAC_name;              ///< Имя PAC - для контроля соответствия.
    char    is_get_changed_devices; ///< Запрашивать ли только измененные состояния устройств. Устарело, не используется.
    }; 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/// @brief Глобальный идентификатор тревоги. 
struct alarm_id
    {
    int object_type;
    int object_number;
    int object_alarm_number;
    };

#pragma pack( push, 8 ) //Выравнивание полей структур по 8 байт.
//-----------------------------------------------------------------------------
struct alarm_params 
    {
    double  param1;
    double  param2;
    double  param3;
    double  param4;
    double  param5;
    double  param6;
    double  param7;
    double  param8;
    double  param9;
    double  param10;
    };

struct alarm
    {
    alarm_params params;

    //     atDiscrete      - дискретная ( true/false )
    //     atValue         - контроля значения ( Lo/LoLo, Hi/HiHi )
    //     atDeviation     - отклонения ( MinValue/MajValue )
    //     atRateOfChange  - изменения скорости ( speed )
    //     atSpecial       - специальная
    ALARM_TYPE type;    ///< Тип тревоги.
        
    char *description;  ///< Описание тревоги.
    UCHAR enable;       ///< Блокировка тревоги на этапе проектирования.
    char *group;        ///< Определяет принадлежность тревоги какой либо группе тревог.
    UCHAR  inhibit;     ///< Блокировка тревоги во время работы.

    //  Приоритеты тревоги:
    //     0       - системные
    //     1-249   - критические
    //     250-499 - важные
    //     500-749 - маловажные
    //     750-999 - информационные
    int priority; ///< Приоритет тревоги ( 0 - 999 ).

    //     asNormal    - тревоги нет
    //     asAlarm     - тревога есть
    //     asReturn    - контролируемое значение
    //                вернулось в нормальное
    //                состояние
    //     asAccept    - тревога подтверждена
    ALARM_STATE state;  ///< Состояние тревоги.

    UCHAR  suppress;    ///< Подавление тревоги клиентами.
    alarm_id id;        ///< Глобальный идентификатор тревоги.    
    UCHAR driver_id;    ///< id драйвера.

    bool operator == ( const alarm &alarm2 )
        {
        return !strcmp( this->description, alarm2.description ); 
        }

    bool operator < ( const alarm &alarm2 ) const
        {
        return strcmp( this->description, alarm2.description ) < 0 ? 1 : 0;
        }

    alarm( alarm const& copy );

    alarm & operator = ( const alarm & copy );

    int load_from_stream_as_simple_error( char *stream );

    alarm();

    ~alarm();

    void accept()
        {
        state = AS_ACCEPT;
        }
    };

struct all_alarm
    {
    int     cnt;
    alarm   *alarms; 
    int     id;
    };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct error_cmd
    {
    int cmd; 
    int object_type;
    int object_number;
    int object_alarm_number;
    };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // EXCHANGE_DATA_H