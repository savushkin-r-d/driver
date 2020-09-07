$#include "errors.h"
$#include "exchange_data.h"

$#pragma warning(disable:4800)

enum ALARM_STATE
    {
    AS_NORMAL,
    AS_ALARM,
    AS_RETURN, 
    AS_ACCEPT, 
    };

enum ALARM_TYPE
    {
    AT_DISCRETE,
    AT_VALUE,
    AT_DEVIATION,
    AT_RATE_OF_CHANGE,
    AT_SPECIAL,
    };
//-----------------------------------------------------------------------------
/// @brief Глобальный идентификатор тревоги. 
struct alarm_id
    {
    int object_type;
    int object_number;
    int object_alarm_number;	
    };
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
//-----------------------------------------------------------------------------
struct alarm
    {
    alarm_params params;

    ALARM_TYPE type;    ///< Тип тревоги.
        
    char *description;  ///< Описание тревоги.
    unsigned char enable;       ///< Блокировка тревоги на этапе проектирования.
    char *group;        ///< Определяет принадлежность тревоги какой либо группе тревог.
    unsigned char  inhibit;     ///< Блокировка тревоги во время работы.

    //  Приоритеты тревоги:
    //     0       - системные
    //     1-249   - критические
    //     250-499 - важные
    //     500-749 - маловажные
    //     750-999 - информационные
    int priority; ///< Приоритет тревоги ( 0 - 999 ).

    ALARM_STATE state;  ///< Состояние тревоги.

    bool     suppress;       ///< Подавление тревоги клиентами.
    alarm_id id;			 ///< Глобальный идентификатор тревоги.    
    unsigned char driver_id; ///< id драйвера.

    alarm();

    ~alarm();
    };
//-----------------------------------------------------------------------------
$[

alarms = {}
--alarms[ project_descr_id ]

function get_alarms_cnt( project_description_id )
    
	if alarms[ project_description_id ] ~= NULL then	
	        		
        return #alarms[ project_description_id ]
    end

	return 0
end

function get_alarms_id( project_description_id )
    
	if alarms[ project_description_id ] ~= NULL then	
	        		
        return alarms[ project_description_id ].id
    end

	return 0
end

function get_alarm( project_description_id, n )
    
	a = NULL
		
   	if alarms[ project_description_id ] ~= NULL then
	    Lua_a = alarms[ project_description_id ][ n ]	    		
        if Lua_a ~= NULL then

			a = alarm:new() 
			a.type = Lua_a.type or AT_SPECIAL 

			a.description = Lua_a.description or "неизвестная ошибка" 
			a.group		  = Lua_a.group or "?" 

			a.enable   = Lua_a.enable or 0
			a.suppress = Lua_a.suppress or false			

			a.inhibit  = Lua_a.inhibit or 0
			a.priority = Lua_a.priority or 999
			a.state    = Lua_a.state or AS_ALARM

			a.id.object_type		 = Lua_a.id_type or 0
			a.id.object_number		 = Lua_a.id_n or 0
			a.id.object_alarm_number = Lua_a.id_object_alarm_number or 0

			a.driver_id = project_description_id			
		end
    end

	return a
end

$] 