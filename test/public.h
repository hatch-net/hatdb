/*
 * public.h
 *
 *  Created on: May 31, 2016
 *      Author: zpzhao
 */

#ifndef PUBLIC_H_
#define PUBLIC_H_
#include <semaphore.h>

int ForkSubProcess(int argc, char *argv[]);
int SubProcessMain(int argc, char *argv[]);
void WaitEndAllProcess();


void sem_P(sem_t *sem);
void sem_V(sem_t *sem);


int tgetFileSize();
int tsetFileSize(int size);
int tincFileSize(int size);

/*
 * seek file end ,print file size
 */
int SubProcessMain_seek(int argc, char *argv[]);

/* test case here */
void op_seek_file(int flag);

void op_write_no_seek_file(int flag);

/* lock->openfile->seek_end->write->closefile->unlock */
void op_write_close(int flag);

void op_write_local_file(int flag);

void op_seek_file_mySize(int flag);

void SeekFileSize();
/* end test case */

#endif /* PUBLIC_H_ */
