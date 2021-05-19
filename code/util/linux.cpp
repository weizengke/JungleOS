
#ifdef _LINUX_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#include <errno.h>

#include "kernel.h"

int stricmp(char *str1, char *str2)
{
	return strcasecmp(str1, str2);
}

int strnicmp(char *str1, char *str2, unsigned maxlen)
{
	return strncasecmp(str1, str2, maxlen);
}

int closesocket(socket_t socket)
{
	close(socket);
}

int Sleep(int ms)
{
	usleep(ms*1000);
}

int file_access(const char *pathname, int mode)
{
	return access(pathname, mode);
}

int geterror()
{
	return errno;
}

mutex_t mutex_create(char *name)
{	
	mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	
	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		return PTHREAD_MUTEX_INITIALIZER;
	}

	return mutex;	
}

int mutex_lock(mutex_t mutex)
{
	if (pthread_mutex_lock(&mutex) != 0)
	{
		return 1;
	}

	return 0;
}

int mutex_unlock(mutex_t mutex)
{
	if (pthread_mutex_unlock(&mutex) != 0)
	{
		return 1;
	}

	return 0;
}


void* thread_start_func(void *th_ptr)
{
    thread_t *th = (thread_t *)th_ptr;
    thread_id_t thread_id = th->thread_id;

	if (NULL != th->thread_func){
		if (th->thread_func(th->arg)){
		}
	}
	
	free(th_ptr);
	th_ptr = NULL;
	
    return 0;
}

int thread_close(thread_id_t thread_id)
{
	return pthread_cancel(thread_id);
}


thread_id_t thread_create(int (*fn)(void *), void *arg)
{
	int err = 0;
	thread_t *th = NULL;
	pthread_attr_t *const attrp = NULL;
	
	th = (thread_t *)malloc(sizeof(thread_t));
	if (NULL == th){
		return 0;
	}
	memset(th, 0, sizeof(thread_t));
	
	th->thread_func = fn;
	th->arg = arg;

	/* th�ڴ���thread_start_func���ͷ� */
    err = pthread_create(&th->thread_id, attrp, thread_start_func, th);
    if (!err) {
		return 0;
    }

	return th->thread_id;
}

unsigned long thread_get_self()
{
	return (unsigned long)pthread_self();;
}

bool create_directory(char *path_name)
{
	if (-1 == mkdir(path_name, 0755))
	{
		printf("\r\n mkdir failed. (path_name=%s)", path_name);
		return false;
	}
	
	return true;
}

bool get_current_directory(int buf_len, char* current_path)
{
	char *file_path_getcwd;

	if (NULL != getcwd(current_path, buf_len))
	{
		return true;
	}
	
	return false;
}

int CopyFile(const char* src, const char* des, int flag)
{
	int nRet = 0;
	FILE* pSrc = NULL, *pDes = NULL;
	pSrc = fopen(src, "r");
	pDes = fopen(des, "w+");
 

	if (pSrc && pDes)
	{
		int nLen = 0;
		char szBuf[1024] = {0};
		while((nLen = fread(szBuf, 1, sizeof szBuf, pSrc)) > 0)
		{
			fwrite(szBuf, 1, nLen, pDes);
		}
	}
	else
		nRet = -1;
 
	if (pSrc)
		fclose(pSrc), pSrc = NULL;
 
	if (pDes)
		fclose(pDes), pDes = NULL;
 
	return nRet;
}

#endif

