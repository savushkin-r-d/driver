#include "bug_log.h"

#include <algorithm>


using namespace std;


char	 bug_log::msg[ bug_log::C_MSG_SIZE ] = { 0 };
bug_log* bug_log::instance = 0;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int bug_log::add_msg( CString object_name, CString IP4_address, 
    CString msg )
    {
	//TODO Fix later.
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_error_msg( CString object_name, CString IP4_address, 
    CString msg /*= msg*/ )
    {
	//TODO Fix later.
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::commit_error_msg( CString object_name, CString IP4_address, 
    CString msg /*= msg */ )
    {
	//TODO Fix later.
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_warning_msg( CString object_name, CString IP4_address,
    CString msg /*= msg */ )
    {
	//TODO Fix later.
    return 0;
    }
//-----------------------------------------------------------------------------
int bug_log::add_msg_once( CString object_name, 
    CString IP4_address, CString msg /*= msg */ )
    {
	//TODO Fix later.
    return 0;
    }
//-----------------------------------------------------------------------------
void bug_log::set_error( char &is_set_error, const char* PAC_name, 
    const char* ip_address, const char* msg )
    {
    if ( 0 == is_set_error )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "SET  : %s", msg );
        add_error_msg( PAC_name, ip_address );
        is_set_error = 1;

        errors_flags.push_back( &is_set_error );
        }
    }
//-----------------------------------------------------------------------------
void bug_log::reset_error( char &is_set_error, const char* PAC_name, 
    const char* ip_address, const char* msg )
    {
    if ( 1 == is_set_error )
        {
        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "SET  : %s", msg );
        commit_error_msg( PAC_name, ip_address );

        sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
            "RESET: %s", msg );
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