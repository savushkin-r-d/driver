//ОБЩЕЕ ОПИСАНИЕ.
//  Содержит описания классов, которые используются для отображения ошибок работы 
//  драйвера в окне.
#pragma once

#include <stdio.h>

#include <atlbase.h>
#include <atlstr.h>

#include <atlapp.h>
#include <atlctrls.h>
#include <atlwin.h>
#define _ATL_TMP_NO_CSTRING
#include <atlmisc.h>
#include <atlcrack.h>
#include <atlframe.h>
#include <atldlgs.h>

#include <vector>
#include <deque>

#define END_MSG_MAP_EX	END_MSG_MAP

#include <ListCtrl.h>

extern CAppModule _Module; 
//-----------------------------------------------------------------------------
//ОБЩЕЕ ОПИСАНИЕ.
//  Служит для записи ошибок в файл.
class bug_log_f
    {
    public:
        bug_log_f();
        ~bug_log_f()
            {
            close();
            }

        int	 open( char* bug_log_filename );
        void start_new_log_section();
        int  save_msg( char* msg );
        void close();

    private:
        enum L_CONST
            {
            MAX_LOGFILE_SIZE = 1000 * 1024, //Максимальный размер файла логов, байт.
            };

        FILE *bug_log_stream;
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
//-----------------------------------------------------------------------------
class list_message_data : public CListImpl< list_message_data >  
    {
    public:
        DECLARE_WND_CLASS( _T( "UserList" ) )

    protected:
        CImageList m_ilItemImages;
        deque < log_message > messages;

        enum COLUMNS 
            {
            C_DATE, 
            C_TIME, 
            C_OBJECT_NAME,
            C_IP4,
            C_MSG,
            };

    public:
        BOOL Initialise();

        int add_message( log_message msg );

        int add_message_once( log_message msg );

        int commit_error_message( log_message msg );

        void clear_messages();

        // required by CListImpl;
        int GetItemCount(); 

        BOOL get_msg( int nItem, log_message& msg );

        // required by CListImpl;
        CString GetItemText( int nItem, int nSubItem ); 

        // overrides CListImpl::GetItemImage;
        int GetItemImage( int nItem, int nSubItem ); 

        // overrides CListImpl::ReverseItems;
        void ReverseItems(); 

        class CompareItem
            {
            public:
                CompareItem( COLUMNS colColumn );
                bool operator() ( const log_message& msg1, 
                    const log_message& msg2 );

            protected:
                COLUMNS m_colColumn;
            };

        // overrides CListImpl::SortItems;
        void SortItems( int nColumn, BOOL bAscending ); 

    private:
        enum CONSTANTS
            {
            C_MAX_LOG_SIZE = 1000,
            };        
    };

//-----------------------------------------------------------------------------
//ОБЩЕЕ ОПИСАНИЕ.
//  Служит для вывода непосредственно графического окна с ошибками.
class bug_log_wnd : public CWindowImpl< bug_log_wnd, CWindow, CFrameWinTraits >
    {
    public:
        ~bug_log_wnd();

        // Карта сообщений направляет сообщения в нужные обработчики.
        BEGIN_MSG_MAP( CMainWindow )
            MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
            MESSAGE_HANDLER( WM_CLOSE, on_close )
            MESSAGE_HANDLER( WM_SIZE, on_size )

            COMMAND_HANDLER( 100, BN_CLICKED, on_clear_log_button )
            REFLECT_NOTIFICATIONS()
        END_MSG_MAP()

        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);        
        LRESULT on_clear_log_button(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, 
            BOOL& /*bHandled*/);        
        LRESULT on_close(UINT, WPARAM, LPARAM, BOOL& bHandled );
        LRESULT on_size(UINT, WPARAM, LPARAM, BOOL& bHandled );

        void create( CString window_title = "Lua - лог драйвера" );
        void close();

        void clear_log(); 

        int add_msg_to_grid( CString &object_name, CString &IP4_address, 
            CString &msg );
        int add_msg_to_grid_once( CString &object_name, CString &IP4_address, 
            CString &msg );
        int add_error_to_grid( CString &object_name, CString &IP4_address, 
            CString &msg );
        int commit_error_msg_to_grid( CString &object_name, CString &IP4_address, 
            CString &msg );
        int add_warning_msg_to_grid( CString &object_name, CString &IP4_address, 
            CString &msg );

    private:
        CButton           clear_btn;
        int				  is_close;                
        list_message_data message_list;
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

        bool init_window_complete() const;

        void set_error( char &is_set_error, const char* PAC_name, 
            const char* ip_address, const char* msg );

        void reset_error( char &is_set_error, const char* PAC_name, 
            const char* ip_address, const char* msg );

        void clear_errors();

        enum CONSTANTS
            {
            C_MSG_SIZE = 1000,
            };

        static char msg[ C_MSG_SIZE ];

        void hide();

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

        static bug_log_wnd* bug_log_window;
        bug_log_f	bug_log_file;

        static uintptr_t WINAPI bug_log_thread( LPVOID lpParameter );

        static char         is_init_window;

        vector < char* > errors_flags;

        static bug_log *instance;
    };

#define BUG_LOG bug_log::get_instance()
//-----------------------------------------------------------------------------
