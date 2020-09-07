$#include "errors_manager.h"

$#pragma warning(push)
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

enum ALARM_CLASS_PRIORITY
    {
    P_FATAL         = 0,
    P_ERR_CONNECTION = 100,
    P_ALARM          = 250,
    P_MESSAGE        = 500,
    P_ANSWER         = 750,
    P_REMIND         = 1000,
    };

class alarm_manager
    {
    public:
        int set_alarm( unsigned char PAC_description_id, int n,            
            ALARM_TYPE a_type,
            char * a_description,
            char * a_group,
            unsigned char a_enable,
            bool   a_suppress,

            unsigned char a_inhibit,
            int    a_priority,
            ALARM_STATE a_state,

            unsigned char a_driver_id,

            int a_id_object_type,
            int a_id_object_number,
            int a_id_object_alarm_number );

        int set_alarms_id( unsigned char PAC_description_id, int id );
        int set_alarms_cnt( unsigned char PAC_description_id, int cnt );
    }

alarm_manager* G_ALARM_MANAGER(); 


$[

alarms = {}

function get_alarms( PAC_id, err_id )    
    
    if alarms[ PAC_id ] ~= NULL then
        if err_id ~= alarms[ PAC_id ].id then
            for idx, a in ipairs( alarms[ PAC_id ] ) do 
               G_ALARM_MANAGER():set_alarm( PAC_id, idx - 1,
                                    a.type or AT_SPECIAL, 
                                    a.description or "неизвестная ошибка", 
                                    a.group or "?",
                                    a.enable or 0,
                                    a.suppress or false, 
                                    a.inhibit or 0, 
                                    a.priority or 999,
                                    a.state or AS_ALARM,
                                    PAC_id, 
                                    a.id_type or 0,
                                    a.id_n or 0,
                                    a.id_object_alarm_number or 0 ) 
            end

            G_ALARM_MANAGER():set_alarms_id( PAC_id, alarms[ PAC_id ].id )
            G_ALARM_MANAGER():set_alarms_cnt( PAC_id, #alarms[ PAC_id ] )
        end
    end
    
end

$] 