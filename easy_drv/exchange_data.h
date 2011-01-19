/// @file exchange_data.h
/// @brief Содержит описания данных, которые используются для обмена данными
/// драйвера с сервером.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// 
/// @par Текущая версия:
/// @$Rev: 153 $.\n
/// @$Author: id $.\n
/// @$Date:: 2010-10-14 12:46:29#$.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//ОПИСАНИЕ
//  Служит для представления информации о запрашиваемом теге.
//  Порядок полей в структуре важен для передачи данных в библиотеку драйвера
//  EasyDriver!
struct in_tag_info    
    {      
    UCHAR   PAC_descr_id;// Номер описания PAC в базе каналов (номер узла).
    UCHAR   PAC_number;  // Номер контроллера	- 1..255. Для COM-порта. Устарело.
    UINT    tag_id;	     // Уникальный номер тега.

    int     PAC_port;	 // Номер порта.
    char    *PAC_address;// Адрес контроллера - 'IP192.200.0.0'.
    char    *tag_name;   // Строка с именем тега. 

    int     timeout;    		    //Значение таймаута при приеме ответа от PAC.
    char    *PAC_name;              //Имя PAC - для контроля соответствия.
    char    is_get_changed_devices; //Запрашивать ли только измененные состояния устройств. Устарело.
    }; 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Глобальный идентификатор тревоги.
struct alarm_id
    {
    int object_type;
    int object_number;
    int object_alarm_number;
    };

#pragma pack( push, 8 ) //Выравнивание полей структур по 8 байт.
//-----------------------------------------------------------------------------
enum ALARM_STATE
    {
    AS_NORMAL,
    AS_ALARM,
    AS_RETURN, 
    AS_ACCEPT, 
    };

enum PRIORITY_TYPE
    {
    PT_SYSTEM,
    PT_CRITICAL,
    PT_IMPOTENT,
    PT_UNIMPOTENT,
    P_INFORMATIONAL,    
    };

enum ALARM_TYPE
    {
    AT_DISCRETE,
    AT_VALUE,
    AT_DEVIATION,
    AT_RATE_OF_CHANGE,
    AT_SPECIAL,
    };

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

    //  Определяет тип тревоги:
    //     atDiscrete      - дискретная ( true/false )
    //     atValue         - контроля значения ( Lo/LoLo, Hi/HiHi )
    //     atDeviation     - отклонения ( MinValue/MajValue )
    //     atRateOfChange  - изменения скорости ( speed )
    //     atSpecial       - специальная
    ALARM_TYPE type;

    //  Описание тревоги.
    char *description;

    //  Блокировка тревоги на этапе
    //  проектирования.
    UCHAR enable;

    //  Определяет принадлежность тревоги
    //  какой либо группе тревог.
    char *group;

    //  Блокировка тревоги во время работы.
    UCHAR  inhibit;

    //  Приоритет тревоги ( 0 - 999 )
    //  Тип тревоги:
    //     0       - системные
    //     1-249   - критические
    //     250-499 - важные
    //     500-749 - маловажные
    //     750-999 - информационные
    int priority;

    //  Состояние тревоги:
    //     asNormal    - тревоги нет
    //     asAlarm     - тревога есть
    //     asReturn    - контролируемое значение
    //                вернулось в нормальное
    //                состояние
    //     asAccept    - тревога подтверждена
    ALARM_STATE state;

    //  Подавление тревоги клиентами.
    UCHAR  suppress;

    //  Глобальный идентификатор тревоги:
    alarm_id id;

    // id драйвера.
    UCHAR driver_id;

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
