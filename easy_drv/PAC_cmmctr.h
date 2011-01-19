/// @file PAC_cmmctr.h
/// @brief Классы, которые используются ТОЛЬКО в драйвере (PC) для 
/// организации работы с устройствами.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// @c DEBUG - компиляция c дополнительной отладочной информацией.
/// 
/// @par Текущая версия:
/// @$Rev: 198 $.\n
/// @$Author: id $.\n
/// @$Date:: 2010-12-20 18:18:47#$.

#ifndef _PAC_CMMCTR_H
#define _PAC_CMMCTR_H

#include <winsock2.h>

#include <vector>
#include <string>

#include "bug_log.h"
#include "SWMRG.h"

typedef unsigned short int      u_int_2;

class abstract_cmmctr;


#define lua_c

#ifdef  __cplusplus
extern "C" {
#endif

#include    "lua.h"
#include    "lauxlib.h"
#include    "lualib.h"

#ifdef  __cplusplus
    };
#endif
//-----------------------------------------------------------------------------
/// @brief Представление информации об одном контроллере.
///
/// Содержит состояние устройств контроллера, ошибки контроллера и т.д. 
class PAC_cmmctr
    {  
    public:
        enum LOAD_RESULTS
            {
            LOAD_OK              = 0,
            PAC_DEVICES_CHANGING = -1,  //Устройство изменило конфигурацию.
            OTHER_ERROR          = -2,
            };

        const char*  get_address() const;
        const char*  get_name() const;
        char         get_description_id() const;

        abstract_cmmctr  *get_cmmctr();

        PAC_cmmctr( const char* PAC_address, 
            char *PAC_name, 
            UCHAR PAC_descr_id, 
            int port = 10000,
            int timeout = 1500 );

        ~PAC_cmmctr();

        //Обновляет состояние всех объектов для данного контроллера.
        //ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ.
        //   0 - OK.
        // < 0 - ошибка:
        //        -1 - устройства контроллера изменились.
        //        -2 - ошибка получения состояния устройств.
        int get_all_devices_states();

        //Получение объектов для данного контроллера.
        //ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ.
        //   0 - OK.
        // < 0 - ошибка.
        //ОПИСАНИЕ
        //  Дополнительно формируется номер запроса устройств в контроллере, который в 
        //  дальнейшем служит для сопоставления данных о состоянии устройств.
        int get_PAC_devices( );

        //Возвращает 1, если получены устройства контроллера. 0 -  в противном случае.
        int is_got_PAC_devices();

        void print();

        //Получает версию ПО контроллера.
        //ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ.
        // >  0 - версия.
        // =< 0 - ошибка:
        int get_PAC_info();

    private: 
        enum CMD
            {
            CMD_GET_INFO_ON_CONNECT = 10,   //Запрос инф. о PAC перед дальнейшей работой.

            GET_DEVICES = 100,
            GET_DEVICES_STATES,
            GET_DEVICES_CHANGED_STATES,
            EXEC_DEVICE_CMD,

            GET_PAC_ERRORS,
            SET_PAC_ERROR_CMD,
            };

        enum PM_CONST
            {
            // Максимальное число ошибок обмена с PAC для установления ошибки связи.
            PM_MAX_ERRORS_COUNT = 1,
            };

        lua_State *PAC_Lua_state;    ///< Экземпляр Lua для PAC.
        CSWMRG    *dev_synch_access; ///< Синхронизация обращений к Lua.

        char *is_connected;
        char *prev_connected_state;

        // 1 - номер запроса устройств в контроллере. Устанавливается в 0 при загрузке контроллера.
        u_int_2 devices_request_id;             //1

        UCHAR err_retr_count;

        int PAC_protocol_version;

        enum SERVICE_IDS
            {
            PAC_CMMCTR_SERVICE_ID = 1,
            };


        // 1 - указатель на коммуникатор, который непосредственно обеспечивает 
        //     обмен данными с PAC.
        // 2 - строка с ID контроллера - 'COM1' (для COM) или '192.200.0.0' (для IP).        
        // 3 - устройства контроллера.
        abstract_cmmctr *cmmctr;      //1       
        std::string     PAC_address;  //2

        char *has_got_PAC_devices;  
        UINT PAC_descr_id;         // Уникальный номер описания контроллера.

        std::string    PAC_name;   // Имя контроллера.
    };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//ОПИСАНИЕ
//  Служит для представления информации обо всех контроллерах. 
class PAC_cmmctr_group
    {
    public:         
        PAC_cmmctr_group();

        static int get_max_PAC_number() 
            {
            return MAX_PAC_DESCR_NUMBER;
            }

        //  Добавляет контроллер в группу контроллеров сервера.
        PAC_cmmctr* add_PAC( char* const PAC_address, 
            char* const PAC_name, UCHAR PAC_descr_id, 
            int PAC_port, int timeout );

        //  Получение объектов контроллера.
        int get_PAC_devices( char* const PAC_address );

        //  Обновляет состояние всех объектов всех контроллеров.
        int refresh_all_devices_states();

        //  Посылает команду для выполнения в PAC.
        int send_cmd_to_PAC_device( char* const PAC_addres, 
            char* const tag_name, char* const cmd, int cmd_size );

        //Для получения ошибок связи с контроллерами.
        //Возвращает количество PAC.
        unsigned int get_PAC_count() const;

        //Возвращает PAC с индексом idx.
        PAC_cmmctr* get_PAC( int descr_id );

        void print();

    private:          
        enum CONSTANTS
            {
            MAX_PAC_DESCR_NUMBER = 100, ///< Максимальный номер описания PAC.
            };

        vector< PAC_cmmctr* > PAC_descriptions; ///< Все описания PAC сервера.
    };
//-----------------------------------------------------------------------------
//ОПИСАНИЕ
//  Базовый класс. Служит для передачи\получения данных. 
class abstract_cmmctr
    {
    public:
        // Возвращает указатель на полученные от контроллера данные, количество 
        //полученных байт.
        virtual char* get_out_data( UINT &cnt );

        int get_timeout() const
            {
            return timeout;
            }

        abstract_cmmctr( char* PAC_name, int timeout );

        // Посылает заданный массив контроллеру.
        virtual int send_2_PAC( UCHAR service_ID, char *buff, UINT length ) = 0;

    protected:        
        int id;         //Номер.

        static int count;

        int  timeout;        //Время ожидания ответа, мсек.
        char PAC_name[ 20 ]; // Имя контроллера.

        enum PARAMS
            {
            P_MAX_BUFFER_SIZE = 20*1024,
            };

        //1 - буфер для обмена данными с контроллером.
        //2 - буфер для обмена данными с контроллером. 
        //3 - количество полученных от контроллера байт в ответе.
        char in_buff[ P_MAX_BUFFER_SIZE ];  //1
        char buff[ P_MAX_BUFFER_SIZE ];     //2
        UINT answer_size;                   //3
    };
//-----------------------------------------------------------------------------
//ОПИСАНИЕ
//  TCP\IP коммуникатор.
class tcp_cmmctr : public abstract_cmmctr 
    {
    public:
        tcp_cmmctr( char *PAC_name, char* sIP, 
            int iSocket = 10000, int timeout = 1500 );
        virtual ~tcp_cmmctr();

        char* get_out_data( UINT &cnt );

        int send_2_PAC( UCHAR Service_ID, char *data, UINT length );

    private:
        CRITICAL_SECTION m_cs;

        int     port;
        UCHAR   pidx;
        char    ip_address[ 20 ];
        SOCKET  sock;
        int     isConnected;

        enum CONSTANTS
            {
            C_ERRORS_SIZE = 100,
            };
        char        is_errors[ C_ERRORS_SIZE ]; //Флаги ошибок.
        enum ERRORS_FLAGS_NUMBER
            {
            EF_NO_CONNECTION,
            EF_SEND_ERROR,
            EF_ANSWER_ERROR,
            EF_CONNECT_ERROR,
            };

        static int instancesCount;
        static int isInitialized;
        static int InitLib();
        static void DeinitLib();
        int  Connect();
        void Disconnect();
        int  recvtimeout( UINT s, char *buf, int len, int timeout, int usec );
    };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif //_PAC_CMMCTR_H
