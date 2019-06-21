/*
	  �������

	ҹɫ��˷�����
	�Ӳ�֪��
	���������
	ɪ���������ᣬ
	ֻ���㣬
	��ϸ˵��
	�����ߡ�

            By Jungle Wei.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
	
#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif
	
#ifdef _WIN32_
#include <conio.h>
#include <io.h>
#include <winsock2.h>

#endif

#include "kernel.h"

#include "osp/command/include/icli.h"
#include "osp/common/include/osp_common_def.h"
#include "product/include/pdt_common_inc.h"
#include "product/main/root.h"
#include "product/judge/include/judge_inc.h"
#include "product/main/sysmng.h"

using namespace std;


#define APP_MAX_NUM 256

ULONG g_ulAppNum = 0;

APP_INFO_S *g_pstAppArray = NULL;

int APP_RegistInfo(APP_INFO_S *pstAppInfo)
{
	int ret = OS_OK;

	if (g_ulAppNum >= APP_MAX_NUM)
	{
		return OS_ERR;
	}

	if (NULL == g_pstAppArray)
	{
		g_pstAppArray = (APP_INFO_S*)malloc(APP_MAX_NUM * sizeof(APP_INFO_S));
		if (NULL == g_pstAppArray)
		{
			return OS_ERR;
		}
		memset(g_pstAppArray, 0, APP_MAX_NUM * sizeof(APP_INFO_S));
	}
	
	memcpy(&g_pstAppArray[g_ulAppNum++], pstAppInfo, sizeof(APP_INFO_S));

	if (NULL != pstAppInfo->pfInitFunction)
	{
		ret = pstAppInfo->pfInitFunction();
		if (OS_OK != ret)
		{
			printf("%s APP init failed...\r\n", pstAppInfo->taskName);
			printf("%s APP RegistInfo failed...\r\n", pstAppInfo->taskName);
			return OS_ERR;
		}

		printf("%s APP init ok...\r\n", pstAppInfo->taskName);
	}

	printf("%s APP RegistInfo ok...\r\n", pstAppInfo->taskName);

	return OS_OK;

}

int APP_Run()
{
	int ix = 0;
	
	for (ix = 0; ix < g_ulAppNum; ++ix)
	{
		if (NULL != g_pstAppArray[ix].pfTaskMain)
		{
			(VOID)thread_create(g_pstAppArray[ix].pfTaskMain, NULL);

			printf("%s APP running ok...\r\n", g_pstAppArray[ix].taskName);
			write_log(JUDGE_INFO, "%s APP running ok...", g_pstAppArray[ix].taskName);
			
			Sleep(100);
		}
	}

	return OS_OK;
}

ULONG APP_Startup()
{
	{
		/* ϵͳ����ģ�����ȳ�ʼ�� */
		void SYSMNG_RegAppInfo();
		SYSMNG_RegAppInfo();
	}
		
#if (OS_YES == OSP_MODULE_CLI)
	{
		extern void Cmd_RegAppInfo();
		Cmd_RegAppInfo();
	}
#endif

#if (OS_YES == OSP_MODULE_DEBUG)
	{
		extern void Debug_RegAppInfo();
		Debug_RegAppInfo();
	}
#endif

#if (OS_YES == OSP_MODULE_AAA)
	{
		extern void AAA_RegAppInfo();
		AAA_RegAppInfo();
	}
#endif
	
#if (OS_YES == OSP_MODULE_TELNETS)
	{
		extern void TELNET_RegAppInfo();
		TELNET_RegAppInfo();
	}
#endif

#if (OS_YES == OSP_MODULE_JUDGE)
	{
		extern void Judge_RegAppInfo();
		Judge_RegAppInfo();
	}
#endif

#if (OS_YES == OSP_MODULE_NDP)
	{
		extern void NDP_RegAppInfo();
		NDP_RegAppInfo();
	}
#endif

#if (OS_YES == OSP_MODULE_FTPS)
	{
		extern void FTPS_RegAppInfo();
		FTPS_RegAppInfo();
	}
#endif

	/* APPע�����APP_Run��ǰ�� */

	(VOID)APP_Run();

	return OS_OK;
}

int main()
{
	printf("====================================================="
			"\r\nOnline Judge system start running...\r\n");

	(VOID)APP_Startup();

	/* �ȴ�1000ms���Ա����APP�ȶ�ʼ����ɲſ�ʼ */
	Sleep(1000);

	/* ���ûָ����� */
	SYSMNG_CfgRecover();

	printf("Press any key to continue.\r\n");

	/* ѭ����ȡ��Ϣ���� */
	for ( ; ; )
	{
		Sleep(10);
	}

	return 0;
}
