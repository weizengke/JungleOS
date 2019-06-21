/*
  ����OJ MySQL���ݿ����
*/


#include "product/judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

MYSQL *mysql = NULL;
char query[40960];
char Mysql_url[255];
char Mysql_username[255];
char Mysql_password[255];
char Mysql_table[255];
int  Mysql_port;
char Mysql_Character[255];

/* BEGIN: Added by weizengke, 2014/7/10  for ���߳������ź����������ݿ���� */

mutex_t hSemaphore_SQL;

void SQL_CreateSem()
{
	hSemaphore_SQL = mutex_create("SQL_SEM");
}
void SQL_SemP()
{
	(void)mutex_lock(hSemaphore_SQL);
}

void SQL_SemV()
{
	(void)mutex_unlock(hSemaphore_SQL);
}

/* END:   Added by weizengke, 2014/7/10 */


ULONG SQL_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{

	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;
		
	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"mysql url %s port %u username %s password %s table %s", 
						Mysql_url, Mysql_port, Mysql_username, Mysql_password, Mysql_table);

	return OS_OK;
}


/* ��ʼ��mysql���������ַ��� */
int SQL_InitMySQL()
{
	#if 0
	(void)BDN_RegistBuildRun(MID_SQL, VIEW_SYSTEM, BDN_PRIORITY_NORMAL, SQL_BuildRun);
	#endif
	
	SQL_CreateSem();

	mysql = mysql_init((MYSQL*)0);	
	if (mysql != 0 &&
		!mysql_real_connect(mysql,Mysql_url,
							Mysql_username,
							Mysql_password,
							Mysql_table,
							Mysql_port,
							NULL,
							CLIENT_MULTI_STATEMENTS ))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	/* �����Զ����� */
	my_bool my_true= TRUE;
	(void)mysql_options(mysql, MYSQL_OPT_RECONNECT, &my_true);

	strcpy(query,"SET CHARACTER SET gbk"); //���ñ��� gbk
	int ret = mysql_real_query(mysql,query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	write_log(JUDGE_INFO,"Connect MySQL(%s, %s, %s, %s, %d) ok...",Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);
	printf("Connect MySQL(%s, %s, %s, %s, %d) ok...\r\n",Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);

	return 1;
}

int SQL_Destroy()
{
	mysql_close(mysql);

	return 0;
}

#if M_DES("�ڲ�����������P�ź���",1)
int SQL_getFirstACTime_contest(int contestId,int problemId,char *username,time_t &ac_time,time_t start_time,time_t end_time)
{
	string s_t,e_t;
	
	(VOID)util_time_to_string(s_t,start_time);
	(VOID)util_time_to_string(e_t,end_time);

	sprintf(query,"select submit_date from solution where contest_id=%d and problem_id=%d and username='%s'and verdict=%d and submit_date between '%s' and '%s' order by solution_id ASC limit 1;",contestId,problemId,username,V_AC,s_t.c_str(),e_t.c_str());
	int ret = mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		(void)util_string_to_time(row[0], ac_time);
	}
	else
	{
		return 0;
	}

	mysql_free_result(recordSet);

	return 1;
}

/* Note: No need to P Sem!!!! */
int SQL_countACContestProblems(int contestId,char *username,time_t start_time,time_t end_time){

	string s_t,e_t;
	
	(VOID)util_time_to_string(s_t,start_time);
	(VOID)util_time_to_string(e_t,end_time);

	sprintf(query,"select count(distinct(problem_id)) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s'",V_AC,contestId,username,s_t.c_str(),e_t.c_str());

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	int nCount=0;
	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return nCount;
}


int SQL_getContestScore(int contestId,char *username,time_t start_time,time_t end_time){

	if(SQL_countACContestProblems(contestId,username,start_time,end_time)==0)
	{
		return 0;
	}

	string s_t,e_t;
	
	(VOID)util_time_to_string(s_t,start_time);
	(VOID)util_time_to_string(e_t,end_time);

	sprintf(query,"SELECT sum(point) from contest_problem where contest_id=%d and problem_id in (select distinct(problem_id) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s')",contestId,V_AC,contestId,username,s_t.c_str(),e_t.c_str());

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		write_log(JUDGE_ERROR,"SQL_getContestScore recordSet JUDGE_ERROR");
	}

	int point=0;
	MYSQL_ROW row;

	if(row=mysql_fetch_row(recordSet))
	{
		point=atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return point;
}
int SQL_getContestAttend(int contestId,char *username,char num,long &ac_time,int &wrongsubmits)
{
	sprintf(query,"select %c_time,%c_wrongsubmits from attend where contest_id=%d and username='%s';",num,num,contestId,username);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		ac_time=atol(row[0]);
		wrongsubmits=atoi(row[1]);
	}

	mysql_free_result(recordSet);

	return 1;
}

int SQL_countContestProblems(int contestId)
{
	sprintf(query,"select count(problem_id) from contest_problem where contest_id=%d",contestId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	int nCount=0;
	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return nCount;
}

long SQL_countProblemVerdict(int contestId,int problemId,int verdictId,char *username)
{

	sprintf(query,"select count(solution_id) from solution where contest_id=%d and problem_id=%d and verdict=%d and username='%s'",contestId,problemId,verdictId,username);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	long nCount=0;

	//��ȡ����
	MYSQL_ROW row; //һ�������ݵ����Ͱ�ȫ(type-safe)�ı�ʾ
	if(row=mysql_fetch_row(recordSet))  //��ȡ��һ����¼
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);//�ͷŽ����

	return nCount;
}


#endif

int SQL_getSolutionSource(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"SQL_getSolutionSourceEx, Input param is null...");
		return OS_ERR;
	}

	SQL_SemP();

	sprintf(query,"select source from solution_source where solution_id=%d",
			pstJudgeSubmission->stSolution.solutionId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		write_log(JUDGE_ERROR,"SQL_getSolutionSource");
		SQL_SemV();
		return OS_ERR;
	}

	FILE *fp_source = fopen(pstJudgeSubmission->sourcePath, "w");
	char code[MAX_CODE]={0};
	MYSQL_ROW row;

	if(row = mysql_fetch_row(recordSet))
	{
		sprintf(code, "%s", row[0]);
	}
	else
	{
		write_log(JUDGE_ERROR,"SQL_getSolutionSource Error");
	}

	if(pstJudgeSubmission->isTranscoding == 1)
	{
		int ii=0;
		/* ���VS���ַ����� */
		while (code[ii]!='\0')
		{
			if (code[ii]=='\r')
			{
				code[ii] = '\n';
			}

			ii++;
		}
	}

	fprintf(fp_source, "%s", code);

	/* add for vjudge*/
	strcpy(pstJudgeSubmission->szSource, code);

	mysql_free_result(recordSet);
	fclose(fp_source);

	SQL_SemV();

    write_log(JUDGE_INFO, "SQL get solution source ok. (solutionId=%u)", pstJudgeSubmission->stSolution.solutionId);
    
	return OS_OK;
}

void SQL_getUnJudgeSolutions(JUDGE_DATA_S *pJudgeData, int *n, int iMax)
{
	int ret = OS_OK;
	int m = 0;
	MYSQL_RES *recordSet = NULL;
	MYSQL_ROW row;
	JUDGE_DATA_S *pJudgeData2 = NULL;
	
	if (NULL == pJudgeData
		|| NULL == n)
	{
		return ;
	}

	
	SQL_SemP();
	
	*n = 0;
	pJudgeData2 = pJudgeData;

	sprintf(query,"select solution_id from solution where verdict=%d or verdict=%d", V_SK, V_Q);

	ret = mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return ;
	}

	recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		write_log(JUDGE_ERROR,"Error SQL_getSolutionData");
		SQL_SemV();
		return ;
	}

	while (row = mysql_fetch_row(recordSet))
	{	
		pJudgeData2->solutionId = atoi(row[0]);
		SQL_Debug(DEBUG_TYPE_FUNC, "SQL_getUnJudgeSolutions. (solutionId=%d)", pJudgeData2->solutionId);

		pJudgeData2++;
		m++;
		
		if (m == iMax)
		{
			break;
		}
	}

	*n = m;	

	SQL_Debug(DEBUG_TYPE_FUNC, "SQL_getUnJudgeSolutions. (n=%d)", m);
	
	/* �ͷŽ���� */
	mysql_free_result(recordSet);

	SQL_SemV();

	return ;
}


int SQL_getSolutionByID(int solutionID, JUDGE_SOLUTION_S *pstJudgeSolution, int *pIsExist)
{
	int ret = OS_OK;
	MYSQL_RES *recordSet = NULL;
	MYSQL_ROW row;

	if (NULL == pstJudgeSolution
	    ||NULL == pIsExist)
	{
		return OS_ERR;
	}

	*pIsExist = OS_NO;

	SQL_SemP();

	sprintf(query,"select problem_id,contest_id,language,username,submit_date from solution where solution_id=%d",
		    solutionID);

	ret = mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		write_log(JUDGE_ERROR,"Error SQL_getSolutionData");
		SQL_SemV();
		return OS_ERR;
	}

	if (row = mysql_fetch_row(recordSet))
	{
		pstJudgeSolution->solutionId = solutionID;

		if (NULL == row[0])
		{
			write_log(JUDGE_INFO,"solutionID %d data(problemId) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[1])
		{
			write_log(JUDGE_INFO,"solutionID %d data(contestId) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[2])
		{
			write_log(JUDGE_INFO,"solutionID %d data(languageId) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[3])
		{
			write_log(JUDGE_INFO,"solutionID %d data(username) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[4])
		{
			write_log(JUDGE_INFO,"solutionID %d data(submitDate) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}		
		
		pstJudgeSolution->problemId = atoi(row[0]);
		pstJudgeSolution->contestId = atoi(row[1]);
		pstJudgeSolution->languageId = atoi(row[2]);
		strcpy(pstJudgeSolution->username, row[3]);
		(void)util_string_to_time(row[4],pstJudgeSolution->submitDate);
		*pIsExist = OS_YES;

		//write_log(JUDGE_INFO,"Found record. (solutionID=%d)", solutionID);
	}
	else
	{
		write_log(JUDGE_ERROR,"No such record. (solutionID=%d)", solutionID);
	}

	/* �ͷŽ���� */
	mysql_free_result(recordSet);

	SQL_SemV();

	return OS_OK;
}

int SQL_getProblemInfo(JUDGE_PROBLEM_INFO_S *pstProblem)
{
	if (NULL == pstProblem)
	{
		write_log(JUDGE_ERROR,"SQL_getProblemInfo, Input param is null...");
		return OS_ERR;
	}

	SQL_SemP();

	sprintf(query,"select time_limit,memory_limit,spj,isvirtual,oj_pid,oj_name from problem where problem_id=%d",
			pstProblem->problemId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		SQL_SemV();
		return OS_ERR;
	}


	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		pstProblem->time_limit = atoi(row[0]);
		pstProblem->memory_limit = atoi(row[1]);
		pstProblem->isSpecialJudge = atoi(row[2]);
		pstProblem->isVirtualJudge = atoi(row[3]);
		pstProblem->virtualPID = atoi(row[4]);
		strcpy(pstProblem->szVirJudgerName, row[5]);
	}

	mysql_free_result(recordSet);//�ͷŽ����

	SQL_SemV();

    write_log(JUDGE_INFO, "SQL get problem info ok. (problemId=%u)", pstProblem->problemId);
    
	return OS_OK;
}

int SQL_getProblemInfo_contest(int contestId,int problemId,char *num)
{
	SQL_SemP();

	sprintf(query,"select num from contest_problem where contest_id=%d and problem_id=%d",contestId,problemId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		write_log(JUDGE_INFO,"SQL_getProblemInfo_contest ,No record");
		SQL_SemV();
		return 0;
	}

	//��ȡ����
	MYSQL_ROW row; //һ�������ݵ����Ͱ�ȫ(type-safe)�ı�ʾ
	if(row=mysql_fetch_row(recordSet))  //��ȡ��һ����¼
	{
		strcpy(num,row[0]);
	}

	mysql_free_result(recordSet);//�ͷŽ����

	SQL_SemV();

	return 1;
}

int SQL_getContestInfo(int contestId,time_t &start_time,time_t &end_time)
{
	SQL_SemP();

	//start_time,end_time
	sprintf(query,"select start_time,end_time from contest where contest_id=%d",contestId);
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		SQL_SemV();
		return 0;
	}

	//��ȡ����
	MYSQL_ROW row; //һ�������ݵ����Ͱ�ȫ(type-safe)�ı�ʾ
	if(row=mysql_fetch_row(recordSet))  //��ȡ��һ����¼
	{
		(void)util_string_to_time(row[0],start_time);
		(void)util_string_to_time(row[1],end_time);
	}

	mysql_free_result(recordSet);//�ͷŽ����

	SQL_SemV();

	return 1;
}

/* update Solution table*/
void SQL_updateSolution(int solutionId,int verdictId,int testCase,int failcase,int time,int memory)
{
	SQL_SemP();

	sprintf(query,"update solution set verdict=%d,testcase=%d,failcase=%d,time=%d,memory=%d where solution_id=%d;",verdictId,testCase,failcase,time,memory,solutionId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));

		/* �����ϰ汾��֧��failcase�ֶ� */
		memset(query, 0, sizeof(query));
		sprintf(query,"update solution set verdict=%d,testcase=%d,time=%d,memory=%d where solution_id=%d;",verdictId,testCase,time,memory,solutionId);
		mysql_real_query(mysql,query,(unsigned int)strlen(query));
	}

	SQL_SemV();
}

void SQL_updateSolutionJsonResult(int solutionId, char *pJsonResult)
{
	SQL_SemP();

	sprintf(query,"update solution set json_result='%s' where solution_id=%d;", pJsonResult, solutionId);

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	SQL_SemV();
}


/* update problem table*/
void SQL_updateProblem(int problemId)
{
	SQL_SemP();

	sprintf(query,"update problem set accepted=(SELECT count(*) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;",problemId,V_AC,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update problem set submit=(SELECT count(*) FROM solution WHERE problem_id=%d)  where problem_id=%d;",problemId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;",problemId,V_AC,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d) where problem_id=%d;",problemId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	SQL_SemV();

}

void SQL_updateProblem_contest(int contestId,int problemId)
{
	SQL_SemP();

	sprintf(query,"update contest_problem set accepted=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;",contestId,V_AC,problemId,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update contest_problem set submit=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d)  where contest_id=%d and problem_id=%d;",contestId,problemId,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update contest_problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;",contestId,problemId,V_AC,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update contest_problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d) where contest_id=%d and problem_id=%d;",contestId,problemId,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	SQL_SemV();
}

void SQL_updateAttend_contest(int contestId,int verdictId,int problemId,char *num,char *username,time_t start_time,time_t end_time){

	//�Ѿ�AC�Ĳ���Ҫ�޸�attend
	//update ac_time
	long AC_time=0;
	time_t first_ac_t;

	SQL_SemP();

	if(SQL_getFirstACTime_contest(contestId,problemId,username,first_ac_t,start_time,end_time))
	{
		AC_time=util_getdiftime(first_ac_t,start_time);
	}
	else
	{
		AC_time=0;
		first_ac_t = end_time;
	}

	sprintf(query,"update attend set %s_time=%ld where contest_id=%d and username='%s';",num,AC_time,contestId,username);
	//cout<<query<<endl;
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	long ac_nCount=SQL_countProblemVerdict(contestId,problemId,V_AC,username);
	int score_ = SQL_getContestScore(contestId,username,start_time,end_time);
	string s_t,e_t,fAC_t;
	(VOID)util_time_to_string(s_t,start_time);
	(VOID)util_time_to_string(e_t,end_time);
	(VOID)util_time_to_string(fAC_t,first_ac_t);

	//update score solved ,wrongsubmits
	sprintf(query,"update attend set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE contest_id=%d and username='%s' and verdict=%d and submit_date between '%s' and '%s'),%s_wrongsubmits=(SELECT count(solution_id) FROM solution WHERE contest_id=%d and problem_id=%d and username='%s' and verdict>%d and submit_date between '%s' and '%s'),score=%d  where contest_id=%d and username='%s';",contestId,username,V_AC,s_t.c_str(),e_t.c_str(),   num,contestId,problemId,username,V_AC,s_t.c_str(),fAC_t.c_str(),  score_,  contestId,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	//penalty
	int nCountProblems=SQL_countContestProblems(contestId);
	char index='A';
	long penalty=0;
	for(int i=0;i<nCountProblems;i++)
	{
		long a_time_=0;
		int wrongsubmits_=0;
		SQL_getContestAttend(contestId,username,index+i,a_time_,wrongsubmits_);

		if(a_time_>0)
		{
			penalty=penalty+a_time_+wrongsubmits_*60*20;
		}
	}

	sprintf(query,"update attend set penalty=%ld where contest_id=%d and username='%s';",penalty,contestId,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	SQL_SemV();

}

/* update user table*/
void SQL_updateUser(char *username)
{
	SQL_SemP();

	sprintf(query,"update users set submit=(SELECT count(*) FROM solution WHERE username='%s') where username='%s';",username,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update users set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE username='%s' and verdict=%d) where username='%s';",username,V_AC,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	SQL_SemV();
}

void SQL_updateCompileInfo(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	FILE *fp;
	char buffer[4096]={0};

	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"SQL_updateCompileInfo, Input param is null...");
		return ;
	}


	if ((fp = fopen (pstJudgeSubmission->DebugFile, "r")) == NULL)
	{
		write_log(JUDGE_ERROR,"DebugFile open error");
		return ;
	}

	SQL_SemP();

	//��ɾ��
	sprintf(query,"delete from compile_info where solution_id=%d;",pstJudgeSubmission->stSolution.solutionId);
	mysql_real_query(mysql,query,(unsigned int)strlen(query));

	//�Ȳ���
	if((fgets(buffer, 4095, fp))!= NULL)
	{	
		string str = buffer;
    	string::iterator   it;
 
	    for (it =str.begin(); it != str.end(); ++it)
	    {
	        if ( *it == '\"')
	        {
	            str.erase(it);
	        }
	    }

		sprintf(query,"insert into compile_info values(%d,\"%s\");",pstJudgeSubmission->stSolution.solutionId,str.c_str());
		int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
		if(ret)
		{
			write_log(JUDGE_ERROR,mysql_error(mysql));
			fclose(fp);
			SQL_SemV();
			return ;
		}
	}

	//������
	while ((fgets (buffer, 4095, fp))!= NULL)
	{
		buffer[strlen(buffer)];

		string str = buffer;
    	string::iterator   it;
 
	    for (it =str.begin(); it != str.end(); ++it)
	    {
	        if ( *it == '\"')
	        {
	            str.erase(it);
	        }
	    }
		
		sprintf(query,"update compile_info set error=CONCAT(error,\"%s\") where solution_id=%d;",
				buffer, pstJudgeSubmission->stSolution.solutionId);

		int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
		if(ret)
		{
			write_log(JUDGE_ERROR,mysql_error(mysql));
			fclose(fp);
			SQL_SemV();
			return ;
		}
 	}

	fclose(fp);

	SQL_SemV();

}

int SQL_getUserByname(const char *username)
{
	int ret = OS_OK;
	
	SQL_SemP();

	if (strlen(username) >= MAX_NAME
		|| mysql == NULL)
	{
		SQL_SemV();
		return OS_ERR;
	}

	sprintf(query,"select id from users where username='%s'",username);

	ret = mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_ROW row;
	if(row = mysql_fetch_row(recordSet))
	{
		//printf("\r\nID:%u",atol(row[0]));
	}
	else
	{
		ret = OS_ERR;
	}

	mysql_free_result(recordSet);//�ͷŽ����

	SQL_SemV();

	return ret;
}

int SQL_UserLogin(const char *username, const char *password)
{
	int ret = OS_OK;
	
	SQL_SemP();

	if (strlen(username) >= MAX_NAME
		|| strlen(password) >= MAX_NAME
		|| mysql == NULL)
	{
		SQL_SemV();
		return OS_ERR;
	}

	sprintf(query,"select id from users where username='%s' and password='%s'",username, password);

	ret = mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_ROW row;
	if(row = mysql_fetch_row(recordSet))
	{
		//printf("\r\nID:%u",atol(row[0]));
	}
	else
	{
		ret = OS_ERR;
	}

	mysql_free_result(recordSet);//�ͷŽ����

	SQL_SemV();

	return ret;
}

#endif
