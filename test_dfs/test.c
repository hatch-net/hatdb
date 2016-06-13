/*
	dfs interface test
*/
#include "public.h"

#include "multiproc_test.h"
#include "fmng_check.h"

//#define FMNG_CHECK

int main(int argc, char *argv[])
{
	int ret = 0;
#ifdef MULTIPROC_TEST
	ret = test_main(argc, argv);
#endif

#ifdef FMNG_CHECK
	fmng_main(argc, argv);
#endif

	return 0;
}
