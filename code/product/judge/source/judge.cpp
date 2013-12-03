//#define   _WIN32_WINNT     0x0500

#include <windows.h>
#include <process.h>
#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <queue>
#include <string>
#include <sstream>

#include "tlhelp32.h"


#include "..\include\judge_inc.h"


using namespace std;


char INI_filename[]="GDOJ\\data.ini";
char Mysql_url[255];
char Mysql_username[255];
char Mysql_password[255];
char Mysql_table[255];
int  Mysql_port;
char Mysql_Character[255];  //编码

int  nLanguageCount=0;
int isDeleteTemp=0;
int isRestrictedFunction=0;
int  limitJudge=20;  //裁判队列最大等待数量
DWORD OutputLimit=10000; //最大输出
char workPath[MAX_PATH];  //临时工作目录
char judgeLogPath[MAX_PATH];
int JUDGE_LOG_BUF_SIZE = 200;
char dataPath[MAX_PATH];  //数据
char logPath[MAX_PATH]="log\\";  //log
char judgePath[MAX_PATH]; //judge.exe

char judge_log_filename[MAX_PATH] = {0};




char compileCmd_str[1024]={0};
char runCmd_str[1024]={0};


clock_t startt,endt ; //每次run的时间点

STARTUPINFO si;
PROCESS_INFORMATION G_pi = {0};
PROCESS_INFORMATION G_pi_com = {0};

class CSolution
{
public:
	char username[MAX_NAME];
	int solutionId;
	int problemId;
	int languageId;
	int verdictId;
	int contestId;
	int time;
	int memory;
	int reJudge;
	time_t submitDate;
protected:
private:
};


void set_debug_switch(ULONG ds)
{
	g_oj_debug_switch = ds;
}

ULONG Judge_DebugSwitch(ULONG st)
{
	set_debug_switch(st);

	return OS_TRUE;
}
/* END HDU VJUDGE */


/////////////////////////LOG
//#define LOG(level) Log(__FILE__, __LINE__, level).GetStream()
extern void pdt_debug_print(const char *format, ...);

///////////END LOG


////////////////////////////////////////////////////socket
#define PORT 5000
#define BUFFER 1024

int port=PORT;

typedef struct
{
	int solutionId;
}JUDGE_DATA;

int GL_currentId;//当前裁判id
queue <JUDGE_DATA> Q;//全局队列

SOCKET sListen;

//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup") // 设置连接器选项

int initSocket()
{
	write_log(JUDGE_INFO,"Start initialization of Socket...");

	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
	//加载winsock库
	if(WSAStartup(sockVersion, &wsaData) != 0)
		return 0;
	// 创建套节字
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sListen == INVALID_SOCKET)
	{
		write_log(JUDGE_SYSTEM_ERROR,"create socket error");
		return 0;
	}
	// 在sockaddr_in结构中装入地址信息
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);	// htons函数 将主机的无符号短整形数转换成网络
	//字节顺序
	sin.sin_addr.S_un.S_addr = INADDR_ANY;


	int trybind=50;  //重试bind次数
	int ret=0;
	while((ret=bind(sListen,(LPSOCKADDR)&sin,sizeof(sin)))==SOCKET_ERROR&&trybind>0)
	{
		write_log(JUDGE_SYSTEM_ERROR,"bind failed:%d , it will try later...",WSAGetLastError());
		trybind--;
		Sleep(100);
	}
	if(ret<0) {
		write_log(JUDGE_SYSTEM_ERROR,"Bind failed...");
		return 0;
	}
	write_log(JUDGE_INFO,"Bind success...");

	//进入监听状态
	int trylisten=50; //重试listen次数
	while((ret=listen(sListen,20))==SOCKET_ERROR&&trylisten)
	{
		write_log(JUDGE_SYSTEM_ERROR,"listen failed:%d , it will try later..",WSAGetLastError());
		trylisten--;
		Sleep(100);
		return 0;
	}
	if(ret<0) {
		write_log(JUDGE_SYSTEM_ERROR,"Listen failed...");
		return 0;
	}
	write_log(JUDGE_INFO,"Listen success...");

	return 1;
}
//////////////////////////////////////////////////////////////end socket

int InitMySQL()   //初始化mysql，并设置字符集
{
	mysql=mysql_init((MYSQL*)0);
	if(mysql!=0 && !mysql_real_connect(mysql,Mysql_url, Mysql_username, Mysql_password, Mysql_table,Mysql_port,NULL,CLIENT_MULTI_STATEMENTS )){
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}
	strcpy(query,"SET CHARACTER SET gbk"); //设置编码 gbk

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}
	return 1;
}


void InitMySqlConfig(){
	port=GetPrivateProfileInt("Tool","Port",PORT,INI_filename);
	isDeleteTemp=GetPrivateProfileInt("Tool","DeleteTemp",0,INI_filename);
	limitJudge=GetPrivateProfileInt("Tool","LimitJudge",20,INI_filename);
	OutputLimit=GetPrivateProfileInt("Tool","OutputLimit",10000,INI_filename);
	JUDGE_LOG_BUF_SIZE=GetPrivateProfileInt("Tool","JUDGE_LOG_BUF_SIZE",500,INI_filename);

	isRestrictedFunction=GetPrivateProfileInt("Tool","isRestrictedFunction",0,INI_filename);
	GetPrivateProfileString("Tool","WorkingPath","",workPath,sizeof(workPath),INI_filename);
	GetPrivateProfileString("Tool","DataPath","",dataPath,sizeof(dataPath),INI_filename);
	GetPrivateProfileString("Tool","JudgePath","",judgePath,sizeof(judgePath),INI_filename);
	GetPrivateProfileString("Tool","JudgeLogPath","",judgeLogPath,sizeof(judgeLogPath),INI_filename);

	GetPrivateProfileString("MySQL","url","",Mysql_url,sizeof(Mysql_url),INI_filename);
	GetPrivateProfileString("MySQL","username","",Mysql_username,sizeof(Mysql_username),INI_filename);
	GetPrivateProfileString("MySQL","password","",Mysql_password,sizeof(Mysql_password),INI_filename);
	GetPrivateProfileString("MySQL","table","",Mysql_table,sizeof(Mysql_table),INI_filename);
	Mysql_port=GetPrivateProfileInt("MySQL","port",0,INI_filename);

	GetPrivateProfileString("HDU","username","",hdu_username,sizeof(hdu_username),INI_filename);
	GetPrivateProfileString("HDU","password","",hdu_password,sizeof(hdu_password),INI_filename);

	write_log(JUDGE_INFO,"Socketport:%d, Data:%s, Workpath:%s",port,dataPath,workPath);
	write_log(JUDGE_INFO,"MySQL:%s %s %s %s %d",Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);

}


void InitPath()
{
	if( (_access(workPath, 0 )) == -1 )   {
		CreateDirectory(workPath,NULL);
	}

	//GetPrivateProfileString(lpAppName,lpKeyName,lpDefault,lpReturnedString,nSize,lpFileName);
	//GetPrivateProfileInt(lpAppName,lpKeyName,nDefault,lpFileName);

	char keyname[100]={0};
	sprintf(keyname,"Language%d",GL_languageId);

	GetPrivateProfileString("Language",keyname,"",GL_languageName,100,INI_filename);

	//sprintf(keyname,"%s",GL_languageName);

	GetPrivateProfileString("LanguageExt",GL_languageName,"",GL_languageExt,10,INI_filename);

	GetPrivateProfileString("LanguageExe",GL_languageName,"",GL_languageExe,10,INI_filename);

	GetPrivateProfileString("CompileCmd",GL_languageName,"",compileCmd_str,1024,INI_filename);

	GetPrivateProfileString("RunCmd",GL_languageName,"",runCmd_str,1024,INI_filename);

	GetPrivateProfileString("SourcePath",GL_languageName,"",sourcePath,1024,INI_filename);

	GetPrivateProfileString("ExePath",GL_languageName,"",exePath,1024,INI_filename);

	isTranscoding=GetPrivateProfileInt("Transcoding",GL_languageName,0,INI_filename);

	limitIndex=GetPrivateProfileInt("TimeLimit",GL_languageName,1,INI_filename);

	nProcessLimit=GetPrivateProfileInt("ProcessLimit",GL_languageName,1,INI_filename);


	char buf[1024];
	sprintf(buf, "%d", GL_solutionId);
	string name = buf;
	string compile_string=compileCmd_str;
	replace_all_distinct(compile_string,"%PATH%",workPath);
	replace_all_distinct(compile_string,"%NAME%",name);
	replace_all_distinct(compile_string,"%EXT%",GL_languageExt);
	replace_all_distinct(compile_string,"%EXE%",GL_languageExe);
	strcpy(compileCmd_str,compile_string.c_str());      //编译命令行
	//	cout<<CompileCmd_str<<endl;

	string runcmd_string=runCmd_str;
	replace_all_distinct(runcmd_string,"%PATH%",workPath);
	replace_all_distinct(runcmd_string,"%NAME%",name);
	replace_all_distinct(runcmd_string,"%EXT%",GL_languageExt);
	replace_all_distinct(runcmd_string,"%EXE%",GL_languageExe);
	strcpy(runCmd_str,runcmd_string.c_str());			//运行命令行
	//	cout<<RunCmd_str<<endl;

	string sourcepath_string=sourcePath;
	replace_all_distinct(sourcepath_string,"%PATH%",workPath);
	replace_all_distinct(sourcepath_string,"%NAME%",name);
	replace_all_distinct(sourcepath_string,"%EXT%",GL_languageExt);
	strcpy(sourcePath,sourcepath_string.c_str());		//源程序路径
	//  cout<<SourcePath<<endl;

	string exepath_string=exePath;
	replace_all_distinct(exepath_string,"%PATH%",workPath);
	replace_all_distinct(exepath_string,"%NAME%",name);
	replace_all_distinct(exepath_string,"%EXE%",GL_languageExe);
	strcpy(exePath,exepath_string.c_str());				//可执行文件路径
	//	cout<<ExePath<<endl;

	sprintf(DebugFile,"%s%s.txt",workPath,name.c_str()); //debug文件路径
	sprintf(ErrorFile,"%s%s_re.txt",workPath,name.c_str()); //re文件路径

	if( (_access(judgeLogPath, 0 )) == -1 )   {
		CreateDirectory(judgeLogPath,NULL);
	}

	sprintf(judge_log_filename,"%sjudge-log-%d.log",judgeLogPath,GL_solutionId);

	//	cout<<DebugFile<<endl;
}

DWORD WINAPI CompileThread(LPVOID lp) //ac
{
	STARTUPINFO StartupInfo = {0};

	SQL_updateSolution(GL_solutionId,V_C,0,0,0); //V_C Compiling

	system(compileCmd_str);


	return 0;
}
//编译,是否应该防止编译器假死造成的卡死
int compile(){

	if(strcmp(compileCmd_str,"NULL")==0) return 1;

	HANDLE hThread_com;

	hThread_com=CreateThread(NULL,NULL,CompileThread,NULL,0,NULL);
	if(hThread_com==NULL) {
		write_log(JUDGE_ERROR,"Create CompileThread Error");
		CloseHandle(hThread_com);
	}

	DWORD status_ = WaitForSingleObject(hThread_com,30000);   //30S 编译时间,返回值大于零说明超时
	if(status_>0){
		write_log(JUDGE_WARNING,"Compile over time_limit");
		TerminateProcess(G_pi_com.hProcess, 0);
	}

	//是否正常生成用户的可执行程序
	if( (_access(exePath, 0 )) != -1 )   {
		/* ok */
		return 1;
	}
	else
	{
		return 0;
	}
}

//是否存在异常
BOOL RUN_existException(DWORD dw){
	switch(dw){
	case EXCEPTION_ACCESS_VIOLATION:
		return TRUE;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return TRUE;
	case EXCEPTION_BREAKPOINT:
		return TRUE;
	case EXCEPTION_SINGLE_STEP:
		return TRUE;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return TRUE;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return TRUE;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return TRUE;
	case EXCEPTION_FLT_INEXACT_RESULT:
		return TRUE;
	case EXCEPTION_FLT_INVALID_OPERATION:
		return TRUE;
	case EXCEPTION_FLT_OVERFLOW:
		return TRUE;
	case EXCEPTION_FLT_STACK_CHECK:
		return TRUE;
	case EXCEPTION_FLT_UNDERFLOW:
		return TRUE;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return TRUE;
	case EXCEPTION_INT_OVERFLOW:
		return TRUE;
	case EXCEPTION_PRIV_INSTRUCTION:
		return TRUE;
	case EXCEPTION_IN_PAGE_ERROR:
		return TRUE;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return TRUE;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return TRUE;
	case EXCEPTION_STACK_OVERFLOW:
		return TRUE;
	case EXCEPTION_INVALID_DISPOSITION:
		return TRUE;
	case EXCEPTION_GUARD_PAGE:
		return TRUE;
	case EXCEPTION_INVALID_HANDLE:
		return TRUE;
	default:
		return FALSE;
	}
}

HANDLE G_job=NULL;
HANDLE InputFile ;  //父进程输入文件句柄
HANDLE OutputFile;  //子进程标准输出句柄
DWORD g_dwCode;	//定义进程状态

HANDLE CreateSandBox(){
	HANDLE hjob =CreateJobObject(NULL,NULL);
	if(hjob!=NULL)
	{
		JOBOBJECT_BASIC_LIMIT_INFORMATION jobli;
		 memset(&jobli,0,sizeof(jobli));
		jobli.LimitFlags=JOB_OBJECT_LIMIT_PRIORITY_CLASS|JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
		jobli.PriorityClass=IDLE_PRIORITY_CLASS;
		jobli.ActiveProcessLimit=nProcessLimit; //Limit of processes
	//	jobli.MinimumWorkingSetSize= 1;
	//	jobli.MaximumWorkingSetSize= 1024*GL_memory_limit;|JOB_OBJECT_LIMIT_WORKINGSET|JOB_OBJECT_LIMIT_PROCESS_TIME
	//	jobli.PerProcessUserTimeLimit.QuadPart=10000*(GL_time_limit+2000);
		if(SetInformationJobObject(hjob,JobObjectBasicLimitInformation,&jobli,sizeof(jobli)))
		{
			JOBOBJECT_BASIC_UI_RESTRICTIONS jobuir;
			jobuir.UIRestrictionsClass=JOB_OBJECT_UILIMIT_NONE;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_EXITWINDOWS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_READCLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_WRITECLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_HANDLES;

			if(SetInformationJobObject(hjob,JobObjectBasicUIRestrictions,&jobuir,sizeof(jobuir)))
			{
				return hjob;
			}
			else
			{
				write_log(JUDGE_SYSTEM_ERROR,"SetInformationJobObject  JOBOBJECT_BASIC_UI_RESTRICTIONS   [Error:%d]\n",GetLastError());
			}
		}
		else
		{
			write_log(JUDGE_SYSTEM_ERROR,"SetInformationJobObject  JOBOBJECT_BASIC_LIMIT_INFORMATION   [Error:%d]\n",GetLastError());
		}
	}
	else
	{
		write_log(JUDGE_SYSTEM_ERROR,"CreateJobObject     [Error:%d]\n",GetLastError());
	}
	return NULL;
}

bool ProcessToSandbox(HANDLE job,PROCESS_INFORMATION p){
	if(AssignProcessToJobObject(job,p.hProcess))
	{
		//顺便调整本进程优先级为高
		/*HANDLE   hPS   =   OpenProcess(PROCESS_ALL_ACCESS,   false,  p.dwProcessId);
		if(!SetPriorityClass(hPS,   HIGH_PRIORITY_CLASS)){
			write_log(JUDGE_SYSTEM_ERROR,"SetPriorityClass        [Error:%d]\n",GetLastError());
		}
		CloseHandle(hPS);*/
		return true;
	}
	else
	{
		write_log(JUDGE_SYSTEM_ERROR,"AssignProcessToJobObject Error:%s",GetLastError());
	}
	return false;
}

DWORD WINAPI RUN_ProgramThread(LPVOID lp) //ac
{
	/// cmd/c solution.exe <data.in >data.out 2>error.txt
	//ChildIn_Write是子进程的输入句柄，ChildIn_Read是父进程用于写入子进程输入的句柄
	HANDLE ChildIn_Read, ChildIn_Write;
	//ChildOut_Write是子进程的输出句柄，ChildOut_Read是父进程用于读取子进程输出的句柄
	HANDLE ChildOut_Read, ChildOut_Write;

	SECURITY_ATTRIBUTES saAttr = {0};
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	CreatePipe(&ChildIn_Read, &ChildIn_Write, &saAttr, 0);
	SetHandleInformation(ChildIn_Write, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&ChildOut_Read, &ChildOut_Write, &saAttr, 0);
	SetHandleInformation(ChildOut_Read, HANDLE_FLAG_INHERIT, 0);

	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	STARTUPINFO StartupInfo = {0};
	StartupInfo.cb = sizeof(STARTUPINFO);
	//	StartupInfo.hStdError = h_ErrorFile;
	StartupInfo.hStdOutput = ChildOut_Write;
	StartupInfo.hStdInput = ChildIn_Read;
	StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

	/* |CREATE_NEW_CONSOLE */
	if(CreateProcess(NULL,runCmd_str,NULL,NULL,TRUE,CREATE_SUSPENDED,NULL,NULL,&StartupInfo,&G_pi))
	{

		G_job = CreateSandBox();
		if(G_job!=NULL)
		{
			if(ProcessToSandbox(G_job,G_pi))
			{
				ResumeThread(G_pi.hThread);
				CloseHandle(G_pi.hThread);

				InputFile= CreateFile(inFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
				BOOL flag = FALSE;
				while (true)
				{
					char buffer[BUFSIZE] = {0};
					DWORD BytesRead, BytesWritten;
					flag = ReadFile(InputFile, buffer, BUFSIZE, &BytesRead, NULL);
					if (!flag || (BytesRead == 0)) break;
					flag = WriteFile(ChildIn_Write, buffer, BytesRead, &BytesWritten, NULL);

					if (!flag){ break;}
				}

				CloseHandle(InputFile);InputFile=NULL;
				CloseHandle(ChildIn_Write);ChildIn_Write=NULL;
				CloseHandle(ChildOut_Write);ChildOut_Write=NULL;

				//读取子进程的标准输出，并将其传递给文件输出
				OutputFile= CreateFile(outFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				startt = clock();

				DWORD limit_output =0;
				while (true)
				{
					char buffer[BUFSIZE] = {0};
					DWORD BytesRead, BytesWritten;
					flag = ReadFile(ChildOut_Read, buffer, BUFSIZE, &BytesRead, NULL);
					if (!flag || (BytesRead == 0)) break;
					flag = WriteFile(OutputFile, buffer, BytesRead, &BytesWritten, NULL);
					if (!flag) break;

					limit_output+=BytesWritten;
					if(limit_output>OutputLimit){
						write_log(JUDGE_INFO,"OLE");
						GL_verdictId = V_OLE;
						//CloseHandle(pi.hProcess);
						break;
					}
				}
				endt = clock();

				CloseHandle(ChildIn_Read);ChildIn_Read=NULL;
				CloseHandle(ChildOut_Read);ChildOut_Read=NULL;
				CloseHandle(OutputFile);OutputFile=NULL;
				//printf("OK\n");
				return 1;
			}else{
				write_log(JUDGE_SYSTEM_ERROR,"ProcessToSandBox Error:%s",GetLastError());
			}
		}
		else{
			write_log(JUDGE_SYSTEM_ERROR,"CreateSandBox Error:%s",GetLastError());
		}
	}
	else
	{
		write_log(JUDGE_SYSTEM_ERROR,"CreateProcess       [Error:%d]\n",GetLastError());
	}
	GL_verdictId = V_SE;
	return 0;
}

int special_judge(const char *inFile,const char *uOutFile){ //return 0 ====Error,return 1 ====Accepted
	int judge ;
	char spj_path[MAX_PATH];
	sprintf(spj_path,"%s%d\\spj_%d.exe %s %s",dataPath,GL_problemId,GL_problemId,inFile,uOutFile);
	judge = system(spj_path) ;

	if (judge == -1){
		//printf("system error!") ;
		return 0;
	}
	if(judge == 1){  //spj返回1,表示程序正确
		return 1;
	}
	return 0;
}

int RUN_solution(int solutionId)
{
	long caseTime=0;
	int i,case_;
	char srcPath[MAX_PATH];
	char ansPath[MAX_PATH];
	char buf[40960];

	g_dwCode=0;

	for(i=0;;++i){
		if(i==0){
			case_=1;
		}else{
			case_=i;
		}
		sprintf(inFileName,"%s%d\\data%d.in",dataPath,GL_problemId,case_);
		sprintf(outFileName,"%s%d_%d.out",workPath,solutionId,case_);
		sprintf(srcPath, "%s",outFileName);
		sprintf(ansPath, "%s%d\\data%d.out",dataPath,GL_problemId,case_);

		if( (_access(inFileName, 0 )) == -1 )   {
			write_log(JUDGE_INFO,"Test over..");
			break ;
		}
		GL_testcase = case_;

		SQL_updateSolution(solutionId,V_RUN,case_,GL_time-GL_time%10,GL_memory);

		HANDLE hThread_run;
		hThread_run=CreateThread(NULL,NULL,RUN_ProgramThread,NULL,0,NULL);
		if(hThread_run==NULL) {
			write_log(JUDGE_ERROR,"Create thread error");
			CloseHandle(hThread_run);
		}

		DWORD status_ = WaitForSingleObject(hThread_run,GL_time_limit+2000);   //放宽时限2S,返回值大于零说明超时.
		if(status_>0){
			write_log(JUDGE_INFO,"hThread_run TIME LIMIT");
			GL_time = GL_time_limit;
			if(GL_verdictId==V_AC)
			{
				GL_verdictId = V_TLE;
			}
		}

		if(InputFile!=NULL) CloseHandle(InputFile);
		if(OutputFile!=NULL) CloseHandle(OutputFile);

		//get memory info
		PROCESS_MEMORY_COUNTERS   pmc;
		unsigned long tmp_memory=0;

		#if 0
		 //del for mingw
		if(GetProcessMemoryInfo(G_pi.hProcess,&pmc,sizeof(pmc))) {
			tmp_memory=pmc.PeakWorkingSetSize/1024;
			if(tmp_memory>GL_memory) GL_memory=tmp_memory;
		}
        #endif

		//get process state
		GetExitCodeProcess(G_pi.hProcess, &g_dwCode);
		if(RUN_existException(g_dwCode))
		{
			GL_verdictId=V_RE;
			goto l;
		}
		else if(g_dwCode==STILL_ACTIVE)
		{	//超时
			puts("TIME LIMIT");
			TerminateProcess(G_pi.hProcess, 0);
			if(GL_verdictId==V_AC)
			{
				//TLE 超时
				GL_verdictId=V_TLE;
				GL_time = GL_time_limit;
				goto l;
			}

		}
		caseTime = endt - startt;
		if(caseTime<0){
			caseTime = GL_time_limit;
		}

		TerminateJobObject(G_job,0);//exit
		CloseHandle(G_pi.hProcess);
		CloseHandle(G_job);G_job=NULL;


		GL_time = (caseTime>GL_time)?caseTime:GL_time;

		if(GL_time>=GL_time_limit){
			GL_verdictId=V_TLE;
			GL_time = GL_time_limit;
			goto l;
		}
		if(GL_memory>=GL_memory_limit){
			GL_verdictId=V_MLE;
			GL_memory = GL_memory_limit;
			goto l;
		}
		//OLE
		if(GL_verdictId!=V_AC){
			goto l;
		}
		//judge file，spj or not

		if(GL_spj==1){
			int verdict_ = special_judge(inFileName,outFileName);
			if(verdict_) GL_verdictId=V_AC;
			else GL_verdictId=V_WA;
		}else{
			GL_verdictId = compare(srcPath,ansPath);
		}

l:		write_log(JUDGE_INFO,"ID:%d Test%d ,%s ,%dms %dkb ,Return code:%u",GL_solutionId,i,VERDICT_NAME[GL_verdictId],caseTime,tmp_memory,g_dwCode);

		/* write judge-log */

		if (i == 0)
		{
			reset_file(judge_log_filename);
			if (GL_verdictId!=V_AC)
			{
				i  = 1;
				write_buffer(judge_log_filename,"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
					i,GL_time-GL_time%10,tmp_memory,g_dwCode,VERDICT_NAME[GL_verdictId]);

				memset(buf,0,sizeof(buf));
				read_buffer(inFileName, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(judge_log_filename,"\nInput\n",i);
				write_buffer(judge_log_filename,buf);

				memset(buf,0,sizeof(buf));
				read_buffer(outFileName, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(judge_log_filename,"\nOutput\n",i);
				write_buffer(judge_log_filename,buf);

				memset(buf,0,sizeof(buf));
				read_buffer(ansPath, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(judge_log_filename,"\nAnswer\n");
				write_buffer(judge_log_filename,buf);

				write_buffer(judge_log_filename,"\n------------------------------------------------------------------\n");
				break;
			}
		}
		else
		{
			write_buffer(judge_log_filename,"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
				i,caseTime - caseTime%10,tmp_memory,g_dwCode,VERDICT_NAME[GL_verdictId]);

			memset(buf,0,sizeof(buf));
			read_buffer(inFileName, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(judge_log_filename,"\nInput\n");
			write_buffer(judge_log_filename,buf);

			memset(buf,0,sizeof(buf));
			read_buffer(outFileName, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(judge_log_filename,"\nOutput\n");
			write_buffer(judge_log_filename,buf);

			memset(buf,0,sizeof(buf));
			read_buffer(ansPath, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(judge_log_filename,"\nAnswer\n");
			write_buffer(judge_log_filename,buf);

			write_buffer(judge_log_filename,"\n------------------------------------------------------------------\n");
		}

		if(GL_verdictId!=V_AC){
			break;
		}
		if(i==0){
			GL_time=0;
			GL_memory=0;
		}
	}
	return 0;

}

void resetVal(){
	GL_verdictId=V_AC;
	GL_contestId=0;
	GL_time=0;
	GL_memory=0;
	GL_time_limit=1000;
	GL_memory_limit=65535;
	GL_reJudge=0;
	GL_testcase=0;
}



int Judge_Local()
{
	char buf[4096] = {0};

	if(0 == compile())
	{
		GL_verdictId=V_CE;
		SQL_updateCompileInfo(GL_solutionId);

		reset_file(judge_log_filename);

		write_buffer(judge_log_filename,"Test: #1, time: 0 ms, memory: 0 kb, exit code: 0,verdict: %s\n",VERDICT_NAME[GL_verdictId]);
	}
	else
	{
		write_log(JUDGE_INFO,"Start Run...");
		RUN_solution(GL_solutionId);
	}

	return OS_TRUE;
}


int work(int solutionId)
{
	int ret = OS_OK;
	int isExist = OS_NO;

	GL_solutionId = solutionId;

	resetVal();//重置

	ret = SQL_getSolutionInfo(&isExist);
	if (OS_ERR == ret || OS_NO == isExist)
	{
		pdt_debug_print("No such solution %d.", solutionId);
		return OS_ERR;
	}

	//包含sourcePath,所以在SQL_getSolutionSource之前
	InitPath();

	//取出source，并保存到sourcePath
	ret = SQL_getSolutionSource();
	if (OS_OK != ret)
	{
		pdt_debug_print("SQL_getSolutionSource failed.(solutionId=%d)", solutionId);
		write_log(JUDGE_INFO,"SQL_getSolutionSource failed.(solutionId=%d)", solutionId);
		return OS_ERR;
	}

	ret = SQL_getProblemInfo();
	if (OS_OK != ret)
	{
		pdt_debug_print("SQL_getProblemInfo failed.(solutionId=%d)", solutionId);
		write_log(JUDGE_INFO,"SQL_getProblemInfo failed.(solutionId=%d)", solutionId);
		return OS_ERR;
	}

	if (1 == GL_vjudge)
	{
		#if(JUDGE_VIRTUAL == VOS_YES)
		(void)Judge_Remote();
		#else
		pdt_debug_print("virtua-judge is not support.");
		return OS_ERR;
		#endif
		g_dwCode = 0;
		GL_testcase = 0;
	}
	else
	{
		GL_time_limit*=limitIndex;
		GL_memory_limit*=limitIndex;
		(void)Judge_Local();
	}


	//update MySQL............
	//write_log(JUDGE_INFO,"ID:%d ->Rusult:%s Case:%d %dms %dkb ,Return code:%u at %s by %s",GL_solutionId,VERDICT_NAME[GL_verdictId],GL_testcase,GL_time-GL_time%10,GL_memory,g_dwCode,time_string_.c_str(),GL_username);

	SQL_updateSolution(GL_solutionId,GL_verdictId,GL_testcase,GL_time-GL_time%10,GL_memory);
	SQL_updateProblem(GL_problemId);
	SQL_updateUser(GL_username);

	//contest or not
	if(GL_contestId > 0)
	{
		//contest judge
		time_t contest_s_time,contest_e_time;
		char num[10]={0};

		/* 获取contest problem题目标号 */
		SQL_getProblemInfo_contest(GL_contestId,GL_problemId,num);
		SQL_getContestInfo(GL_contestId,contest_s_time,contest_e_time);

		if(contest_e_time>GL_submitDate)
		{
			/* 比赛running ，修改Attend */
			SQL_updateAttend_contest(GL_contestId,GL_verdictId,GL_problemId,num,GL_username,contest_s_time,contest_e_time);
		}

		SQL_updateProblem_contest(GL_contestId,GL_problemId);
	}

	DeleteFile(sourcePath);
	DeleteFile(DebugFile);
	DeleteFile(exePath);

	return OS_OK;
}

void Judge_PushQueue(int solutionId)
{
	JUDGE_DATA jd = {0};

	jd.solutionId = solutionId;
	Q.push(jd);
}

DWORD WINAPI WorkThread(LPVOID lpParam)
{
	int ret = OS_OK;
	JUDGE_DATA jd;
	time_t first_t,second_t;
	string str_time;
    time(&first_t);
	for (;;)
	{
		if(Q.size()>limitJudge)
		{
			int a=0;
			return 0;
		}

		if(!Q.empty())
		{
				jd=Q.front();
			  	GL_currentId=jd.solutionId;

				//judge_outstring("Info: Start to judge the solution, please wait...");
				//MSG_StartDot();

				/* 启动评判 */
				ret = work(GL_currentId);
				Q.pop();

				//MSG_StopDot();
				//judge_outstring("done.\r\n");
				if (OS_OK == ret)
				{
					string time_string_;
					API_TimeToString(time_string_,GL_submitDate);
					judge_outstring("\r\n -----------------------"
								"\r\n     *Judge verdict*"
								"\r\n -----------------------"
								"\r\n SolutionId   : %3d"
								"\r\n Pasted cases : %3d"
								"\r\n Time-used    : %3d ms"
								"\r\n Memory-used  : %3d kb"
								"\r\n Return code  : %3u"
								"\r\n Verdict      : %3s"
								"\r\n Submit Date  : %3s"
								"\r\n Username     : %3s"
								"\r\n -----------------------\r\n",
									GL_solutionId,GL_testcase,GL_time-GL_time%10,GL_memory,
									g_dwCode,VERDICT_NAME[GL_verdictId],time_string_.c_str(),GL_username);

				}

		}
		Sleep(1);
	}

	write_log(JUDGE_ERROR,"WorkThread Crash");

	return 0;
}

DWORD WINAPI ListenThread(LPVOID lpParam)
{
	// 循环接受客户的连接请求
	sockaddr_in remoteAddr;
	SOCKET sClient;
	//初始化客户地址长度
	int nAddrLen = sizeof(remoteAddr);
	JUDGE_DATA j;
	while(TRUE)
	{
		sClient = accept(sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if(sClient == INVALID_SOCKET){
			write_log(JUDGE_ERROR,"Accept() Error");
			continue;
		}
		int ret=recv(sClient,(char*)&j,sizeof(j),0);
		if(ret>0){
			write_log(JUDGE_INFO,"Push SolutionId:%d into Judge Queue....",j.solutionId);
			Q.push(j);
		}
		Sleep(1);
	}
	write_log(JUDGE_ERROR,"ListenThread Crash");
	closesocket(sClient);
	return 0;
}

long WINAPI ExceptionFilter(EXCEPTION_POINTERS * lParam)
{
	write_log(JUDGE_ERROR,"System Error! \r\n System may restart after 2 seconds...");
	Sleep(2000);
	ShellExecuteA(NULL,"open",judgePath,NULL,NULL,SW_SHOWNORMAL);
	return EXCEPTION_EXECUTE_HANDLER;
}

int OJ_main()
{
	SetUnhandledExceptionFilter(ExceptionFilter);
 	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	if( (_access(logPath, 0 )) == -1 )   {
		CreateDirectory(logPath,NULL);
	}

	//关闭调试开关
	Judge_DebugSwitch(JUDGE_DEBUG_OFF);

	write_log(JUDGE_INFO,"Running Judge Core...");
	InitMySqlConfig();
	if(InitMySQL()==0){//初始化mysql
		write_log(JUDGE_ERROR,"Init MySQL JUDGE_ERROR...");
		return 0;
	}
	write_log(JUDGE_INFO,"Init MySQL Success...");
	if(initSocket()==0){         // 初始化socket
		write_log(JUDGE_ERROR,"Init Socket JUDGE_ERROR...");
		return 0;
	}

	//创建工作线程
	HANDLE hThreadW;
	hThreadW=CreateThread(NULL,NULL,WorkThread,0,0,0);
	//进入循环，等待客户连接请求
	HANDLE hThreadR=CreateThread(NULL,NULL,ListenThread,0,0,0);

	write_log(JUDGE_INFO,"Judge Task init ok...");

	for(;;)
	{
		Sleep(1);
	}

	closesocket(sListen);
	WSACleanup();
}

void OJ_TaskEntry()
{
	pdt_debug_print("OJ task init ok...");

	(void)OJ_main();

	//t_oj.detach();

	/* 在主线程里面执行t.detach()将子线程从主线程里分离，
		子线程执行完成后会自己释放掉资源。分离后的线程，主线程将对它没有控制权了。
	*/

}

#if 0
int main(int argc, char **argv)
{
	OJ_TaskEntry();
	return 0;
}
#endif
