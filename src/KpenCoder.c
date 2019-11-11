#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#define SERVICE_NAME "KpenCoder"
#define SLEEP_TIME 1000*3

//服务默认要的
SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus;

int isRuning = 0;

char basePathBin[1024];
char basePath[1024];
char ServiceBasePath[1024];

int init();

/**
*** 取得字符串日期时间
**/
int get_datetime(char * str_datetime)
{
	char acYear[6] = {0};
	char acMonth[5] = {0};
	char acDay[5] = {0};
	char acHour[5] = {0};
	char acMin[5] = {0};
	char acSec[5] = {0};

	time_t now;
    struct tm* timenow;
    
    time(&now);
    timenow = localtime(&now);

    strftime(acYear,sizeof(acYear),"%Y-",timenow);
    strftime(acMonth,sizeof(acMonth),"%m-",timenow);
    strftime(acDay,sizeof(acDay),"%d ",timenow);
    strftime(acHour,sizeof(acHour),"%H:",timenow);
    strftime(acMin,sizeof(acMin),"%M:",timenow);
    strftime(acSec,sizeof(acSec),"%S",timenow);

	strncat(str_datetime, acYear, 5);
	strncat(str_datetime, acMonth, 3);
	strncat(str_datetime, acDay, 3);
	strncat(str_datetime, acHour, 3);
	strncat(str_datetime, acMin, 3);
	strncat(str_datetime, acSec, 2);
	
	return 0;
}

int log_w(char* path,char* str)
{
	//log文件的绝对路径
	char logPath[1024]={""};
	strcpy(logPath,ServiceBasePath);
	strcat(logPath,path);
	
	char logStr[1024]={0};
	get_datetime(logStr);
	strcat(logStr," : ");
	strcat(logStr,str);
	strcat(logStr,"\n");
	
	//写日志
	FILE* fp = fopen(logPath,"a+");
	fputs(logStr,fp);
	fclose(fp);
	return 0;
}
int log_w_base(char* path,char* str)
{
	//写日志
	FILE* fp = fopen(path,"a+");
	fputs(str,fp);
	fclose(fp);
	return 0;
}


//主要服务 循环运行
int whileRuningService()
{
	isRuning = 1;
	while(isRuning==1)
	{
		log_w("\\logs\\success.log","循环开始中...");
		Sleep(SLEEP_TIME);
	}
	return 0;
}

//服务注册的一些初始化
int initReg()
{
	char regname[] = "SYSTEM\\CurrentControlSet\\Services\\";
	strcat(regname,SERVICE_NAME);//注册表中的服务名 拼接
	HKEY hkResult;
	int ret = RegOpenKey(HKEY_LOCAL_MACHINE,regname,&hkResult);
	char szpath[1024];
	DWORD dwSize = sizeof(szpath);
	RegQueryValueEx(hkResult,"ImagePath",NULL,NULL,(LPBYTE)szpath,&dwSize);//提取内容	
	strncpy(ServiceBasePath, szpath, (dwSize-(9+strlen(SERVICE_NAME)+12)));
	//printf("Hello:%s \n",basePath);
	return 0;
}

//控制处理器
void WINAPI ControlHandler(DWORD request)
{
	//响应服务控制台的消息 
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwWin32ExitCode= 0;
	
	switch(request)
	{
		case SERVICE_CONTROL_STOP:
			isRuning = 0;
			ServiceStatus.dwCurrentState=SERVICE_STOPPED;
			break;
		case SERVICE_CONTROL_SHUTDOWN:
			isRuning = 0;
			ServiceStatus.dwCurrentState=SERVICE_STOPPED;
			break;
		default:
			break;
	}
	
    SetServiceStatus(hStatus,&ServiceStatus);
	return;
}

/**
*** 服务主要函数 循环
***
*** dwServiceType ：指示服务类型，创建 Win32 服务。赋值 SERVICE_WIN32；
*** dwCurrentState ：指定服务的当前状态。初始化状态为 SERVICE_START_PENDING；即正在初始化
*** dwControlsAccepted ：这个域通知 SCM 服务接受哪个域。SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_STOP;关机和停止服务两种控制命令
*** dwWin32ExitCode 和 dwServiceSpecificExitCode : 这两个域在你终止服务并报告退出细节时很有用。
*** dwCheckPoint 和 dwWaitHint：这两个域表示初始化某个服务进程时要30秒以上。
*** RegisterServiceCtrlHandler函数 ：为服务注册控制处理器
**/
void WINAPI ServiceMain(DWORD argc, char* argv[])
{
	initReg();
	ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_STOP;	//接受两种服务控制台请求
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;
    hStatus = RegisterServiceCtrlHandlerA(TEXT(SERVICE_NAME),ControlHandler);
    if(0==hStatus){
		//log_start_failed();//日志记录 启动初始化失败		
    	//注册服务失败
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode= -1;
		SetServiceStatus(hStatus,&ServiceStatus);
    }else{
		//log_start_success();//日志记录 启动初始化成功
    	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    	SetServiceStatus(hStatus,&ServiceStatus);
    }
	
/*
	运行到这里，服务注册完毕。这里放要执行的代码 
*/

whileRuningService();

/*
	主动退出进程的方法 
	ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
	ServiceStatus.dwWin32ExitCode= 0; 
	SetServiceStatus(hStatus,&ServiceStatus);//通知服务管理器结束进程 
*/
	return;//这里返回后，程序不会自动结束 
}
//服务安装
int install()
{
	init();
	
	char ServiceBinPath[1024];
	strcpy(ServiceBinPath,basePathBin);
	strcat(ServiceBinPath,"\\");
	strcat(ServiceBinPath,SERVICE_NAME);
	strcat(ServiceBinPath,".exe");
	
	char cmd_install[1024];
	strcpy(cmd_install,"sc create ");
	strcat(cmd_install,SERVICE_NAME);
	strcat(cmd_install," displayname= ");
	strcat(cmd_install,SERVICE_NAME);
	strcat(cmd_install," depend= Tcpip start= auto ");
	strcat(cmd_install," binpath= \"");
	strcat(cmd_install,ServiceBinPath);
	strcat(cmd_install," --service ");
	strcat(cmd_install,"\"");
	
	char cmd_config[1024];
	strcpy(cmd_config,"sc config ");
	strcat(cmd_config,SERVICE_NAME);
	strcat(cmd_config," binpath= \"");
	strcat(cmd_config,ServiceBinPath);
	strcat(cmd_config," --service ");
	strcat(cmd_config,"\"");
	
	char cmd_description[1024];
	strcpy(cmd_description,"sc description ");
	strcat(cmd_description,SERVICE_NAME);
	strcat(cmd_description," \"坤朋软件视频编码服务，提供视频WMV格式转成MP4格式。\"");
	
	char cmd_start_service[1024];
	strcpy(cmd_start_service,"net start ");
	strcat(cmd_start_service,SERVICE_NAME);
	
	
	WinExec(cmd_install,SW_HIDE);
	WinExec(cmd_config,SW_HIDE);
	WinExec(cmd_description,SW_HIDE);
	system(cmd_start_service);
	
/* 	printf("ServiceBinPath:%s\n",ServiceBinPath);
	printf("cmd_install:%s\n",cmd_install);
	printf("cmd_config:%s\n",cmd_config);
	printf("cmd_description:%s\n",cmd_description);
	printf("cmd_start_service:%s\n",cmd_start_service);
 */	
	return 0;
}

// 服务卸载
int uninstall()
{
	char cmd_stop_service[1024];
	strcpy(cmd_stop_service,"net stop ");
	strcat(cmd_stop_service,SERVICE_NAME);
	system(cmd_stop_service);//停止服务再卸载

	char cmdStr[40];
	strcpy(cmdStr, "sc delete ");
	strcat(cmdStr,SERVICE_NAME);
	//printf("cmdStr:%s\n",cmdStr);
	WinExec(cmdStr,SW_HIDE);
}

//初始设置
int init()
{
	getcwd(basePathBin,sizeof(basePathBin));//程序运行根目录
	strncpy(basePath, basePathBin,(strlen(basePathBin)-4) );//程序主目录
	//printf("cmdStr:%s \n %s\n",basePath,basePathBin);
	return 0;
}

// 入口函数
int main(int argc, char *argv[])
{
	/*
	** 初始化一个SERVICE_TABLE_ENTRY 分派表结构体
	** 然后调用StartServiceCtrlDispatcher()调用进程的主线程转换为控制分派器
	** 分派器启动一个新线程，该线程运行分派表中对应于你的服务的ServiceMain（）函数
	** 在这之后系统将自动创建一个线程去执行ServiceMain函数的内容
	*/

	SERVICE_TABLE_ENTRYA ServiceTable[2];
    ServiceTable[0].lpServiceName = TEXT(SERVICE_NAME);
    ServiceTable[0].lpServiceProc = ServiceMain;
    ServiceTable[1].lpServiceName = 0;
    ServiceTable[1].lpServiceProc = 0;
	
	if(argc>1){
		if(stricmp(argv[1],"--service")==0){
			StartServiceCtrlDispatcherA(ServiceTable);
		}else if(stricmp(argv[1],"--install")==0){
			install();
		}else if(stricmp(argv[1],"--uninstall")==0){
			uninstall();
		}else{
			printf("参数没有对上，执行默认\n");
		}
	}else{
		printf("没有参数，执行默认\n");
	}

    return 0;//这里返回后，程序不会自动结束 
}