/*
 * multiproc_test.h
 *
 *  Created on: Jun 13, 2016
 *      Author: zpzhao
 */

#ifndef MULTIPROC_TEST_H_
#define MULTIPROC_TEST_H_
#include <semaphore.h>


/*
 * main function: entry test
 */
int test_main(int argc, char *argv[]);


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

#endif /* MULTIPROC_TEST_H_ */
