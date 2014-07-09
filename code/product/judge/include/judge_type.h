#ifndef _JUDGE_TYPE_H_
#define _JUDGE_TYPE_H_

#include "product\judge\include\judge_inc.h"

#define JUDGE_OJNAME_SIZE_MAX 32

typedef struct tag_Judge_ProblemInfo_ST
{
	int problemId;
	int time_limit;
	int memory_limit;
	int isSpecialJudge;

	int isVirtualJudge;
	int virtualPID;
	char szVirJudgerName[JUDGE_OJNAME_SIZE_MAX]; /* oj name */

}JUDGE_PROBLEM_INFO_ST;


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

	int reJudge;
	char languageName[100]={0};
	char languageExt[10]={0};
	char languageExe[10]={0};

}JUDGE_SOLUTION_ST;

#define JUDGE_CMD_BUFFER 1024

typedef struct tag_Judge_Submission_ST
{
	JUDGE_SOLUTION_ST     stSolution;
	JUDGE_PROBLEM_INFO_ST stProblem;

    int isTranscoding;   /* ���VS��ת�� */
    int limitIndex;
    int nProcessLimit;

	char szSource[MAX_CODE];

	char languageName[100];
	char languageExt[10];
	char languageExe[10];

	char sourcePath[MAX_PATH];
	char exePath[MAX_PATH];
    char inFileName[MAX_PATH];
    char outFileName[MAX_PATH];
    char DebugFile[MAX_PATH];
    char ErrorFile[MAX_PATH];
    char judge_log_filename[MAX_PATH];

    char compileCmd[1024];
    char runCmd[1024];

	PROCESS_INFORMATION pProRunInfo; /* ���н�����Ϣ */
	PROCESS_INFORMATION pProComInfo; /* ���������Ϣ */

	HANDLE hJob;         /* ��ҵ��� */

    HANDLE hInputFile ;  /* �����������ļ���� */
    HANDLE hOutputFile;  /* �ӽ��̱�׼������ */

    DWORD dwProStatusCode;     /* �������״̬ */

	clock_t startt; /* ÿ��run��ʱ��� */
	clock_t endt ;  /* ÿ��run��ʱ��� */

}JUDGE_SUBMISSION_ST;

#endif