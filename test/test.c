/*
	dfs interface test
*/
#include "public.h"
#include "libxtreemfs_fmgr.h"
#include "libxtreemfs4c.h"
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include<sys/wait.h>


#define LOG_POSITION(format, ...) printf(format,##__VA_ARGS__);
#define LOG(format, ...) LOG_POSITION("[%s][%d]  "format, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define SINGLE_PROCESS	0
#define PROCESS_COUNT	3

#define WRITE_COUNT 1
#define BUFFER_COUNT 1

#define MAIN_DIR "test"
#define TEST_FILENAME "test.test"
#define TEST_FILENAME_LOCAL "/home/zpzhao/uxdbinstall/bin/test.test"
#define SEM_NAME "testsem"

//#define USE_MY_FILESIZE
#define COMM_SHM_MEM_ID 5432
#define COMM_MEMORY_SIZE 4

static sem_t *sem = NULL;
static int s_CommShm = 0;
static int *pfilesize = NULL;
#define filesize *pfilesize

#define lock sem_P(sem)
#define unlock sem_V(sem)


int SubProcessMain(int argc, char *argv[])
{
	int 			flag 	= 0xA1+argc;
	DFS				*dfs 	= NULL;
	struct DFSDIR 	*_dir 	= NULL;

	// init dfs
	if(initDFS(&dfs) == 0)
		setDFS(dfs);
	dfs = getDFS();
	if(NULL == dfs)
	{
		LOG("dfs is null pid[%d]. \n",getpid());
		return -1;
	}

	_dir = DFS_openDir(dfs, MAIN_DIR);
	if(NULL != _dir)
	{
		DFS_saveMainDir(dfs, _dir);
	}
	LOG("DFS_openDir. pid[%d] dir[%p] \n", getpid(), _dir);

	// main test case here
	op_seek_file(flag);

	_dir = DFS_getMainDir(dfs);
	if(NULL != _dir)
	{
		LOG("DFS_closeDir. pid[%d] dir[%p] \n", getpid(), _dir);
		DFS_closeDir(dfs, _dir);
	}

	destroyDFS();
	sem_close(sem);
	LOG("success. pid[%d] \n", getpid());
	return 0;
}


int SubProcessMain_seek(int argc, char *argv[])
{
	int 			flag 	= 0xA1+argc;
	DFS				*dfs 	= NULL;
	struct DFSDIR 	*_dir 	= NULL;

	// init dfs
	if(initDFS(&dfs) == 0)
		setDFS(dfs);
	dfs = getDFS();
	if(NULL == dfs)
	{
		LOG("dfs is null pid[%d]. \n",getpid());
		return -1;
	}

	_dir = DFS_openDir(dfs, MAIN_DIR);
	if(NULL != _dir)
	{
		DFS_saveMainDir(dfs, _dir);
	}
	LOG("DFS_openDir. pid[%d] dir[%p] \n", getpid(), _dir);

	// main test case here
	SeekFileSize();

	_dir = DFS_getMainDir(dfs);
	if(NULL != _dir)
	{
		LOG("DFS_closeDir. pid[%d] dir[%p] \n", getpid(), _dir);
		DFS_closeDir(dfs, _dir);
	}

	destroyDFS();
	sem_close(sem);
	LOG("success. pid[%d] \n", getpid());
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int pid = 0;
	int index=0;

	/* semphore */
	sem = sem_open(SEM_NAME, O_RDWR|O_CREAT, 00777, 1);
	LOG("fork process parent pid[%d] \n", getpid());
#ifdef USE_MY_FILESIZE
	// share memory
	s_CommShm = shmget(COMM_SHM_MEM_ID, COMM_MEMORY_SIZE, 0666 | IPC_CREAT);
	if(-1 == s_CommShm)
	{
		LOG("shmget err \n", getpid());
		return 0;
	}

	pfilesize = (int *)shmat(s_CommShm, NULL, 0);
	if(NULL == pfilesize)
	{
		LOG("shmat err \n", getpid());
		return 0;
	}
#endif
	if(!SINGLE_PROCESS)
	{
		for(index=0; index < PROCESS_COUNT; ++index)
		{
			pid = ForkSubProcess(index+1, argv);
			if(0 > pid)
			{
				LOG("fork err \n");
				return 0;
			}
		}
	}

	//LOG("parent process pid[%d] \n", getpid());
	ret = SubProcessMain(0, argv);
	//LOG("parent process end pid[%d] ret[%d] \n", getpid(), ret);

	WaitEndAllProcess();
#ifdef USE_MY_FILESIZE
	shmdt(pfilesize);
#endif
	sem_unlink(SEM_NAME);
	return 0;
}

void sem_P(sem_t *sem)
{
	sem_wait(sem);
}

void sem_V(sem_t *sem)
{
	sem_post(sem);
}

void WaitEndAllProcess()
{
	int pid = -1;
	int status = 0;

	do
	{
		pid = wait(&status);
		if(pid < 0)	// all subprocess exit
			break;

		if(WIFEXITED(status))
		{
			LOG("catch pid[%d] exit(%d) \n", pid, WEXITSTATUS(status));
		}
		else
		{
			LOG("catch pid[%d] exit abnormal(%d) \n", pid, WEXITSTATUS(status));
		}
	}while(1);

	LOG("all sub process end pid[%d] \n", getpid());
}

int ForkSubProcess(int argc, char *argv[])
{
	int pid = -1;
	int ret = 0;

	pid = fork();
	if(0 > pid)
	{
		LOG("fork err \n");
	}
	else if(0 == pid)
	{
		//LOG("sub process pid[%d] \n", getpid());
		ret = SubProcessMain_seek(argc, argv);
		//LOG("sub process end pid[%d] ret[%d] \n", getpid(), ret);
		exit(ret);
	}

	// parent process
	return pid;
}



void op_seek_file(int flag)
{
	int ret = 0;
	int filelen = 0;
	char buffer[BUFFER_COUNT] = {0};
	char readbuff[BUFFER_COUNT] = {0};
	int i = 1;
	DFS			*dfs = NULL;
	struct DFSFD* dfs_fd = NULL;
	char ajsPath[1024] = {0};

	dfs = getDFS();
	if(NULL == dfs)
	{
		LOG("dfs is null pid[%d] \n", getpid());
		return;
	}

	memset(ajsPath, 0x00, sizeof(ajsPath));
	ADJUSTPATH(dfs, TEST_FILENAME, ajsPath);
	dfs_fd = DFS_openFile(dfs, ajsPath, O_RDWR | O_CREAT | O_APPEND | O_SYNC, DFS_MODE);
	if(NULL == dfs_fd)
	{
		LOG("[%d]open file error. \n",getpid());
		return ;
	}

	lock;
	filelen = DFS_seekFile(dfs, dfs_fd, 0, SEEK_END);
	unlock;

	LOG(" pid[%d]filelen:%d \n", getpid(), filelen);

	do
	{
		memset(buffer, flag, sizeof(buffer));
		lock;
		filelen = DFS_seekFile(dfs, dfs_fd, 0, SEEK_END);
		LOG(" pid[%d] w1filelen:%d \n", getpid(), filelen);
		ret = DFS_writeFile(dfs, dfs_fd, buffer, sizeof(buffer));
		//sleep(10);
		unlock;

		lock;
		filelen = DFS_seekFile(dfs, dfs_fd, 0, SEEK_END);
		LOG("[%d] w2filelen:%d \n", getpid(), filelen);

		memset(buffer, flag, sizeof(buffer));
		ret = DFS_writeFile(dfs, dfs_fd, buffer, sizeof(buffer));
		//sleep(10);
		unlock;

		lock;
		filelen = DFS_seekFile(dfs, dfs_fd, filelen, SEEK_SET);
		LOG("[%d] r filelen:%d \n", getpid(), filelen);

		memset(readbuff, 0x00, sizeof(readbuff));
		ret = DFS_readFile(dfs, dfs_fd, readbuff, sizeof(readbuff));
		unlock;

		LOG(" pid[%d]readlen:%d read[%#x]\n", getpid(), ret, readbuff[0]);
	}while(i++ < WRITE_COUNT);

	DFS_closeFile(dfs, dfs_fd);

	return ;
}


void op_write_no_seek_file(int flag)
{
	int ret = 0;
	int filelen = 0;
	char buffer[BUFFER_COUNT] = {0};
	char readbuff[BUFFER_COUNT] = {0};
	int i = 1;
	DFS			*dfs = NULL;
	struct DFSFD* dfs_fd = NULL;
	char ajsPath[1024] = {0};

	dfs = getDFS();
	if(NULL == dfs)
	{
		LOG("dfs is null pid[%d] \n", getpid());
		return;
	}

	memset(ajsPath, 0x00, sizeof(ajsPath));
	ADJUSTPATH(dfs, TEST_FILENAME, ajsPath);
	dfs_fd = DFS_openFile(dfs, ajsPath, O_RDWR | O_CREAT , DFS_MODE);
	if(NULL == dfs_fd)
	{
		LOG("[%d]open file error. \n",getpid());
		return ;
	}

	filelen = DFS_seekFile(dfs, dfs_fd, filelen, SEEK_END);
	LOG(" pid[%d]filelen:%d \n", getpid(), filelen);

	do
	{
		memset(buffer, flag, sizeof(buffer));
		ret = DFS_writeFile(dfs, dfs_fd, buffer, sizeof(buffer));

		//filelen = DFS_seekFile(dfs, dfs_fd, sizeof(buffer), SEEK_CUR);
		//filelen = DFS_seekFile(dfs, dfs_fd, filelen, SEEK_END);
		//LOG("[%d]filelen:%d \n", getpid(), filelen);

		memset(buffer, flag+1, sizeof(buffer));
		ret = DFS_writeFile(dfs, dfs_fd, buffer, sizeof(buffer));

		//filelen = DFS_seekFile(dfs, dfs_fd, filelen, SEEK_END);
		//LOG("[%d]filelen:%d \n", getpid(), filelen);

		//filelen-=sizeof(buffer);
		//filelen = DFS_seekFile(dfs, dfs_fd, filelen, SEEK_SET);
		//LOG("[%d]filelen:%d \n", getpid(), filelen);

		//memset(readbuff, 0x00, sizeof(readbuff));
		//ret = DFS_readFile(dfs, dfs_fd, readbuff, sizeof(readbuff));
		//LOG(" pid[%d]readlen:%d read[%#x]\n", getpid(), ret, readbuff[0]);
	}while(i++ < WRITE_COUNT);

	DFS_closeFile(dfs, dfs_fd);

	return ;
}

void op_write_close(int flag)
{
	int ret = 0;
	int filelen = 0;
	char buffer[BUFFER_COUNT] = {0};
	char readbuff[BUFFER_COUNT] = {0};
	int i = 1;
	DFS			*dfs = NULL;
	struct DFSFD* dfs_fd = NULL;
	char ajsPath[1024] = {0};

	dfs = getDFS();
	if(NULL == dfs)
	{
		LOG("dfs is null pid[%d] \n", getpid());
		return;
	}

	memset(ajsPath, 0x00, sizeof(ajsPath));
	ADJUSTPATH(dfs, TEST_FILENAME, ajsPath);

	do
	{
		lock;

		dfs_fd = DFS_openFile(dfs, ajsPath, O_RDWR | O_CREAT , DFS_MODE);
		if(NULL == dfs_fd)
		{
			LOG("[%d]open file error. \n",getpid());
			unlock ;
			break;
		}

		filelen = DFS_seekFile(dfs, dfs_fd, 0, SEEK_END);
		LOG("[%d] w1filelen:%d \n", getpid(), filelen);

		memset(buffer, flag, sizeof(buffer));
		ret = DFS_writeFile(dfs, dfs_fd, buffer, sizeof(buffer));

		DFS_closeFile(dfs, dfs_fd);
		unlock;
	}while(i++ < WRITE_COUNT);

	return ;
}


void op_write_local_file(int flag)
{
	int ret = 0;
	int filelen = 0;
	char buffer[BUFFER_COUNT] = {0};
	char readbuff[BUFFER_COUNT] = {0};
	int i = 1;
	char ajsPath[1024] = {0};
	int fd = 0;

	fd = open(TEST_FILENAME_LOCAL, O_RDWR | O_CREAT | O_SYNC , DFS_MODE);
	if(0 > fd)
	{
		LOG("[%d]open file error. \n",getpid());
		return ;
	}

	do
	{
		memset(buffer, flag, sizeof(buffer));
		lock;
		filelen = lseek(fd, 0, SEEK_END);
		LOG(" pid[%d] w1filelen:%d \n", getpid(), filelen);
		ret = write(fd, buffer, sizeof(buffer));
		unlock;

		lock;
		filelen = lseek(fd, 0, SEEK_END);
		LOG("[%d] w2filelen:%d \n", getpid(), filelen);

		memset(buffer, flag+1, sizeof(buffer));
		ret = write(fd, buffer, sizeof(buffer));
		unlock;

		lock;
		filelen = lseek(fd, filelen, SEEK_SET);
		LOG("[%d] r filelen:%d \n", getpid(), filelen);

		memset(readbuff, 0x00, sizeof(readbuff));
		ret = read(fd, readbuff, sizeof(readbuff));
		unlock;

		LOG(" pid[%d]readlen:%d read[%#x]\n", getpid(), ret, readbuff[0]);
	}while(i++ < WRITE_COUNT);

	close(fd);

	return ;
}

#ifdef USE_MY_FILESIZE
void op_seek_file_mySize(int flag)
{
	int ret = 0;
	int filelen = 0;
	char buffer[BUFFER_COUNT] = {0};
	char readbuff[BUFFER_COUNT] = {0};
	int i = 1;
	DFS			*dfs = NULL;
	struct DFSFD* dfs_fd = NULL;
	char ajsPath[1024] = {0};

	dfs = getDFS();
	if(NULL == dfs)
	{
		LOG("dfs is null pid[%d] \n", getpid());
		return;
	}

	memset(ajsPath, 0x00, sizeof(ajsPath));
	ADJUSTPATH(dfs, TEST_FILENAME, ajsPath);
	dfs_fd = DFS_openFile(dfs, ajsPath, O_RDWR | O_CREAT | O_SYNC , DFS_MODE);
	if(NULL == dfs_fd)
	{
		LOG("[%d]open file error. \n",getpid());
		return ;
	}

	lock;
	filelen = DFS_seekFile(dfs, dfs_fd, filelen, SEEK_END);
	tsetFileSize(filelen);
	unlock;

	LOG(" pid[%d]filelen:%d \n", getpid(), filelen);

	do
	{
		memset(buffer, flag, sizeof(buffer));
		lock;
		setFileSize(tgetFileSize());
		filelen = DFS_seekFile(dfs, dfs_fd, 0, SEEK_END);
		LOG(" pid[%d] w1filelen:%d \n", getpid(), filelen);
		ret = DFS_writeFile(dfs, dfs_fd, buffer, sizeof(buffer));
		tincFileSize(sizeof(buffer));
		unlock;

		lock;
		setFileSize(tgetFileSize());
		filelen = DFS_seekFile(dfs, dfs_fd, 0, SEEK_END);
		LOG("[%d] w2filelen:%d \n", getpid(), filelen);

		memset(buffer, flag+1, sizeof(buffer));
		ret = DFS_writeFile(dfs, dfs_fd, buffer, sizeof(buffer));
		tincFileSize(sizeof(buffer));
		unlock;

		lock;
		setFileSize(tgetFileSize());
		filelen = DFS_seekFile(dfs, dfs_fd, filelen, SEEK_SET);
		LOG("[%d] r filelen:%d \n", getpid(), filelen);

		memset(readbuff, 0x00, sizeof(readbuff));
		ret = DFS_readFile(dfs, dfs_fd, readbuff, sizeof(readbuff));
		unlock;

		LOG(" pid[%d]readlen:%d read[%#x]\n", getpid(), ret, readbuff[0]);
	}while(i++ < WRITE_COUNT);

	DFS_closeFile(dfs, dfs_fd);

	return ;
}

int tgetFileSize()
{
	return filesize;
}

int tsetFileSize(int size)
{
	if(size < filesize)
		return filesize;
	return filesize=size;
}

int tincFileSize(int size)
{
	return filesize+=size;
}
#endif



void SeekFileSize()
{
	int ret = 0;
	int filelen = 0;
	char buffer[BUFFER_COUNT] = {0};
	char readbuff[BUFFER_COUNT] = {0};
	int i = 1;
	DFS			*dfs = NULL;
	struct DFSFD* dfs_fd = NULL;
	char ajsPath[1024] = {0};

	dfs = getDFS();
	if(NULL == dfs)
	{
		LOG("dfs is null pid[%d] \n", getpid());
		return;
	}

	memset(ajsPath, 0x00, sizeof(ajsPath));
	ADJUSTPATH(dfs, TEST_FILENAME, ajsPath);
	dfs_fd = DFS_openFile(dfs, ajsPath, O_RDWR | O_CREAT | O_SYNC , DFS_MODE);
	if(NULL == dfs_fd)
	{
		LOG("[%d]open file error. \n",getpid());
		return ;
	}

	do
	{
		//lock;
		filelen = DFS_seekFile(dfs, dfs_fd, 0, SEEK_END);
		LOG("[%d] seek  filelen:%d \n", getpid(), filelen);
		//unlock;
	}while(1);

	DFS_closeFile(dfs, dfs_fd);
}
