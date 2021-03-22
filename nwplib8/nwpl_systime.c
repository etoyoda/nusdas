/*
 * NWP LIB: Numerical Weather Prediction Library
 *
 * nwpl_systime.c: get system-clock through environmental variable, time card,
 *                 or time(2) library function
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "nwpl_timecnv.h"

/* ============================================================================
 * NWP_gettime(): get year, month, day, hour, minute, and sequential miutes
 *                from a specified time card file
 *
 *  tcname:          [IN]  file name of a specified time card
 *  py, pm, pd, ph, pmn: [OUT] year, month, day, hour, minute
 *  return code:     sequential minutes since 1801/01/01: success
 *                   -1: spesified year is out of range: error
 *                   -2: cannot open time card: error
 *                   -3: cannot read time card: error
 */
int NWP_gettime(const char *tcname,
		int *py, int *pm, int *pd, int *ph, int *pmn)
{
    int rc;

    *py = *pm = *pd = *ph = *pmn = 0;

    if (tcname) {
        FILE *fp;
	char *xname, *fname;
        xname = malloc(strlen(tcname) + 1);
	strcpy(xname, tcname);
        fname = strtok(xname, " ");       /* remove space */

        if (!(fp = fopen(fname, "r"))) {
            fprintf(stderr, "Error: NWP_gettime(): ");
            fprintf(stderr, "cannot open time card [%s]\n", tcname);
            free(xname);
            return -2;
        }
        rc = fscanf(fp, "%d %d %d %d %d", py, pm, pd, ph, pmn);
        fclose(fp);
        free(xname);
    } else {                    /* if tcname==NULL */
        rc = fscanf(stdin, "%d %d %d %d %d", py, pm, pd, ph, pmn);
        tcname = "<stdin>";
    }

    if (rc == EOF || rc == 0) {
        fprintf(stderr, "Error: NWP_gettime(): ");
        fprintf(stderr, "cannot read time card [%s]\n", tcname);
        return -3;
    }
    fprintf(stderr, "file [%s] says: %04d %02d %02d %02d %02d\n",
            tcname, *py, *pm, *pd, *ph, *pmn);

    if (*py < 1801 || 9999 < *py) {
        fprintf(stderr, "Error: NWP_gettime(): ");
        fprintf(stderr, "given year %d is out of range (1801 .. 9999)\n", *py);
        return -1;
    }
    return NWP_ymdhm2seq(*py, *pm, *pd, *ph, *pmn);
}

/* ----------------------------------------------------------------------------
 * NWP_gettimeenv(): get year, month, day, hour, minute, and sequential miutes
 *                   from a specified environmental variable
 *
 *  envname:         [IN]  environmental variable specifying time card
 *  py, pm, pd, ph, pmn: [OUT] year, month, day, hour, minute
 *  return code:     sequential minutes since 1801/01/01: success
 *                   -1: specified year is out of range: error
 *                   -4: specified environmental variable is not defined: error
 */
static int NWP_gettimeenv(char *envname,
                            int *py, int *pm, int *pd, int *ph, int *pmn)
{
    char *val = getenv(envname);

    *py = *pm = *pd = *ph = *pmn = 0;

    if (val == NULL) {
        fprintf(stderr, "Error: NWP_gettimeenv(): ");
        fprintf(stderr, "environmental variable [%s] is not defined\n",
                envname);
        return -4;
    }
    sscanf(val, "%d %d %d %d %d", py, pm, pd, ph, pmn);

    fprintf(stderr,
            "environmental variable [%s] says: %04d %02d %02d %02d %02d\n",
            envname, *py, *pm, *pd, *ph, *pmn);

    if (*py < 1801 || 9999 < *py) {
        fprintf(stderr, "Error: NWP_gettimeenv(): ");
        fprintf(stderr, "given year %d is out of range (1801 .. 9999)\n", *py);
        return -1;
    }
    return NWP_ymdhm2seq(*py, *pm, *pd, *ph, *pmn);
}

/* ============================================================================
 * NWP_systime(): get system-clock through environmental variable, time card,
 *                or time(2) library function
 *
 *  py, pm, pd, ph, pmn, ps: [OUT] year, month, day, hour, minute, second
 *                           (ps is valid only if got from system-clock)
 *  return code:     sequential minutes since 1801/01/01: success
 *                   -1: specified year is out of range: error
 *                   -2: cannot open time card: error
 *                   -3: cannot read time card: error
 *                   -4: specified environmental variable is not defined: error
 */
int NWP_systime(int *py, int *pm, int *pd, int *ph, int *pmn, int *ps)
{
    int iseq, i70seq;
    time_t tloc;
    struct stat buf;

    fprintf(stderr, "\n--- You requested system-clock ...\n");

    *ps = 0;                    /* default is zero */
    if (getenv("SYSTIME")) {
        fprintf(stderr, "environmental variable SYSTIME is defined ...\n");
        if ((iseq = NWP_gettimeenv("SYSTIME", py, pm, pd, ph, pmn)) >= 0)
            return iseq;
    }

    if (!stat("SYSTIME", &buf)) {
        fprintf(stderr, "file SYSTIME exists ...\n");
        if ((iseq = NWP_gettime("SYSTIME", py, pm, pd, ph, pmn)) >= 0)
            return iseq;
    }

    fprintf(stderr, "read system-clock ...\n");
    time(&tloc);

    i70seq = NWP_ymdhm2seq(1970, 1, 1, 0, 0);       /* unix standard time */
    *ps = tloc % 60;
    iseq = tloc / 60 + i70seq;
    NWP_seq2ymdhm(py, pm, pd, ph, pmn, iseq);

    fprintf(stderr, "system-clock says: %04d %02d %02d %02d %02d %02d\n",
            *py, *pm, *pd, *ph, *pmn, *ps);

    return iseq;
}

/* nwpl_systime.c */
