#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys_time.h"
#include "sys_err.h"
#include "sys_mem.h"
# define NEED_STRING_COPY
#include "sys_string.h"

#ifdef USE_NUS_PROFILE
static unsigned long *Counter = NULL;
static int Current_Part = -1;
static struct timeval LastTime;
static char FileName[80] = "nusprof.txt";
#endif

	void
nusprof_diag(void)
{
#ifdef USE_NUS_PROFILE
	FILE *fp;
	int ipart;
	nuserr_final();
	fp = fopen(FileName, "a");
	for (ipart = 0; ipart < NP_COUNT; ipart++) {
		fprintf(fp, "GRPF %02d: %10lu.%06lu\n", ipart,
				Counter[ipart] / 1000000ul,
				Counter[ipart] % 1000000ul);
	}
	fclose(fp);
#endif
}

	int
nusprof_ini(const char *filename UNUSED)
{
#ifdef USE_NUS_PROFILE
	int i;
	if (Counter) {
		return -1;
	}
	if (filename && filename[0]) {
		string_copy(FileName, filename, sizeof FileName);
	}
	Counter = nus_malloc(NP_COUNT * sizeof(unsigned long));
	for (i = 0; i < NP_COUNT; i++) {
		Counter[i] = 0;
	}
	if (Counter == NULL) return -1;
	return atexit(nusprof_diag);
#else
	return -1;
#endif
}

	enum nusprof_sectype_t
nusprof_state(void)
{
#ifdef USE_NUS_PROFILE
	if (Counter == NULL)
		return (enum nusprof_sectype_t)-1;
	else
		return Current_Part;
#else
	return 0;
#endif
}

	void
nusprof_mark(enum nusprof_sectype_t ipart UNUSED)
{
#ifdef USE_NUS_PROFILE
	struct timeval Now;
	struct timezone Tz;

	if (Counter == NULL) {
		return;
	}
	gettimeofday(&Now, &Tz);
	if ((Current_Part >= 0) && (Current_Part < NP_COUNT)) {
		Counter[Current_Part] += (Now.tv_usec - LastTime.tv_usec)
			+ 1000000ul * (Now.tv_sec - LastTime.tv_sec);
	}
	LastTime = Now;
	Current_Part = ipart;
#endif
}
