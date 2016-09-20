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
/// @$Rev: 1795 $.\n
/// @$Author: id $.\n
/// @$Date:: 2014-11-05 12:36:52#$.

#ifndef _PAC_CMMCTR_H
#define _PAC_CMMCTR_H

#include <winsock2.h>

#include <vector>
#include <string>

#include "SWMRG.h"
#include "WSA_err_decode.h"
#include "quicklz.h"
#include "g_device.h"
#include "bug_log.h"

typedef unsigned short int u_int_2;

class abstract_cmmctr;

#define lua_c

extern "C"
    {
#include    "lua.h"
#include    "lauxlib.h"
#include    "lualib.h"

//#include "snprintf.h"
    };
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
            PAC_DEVICES_CHANGING = -1,  ///< Устройство изменило конфигурацию.
            OTHER_ERROR          = -2,
            };

        /// @brief Получение IP-адреса PAC.
        ///
        /// @return IP-адрес PAC.
        const char*  get_address() const;

        /// @brief Получение имени PAC.
        ///
        /// @return Имя PAC.
        const char*  get_name() const;

        /// @brief Получение номера описания PAC.
        ///
        /// @return Номер описания PAC.
        UINT get_description_id() const;

        abstract_cmmctr  *get_cmmctr();

        /// @brief Получение номера описания PAC.
        ///
        /// @param PAC_address [ in ]  - IP-адрес PAC.
        /// @param PAC_name [ in ]     - имя PAC.
        /// @param PAC_descr_id [ in ] - номер описания PAC.
        /// @param port [ in ]         - порт.
        /// @param timeout [ out ]     - время ожидания ответа от PAC.
        PAC_cmmctr( const char* PAC_address, 
            char *PAC_name, 
            UCHAR PAC_descr_id, 
            int port = 10000,
            int timeout = 1500 );

         ~PAC_cmmctr()
             {
             lua_close( PAC_Lua_state );
             
             PAC_Lua_state = 0;
             }

        /// @brief Очистка таблицы тегов.
        ///
        /// @return 0 - ок.
        int clear_tags();
               

        /// @brief Обновление состояния всех объектов для данного контроллера.
        ///
        /// @return   0 - ок.
        /// @return > 0 - ошибка.
        LOAD_RESULTS get_PAC_all_devices_states();

        /// @brief Получение объектов для данного контроллера.
        ///
        ///  Дополнительно формируется номер запроса устройств в контроллере, который в 
        ///  дальнейшем служит для сопоставления данных о состоянии устройств.
        ///
        /// @return  0  - ок.
        /// @return > 0 - ошибка.
        int get_PAC_devices( );

        /// @brief Получение версии ПО контроллера.
        ///
        /// @return  > 0 - версия.
        /// @return <= 0 - ошибка.
        int get_PAC_info();

        /// @brief Получение информации о получении устройств для данного 
        /// контроллера.
        ///
        /// @return true  - получены устройства контроллера.
        /// @return false - устройства контроллера не получены.
        bool is_got_PAC_devices();

        //- Получение значений тегов через Lua.

        /// @brief Получение строкового значения тега на основе номер тега.
        ///
        /// @param tag_id [ in ]        - номер тега.
        /// @param is_exist_tag [ out ] - найден ли данный тег.
        /// @param str_value [ out ]    - строковое значение тега.
        /// @param max_length [ in ]    - максимальная длина значение тега.
        void get_tag_str_value( int tag_id, bool &is_exist_tag, char *str_value,
            int max_length );

        /// @brief Получение строкового значения тега на основе имени тега.
        ///
        /// @param tag_name [ in ]      - имя тега.
        /// @param is_exist_tag [ out ] - найден ли данный тег.
        /// @param str_value [ out ]    - строковое значение тега.
        /// @param max_length [ in ]    - максимальная длина значение тега.
        void get_tag_str_value( const char *tag_name, bool &is_exist_tag, 
            char *str_value, int max_length );

        /// @brief Получение числового значения тега на основе номера тега.
        ///
        /// @param tag_id [ in ]        - номер тега.
        /// @param is_exist_tag [ out ] - найден ли данный тег.
        ///
        /// @return Числовое значение тега.
        double get_tag_value( int tag_id, bool &is_exist_tag );

        /// @brief Получение числового значения тега на основе имени тега.
        ///
        /// @param tag_name [ in ]      - имя тега.
        /// @param is_exist_tag [ out ] - найден ли данный тег.
        ///
        /// @return Числовое значение тега.
        double get_tag_value( const char *tag_name, bool &is_exist_tag );

        /// @brief Добавление тега, которого нет в PAC, в Lua.
        ///
        /// @param tag_id [ in ] - номер тега.
        void add_nill_tag( int tag_id );

        /// @brief Добавление тега в Lua для быстрого доступа через номер тега.
        ///
        /// @param tag_name [ in ] - имя тега.
        /// @param tag_id [ in ]   - номер тега.
        void add_exist_tag( const char *tag_name, int tag_id );
        //- Получение значений тегов через Lua.


        /// @brief Получение объекта синхронизации.
        ///
        /// Объект синхронизации используется для разделяемого доступа к
        /// экземпляру интерпретатора Lua.
        ///
        /// @return Объект синхронизации.
        CSWMRG* get_dev_synch_access() const;

        /// @brief Отсылка команды в PAC через интерпретатор Lua.
        ///
        /// @param cmd [ in ] - строка скрипта для обработки Lua.
        void set_tag_Lua_cmd( const char *cmd );


        /// @brief Получение флага состояния связи с PAC.
        char get_connection_state() const
            {
            return ( *is_connected ) ? 1 : 0;
            }

        /// @brief Получение флага состояния связи с PAC.
        bool get_prev_connection_state() const;


        /// @brief Получение параметров объектов контроллера.
        ///
        /// @return   0 - ок.
        /// @return < 0 - ошибка.
        int backup_PAC_params();

        int check_PAC_params();

        int get_PAC_params_CRC();

        int get_saved_CRC() const
            {
            return PAC_params_CRC;
            }

        int set_saved_CRC( int PAC_params_CRC )
            {
            return this->PAC_params_CRC = PAC_params_CRC;
            }


        int get_PAC_errors();

    private: 
        //- Lua.

        /// @brief Получение числового значения переменной из машины Lua.
        ///
        /// @param param_name [ in ]      - имя переменной.
        /// @param c_function_name [ in ] - имя функции, для вывода сообщения
        /// об ошибке.
        /// @param is_exist [ out ]       - найдена ли данная переменная.
        ///
        /// @return Числовое значение переменной из машины Lua.
        double get_double_param_from_Lua( const char *param_name, 
            const char *c_function_name, bool &is_exist ) const;

        /// @brief Получение числового значения переменной из машины Lua.
        ///
        /// @param param_name [ in ]      - имя переменной.
        /// @param c_function_name [ in ] - имя функции, для вывода сообщения
        /// об ошибке.
        ///
        /// @return Числовое значение переменной из машины Lua.
        int get_int_param_from_Lua( const char *param_name, 
            const char *c_function_name ) const;

        /// @brief Получение строкового значения переменной из машины Lua.
        ///
        /// @param param_name [ in ]      - имя переменной.
        /// @param c_function_name [ in ] - имя функции, для вывода сообщения
        /// об ошибке.
        ///
        /// @return Строковое значение переменной из машины Lua.
        const char* get_str_param_from_Lua( const char *param_name, 
            const char *c_function_name ) const;

        /// @brief Выполнение строки Lua.
        ///
        /// @param Lua_str [ in ]   - строка для выполнения.
        /// @param error_str [ in ] - строка для вывода сообщения
        /// об ошибке.
        /// @param is_print_error_msg [ in ] - выводить ли строки с описанием 
        /// при ошибке.
        ///
        /// @return 0 - ок.
        /// @return 1 - ошибка выполнения строки.
        int exec_Lua_str( const char *Lua_str,
            const char *error_str, bool is_print_error_msg = true ) const;
        
        //-Lua.-!>

        enum PM_CONST
            {
            /// @brief Максимальное число ошибок обмена с PAC для возникновения
            /// ошибки связи.
            PM_MAX_ERRORS_COUNT = 2,

            /// @brief Число циклов обмена данными с PAC, после которого 
            /// происходит уборка мусора.
            PM_GARBAGE_CYCLE = 100,
            };

        lua_State *PAC_Lua_state;    ///< Экземпляр Lua для PAC.
        CSWMRG    *dev_synch_access; ///< Синхронизация обращений к Lua.

        bool *is_connected;

        /// @brief Номер запроса устройств в PAC. Устанавливается в 0 при
        /// загрузке PAC.
        u_int_2 devices_request_id;            

        UCHAR err_retr_count;

        int PAC_protocol_version;

        enum SERVICE_IDS
            {
            PAC_CMMCTR_SERVICE_ID = 1,
            };

        abstract_cmmctr *cmmctr;    ///< Коммуникатор для обмена данными с PAC.       
        std::string     PAC_address;///< IP-адрес PAC.

        bool *has_got_PAC_devices;  ///< Флаг получения устройств от PAC.
        UINT  PAC_descr_id;         ///< Уникальный номер описания PAC.

        std::string    PAC_name;    ///< Имя PAC.

        void get_param_file_name( char * file_name, int max_len );

        int save_to_file( const char* file_name, const char * str );

        int  PAC_params_CRC;
        bool is_process_PAC_params;

        std::string tags_str;
    };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/// @brief Представление информации обо всех контроллерах. 
class PAC_cmmctr_group
    {
    public:         
        PAC_cmmctr_group();

        ~PAC_cmmctr_group()
            {
            for ( unsigned int i = 0; i < MAX_PAC_DESCR_NUMBER; i++ )
                {
                delete PAC_descriptions[ i ];
                PAC_descriptions[ i ] = 0;
                }            
            }

        /// @brief Добавление описания PAC в группу описаний сервера.
        PAC_cmmctr* add_PAC( char* const PAC_address, 
            char* const PAC_name, UCHAR PAC_descr_id, 
            int PAC_port, int timeout );
                
        /// @brief Получение описания PAC с заданным номером.
        PAC_cmmctr* get_PAC( int descr_id );
                     
        enum CONSTANTS
            {
            MAX_PAC_DESCR_NUMBER = 255, ///< Максимальный номер описания PAC.
            };

    private: 
        PAC_cmmctr* PAC_descriptions[ MAX_PAC_DESCR_NUMBER ]; ///< Все описания PAC сервера.
    };
//-----------------------------------------------------------------------------
/// @brief Базовый класс. Служит для передачи\получения данных. 
class abstract_cmmctr
    {
    public:
        /// @brief Получение данных от контроллера.
        virtual char* get_out_data( UINT &cnt );

        /// @brief 
        int get_timeout() const;

        /// @brief 
        abstract_cmmctr( const char* PAC_name, int timeout );

        virtual ~abstract_cmmctr()
            {
            delete [] PAC_name;
            PAC_name = 0;
            }

        /// @brief Отсылка заданного массива PAC.
        virtual int send_2_PAC( UCHAR service_ID, const char *buff,
            UINT length ) = 0;

    protected:        
        int id;         ///< Номер.

        static int count;

        int  timeout;        ///< Время ожидания ответа, мсек.
        char *PAC_name;      ///< Имя PAC.

        enum PARAMS
            {
            P_MAX_BUFFER_SIZE = 50*1024,
            };

        /// @brief Буфер для обмена данными с контроллером.
        char in_buff[ P_MAX_BUFFER_SIZE ];

        /// @brief Буфер для обмена данными с контроллером. 
        char buff[ P_MAX_BUFFER_SIZE ];
        
        /// @brief Количество полученных от контроллера байт в ответе.
        UINT answer_size;
    };
//-----------------------------------------------------------------------------
/// @brief TCP/IP коммуникатор.
class tcp_cmmctr : public abstract_cmmctr 
    {
    public:
        tcp_cmmctr( const char *PAC_name, const char* sIP, 
            int iSocket = 10000, int timeout = 1500 );
        virtual ~tcp_cmmctr();

        char* get_out_data( UINT &cnt );

        int send_2_PAC( UCHAR Service_ID, const char *data, UINT length );

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
        char is_errors[ C_ERRORS_SIZE ]; ///< Флаги ошибок.

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

    private:
        qlz_state_decompress *state_decompress;
        char buff[ P_MAX_BUFFER_SIZE + 400 ];
    };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif //_PAC_CMMCTR_H
