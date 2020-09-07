/// @file bug_log.h
/// @brief Классы, которые используются для отображения ошибок работы 
///  драйвера в окне.
/// 
/// @author  Иванюк Дмитрий Сергеевич.
///
/// @par Описание директив препроцессора:
/// @c DEBUG - компиляция c дополнительной отладочной информацией.
/// 
/// @par Текущая версия:
/// @$Rev: 946 $.\n
/// @$Author: id $.\n
/// @$Date:: 2014-01-16 09:12:32#$.

#pragma once

#include <stdio.h>

#include <atlbase.h>
#include <atlstr.h>

#include <vector>
#include <deque>

//-----------------------------------------------------------------------------
//  Служит для записи ошибок в файл.
class bug_log_f
    {
    public:
        bug_log_f();
        ~bug_log_f()
            {
            close();
            }

        int	 open( CString bug_log_filename );
        void start_new_log_section();
        int  save_msg( CString msg );
        void close();

    private:
        enum L_CONST
            {
            MAX_LOGFILE_SIZE = 1000 * 1024, //Максимальный размер файла логов, байт.
            };

        FILE *bug_log_stream;
        CString bug_log_filename;
    };
//-----------------------------------------------------------------------------
class log_message
    {     
    public:
        enum MSG_TYPES
            {
            MSG_RETURN,
            MSG_NORMAL,
            MSG_ERROR,
            MSG_WARNING,
            };

        CString   date;
        CString   time;
        CString   object_name;
        CString   IP4_ADDRESS;
        CString   msg;
        MSG_TYPES type;

    public:
        log_message();

        log_message( CString object_name, CString IP4_ADDRESS, CString msg,
            MSG_TYPES type = MSG_NORMAL );

        bool operator == ( const log_message &log_message2 );
    };
////-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//ОБЩЕЕ ОПИСАНИЕ.
//  Служит для организации работы с отображением ошибок, протоколированием их - 
//  осуществляет всю обработку ошибок.
class bug_log
    { 
    public:   
        bug_log();

        ~bug_log();

        void set_error( char &is_set_error, const char* PAC_name, 
            const char* ip_address, const char* msg );

        void reset_error( char &is_set_error, const char* PAC_name, 
            const char* ip_address, const char* msg );

        void clear_errors();

        enum CONSTANTS
            {
            C_MSG_SIZE = 1000,
            };

        static CString msg;

        int add_error_msg( CString object_name, CString IP4_address, 
            CString msg = bug_log::msg );
        int commit_error_msg( CString object_name, CString IP4_address, 
            CString msg = bug_log::msg ); 

        int add_warning_msg( CString object_name, CString IP4_address, 
            CString msg = bug_log::msg );

        int add_msg( CString object_name, CString IP4_address, 
            CString msg = bug_log::msg );
        int add_msg_once( CString object_name, CString IP4_address, 
            CString msg = bug_log::msg );

        static bug_log& get_instance();

        static void free_instance();

    private:
        HANDLE   bug_log_window_thread_handle;
        char     tmp_str[ C_MSG_SIZE ];

        std::vector < char* > errors_flags;

        static bug_log *instance;

        bug_log_f	bug_log_file;
    };

#define BUG_LOG bug_log::get_instance()
//-----------------------------------------------------------------------------
