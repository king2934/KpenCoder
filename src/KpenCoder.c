#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#define SERVICE_NAME "KpenCoder"
#define SLEEP_TIME 1000*3

//����Ĭ��Ҫ��
SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus;

int isRuning = 0;

char basePathBin[1024];
char basePath[1024];
char ServiceBasePath[1024];

int init();

/**
*** ȡ���ַ�������ʱ��
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
	//log�ļ��ľ���·��
	char logPath[1024]={""};
	strcpy(logPath,ServiceBasePath);
	strcat(logPath,path);
	
	char logStr[1024]={0};
	get_datetime(logStr);
	strcat(logStr," : ");
	strcat(logStr,str);
	strcat(logStr,"\n");
	
	//д��־
	FILE* fp = fopen(logPath,"a+");
	fputs(logStr,fp);
	fclose(fp);
	return 0;
}
int log_w_base(char* path,char* str)
{
	//д��־
	FILE* fp = fopen(path,"a+");
	fputs(str,fp);
	fclose(fp);
	return 0;
}


//��Ҫ���� ѭ������
int whileRuningService()
{
	isRuning = 1;
	while(isRuning==1)
	{
		log_w("\\logs\\success.log","ѭ����ʼ��...");
		Sleep(SLEEP_TIME);
	}
	return 0;
}

//����ע���һЩ��ʼ��
int initReg()
{
	char regname[] = "SYSTEM\\CurrentControlSet\\Services\\";
	strcat(regname,SERVICE_NAME);//ע����еķ����� ƴ��
	HKEY hkResult;
	int ret = RegOpenKey(HKEY_LOCAL_MACHINE,regname,&hkResult);
	char szpath[1024];
	DWORD dwSize = sizeof(szpath);
	RegQueryValueEx(hkResult,"ImagePath",NULL,NULL,(LPBYTE)szpath,&dwSize);//��ȡ����	
	strncpy(ServiceBasePath, szpath, (dwSize-(9+strlen(SERVICE_NAME)+12)));
	//printf("Hello:%s \n",basePath);
	return 0;
}

//���ƴ�����
void WINAPI ControlHandler(DWORD request)
{
	//��Ӧ�������̨����Ϣ 
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
*** ������Ҫ���� ѭ��
***
*** dwServiceType ��ָʾ�������ͣ����� Win32 ���񡣸�ֵ SERVICE_WIN32��
*** dwCurrentState ��ָ������ĵ�ǰ״̬����ʼ��״̬Ϊ SERVICE_START_PENDING�������ڳ�ʼ��
*** dwControlsAccepted �������֪ͨ SCM ��������ĸ���SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_STOP;�ػ���ֹͣ�������ֿ�������
*** dwWin32ExitCode �� dwServiceSpecificExitCode : ��������������ֹ���񲢱����˳�ϸ��ʱ�����á�
*** dwCheckPoint �� dwWaitHint�����������ʾ��ʼ��ĳ���������ʱҪ30�����ϡ�
*** RegisterServiceCtrlHandler���� ��Ϊ����ע����ƴ�����
**/
void WINAPI ServiceMain(DWORD argc, char* argv[])
{
	initReg();
	ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_STOP;	//�������ַ������̨����
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;
    hStatus = RegisterServiceCtrlHandlerA(TEXT(SERVICE_NAME),ControlHandler);
    if(0==hStatus){
		//log_start_failed();//��־��¼ ������ʼ��ʧ��		
    	//ע�����ʧ��
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode= -1;
		SetServiceStatus(hStatus,&ServiceStatus);
    }else{
		//log_start_success();//��־��¼ ������ʼ���ɹ�
    	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    	SetServiceStatus(hStatus,&ServiceStatus);
    }
	
/*
	���е��������ע����ϡ������Ҫִ�еĴ��� 
*/

whileRuningService();

/*
	�����˳����̵ķ��� 
	ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
	ServiceStatus.dwWin32ExitCode= 0; 
	SetServiceStatus(hStatus,&ServiceStatus);//֪ͨ����������������� 
*/
	return;//���ﷵ�غ󣬳��򲻻��Զ����� 
}
//����װ
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
	strcat(cmd_description," \"���������Ƶ��������ṩ��ƵWMV��ʽת��MP4��ʽ��\"");
	
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

// ����ж��
int uninstall()
{
	char cmd_stop_service[1024];
	strcpy(cmd_stop_service,"net stop ");
	strcat(cmd_stop_service,SERVICE_NAME);
	system(cmd_stop_service);//ֹͣ������ж��

	char cmdStr[40];
	strcpy(cmdStr, "sc delete ");
	strcat(cmdStr,SERVICE_NAME);
	//printf("cmdStr:%s\n",cmdStr);
	WinExec(cmdStr,SW_HIDE);
}

//��ʼ����
int init()
{
	getcwd(basePathBin,sizeof(basePathBin));//�������и�Ŀ¼
	strncpy(basePath, basePathBin,(strlen(basePathBin)-4) );//������Ŀ¼
	//printf("cmdStr:%s \n %s\n",basePath,basePathBin);
	return 0;
}

// ��ں���
int main(int argc, char *argv[])
{
	/*
	** ��ʼ��һ��SERVICE_TABLE_ENTRY ���ɱ�ṹ��
	** Ȼ�����StartServiceCtrlDispatcher()���ý��̵����߳�ת��Ϊ���Ʒ�����
	** ����������һ�����̣߳����߳����з��ɱ��ж�Ӧ����ķ����ServiceMain��������
	** ����֮��ϵͳ���Զ�����һ���߳�ȥִ��ServiceMain����������
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
			printf("����û�ж��ϣ�ִ��Ĭ��\n");
		}
	}else{
		printf("û�в�����ִ��Ĭ��\n");
	}

    return 0;//���ﷵ�غ󣬳��򲻻��Զ����� 
}