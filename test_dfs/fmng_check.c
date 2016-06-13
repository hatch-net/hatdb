/*
 * fmng_check.c
 *
 *  Created on: Jun 13, 2016
 *      Author: zpzhao
 */


#include "public.h"
#include "libxtreemfs4c.h"

/*
 * main function: entry test
 */
int fmng_main(int argc, char *argv[])
{
	char *filePath = NULL;
	if(argc > 2)
	{
		printf("args err \n");
		return -1;
	}
	else if(argc == 2)
	{
		filePath = argv[1];
	}
	ShowFmng(filePath);

	LOG("success. pid[%d] \n", getpid());
	return 0;
}


