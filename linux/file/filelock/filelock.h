/*
 * filelock.h
 *
 *  Created on: Jun 22, 2016
 *      Author: zpzhao
 */

#ifndef FILELOCK_H_
#define FILELOCK_H_


#define DEBUG_LOG
#define TRACE
#ifdef DEBUG_LOG
#include <stdio.h>
#define LOG_POSITION(format, ...) printf(format,##__VA_ARGS__)
#define LOG(format, ...) LOG_POSITION("[%s][%s][%d][%d]  "format, __FILE__, __FUNCTION__, getpid(), __LINE__, ##__VA_ARGS__)

#ifdef TRACE
#define TRACE_LOG LOG
#else
#define TRACE_LOG(format, ...)
#endif


#else

#define LOG(format, ...)
#define TRACE_LOG LOG

#endif

#define TRACE0_LOG  TRACE_LOG
#define TRACE1_LOG  TRACE_LOG
#define TRACE2_LOG  TRACE_LOG


void unlockFile(int fd, int offset, int len);
void wLockFile(int fd, int offset, int len);
void appendUnlockFile(int fd);
void rLockFile(int fd, int offset, int len);


#endif /* FILELOCK_H_ */
