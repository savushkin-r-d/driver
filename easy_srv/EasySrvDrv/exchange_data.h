/// @file exchange_data.h
/// @brief Содержит описания данных, которые используются для обмена данными
/// драйвера с сервером.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// 
/// @par Текущая версия:
/// @$Rev: 643 $.\n
/// @$Author: id $.\n
/// @$Date:: 2013-04-15 13:27:13#$.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/// @brief Представление информации о запрашиваемом теге.
///
///  Порядок полей в структуре важен для передачи данных в библиотеку драйвера
///  EasyDriver!

#ifndef EXCHANGE_DATA_H
#define EXCHANGE_DATA_H

#include "PAC-driver\errors.h"

typedef unsigned char UCHAR;
typedef unsigned int  UINT;

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

    alarm_id(): object_type( 0 ), object_number( 0 ), object_alarm_number( 0 )
        {
        }
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

    alarm_params(): param1( 0 ), param2( 0 ), param3( 0 ), param4( 0 ),
        param5( 0 ), param6( 0 ), param7( 0 ), param8( 0 ),
        param9( 0 ), param10( 0 )
        {
        }
    };

struct alarm
    {
    alarm_params params;

    //     atDiscrete      - дискретная ( true/false )
    //     atValue         - контроля значения ( Lo/LoLo, Hi/HiHi )
    //     atDeviation     - отклонения ( MinValue/MaxValue )
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

    bool suppress;      ///< Подавление тревоги клиентами.
    alarm_id id;        ///< Глобальный идентификатор тревоги.    
    UCHAR driver_id;    ///< id драйвера.

    alarm(): type( AT_SPECIAL ), description( 0 ), enable( 0 ), group( 0 ), inhibit( 0 ),
        priority( 999 ), state( AS_ACCEPT ), suppress( false ), driver_id( 0 )
        {
        }

    alarm& operator = ( const alarm & copy )
        {
        if ( this != &copy ) // Защита от неправильного самоприсваивания.
            {
            if ( description )
                {
                delete [] description;
                description = 0;
                }

            int descr_copy_len = strlen( copy.description );
            if ( descr_copy_len > 0 )
                {
                description = new char[ descr_copy_len + 1 ];
                strcpy_s( description, descr_copy_len + 1, copy.description );
                }
            else
                {
                description = new char[ 1 ];
                description[ 0 ] = 0;
                }
            
            if ( group )
                {
                delete [] group;
                group = 0;
                }
            
            int group_copy_len = strlen( copy.group );
            if ( group_copy_len > 0 )
                {
                group = new char[ group_copy_len + 1 ];
                strcpy_s( group, group_copy_len + 1, copy.group );
                }
            else
                {
                group = new char[ 1 ];
                group[ 0 ] = 0;
                }

            params      = copy.params;
            type        = copy.type;
            enable      = copy.enable;
            inhibit     = copy.inhibit;
            priority    = copy.priority;
            state       = copy.state;
            suppress    = copy.suppress;
            id          = copy.id;
            driver_id   = copy.driver_id;
            }
        // По соглашению всегда возвращаем *this.
        return *this;
        }

    ~alarm()
        {
       delete[] description;
        description = 0;

        delete[] group;
        group = 0;
        }

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