#include <windows.h> 
#include <strsafe.h>

#include <stdio.h> 


#define uchar unsigned char 
#define uint  unsigned int 
//-----------------------------------------------------------------------------
//ОПИСАНИЕ
//  Служит для представления информации о запрашиваемом теге.
//  Порядок полей в структуре важен для передачи данных в библиотеку драйвера EasyDriver!
struct in_tag_info    
    {      
    uchar   PAC_id;				    //Уникальный номер контроллера - 1..255. Соответствует номеру узла базы каналов.
    uchar   PAC_number;				//Номер контроллера	- 1..255. Для COM-порта. Устарело.
    uint    tag_id;				    //Уникальный номер тега.

    //1 - скоростью порт (для COM) или номер порта (для TCP).
    //2 - строка с адресом контроллера - 'COM5' (для COM) или 'IP192.200.0.0' (для TCP).
    //3 - строка с именем тега. Простой тег - 'V526', сложный тег - 'ТNK1ST1'.   
    int     PAC_baudrate_or_port;	//1
    char    *PAC_address;			//2
    char    *tag_name;              //3   		 

    int     timeout;    		    //Значение таймаута при приеме ответа от PAC.
    char    *PAC_name;              //Имя PAC - для контроля соответствия.
    char    is_get_changed_devices; //Запрашивать ли только измененные состояния устройств.

    ~in_tag_info()
        {
        delete PAC_address;
        PAC_address = 0;

        delete tag_name;
        tag_name = 0;

        delete PAC_name;
        PAC_name = 0;
        }

    }; 

enum SET_TAG_VAL_TYPE
    {
    T_ULONG,
    T_FLOAT,
    };

//Prints the error message and terminates the process.
void ErrorExit(LPTSTR lpszFunction) 
    { 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
    }
//-----------------------------------------------------------------------------
int main()
    {
    for ( int i = 0; i < 2; i++ )
        {
        //Загрузка библиотеки.
        HMODULE hLib;
        hLib = LoadLibrary( TEXT( "p:\\Monitor\\drivers\\PAC_easy_drv.dll" ) );
        if ( hLib == NULL )
            {
            ErrorExit( "LoadLibrary" );    
            }

        //Вызов функции инициализации проекта (узел базы каналов).
        int ( __stdcall *init_driver_thread )( int );
        ( FARPROC& )init_driver_thread = GetProcAddress( hLib, "init_driver_thread" );
        if ( init_driver_thread == 0 )
            {
            ErrorExit( "GetProcAddress" ); 
            }

        init_driver_thread( 1 );

        //Создание тега для теста.
        in_tag_info tag1;
        tag1.PAC_id                 = 1;
        tag1.PAC_number             = 1;
        tag1.tag_id                 = 0x53000001;  
        tag1.timeout                = 1000;
        tag1.is_get_changed_devices = 0;

        tag1.PAC_address = new char[ 20 ];
        strcpy_s( tag1.PAC_address, 20, "IP10.0.200.50" );
        tag1.PAC_baudrate_or_port = 10000;

        tag1.tag_name = new char[ 40 ];
        strcpy_s( tag1.tag_name, 40, "V101 : 1V1 Донный танка №1" );

        tag1.PAC_name = new char[ 40 ];
        strcpy_s( tag1.PAC_name, 40, "I7186_APP_1_9" );    

        //Вызов функции получения значения тега.
        double ( __stdcall *get_value )( in_tag_info& );
        ( FARPROC& )get_value = GetProcAddress( hLib, "get_value" );
        if ( get_value == 0 )
            {
            ErrorExit( "GetProcAddress" );         
            }

        int counter = 0;
        while ( 1 )
            {
            double res = get_value( tag1 );
            Sleep( 1000 );

            counter++;
            if ( counter > 10 )
                {
                break;
                }
            }

        //Вызов функции установки значения тега.
        double ( __stdcall *set_value )( in_tag_info&, double, SET_TAG_VAL_TYPE );
        ( FARPROC& )set_value = GetProcAddress( hLib, "set_value" );
        if ( set_value == 0 )
            {
            ErrorExit( "GetProcAddress" );         
            }
        set_value( tag1, 1, T_ULONG );

        //Выгрузка потока опроса PAC.
        int ( __stdcall *stop_driver_thread )( int );
        ( FARPROC& )stop_driver_thread = GetProcAddress( hLib, "stop_driver_thread" );
        if ( stop_driver_thread == 0 )
            {
            return -1;
            }

        stop_driver_thread( 1 );
        Sleep( 2000 );

        //Выгрузка библиотеки.
        FreeLibrary( hLib );

        Sleep( 200 );
        }

    return 0;
    }