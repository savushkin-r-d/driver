#include "bug_log.h"
#include "CmnHdr.h"

#include <algorithm>

#include <stdio.h>
#include <io.h>

using namespace std;


CString	 bug_log::msg;
bug_log* bug_log::instance = 0;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bug_log_f::bug_log_f()
    {
    bug_log_stream = 0;
    }
//-----------------------------------------------------------------------------
int bug_log_f::open( CString bug_log_filename_ )
    {		

    if( ( bug_log_stream  = _wfopen( bug_log_filename_, _T( "a+,ccs=UTF-8" ) ) ) == NULL ) 
        {        
        return 1;               
        }   
    
    this->bug_log_filename = bug_log_filename_;

    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log_f::save_msg( CString msg )
    {
    if ( bug_log_stream )
        {
        SYSTEMTIME st;
        GetLocalTime( &st ); 

        ATL::CString str;
        str.Format( _T( "%.2d.%.2d.%.4d %.2d:%.2d:%.2d   %s\n" ), 
            st.wDay, st.wMonth, st.wYear, 
            st.wHour, st.wMinute, st.wSecond, msg );

        fwrite( str, sizeof( str[ 0 ] ), str.GetLength(), bug_log_stream );
        fflush( bug_log_stream );

        int size = ftell( bug_log_stream );

        if ( size > 1024 * 1024 )
            {
            fclose( bug_log_stream );
            _wremove( bug_log_filename + _T( ".old" ) );
            _wrename( bug_log_filename, bug_log_filename + _T( ".old" ) );            
            bug_log_stream  = _wfopen( bug_log_filename, _T( "a+,ccs=UTF-8" ) ); 
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
    fwrite( _T( "\n\n" ), sizeof( wchar_t ), 2, bug_log_stream );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int bug_log::add_msg( CString object_name, CString IP4_address, 
    CString msg_ )
    {
    bug_log_file.save_msg( object_name + _T( " - " ) + msg_ );

    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_error_msg( CString object_name, CString IP4_address, 
    CString msg_ /*= msg*/ )
    {
    bug_log_file.save_msg( _T( "ERROR: " ) + object_name +  
        _T( "[ " ) + IP4_address + _T( " ]\t - " ) + msg_ );

    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::commit_error_msg( CString object_name, CString IP4_address, 
    CString msg_ /*= msg */ )
    {
	//TODO Fix later.
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_warning_msg( CString object_name, CString IP4_address,
    CString msg_ /*= msg */ )
    {
    bug_log_file.save_msg( _T( "WARNING: " ) + object_name +  
        _T( "[ " ) + IP4_address + _T( " ]\t - " ) + msg_ );

    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_msg_once( CString object_name, 
    CString IP4_address, CString msg_ /*= msg */ )
    {
    bug_log_file.save_msg( _T( "MESSAGE: " ) + object_name +  
        _T( "[ " ) + IP4_address + _T( " ]\t - " ) + msg_ );

    return 0;
    }
//-----------------------------------------------------------------------------
void bug_log::set_error( char &is_set_error, const char* PAC_name, 
    const char* ip_address, const char* msg_ )
    {
    if ( 0 == is_set_error )
        {
        bug_log::msg.Format( _T( "SET  : %s" ), msg_ );
        add_error_msg( PAC_name, ip_address );

        is_set_error = 1;

        errors_flags.push_back( &is_set_error );
        }
    }
//-----------------------------------------------------------------------------
void bug_log::reset_error( char &is_set_error, const char* PAC_name, 
    const char* ip_address, const char* msg_ )
    {
    if ( 1 == is_set_error )
        {
        bug_log::msg.Format( _T( "SET  : %s" ), msg_ );
        commit_error_msg( PAC_name, ip_address );

        bug_log::msg.Format( _T( "RESET  : %s" ), msg_ );
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
    errors_flags.clear();

    TCHAR szFileName[ MAX_PATH ];
    GetModuleFileName( 0, szFileName, MAX_PATH );
    *wcsrchr(szFileName, '\\') = '\0';

    CString path = szFileName;
    path += _T( "\\drv_buglog.log" );

    bug_log_file.open( path );
    bug_log_file.start_new_log_section();
    bug_log_file.save_msg( _T( "Driver is initialized." ) );
    }
//-----------------------------------------------------------------------------
bug_log::~bug_log()
    {
    WaitForSingleObject( bug_log_window_thread_handle, 2000 );
    }
//-----------------------------------------------------------------------------
void bug_log::free_instance()
    {
    delete instance;
    instance = 0;
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