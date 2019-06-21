#ifndef _COMMAND_H_
#define _COMMAND_H_

#ifndef UCHAR
#undef UCHAR
#define UCHAR unsigned char
#endif

#ifndef ULONG
#undef ULONG
#define ULONG unsigned long
#endif

#ifndef CHAR
#undef CHAR
#define CHAR char
#endif

#ifndef VOID
#undef VOID
#define VOID void
#endif

#ifndef BOOL
#undef BOOL
#define BOOL bool
#endif

#ifndef FALSE
#undef FALSE
#define FALSE false
#endif

#ifndef TRUE
#undef TRUE
#define TRUE true
#endif

#ifdef _WIN32_
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

/* ���������� */
enum CMD_ELEM_TYPE_E
{
	CMD_ELEM_TYPE_VALID,
	CMD_ELEM_TYPE_KEY,     /* �ؼ��� */
	CMD_ELEM_TYPE_INTEGER, /* ���β��� INTEGER<A-B>*/
	CMD_ELEM_TYPE_STRING,  /* �ַ��Ͳ��� STRING<A-B>*/
	CMD_ELEM_TYPE_IP,  	   /* ip��ַ���Ͳ��� X.X.X.X*/
	CMD_ELEM_TYPE_MAC,     /* MAC��ַ���Ͳ��� H-H-H*/
	
	CMD_ELEM_TYPE_END,     /* �����н�����<CR> */
	CMD_ELEM_TYPE_MAX,
};

/* 
��ͼid�����255
VIEW_GLOBAL��ʾ������ͼ
VIEW_USER��ʾ�û���ͼ
VIEW_SYSTEM��ʾϵͳ��ͼ
VIEW_DIAGNOSE��ʾ�����ͼ
����û��Զ���������ͼ����Ҫ�����5��ʼ
*/
enum CMD_VIEW_ID_E {
	VIEW_NULL = 0,
	VIEW_GLOBAL = 1,
	VIEW_USER,
	VIEW_SYSTEM,
	VIEW_JUDGE_MGR,
	VIEW_VJUDGE_MGR,
	VIEW_AAA,
	VIEW_USER_VTY,
	VIEW_DIAGNOSE,

	VIEW_ID_MAX = 255,	
};

#define CMD_ELMT_IP  		"X.X.X.X"
#define CMD_ELMT_MAC  		"H-H-H"


// maximum number of command to remember
#define HISTORY_MAX_SIZE	256

// maximum number of commands that can matched, ��Ҫ�޸�Ϊ��̬����
#define CMD_MAX_MATCH_SIZE	1024  

// maximum number of command element
#define CMD_MAX_CMD_NUM		16

// maximum length of command element
#define CMD_MAX_CMD_ELEM_SIZE 128

// maximum length of view name or ais-name
#define CMD_MAX_VIEW_SIZE   CMD_MAX_CMD_ELEM_SIZE

// size of input buffer size
#define CMD_BUFFER_SIZE		(CMD_MAX_CMD_NUM * (CMD_MAX_CMD_ELEM_SIZE + 2))


#define CMD_VTY_MAXUSER_NUM 32
#define CMD_VTY_CONSOLE_ID CMD_VTY_MAXUSER_NUM
#define CMD_VTY_CFM_ID CMD_VTY_MAXUSER_NUM + 1

#define CMD_VTY_INVALID_ID 0xFFFFFFFF


/* high 16bit ucMID,ucTBL, low 16bit usID */
#define CMD_ELEMID_DEF(ucMID, ucTBL, usID)  ( ( 0xFF000000 & (ucMID<<24)) + (0x00FF0000 & (ucTBL<<16)) + (0x0000FFFF & usID) )
#define CMD_ELEMID_NULL 0xFFFFFFFF

#define CMD_VECTOR_NEW(vec) vec = cmd_vector_new()
#define CMD_VECTOR_FREE(vec) cmd_vector_free(&vec);


typedef struct vty_user_st {
	int level;
	int type;  /* 0:console, 1:telnet */
	int state;  /* 0:idle, 1:access */
	int terminal_debugging;
	socket_t socket;
	char user_name[32];
	char user_psw[32];
	time_t lastAccessTime;	
}VTY_USER_S;

typedef struct cmd_vty_st {
	ULONG used;      /* vty �Ƿ�ռ�� */
	ULONG vtyId;     /* vty id         */
	ULONG view_id;   /* ��ǰ������ͼ   */	
	VTY_USER_S user; /* ��ǰvty��Ӧ���û���Ϣ   */	
	ULONG ulBufMaxLen;   /* �������������󳤶�    */
	ULONG ulUsedLen;     /* ��ǰ����������ĳ���    */
	ULONG ulCurrentPos;  /* ��ǰ�����й������λ��  */
	UCHAR c;   			 /* ��ǰ������ַ�		    */
	/* BEGIN: for support TAB agian and agian */
	UCHAR ucKeyTypePre;	    					/* �ϴ���������   */
	UCHAR ucKeyTypeNow;							/* ������������   */
	CHAR tabbingString[CMD_MAX_CMD_ELEM_SIZE];	/* ���ʼ������ȫ���ҵ��ִ�*/
	CHAR tabString[CMD_MAX_CMD_ELEM_SIZE];		/* ���һ�β�ȫ������ */
	/* END: for support TAB agian and agian */
	CHAR res;
	CHAR szBuffer[CMD_BUFFER_SIZE];      /* ��ǰ�����л���������    */
	ULONG ulhpos;					     /* ��ǰ��ѯ����ʷ�������±� */
	ULONG ulhNum;	   					 /* ��ǰ���ڵ���ʷ�������   */
	CHAR *pszHistory[HISTORY_MAX_SIZE];  /* ��ǰ���ڵ���ʷ��������   */	
}CMD_VTY_S;

typedef struct tagCMD_VECTOR_S {
	ULONG ulSize;
	VOID **ppData;
} CMD_VECTOR_S;

#define CMD_MAX_CMD_ELEM_SIZE 64
#define CMD_MAX_CMD_ELEM_HELP_SIZE 256

typedef struct tagCMD_ELMTHELP_S {
	CHAR szElmtName[CMD_MAX_CMD_ELEM_SIZE];
	CHAR szElmtHelp[CMD_MAX_CMD_ELEM_HELP_SIZE];
} CMD_ELMTHELP_S;

typedef ULONG (*PFELMTHELPFUNC)(VOID *pRcvMsg, CMD_ELMTHELP_S **ppstCmdElmtHelp, ULONG *pulNum);  
typedef ULONG (*PFELMTCHECKFUNC)(VOID *pRcvMsg); 


/* 
cmd_vector_new��������: ��������������
����ע��������ʱʹ��
*/
extern CMD_VECTOR_S *cmd_vector_new();

/* 
cmd_vector_free��������:�ͷ�����������
����: CMD_VECTOR_S **ppVec - - ����������ָ��

����ע��������ʱʹ��
*/
extern VOID cmd_vector_free(CMD_VECTOR_S **ppVec);

extern ULONG cmd_regelement(ULONG cmd_elem_id,
								CMD_ELEM_TYPE_E cmd_elem_type,
								CHAR *cmd_name,
								CHAR *cmd_help,
								PFELMTHELPFUNC pfElmtHelpFunc,
								PFELMTCHECKFUNC pfElmtCheckFunc,
								CMD_VECTOR_S * pVec);


/* 
cmd_regelement_new��������:ע������Ԫ��
����: ULONG cmd_elem_id - ����Ԫ��id
      CMD_ELEM_TYPE_E cmd_elem_type -  ����Ԫ������
      CHAR *cmd_name -  ����Ԫ������
      CHAR *cmd_help - ����Ԫ�ذ�����Ϣ
      CMD_VECTOR_S *pVec - - ����������

����ע��������ʱʹ��
*/
extern ULONG cmd_regelement_new(ULONG cmd_elem_id, CMD_ELEM_TYPE_E cmd_elem_type, CHAR *cmd_name, CHAR *cmd_help, CMD_VECTOR_S *pVec);

/* 
cmd_install_command��������:ע��������
����: ULONG mid - ģ��id
      ULONG cmd_view - ������ע�����ͼid
	  CHAR *cmd_string - ��Ҫע����������������ʽ
	  CMD_VECTOR_S *pVec - ����������
����ע��������ʱʹ��
*/
extern VOID cmd_install_command(ULONG mid, ULONG cmd_view, CHAR *cmd_string, CMD_VECTOR_S *pVec);

/* 
vty_view_set��������: ����vty�û�����ָ������ͼ
���: ULONG vtyId - vty�û�id
		 ULONG view_id - ��ͼid
����ֵ: ULONG - ���������ɹ�����0��ʧ�ܷ���1
*/
extern VOID vty_view_set(ULONG vtyId, ULONG view_id);

/* 
vty_view_quit��������: �˳�vty�û����ڵ�ǰ��ͼ�����ص���һ����ͼ
���: ULONG vtyId - vty�û�id
����ֵ: ULONG - ���������ɹ�����0��ʧ�ܷ���1
*/
extern VOID vty_view_quit(ULONG vtyId);

/* ִ��ָ�������� */
extern ULONG cmd_pub_run(CHAR *szCmdBuf);

/* ���ڸ�ָ���û���ӡ��Ϣ */
extern VOID vty_printf(ULONG vtyId, CHAR *format, ...);

/* �����������û���ӡ��Ϣ */
extern VOID vty_print2all(CHAR *format, ...);

/* ���ڻ�ȡ���е�vtyid */
extern ULONG cmd_get_idle_vty();

/* �����û����� */
extern VOID vty_offline(ULONG vtyId);	

/* ����ָ���û����û����� */
extern VOID vty_offline_by_username(CHAR *pszName);

/* ����vty��socket */
extern ULONG vty_set_socket(ULONG vtyId, ULONG socket);

/* vty�û���ʼ���� */
extern VOID vty_go(ULONG vtyId);

/* �û�������ͼ */
extern ULONG vty_get_current_viewid(ULONG vtyId);

/* 
cmd_view_regist��������: ע���Զ�����ͼ
���: CHAR *view_name - ��ͼ����
		CHAR *view_ais - ��ͼ����
		ULONG view_id - ��ͼid
		ULONG parent_view_id - ��һ����ͼid
����ֵ: ULONG - ���������ɹ�����0��ʧ�ܷ���1
*/
extern ULONG cmd_view_regist(CHAR *view_name, CHAR *view_ais, ULONG view_id, ULONG parent_view_id);

/* 
cmd_get_vty_id��������: ��ȡvty�û�id
���: VOID *pRunMsg - �����лص�����Ϣָ��
����ֵ: ULONG - Ϊvty�û�id
*/
extern ULONG cmd_get_vty_id(VOID *pRunMsg);

/* 
cmd_get_elem_by_index��������: ���������в���������ȡ����Ԫ��
���: VOID *pRunMsg - �����лص�����Ϣָ��
	  ULONG index - �����в�������
����ֵ: VOID * - Ϊ����Ԫ��ָ��
*/
extern VOID *cmd_get_elem_by_index(VOID *pRunMsg, ULONG index);

/* 
cmd_get_elem_num��������: ��ȡ����Ԫ�صĸ���
���: VOID *pRunMsg - �����лص�����Ϣָ��
����ֵ: ULONG - Ϊ����Ԫ�صĸ���
*/
extern ULONG cmd_get_elem_num(VOID *pRunMsg);

/* 
cmd_get_elemid��������: ��ȡ����Ԫ��id
���: VOID *pElemMsg - ������Ԫ��ָ��
����ֵ: ULONG - ����Ԫ��id
*/
extern ULONG cmd_get_elemid(VOID *pElemMsg);

/* 
cmd_get_ulong_param��������: ������Ԫ���л�ȡ���β�����ֵ
���: VOID *pElemMsg - ������Ԫ��ָ��
����ֵ: ULONG - ���β�����ֵ
ע: ��������Ԫ��Ϊ���β���ʱʹ��
*/
extern ULONG cmd_get_ulong_param(VOID *pElemMsg);

/* 
cmd_copy_string_param��������: ������Ԫ���л�ȡ�ַ��Ͳ�����ֵ
���: VOID *pElemMsg - ������Ԫ��ָ��
����ֵ: CHAR *param  - �ַ��Ͳ�����ֵ
ע: ��������Ԫ��Ϊ�ַ��Ͳ���ʱʹ��
*/
extern VOID cmd_copy_string_param(VOID *pElemMsg, CHAR *param);

/* 
cmd_get_ulong_param��������: ������Ԫ���л�ȡ���β�����ֵ
���: VOID *pElemMsg - ������Ԫ��ָ��
����ֵ: ULONG - ���β�����ipֵ
ע: ��������Ԫ��Ϊ���β���ipʱʹ��
*/
extern ULONG cmd_get_ip_ulong_param(VOID *pElemMsg);


/* 
cmd_get_first_elem_tblid��������: ��ȡ�����д����tableid
���: VOID *pRunMsg - �����лص�����Ϣָ��
����ֵ: ULONG - tableid
����һ��ģ��ע������ͬ��ģ������ʱ�����ֻص�����
*/
extern ULONG cmd_get_first_elem_tblid(VOID *pRunMsg);

/*  */
/* 
cmd_regcallback��������: �����д���ص�ע��
���: ULONG ulMid - ����ûص���ģ��id(��ע�������ֵ�tblid��Ҫһ��)
����ֵ: ULONG (*pfCallBackFunc)(VOID *pRcvMsg) - ��Ҫע��Ļص�����ָ��
ÿһ��ģ�鶼Ҫע��һ���ص����Ա㴦��������ִ�й���
*/
extern ULONG cmd_regcallback(ULONG ulMid, ULONG (*pfCallBackFunc)(VOID *pRcvMsg));


extern CMD_VTY_S *cmd_vty_getById(ULONG vtyId);
extern CMD_VTY_S g_vty[CMD_VTY_MAXUSER_NUM];
extern CMD_VTY_S *g_con_vty;

#endif

