#ifndef _JUDGE_SQL_H_
#define _JUDGE_SQL_H_


extern MYSQL *mysql;               //mysql����
extern char query[];           //��ѯ���
extern char Mysql_url[255];
extern char Mysql_username[255];
extern char Mysql_password[255];
extern char Mysql_table[255];
extern int  Mysql_port;
extern char Mysql_Character[255];  //����

#define SQL_Debug(x, format, ...) debugcenter_print(MID_SQL, x, format, ##__VA_ARGS__)

extern int SQL_InitMySQL();
extern void SQL_getUnJudgeSolutions(JUDGE_DATA_S *pJudgeData, int *n, int iMax);
extern int SQL_getSolutionSource(JUDGE_SUBMISSION_ST *pstJudgeSubmission);
extern int SQL_getSolutionByID(int solutionID, JUDGE_SOLUTION_S *pstJudgeSolution, int *pIsExist);
extern int SQL_getProblemInfo(JUDGE_PROBLEM_INFO_S *pstProblem);
extern int SQL_getProblemInfo_contest(int contestId,int problemId,char *num);
extern int SQL_getContestInfo(int contestId,time_t &start_time,time_t &end_time);
extern int SQL_getContestAttend(int contestId,char *username,char num,long &ac_time,int &wrongsubmits);
extern int SQL_countContestProblems(int contestId);
extern int SQL_getFirstACTime_contest(int contestId,int problemId,char *username,time_t &ac_time,time_t start_time,time_t end_time);
extern long SQL_countProblemVerdict(int contestId,int problemId,int verdictId,char *username);
extern void SQL_updateSolution(int solutionId,int verdictId,int testCase,int failcase,int time,int memory);
extern void SQL_updateSolutionJsonResult(int solutionId, char *pJsonResult);
extern void SQL_updateProblem(int problemId);
extern void SQL_updateProblem_contest(int contestId,int problemId);
extern int SQL_countACContestProblems(int contestId,char *username,time_t start_time,time_t end_time);
extern int SQL_getContestScore(int contestId,char *username,time_t start_time,time_t end_time);
extern void SQL_updateAttend_contest(int contestId,int verdictId,int problemId,
										char *num,char *username,time_t start_time,time_t end_time);
extern void SQL_updateUser(char *username);
extern void SQL_updateCompileInfo(JUDGE_SUBMISSION_ST *pstJudgeSubmission);
extern int SQL_getUserByname(const char *username);
extern int SQL_UserLogin(const char *username, const char *password);
#endif
