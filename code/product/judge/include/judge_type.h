#ifndef _JUDGE_TYPE_H_
#define _JUDGE_TYPE_H_

#define JUDGE_OJNAME_SIZE_MAX 32

typedef struct tagJudge_Data_S
{
	int solutionId;
	int vtyId;
	
}JUDGE_DATA_S;

typedef struct tag_Judge_ProblemInfo_ST
{
	int problemId;
	int time_limit;
	int memory_limit;
	int isSpecialJudge;

	int isVirtualJudge;
	int virtualPID;
	char szVirJudgerName[JUDGE_OJNAME_SIZE_MAX]; /* oj name */

}JUDGE_PROBLEM_INFO_S;


typedef struct tag_Judge_Solution_ST
{
	int solutionId;
	int problemId;
	int languageId;
	int verdictId;
	int contestId;
	time_t submitDate;
	char username[MAX_NAME];

	int time_used;
	int memory_used;
	int testcase;
	int failcase; 
	
	int reJudge;
	char languageName[100];
	char languageExt[10];
	char languageExe[10];
	
}JUDGE_SOLUTION_S;

#define JUDGE_CMD_BUFFER 1024

typedef struct tag_Judge_Submission_ST
{
	JUDGE_SOLUTION_S     stSolution;
	JUDGE_PROBLEM_INFO_S stProblem;

	int time_used_case;     /* ����case��ʱ */
	int memory_used_case;

    int isTranscoding;   /* ���VS��ת�� */
    int limitIndex;
    int nProcessLimit;

	char szSource[MAX_CODE];

	char languageName[100];
	char languageExt[10];
	char languageExe[10];

	unsigned long ulSeed;  /* ������� */
	char subPath[MAX_PATH]; /* ��Ŀ¼ */


	char sourcePath[MAX_PATH];
	char exePath[MAX_PATH];
    char inFileName[MAX_PATH];
    char outFileName[MAX_PATH];
	char stdOutFileName[MAX_PATH];
    char DebugFile[MAX_PATH];
    char ErrorFile[MAX_PATH];
    char judge_log_filename[MAX_PATH];

	char *pszjudgeResult_Json; /* ��ʽ: json */

    char compileCmd[1024];
    char runCmd[1024];

#if (OS_YES == OSP_MODULE_JUDGE_LOCAL)
	#ifdef _WIN32_
	PROCESS_INFORMATION stProRunInfo; /* ���н�����Ϣ */
	PROCESS_INFORMATION stProComInfo; /* ���������Ϣ */

	HANDLE hJob;         /* ��ҵ��� */
    HANDLE hInputFile ;  /* �����������ļ���� */
    HANDLE hOutputFile;  /* �ӽ��̱�׼������ */
	#endif
#endif

    DWORD dwProStatusCode;     /* �������״̬ */

	clock_t startt; /* ÿ��run��ʱ��� */
	clock_t endt ;  /* ÿ��run��ʱ��� */

}JUDGE_SUBMISSION_ST;

#endif
