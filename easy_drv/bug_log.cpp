//#include "bug_log.h"
//
//#include <algorithm>

#include "stdafx.h"

CAppModule _Module;

using namespace std;

bug_log_wnd* bug_log::bug_log_window = 0;
char         bug_log::is_init_window = 0;

char	 bug_log::msg[ bug_log::C_MSG_SIZE ] = { 0 };
bug_log* bug_log::instance = 0;
//-----------------------------------------------------------------------------
LRESULT bug_log_wnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
    BOOL& bHandled)
    {
    PostQuitMessage( WM_QUIT );

    bHandled = TRUE; 

    m_hWnd = 0;
    return 0;
    }
//-----------------------------------------------------------------------------    
LRESULT bug_log_wnd::on_clear_log_button(WORD /*wNotifyCode*/, WORD /*wID*/, 
    HWND /*hWndCtl*/, BOOL& /*bHandled*/)    
    {
    clear_log(); 
    return 0;
    }
//-----------------------------------------------------------------------------
LRESULT bug_log_wnd::on_size(UINT, WPARAM, LPARAM, BOOL& bHandled )
    {
    RECT rc;
    GetClientRect( &rc );
    CRect rect( rc.right / 2 - 50, rc.bottom - 50, rc.right / 2 + 50, rc.bottom - 10 );     
    if ( clear_btn ) clear_btn.SetWindowPos( HWND_TOP, rect, 0 );

    CRect rect1( rc.left + 10, rc.top + 10, rc.right - 10, rc.bottom - 60 );             
    if ( message_list ) message_list.SetWindowPos( HWND_TOP, rect1, 0 );

    return 0;
    }
//-----------------------------------------------------------------------------
LRESULT bug_log_wnd::on_close(UINT, WPARAM, LPARAM, BOOL& bHandled )
    {
    if ( is_close )
        {        
        bHandled = false; //Будет выполнен обработчик по умолчанию, уничтожающий окно.        
        }
    else
        {
        ShowWindow( SW_HIDE );
        bHandled = true;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
void bug_log_wnd::create( CString window_title )
    {    
    synch_access->WaitToWrite();

    Create( NULL, CWindow::rcDefault, window_title );
    //CreateWindowExA( 0, 0, 0);

#ifdef _DEBUG
    SetWindowPos( HWND_DESKTOP, 700, 20, 900, 500, 0 );
#else
    SetWindowPos( HWND_DESKTOP, 350, 20, 800, 500, 0 );
#endif // _DEBUG

    RECT rc;
    GetClientRect( &rc );
    CRect rect( rc.right / 2 - 50, rc.bottom - 50, rc.right / 2 + 50, rc.bottom - 10 ); 
    clear_btn.Create( m_hWnd, rect, "Очистить", WS_VISIBLE | WS_CHILD | 
        BS_PUSHBUTTON, 0, 100 );

    is_close = 0; 

    CRect rect2( rc.left + 10, rc.top + 10, rc.right - 10, rc.bottom - 60 ); 
    message_list.RegisterClass(); 

    message_list.Create( m_hWnd, rect2, "", WS_VISIBLE | WS_CHILD | WS_BORDER );

#ifdef DEBUG
    ShowWindow( SW_NORMAL );
#else
    ShowWindow( SW_MINIMIZE );
#endif // DEBUG    

    synch_access->Done();
    }
//-----------------------------------------------------------------------------
void bug_log_wnd::close()
    {   
    is_close = 1;	
    if ( ::IsWindow( this->m_hWnd ) )
        {
        SendMessage( WM_CLOSE );       
        }   
    }  
//-----------------------------------------------------------------------------
void bug_log_wnd::clear_log()
    {
    synch_access->WaitToWrite();
    message_list.clear_messages();  
    BUG_LOG.clear_errors();
    synch_access->Done();
    }
//-----------------------------------------------------------------------------
int bug_log_wnd::add_msg_to_grid( CString &object_name, CString &IP4_address,
    CString &msg )
    {
    if ( IsWindow() )
        {
        ShowWindow( SW_SHOW );
        }
    synch_access->WaitToWrite();
    int res =  message_list.add_message( 
        log_message( object_name, IP4_address, msg, log_message::MSG_NORMAL ) );
    synch_access->Done();

    return res;
    }
//-----------------------------------------------------------------------------
int bug_log_wnd::add_error_to_grid( CString &object_name, 
    CString &IP4_address, CString &msg )
    {
    if ( IsWindow() )
        {
        ShowWindow( SW_SHOW );
        }

    synch_access->WaitToWrite();
    int res = message_list.add_message( 
        log_message( object_name, IP4_address, msg, log_message::MSG_ERROR ) );
    synch_access->Done();

    return res;
    }
//-----------------------------------------------------------------------------
int bug_log_wnd::commit_error_msg_to_grid( CString &object_name,
    CString &IP4_address, CString &msg )
    {
    if ( IsWindow() )
        {
        ShowWindow( SW_SHOW );
        }

    synch_access->WaitToWrite();
    int res = message_list.commit_error_message( 
        log_message( object_name, IP4_address, msg, log_message::MSG_ERROR ) );
    synch_access->Done();

    return res;
    }
//-----------------------------------------------------------------------------
int bug_log_wnd::add_warning_msg_to_grid( CString &object_name, 
    CString &IP4_address, CString &msg )
    {
    if ( IsWindow() )
        {
        ShowWindow( SW_SHOW );
        }

    synch_access->WaitToWrite();
    int res = message_list.add_message( 
        log_message( object_name, IP4_address, msg, log_message::MSG_WARNING ) );
    synch_access->Done();

    return res;
    }
//-----------------------------------------------------------------------------
int bug_log_wnd::add_msg_to_grid_once( CString &object_name, 
    CString &IP4_address, CString &msg )
    {
    if ( IsWindow() )
        {
        ShowWindow( SW_SHOW );
        }

    synch_access->WaitToWrite();
    int res = message_list.add_message_once( 
        log_message( object_name, IP4_address, msg, log_message::MSG_WARNING ) );
    synch_access->Done();

    return res;
    }
//-----------------------------------------------------------------------------
bug_log_wnd::~bug_log_wnd()
    {       
    delete synch_access;
    synch_access = 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------    
uintptr_t WINAPI bug_log::bug_log_thread( LPVOID lpParameter )
    {	
    bug_log_window = new bug_log_wnd();
    bug_log_window->create();           // Создаём окно приложения.
    is_init_window = 1;

    CMessageLoop loop;                  // Запускаем цикл сообщений
    BUG_LOG.add_msg( _T( "Driver" ), _T(""), _T( "Инициализация завершена." ) );

    _Module.AddMessageLoop( &loop );
    loop.Run(); 

    //_endthreadex( 0 );        
    return 0;
    } 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int bug_log::add_msg( CString object_name, CString IP4_address, 
    CString str )
    {
    bug_log_file.save_msg( ( object_name + _T( " - " ) + str ).GetBuffer( 0 ) );
    bug_log_window->add_msg_to_grid( object_name, IP4_address, str );
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_error_msg( CString object_name, CString IP4_address, 
    CString str /*= msg*/ )
    {
    bug_log_file.save_msg( ( _T( "ERROR: " ) + object_name +  
        _T( "[ " ) + IP4_address + _T( " ]\t - " ) + str ).GetBuffer( 0 ) );
    bug_log_window->add_error_to_grid( object_name, IP4_address, str );
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::commit_error_msg( CString object_name, CString IP4_address, 
    CString str /*= msg */ )
    {
    bug_log_window->commit_error_msg_to_grid( object_name, IP4_address, str );
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_warning_msg( CString object_name, CString IP4_address,
    CString str /*= msg */ )
    {
    bug_log_file.save_msg( ( _T( "WARNING: " ) + object_name +  
        _T( "[ " ) + IP4_address + _T( " ]\t - " ) + str ).GetBuffer( 0 ) );


    bug_log_window->add_warning_msg_to_grid( object_name, IP4_address, str );
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_msg_once( CString object_name, 
    CString IP4_address, CString str /*= msg */ )
    {
    int res = bug_log_window->add_msg_to_grid_once( object_name, 
        IP4_address, str );

    if ( res != -1 )
        {
        bug_log_file.save_msg( ( _T( "MESSAGE: " ) + object_name +  
            _T( "[ " ) + IP4_address + _T( " ]\t - " ) + str ).GetBuffer( 0 ) );

        }
    return 0;
    }
//-----------------------------------------------------------------------------
void bug_log::set_error( char &is_set_error, const char* PAC_name, 
    const char* ip_address, const char* str )
    {
    if ( 0 == is_set_error )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "SET  : %s", str );
        add_error_msg( PAC_name, ip_address );
        is_set_error = 1;

        errors_flags.push_back( &is_set_error );
        }
    }
//-----------------------------------------------------------------------------
void bug_log::reset_error( char &is_set_error, const char* PAC_name, 
    const char* ip_address, const char* str )
    {
    if ( 1 == is_set_error )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "SET  : %s", str );
        commit_error_msg( PAC_name, ip_address );

        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "RESET: %s", str );
        add_error_msg( PAC_name, ip_address );
        commit_error_msg( PAC_name, ip_address );

        is_set_error = 0;

        vector< char* >::iterator result;
        result = find( errors_flags.begin(), errors_flags.end(), &is_set_error );

        if ( result != errors_flags.end() )
            {   
            errors_flags.erase( result );
            }
        }
    }
//-----------------------------------------------------------------------------
void bug_log::clear_errors()
    {
    for ( unsigned int i = 0; i < errors_flags.size(); i++ )
        {
        *errors_flags[ i ] = 0;
        }

    errors_flags.clear();
    }
//-----------------------------------------------------------------------------
bug_log& bug_log::get_instance()
    {
    if ( 0 == instance )
        {
        instance = new bug_log();
        }

    return *instance;
    }
//-----------------------------------------------------------------------------
bug_log::bug_log()
    {
    const int MAX_PATH_LENGTH = 200;
    char path[ MAX_PATH_LENGTH ];
    GetCurrentDirectory( MAX_PATH_LENGTH, path );
    strcat_s( path, MAX_PATH_LENGTH, "\\drv_buglog2.log" );

    bug_log_window_thread_handle = chBEGINTHREADEX( 0, 0, bug_log_thread, 0, 0, 0 );	

    if ( bug_log_file.open( path ) )
        {       
        sprintf_s( msg, 200, "Не удалось открыть файл логов %s!",
            path );
        add_warning_msg( "Driver", msg );                
        }

    bug_log_file.start_new_log_section();
    bug_log_file.save_msg( "Driver is initialized." );

    errors_flags.clear();
    }
//-----------------------------------------------------------------------------
bug_log::~bug_log()
    {
    bug_log_window->close();  

    WaitForSingleObject( bug_log_window_thread_handle, 2000 );

    bug_log_file.save_msg( "Driver is unloaded." );

    delete bug_log_window;
    bug_log_window = 0;
    }
//-----------------------------------------------------------------------------
bool bug_log::init_window_complete() const
    {
    return is_init_window > 0;
    }
//-----------------------------------------------------------------------------
void bug_log::hide()
    {
    bug_log_window->SendMessage( WM_CLOSE );
    }
//-----------------------------------------------------------------------------
void bug_log::free_instance()
    {
    delete instance;
    instance = 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bug_log_f::bug_log_f()
    {
    bug_log_stream = 0;
    }
//-----------------------------------------------------------------------------
int bug_log_f::open( char* fname )
    {
    bug_log_filename = fname;
    if( ( bug_log_stream  = fopen( bug_log_filename, "a+" ) ) == NULL ) // C4996
        {
        return 1;
        }   

    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log_f::save_msg( const char* msg )
    {
    if ( bug_log_stream )
        {
        SYSTEMTIME st;
        GetLocalTime( &st ); 

        ATL::CString str;
        str.Format( "%.2d.%.2d.%.4d %.2d:%.2d:%.2d   %s\n", 
            st.wDay, st.wMonth, st.wYear, 
            st.wHour, st.wMinute, st.wSecond, msg );

        fwrite( str, sizeof( str[ 0 ] ), str.GetLength(), bug_log_stream );
        fflush( bug_log_stream );

        int size = ftell( bug_log_stream );
        if ( size > MAX_LOGFILE_SIZE )
            {
            fclose( bug_log_stream );
            remove( bug_log_filename + _T( ".old" ) );
            rename( bug_log_filename, bug_log_filename + _T( ".old" ) );
            bug_log_stream = fopen( bug_log_filename, _T( "a+" ) );
            }

        return 0;
        }
    return 1;
    }
//-----------------------------------------------------------------------------
void bug_log_f::close()
    {
    if ( bug_log_stream )
        {
        fclose( bug_log_stream );
        bug_log_stream = 0;
        }
    }
//-----------------------------------------------------------------------------
void bug_log_f::start_new_log_section()
    {
    fwrite( "\n\n", sizeof( char ), 2, bug_log_stream );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL list_message_data::Initialise()
    {
    if ( !m_ilItemImages.CreateFromImage( IDB_BITMAP_MSG, 16, 0, 
        RGB( 255, 0, 255 ), IMAGE_BITMAP, LR_CREATEDIBSECTION ) )
        return FALSE;

    SetImageList( m_ilItemImages );

    AddColumn( _T( "Дата" ), 70 );
    AddColumn( _T( "Время" ), 55 );
    AddColumn( _T( "Объект" ), 145 );
    AddColumn( _T( "IP адрес" ), 80 );
#ifdef DEBUG
    AddColumn( _T( "Сообщение" ), 450 );
#else
    AddColumn( _T( "Сообщение" ), 400 );
#endif // DEBUG      

    return CListImpl< list_message_data >::Initialise();
    }
//-----------------------------------------------------------------------------
int list_message_data::add_message( log_message msg )
    {
    if ( messages.size() > C_MAX_LOG_SIZE )
        {
        messages.clear();
        }

    messages.push_front( msg );

    int res = CListImpl< list_message_data >::
        AddItem() ? GetItemCount() - 1 : -1;

    return res;     
    }
//-----------------------------------------------------------------------------
int list_message_data::commit_error_message( log_message msg )
    {
    deque< log_message >::iterator result;
    result = find( messages.begin(), messages.end(), msg );

    if ( result != messages.end() )
        {   
        result->type = log_message::MSG_RETURN;
        }

    return Invalidate();
    }
//-----------------------------------------------------------------------------
int list_message_data::GetItemCount() /* required by CListImpl */
    {
    return messages.size();
    }
//-----------------------------------------------------------------------------
BOOL list_message_data::get_msg( int nItem, log_message& msg )
    {
    if ( nItem < 0 || nItem >= GetItemCount() ) 
        return FALSE;
    msg = messages[ nItem ];
    return TRUE;
    }
//-----------------------------------------------------------------------------
CString list_message_data::GetItemText( int nItem, int nSubItem ) /* required by CListImpl */
    {
    log_message msg;
    if ( !get_msg( nItem, msg ) )
        return _T( "" );
    switch ( (COLUMNS)nSubItem )
        {
    case C_DATE: return msg.date; 
    case C_TIME: return msg.time;                 
    case C_OBJECT_NAME: return msg.object_name;
    case C_IP4: return msg.IP4_ADDRESS;
    case C_MSG: return msg.msg;                
        }
    return _T( "" );
    }
//-----------------------------------------------------------------------------
int list_message_data::GetItemImage( int nItem, int nSubItem ) /* overrides CListImpl::GetItemImage */
    {
    if ( ( COLUMNS ) nSubItem != C_OBJECT_NAME )
        {
        return -1;
        }

    log_message msg;
    if ( !get_msg( nItem, msg ) ) 
        {
        return -1;
        }

    return msg.type;
    }
//-----------------------------------------------------------------------------
void list_message_data::ReverseItems() /* overrides CListImpl::ReverseItems */
    {
    reverse( messages.begin(), messages.end() );
    }
//-----------------------------------------------------------------------------
void list_message_data::SortItems( int nColumn, BOOL bAscending ) /* overrides CListImpl::SortItems */
    {
    sort( messages.begin(), messages.end(),
        CompareItem( ( COLUMNS ) nColumn ) );
    }
//-----------------------------------------------------------------------------
int list_message_data::add_message_once( log_message msg )
    {
    deque< log_message >::iterator result;
    result = find( messages.begin(), messages.end(), msg );

    int res = -1;
    if ( result == messages.end() )
        {   
        messages.push_front( msg );

        res = CListImpl< list_message_data >::
            AddItem() ? GetItemCount() - 1 : -1;
        }

    return res;
    }
//-----------------------------------------------------------------------------
void list_message_data::clear_messages()
    {
    messages.clear();
    ResetScrollBars();
    Invalidate();
    }
//-----------------------------------------------------------------------------
list_message_data::CompareItem::CompareItem( COLUMNS colColumn 
    ) : m_colColumn( colColumn )
    {
    }
//-----------------------------------------------------------------------------
bool list_message_data::CompareItem::operator()( const log_message& msg1, 
    const log_message& msg2 )
    {
    switch ( m_colColumn )
        {
    case C_DATE: 
        return ( msg1.date.Compare( msg2.date ) < 0 );
    case C_TIME: 
        return ( msg1.time.Compare( msg2.time ) < 0 );
    case C_OBJECT_NAME: 
        return ( msg1.object_name.Compare( msg2.object_name ) < 0 );
    case C_IP4: 
        return ( msg1.IP4_ADDRESS.Compare( msg2.IP4_ADDRESS ) < 0 );
    case C_MSG: 
        return ( msg1.msg.Compare( msg2.msg ) < 0 );
        }
    return false;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
log_message::log_message( CString object_name, CString IP4_ADDRESS,
    CString msg, 
    MSG_TYPES type /*= MSG_NORMAL */ ) : object_name( object_name ),
    IP4_ADDRESS( IP4_ADDRESS ),
    msg( msg ),
    type( type )
    {
    if ( this->IP4_ADDRESS[ 0 ] == 'I' ) this->IP4_ADDRESS.Delete( 0 );
    if ( this->IP4_ADDRESS[ 0 ] == 'P' ) this->IP4_ADDRESS.Delete( 0 );

    SYSTEMTIME st;
    GetLocalTime( &st );
    const int BUFFER_SIZE = 50;
    char str[ BUFFER_SIZE ];    

    sprintf_s( str, BUFFER_SIZE, "%.2d.%.2d.%.4d",
        st.wDay, st.wMonth, st.wYear );
    date = str;

    sprintf_s( str, BUFFER_SIZE, "%.2d:%.2d:%.2d",
        st.wHour, st.wMinute, st.wSecond );
    time = str;
    }
//-----------------------------------------------------------------------------
log_message::log_message()
    {

    }
//-----------------------------------------------------------------------------
bool log_message::operator==( const log_message &log_message2 )
    {
    return this->object_name == log_message2.object_name &&
        this->msg == log_message2.msg;
    }