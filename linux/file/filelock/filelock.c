/*
 * filelock.c
 *
 *  Created on: Jun 22, 2016
 *      Author: zpzhao
 */

/*
 * README.md
 * two type file lock:
 * 1. int flock(int fd, int operation);
 * (1) LOCK_SH  share lock
 * (2) LOCK_EX  exclusive lock
 * (3) LOCK_UN  remove lock
 *
 * 2. struct flock lock;
 *    lock.l_type = F_RDLCK; F_WRLCK; F_UNLCK
 *    lock.l_start = offset;
 *    lock.l_whence = SEEK_SET;
 *    lock.l_len = len;
 *    lock.l_pid = getpid();
 *   fcntl(m_fdInfoFile, F_SETLKW, &lock);
 *   cmd: F_GETLK, F_SETLK(no block),F_SETLKW(block, wait for lock)
 *
 * But above all, if one process lock file, but other process not try flock, then will write sucessful.
 * so, other process must try flock which lock file take effect.
 */


#include "filelock.h"
#include <unistd.h>
#include <fcntl.h>

// flock()
#include <sys/file.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int fd = -1;
    int range = 100;

    if(argc < 1)
    {
        LOG("please input lock range, please. \n");
        return 0;
    }

    range = atoi(argv[1]);

    fd = open("lock.t", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(0 > fd)
    {
        LOG("open file err\n");
        return 0;
    }

    do
    {
        //rLockFile(fd, range, range);
        wLockFile(fd, range, range);
        sleep(600);

        unlockFile(fd, range, range);
    }while(1);

    close(fd);
    return 0;
}


void unlockFile(int fd, int offset, int len)
{
    if(0 > fd)
    {
        LOG("fd is null \n");
        return ;
    }

    static struct flock lock;

    lock.l_type = F_UNLCK;
    lock.l_start = offset;
    lock.l_whence = SEEK_SET;
    lock.l_len = len;
    lock.l_pid = getpid();

    int ret = fcntl(fd, F_SETLKW, &lock);
    if(0 < ret)
    {
        LOG("unlock file err\n");
    }
    TRACE0_LOG("==\n");
}

void wLockFile(int fd, int offset, int len)
{
    if(0 > fd)
    {
        LOG("fd is null \n");
        return ;
    }

    static struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lock.l_whence = SEEK_SET;
    lock.l_len = len;
    lock.l_pid = getpid();

    int ret = fcntl(fd, F_SETLKW, &lock);
    if(0 < ret)
    {
        LOG("lock file err\n");
    }
    TRACE0_LOG("==\n");
}

void appendUnlockFile(int fd)
{
    if(0 > fd)
    {
        LOG("fd is null \n");
        return ;
    }

    static struct flock lock;

    lock.l_type = F_UNLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_END;
    lock.l_len = 0; /* lock start current end to file end, no matter how large the file grows */
    lock.l_pid = getpid();

    int ret = fcntl(fd, F_SETLKW, &lock);
    if(0 < ret)
    {
        LOG("unlock file err\n");
    }
    TRACE0_LOG("==\n");
}


void rLockFile(int fd, int offset, int len)
{
    if(0 > fd)
    {
        LOG("fd is null \n");
        return ;
    }

    static struct flock lock;

    lock.l_type = F_RDLCK;
    lock.l_start = offset;
    lock.l_whence = SEEK_SET;
    lock.l_len = len;
    lock.l_pid = getpid();

    int ret = fcntl(fd, F_SETLKW, &lock);
    if(0 < ret)
    {
        LOG("lock file err\n");
    }
    TRACE0_LOG("==\n");
}
