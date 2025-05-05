// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#pragma comment(linker, "/export:get_alarms=_get_alarms@8")
#pragma comment(linker, "/export:set_alarm_cmd=_set_alarm_cmd@12")

#pragma comment(linker, "/export:get_str_value=_get_str_value@4")
#pragma comment(linker, "/export:get_str_value2=_get_str_value2@12")
#pragma comment(linker, "/export:set_str_value=_set_str_value@8")

#pragma comment(linker, "/export:get_value=_get_value@4")
#pragma comment(linker, "/export:get_value2=_get_value2@12")
#pragma comment(linker, "/export:set_value=_set_value@16")

#pragma comment(linker, "/export:init_driver_thread=_init_driver_thread@4")
#pragma comment(linker, "/export:stop_driver_thread=_stop_driver_thread@4")

#pragma comment(lib, "Ws2_32.lib")

#define EXPORT extern "C" __declspec (dllexport)

/// @brief ���� �������� ����.
enum TAG_VAL_TYPE
    {
    T_NUMBER,///< ������������ (float, 32 ����).
    T_STRING,///< ������.
    };
//-----------------------------------------------------------------------------
int final();

uintptr_t WINAPI PAC_control_thread( LPVOID lpParameter );
//-----------------------------------------------------------------------------
PAC_cmmctr_group *g_PAC_descriptions = 0;   ///< ����������� �������.

alarm_manager    *g_alarm_manager = 0;            ///< ������ � �������� ������������.
alarm            *g_alarms[ MAX_PROJECTS_CNT ];   ///< ������ ������������.
u_int_2           g_alarms_id[ MAX_PROJECTS_CNT ];///< ������ ������������.

//-������ ��� �������, ���������� � �������������.
bool   g_thread_is_terminated[ MAX_PROJECTS_CNT ]       = { 0 };
HANDLE g_commctr_threads_array[ MAX_PROJECTS_CNT + 1 ]  = { 0 };
int    g_chbase_nodes_cont_count                        = 0;

/// @brief ������������� ������� � PAC-��.
CSWMRG g_sync_PAC;

//-----------------------------------------------------------------------------
// ������������ ��� �������� ������������ DLL � ������ � PAC.
extern const u_int_2 G_CURRENT_PROTOCOL_VERSION;

const int MAX_STR_RES_LENGTH = 2000;
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved )
    {
    HRESULT hRes;

    switch ( ul_reason_for_call )
        {
    case DLL_PROCESS_ATTACH:
        try
            {
            hRes = _Module.Init( 0, (HINSTANCE)hModule );            // �������������� ������.
            ATLASSERT( SUCCEEDED( hRes ) );

            BUG_LOG.get_instance();

            g_PAC_descriptions = new PAC_cmmctr_group(); //����������� �������.
            g_alarm_manager    = new alarm_manager();    //������ � �������� ������������.

            memset( g_alarms_id, 0, sizeof( g_alarms_id ) );
            }
        catch (...)
            {
            delete g_PAC_descriptions;
            g_PAC_descriptions = 0;

            delete g_alarm_manager;
            g_alarm_manager = 0;

            return false;
            }


        //������� �����, ������� ����� �������, ���� �� ����� � 
        // �������������. � ������ �� ����������\��������� 
        // �������������\���������� ��������������� ������.
        g_commctr_threads_array[ MAX_PROJECTS_CNT ] = 
            chBEGINTHREADEX( 0, 0, PAC_control_thread, 0, 0, 0 ); //
        break;

    case DLL_THREAD_ATTACH:  
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:        
        final();

        bug_log::free_instance();

        _Module.Term(); // ��������� ���������.
        //MessageBox( 0 , "Final", "Ok", 0 );
        break;
        }
    return TRUE;
    }
//-----------------------------------------------------------------------------
uintptr_t WINAPI PAC_control_thread( LPVOID lpParameter )
    {
    char server_PACs_connection_state[ MAX_PROJECTS_CNT ];
    memset( server_PACs_connection_state, 1, MAX_PROJECTS_CNT );

    while ( !g_thread_is_terminated[ 0 ] )   
        {
        g_sync_PAC.WaitToRead();
        if ( g_thread_is_terminated[ 0 ] )
            {            
            g_sync_PAC.Done();
            break;
            }

        //-��������� ��������� ���� ������������.
        for ( unsigned int i = 0; i < PAC_cmmctr_group::MAX_PAC_DESCR_NUMBER; i++ )
            {   
            PAC_cmmctr *PAC = g_PAC_descriptions->get_PAC( i );
            if ( 0 == PAC )
                {
                continue;
                }

            if ( PAC->get_connection_state() != 
                server_PACs_connection_state[ PAC->get_description_id() ] )
                {
                if ( 0 == PAC->get_connection_state() )
                    {
                    g_alarm_manager->add_no_PAC_connection_error( PAC->get_name(), 
                        PAC->get_description_id(), PAC->get_address(),
                        PAC->get_PAC_protocol_version() );
                    }
                else
                    {
                    g_alarm_manager->remove_no_PAC_connection_error(
                        PAC->get_description_id() );
                    }

                server_PACs_connection_state[ PAC->get_description_id() ] = 
                    PAC->get_connection_state();
                }
            }

        g_sync_PAC.Done();
        Sleep( 1000 );
        }

    //_endthreadex( 0 );  
    return 0;
    }
//-----------------------------------------------------------------------------
uintptr_t WINAPI PAC_communication_thread( LPVOID lpParameter )
    {		        
//#pragma chMSG( ������������ �����������! )

    PAC_cmmctr *PAC_com = ( PAC_cmmctr* ) lpParameter;
    int res;

    // 1 - �������� ������ �����������.
    int sleep_time = 210;                            //1
    if ( PAC_com->get_cmmctr()->get_timeout() > 2000 )
        {
        sleep_time *= 2;
        }
    if ( PAC_com->get_cmmctr()->get_timeout() > 4000 )
        {
        sleep_time *= 3;
        }

    sprintf_s( bug_log::msg, bug_log::C_MSG_SIZE, 
        "����� ������ � ��������� PAC [ $%X ] �������. ���� - %d, �������� ������ - %d ����.",
        PAC_com->get_description_id(), PAC_com->get_cmmctr()->get_port(),sleep_time );
    BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );

    
    while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )
        {
        res = PAC_com->get_PAC_info();//��������� ���������� �� PAC.
        if ( res <= 0 )
            {
            Sleep( 2 * sleep_time );
            continue;
            }

        //��������� ��������� ����� �������� ����� ����, ��� �� ������� 
        //��� ����������� ���������� �� �����������.
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "���������� PAC ����������." );
        BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );

        PAC_com->clear_tags(); // ������� ��� ���� �������.

        //�������� �������� ��� ���������� �����������.
        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   
            {            
            res = PAC_com->get_PAC_devices();

            if ( PAC_cmmctr::LOAD_OK == res )
                {
                snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
                    "�������� ���������� PAC." );
                BUG_LOG.add_msg( PAC_com->get_name(), PAC_com->get_address() );                
                break;
                }  

            Sleep( sleep_time );
            }

        //�������� �������� ��������� ���� ��������� �����������.
        while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )   
            {
            Sleep( sleep_time );
            res = PAC_com->get_PAC_all_devices_states();

            if ( PAC_cmmctr::PAC_DEVICES_CHANGING == res )     
                {   
                break;
                }

            //�������� �������� ��������� ���� ��������� �����������.
            int CRC = PAC_com->get_PAC_params_CRC();
            if ( CRC >= 0 && CRC != PAC_com->get_saved_CRC() ) 
                {                
                PAC_com->backup_PAC_params();
                PAC_com->set_saved_CRC( CRC );
                }

            //�������� ������ ��������� � ��������.
            PAC_com->get_PAC_errors();

            } //  while ( !g_thread_is_terminated[ PAC_com->get_description_id() ] )           
        } // !g_thread_is_terminated[ PAC_com->get_description_id() ]

    //_endthreadex( 0 );
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief ��������� �������� ���� �� ������ ��� ������� ��������.
///
/// ���������� ������� ����������.
///
/// @param [in] tag             - ������ �������� ����.
/// @param [in] tag_type        - ��� �������� ����.
/// @param [in] use_only_tag_id - ������������ ������ ����� ����.
///
/// @return �������� ����.
int get_tag_value( double &res, char* str_res, in_tag_info &tag, TAG_VAL_TYPE tag_type, 
    bool use_only_tag_id = false )
    {
    // �����������, �� ��������� �� ����� �������� PAC ������������ (1). �����
    // ���� �� �������� ������� PAC (2). ���� ���, �� ����� �� ����������� � 
    // ������ ������������ ������� (3) � ��������� �����, �������
    // ��������������� � ������������ (4).
    // ����������� ���� �� �������� tag.tag_id � �������������� Lua (5),
    // ���� ����, ����� ������������ �������� ���� (6), ����� ����������� 
    // ���� �� ���������� tag.tag_name � � �������������� Lua (7). ����� ��
    // ���������� ����������� ����� ��� � � ������������� (8), ���� �� ��� 
    // �� �������, ����������� ����� ��� (9), ������� ������ ���������� 
    // �������� 0.

    if ( tag.PAC_descr_id > PAC_cmmctr_group::MAX_PAC_DESCR_NUMBER ) //1
        {
        snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
            "������ get_tag_value(...) - ����� �������� PAC %d ��������� ���������� %d!",
            tag.PAC_descr_id, PAC_cmmctr_group::MAX_PAC_DESCR_NUMBER );

        BUG_LOG.add_msg_once( "Driver", "" );
        return 1;
        }

    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( tag.PAC_descr_id );
    if ( 0 == current_PAC_cmmctr )                                     //2
        {
        if ( use_only_tag_id ) return 1;

        current_PAC_cmmctr = g_PAC_descriptions->add_PAC(              //3
            tag.PAC_address,
            tag.PAC_name, tag.PAC_descr_id, 
            tag.PAC_port, tag.timeout );     

        if ( 0 == current_PAC_cmmctr )
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE, 
                "get_tag_value(...) - ������ ���������� new_PAC_cmmctr = 0!" );
            BUG_LOG.add_msg_once( "Driver", "" );
            return 1;
            }

        g_commctr_threads_array[ tag.PAC_descr_id ] =
            chBEGINTHREADEX( 0, 0, PAC_communication_thread, 
            current_PAC_cmmctr, 0, 0 );						           //4
        }

    //-�������� �� ���������� �����������.
    if ( current_PAC_cmmctr->is_got_PAC_devices() == 0 ) 
        {        
        return 1; //�� �������� ���������� PAC.
        }
    bool   is_exist_tag = false;
   
    switch ( tag_type )
        {
    case T_NUMBER:
        res = current_PAC_cmmctr->get_tag_value(                       //5
            tag.tag_id, is_exist_tag );
        break;

    case T_STRING:
        current_PAC_cmmctr->get_tag_str_value(                         //5
            tag.tag_id, is_exist_tag, str_res, MAX_STR_RES_LENGTH );
        break;
        }

    if ( false == is_exist_tag )                                       //7
        {
        if ( use_only_tag_id ) return 1;
        switch ( tag_type )
            {
        case T_NUMBER:
            res = current_PAC_cmmctr->get_tag_value(                
                tag.tag_name, is_exist_tag );
            break;

        case T_STRING:
            current_PAC_cmmctr->get_tag_str_value(          
                tag.tag_name, is_exist_tag, str_res, MAX_STR_RES_LENGTH );
            break;
            }

        if ( true == is_exist_tag )                                    //8
            {
            current_PAC_cmmctr->add_exist_tag( tag.tag_name, tag.tag_id );
            }

        if ( false == is_exist_tag )                                   //9
            {
            snprintf( bug_log::msg, bug_log::C_MSG_SIZE,
                "��� \"%s\" �� ������!", 
                tag.tag_name );
            BUG_LOG.add_msg_once( current_PAC_cmmctr->get_name(), 
                current_PAC_cmmctr->get_address() );

            current_PAC_cmmctr->add_nill_tag( tag.tag_id );
            }
        }

    return 0;
    }
//-----------------------------------------------------------------------------
enum SET_TAG_VAL_TYPE
    {
    T_ULONG,
    T_FLOAT,
    };
//-----------------------------------------------------------------------------
/// @brief ������ � ���.
///
/// ���������� ������� ����������.
///
/// @param [in] tag_name            - ��� ����.
/// @param [in] PAC_description_id  - ����� �������� PAC.
/// @param [in] value               - ����� �������� ����.
/// @param [in] tag_type            - ��� �������� ����.
///
/// @return 0 - ��.
int set_tag( const char *tag_name, UCHAR PAC_description_id, void *value, 
    TAG_VAL_TYPE tag_type )
    {
    PAC_cmmctr *current_PAC_cmmctr = g_PAC_descriptions->get_PAC( PAC_description_id );
    if ( current_PAC_cmmctr )
        {	
        char cmd[ 1000 ];

        switch ( tag_type )
            {
        case T_NUMBER:
            sprintf( cmd, "res = make_lua_str( \"%s\", %f )", 
                tag_name, *( double* ) value  );
            break;

        case T_STRING:
            if ( current_PAC_cmmctr->get_PAC_protocol_version() > G_NON_UNICODE_VERSION )
                {
                //Tag example: LIN11.PROPERTY -> __LIN11:set_cmd( "PROPERTY", 0, "value" )
                string object( tag_name );
                string property = "";
                auto index = object.find( "--" );
                if ( index != std::string::npos )
                    {
                    object.erase( index );
                    }
                object.erase( std::remove( object.begin(), object.end(), ' ' ), 
                    object.end() );
                index = object.find( "." );
                if ( index != std::string::npos )
                    {
                    property = object.substr( index + 1 );
                    object.erase( index );
                    }

                char tmp[ MAX_DESCR_LEN ];
                convert_windows1251_to_utf8( tmp, (char*)value );

                snprintf( cmd, sizeof( cmd ), "__%s:set_cmd( \"%s\", 0, \"%s\" )",
                    object.c_str(), property.c_str(), tmp );

                current_PAC_cmmctr->set_tag_cmd( cmd );
                return 0;
                }
            else
                {
                snprintf( cmd, sizeof( cmd ), "res = make_lua_str( \"%s\", \"%s\" )",
                    tag_name, (char*)value );
                }
            break;
            }      

        current_PAC_cmmctr->set_tag_Lua_cmd( cmd );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
const int MAX_THREAD_END_WAIT_TIME = 10000;

EXPORT int __stdcall stop_driver_thread( int prj_id )
    {
    if ( prj_id > MAX_PROJECTS_CNT )
        {
        return 1;
        }

    g_thread_is_terminated[ prj_id ] = 1;
    Sleep( 1 );
        
    if (  g_commctr_threads_array[ prj_id ] )
        {
        WaitForSingleObject( g_commctr_threads_array[ prj_id ],
            MAX_THREAD_END_WAIT_TIME );
        CloseHandle( g_commctr_threads_array[ prj_id ] );
        g_commctr_threads_array[ prj_id ] = 0;
        }

    g_thread_is_terminated[ prj_id ] = 0;
    g_chbase_nodes_cont_count--;

    g_sync_PAC.WaitToWrite();
    g_PAC_descriptions->remove_PAC( ( u_char ) prj_id );
    g_sync_PAC.Done();

    sprintf( bug_log::msg, "������� ��� ���� ���� ������� [ $%X ] ��������.", 
        prj_id ); 
    BUG_LOG.add_msg( "Driver", "" );

    if ( g_chbase_nodes_cont_count <= 0 )
        {
        final();
        bug_log::free_instance();        
        }

    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
EXPORT int __stdcall init_driver_thread( int prj_id )
    {    
    g_thread_is_terminated[ prj_id ] = 0;
    g_chbase_nodes_cont_count++;

    Sleep( 100 );

    if ( BUG_LOG.init_window_complete() )
        {         
        sprintf( bug_log::msg, "������� ��� ���� ���� ������� [ $%X ] ��������.", 
            prj_id ); 
        BUG_LOG.add_msg( "Driver", "" );
        }

    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief ��������� �������� ���� �� ������ ��� ������� ��������.
///
/// �������������� ������� ����������.
///
/// @param [in] tag - ������ �������� ����.
///
/// @return �������� ����.
EXPORT double __stdcall get_value( in_tag_info &tag )
    {
    double value = 0;
    get_tag_value( value, 0, tag, T_NUMBER );

    return value;
    }
//-----------------------------------------------------------------------------
/// @brief ��������� �������� ���� �� ������ ��� ���������� ��������.
///
/// �������������� ������� ����������.
///
/// @param [in]  tag_id - ����� ����.
/// @param [in]  PAC_description_id - ����� �������� ����������� 
/// (���� ���� �������).
/// @param [out] result - ������� ������� ������ �������� ����:
/// 1 - ��������, 0 - ��.
///
/// @return �������� ����.
EXPORT double __stdcall get_value2( UINT tag_id, UCHAR PAC_description_id,
    UCHAR &result )
    {
    in_tag_info tag;
    tag.tag_id = tag_id;
    tag.PAC_descr_id = PAC_description_id;

    double value = 0;
    int res = get_tag_value( value, 0, tag, T_NUMBER, true );
    if ( res == 0 )
        {
        result = 0;
        return value;
        }

    result = 1;
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief ��������� ���������� �������� ���� �� ������ ��� ������� ��������.
///
/// �������������� ������� ����������.
///
/// @param [in] tag - ������ �������� ����.
///
/// @return �������� ����.
EXPORT char* __stdcall get_str_value( in_tag_info &tag )
    {
    double tmp;
    static char str_value[ UCHAR_MAX + 1 ][ MAX_STR_RES_LENGTH ];
    str_value[ tag.PAC_descr_id ][ 0 ] = 0;
    
    get_tag_value( tmp, str_value[ tag.PAC_descr_id ], tag, T_STRING);

    return str_value[ tag.PAC_descr_id ];
    }
//-----------------------------------------------------------------------------
/// @brief ��������� ���������� �������� ���� �� ������ ��� ���������� ��������.
///
/// �������������� ������� ����������.
///
/// @param [in]  tag_id - ����� ����.
/// @param [in]  PAC_description_id - ����� �������� ����������� 
/// (���� ���� �������).
/// @param [out] result - ������� ������� ������ �������� ����:
/// 1 - ��������, 0 - ��.
///
/// @return �������� ����.
EXPORT char* __stdcall get_str_value2( UINT tag_id, UCHAR PAC_description_id,
    UCHAR &result )
    {
    in_tag_info tag;
    tag.tag_id = tag_id;
    tag.PAC_descr_id = PAC_description_id;

    double tmp;
    static char str_value[UCHAR_MAX + 1][MAX_STR_RES_LENGTH];
    str_value[ PAC_description_id ][0] = 0;

    int res = get_tag_value( tmp, str_value[ PAC_description_id ], tag, T_STRING, true );
    if ( res == 0 )
        {
        result = 0;
        return str_value[ PAC_description_id ];
        }

    result = 1;
    return 0;
    }
//-----------------------------------------------------------------------------
/// @brief ������ � ��� �� ������ ��� ������� ��������.
///
/// �������������� ������� ����������.
///
/// @param [in] tag - ������ �������� ����.
/// @param [in] value - ������������ � ��� ��������.
/// @param [in] type - ��� �������� ����.
///
/// @return ����� �������� ����.
EXPORT int __stdcall set_value( in_tag_info &tag, double value, TAG_VAL_TYPE type )
    {
    return set_tag( tag.tag_name, tag.PAC_descr_id, &value, T_NUMBER );    
    }
//-----------------------------------------------------------------------------
/// @brief ������ � ��������� ��� �� ������ ��� ������� ��������.
///
/// �������������� ������� ����������.
///
/// @param [in] tag - ������ �������� ����.
/// @param [in] str_value - ������������ � ��� ��������.
///
/// @return ������� �������� ������: 0 - ��, 1 - ������.
EXPORT int __stdcall set_str_value( in_tag_info &tag, char *str_value )
    {
    return set_tag( tag.tag_name, tag.PAC_descr_id, str_value, T_STRING );
    }
//-----------------------------------------------------------------------------
int final()
    {
    //-���������� ���� �������, ���������� � �������������.
    memset( g_thread_is_terminated, 1, sizeof( g_thread_is_terminated ) );
    Sleep( 1 );
	    
    for ( int i = 0; i < MAX_PROJECTS_CNT + 1; i++ )
        {
        if (  g_commctr_threads_array[ i ] )
            {
            WaitForSingleObject( g_commctr_threads_array[ i ],
                MAX_THREAD_END_WAIT_TIME );
            CloseHandle( g_commctr_threads_array[ i ] );
            g_commctr_threads_array[ i ] = 0;
            }
        }
    Sleep( 1 );

    g_sync_PAC.WaitToWrite();
    delete g_PAC_descriptions;
    g_PAC_descriptions = 0;
    g_sync_PAC.Done();

    delete g_alarm_manager;
    g_alarm_manager = 0;

    for ( int i = 0; i < MAX_PROJECTS_CNT; i++ )
        {
        delete [] g_alarms[ i ];
        g_alarms[ i ] = 0;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall get_alarms( unsigned char PAC_id, all_alarm &alarms )
    {   
    if ( g_PAC_descriptions->get_PAC( PAC_id ) == 0 ) 
        {
        return 0;
        }

    g_PAC_descriptions->get_PAC( PAC_id )->get_alarms( alarms );
    return 0;        
    }
//-----------------------------------------------------------------------------
EXPORT int __stdcall set_alarm_cmd( unsigned char PAC_id, int count,
    error_cmd *errors )
    {   
    if ( g_PAC_descriptions->get_PAC( PAC_id ) != 0 )
        {
        g_PAC_descriptions->get_PAC( PAC_id )->set_alarm_cmd( count, errors );
        return EXIT_SUCCESS;
        }        

    return EXIT_FAILURE;
    }
//-----------------------------------------------------------------------------
