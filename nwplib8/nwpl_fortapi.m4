dnl nwpl_fortapi.m4: source for nwpl_fortapi.c
dnl
dnl If you see this line, you can ignore the next one.
/* DO NOT EDIT; this file is produced from the corresponding m4 source
 * as follows:
 *   m4 source.m4 > thisfile
 *
 * NWP LIB: Numerical Weather Prediction Library
 *
 * nwpl_fortapi.c: NWP LIB Fortran Application Interface
 */
dnl to avoid error caused by single quotation marks in m4 statement
changequote(@<:@, @:>@)dnl
dnl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nwpl_timecnv.h"

extern void NWP_filestat(char *fname, int *rc, int fname_length);

#if defined(IS_PGI) || defined(IS_G95_64)
#define size_t int
#endif

static char *strcpy_nospace(char *dst, const char *src);
static void strcnv_f2c(char *cstr, const char *fstr, size_t len_fstr);
#if 0
static void strcnv_c2f(char *fstr, const char *cstr);
#endif

/* ----------------------------------------------------------------------------
 * strcpy_nospace(): copy source string to destination string unless ' '
 */
static char *strcpy_nospace(char *dst, const char *src)
{
    char *p = dst;

    for ( ; *src != '\0'; src++)
        if (*src != ' ')
            *dst++ = *src;
    *dst = '\0';

    return p;
} /* strcpy_nospace() */

/* ----------------------------------------------------------------------------
 * strcnv_f2c(): convert Fortran-type string to C-type string,
 *               add NULL terminator
 *
 *  cstr:     [OUT] C-type string, '\0' terminated
 *  fstr:     [IN]  Fortran-type string
 *  len_fstr: [IN]  length of fstr
 */
static void strcnv_f2c(char *cstr, const char *fstr, size_t len_fstr)
{
    char wk[256];

    memmove(wk, fstr, len_fstr);
    *(wk + len_fstr) = '\0';
    strcpy_nospace(cstr, wk);
} /* strcnv_f2c() */

#if 0
/* ----------------------------------------------------------------------------
 * strcnv_c2f(): convert C-type string to Fortran-type string,
 *               remove NULL terminator
 *
 *  fstr: [OUT] Fortran-type string
 *  cstr: [IN]  C-type string, '\0' terminated
 */
static void strcnv_c2f(char *fstr, const char *cstr)
{
    strcpy(fstr, cstr);
    *(fstr + strlen(cstr)) = ' ';
} /* strcnv_c2f() */
#endif

dnl
define(@<:@D_NWP_YMD2SEQ@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): year, month, day
 *              -> sequential days since 1801/01/01
 */
void $1(const int *iy, const int *im, const int *id, int *iseq)
{
    *iseq = NWP_ymd2seq(*iy, *im, *id);
} /* $1() */
@:>@)dnl
D_NWP_YMD2SEQ(NWPC_YMD2SEQ)
D_NWP_YMD2SEQ(NWPC_YMD2SEQ_)
D_NWP_YMD2SEQ(nwpc_ymd2seq)
D_NWP_YMD2SEQ(nwpc_ymd2seq_)
D_NWP_YMD2SEQ(nwpc_ymd2seq__)dnl

dnl
define(@<:@D_NWP_YMDH2SEQ@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): year, month, day, hour
 *              -> sequential hours since 1801/01/01
 */
void $1(const int *iy, const int *im, const int *id, const int *ih, int *iseq)
{
    *iseq = NWP_ymdh2seq(*iy, *im, *id, *ih);
} /* $1() */
@:>@)dnl
D_NWP_YMDH2SEQ(NWPC_YMDH2SEQ)
D_NWP_YMDH2SEQ(NWPC_YMDH2SEQ_)
D_NWP_YMDH2SEQ(nwpc_ymdh2seq)
D_NWP_YMDH2SEQ(nwpc_ymdh2seq_)
D_NWP_YMDH2SEQ(nwpc_ymdh2seq__)dnl

dnl
define(@<:@D_NWP_YMDHM2SEQ@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): year, month, day, hour, minute
 *              -> sequential minutes since 1801/01/01
 */
void $1(const int *iy, const int *im, const int *id, const int *ih,
        const int *imn, int *iseq)
{
    *iseq = NWP_ymdhm2seq(*iy, *im, *id, *ih, *imn);
} /* $1() */
@:>@)dnl
D_NWP_YMDHM2SEQ(NWPC_YMDHM2SEQ)
D_NWP_YMDHM2SEQ(NWPC_YMDHM2SEQ_)
D_NWP_YMDHM2SEQ(nwpc_ymdhm2seq)
D_NWP_YMDHM2SEQ(nwpc_ymdhm2seq_)
D_NWP_YMDHM2SEQ(nwpc_ymdhm2seq__)dnl

dnl
define(@<:@D_NWP_SEQ2YMD@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): sequential days since 1801/01/01
 *              -> year, month, day
 */
void $1(int *py, int *pm, int *pd, const int *iseq)
{
    NWP_seq2ymd(py, pm, pd, *iseq);
} /* $1() */
@:>@)dnl
D_NWP_SEQ2YMD(NWPC_SEQ2YMD)
D_NWP_SEQ2YMD(NWPC_SEQ2YMD_)
D_NWP_SEQ2YMD(nwpc_seq2ymd)
D_NWP_SEQ2YMD(nwpc_seq2ymd_)
D_NWP_SEQ2YMD(nwpc_seq2ymd__)dnl

dnl
define(@<:@D_NWP_SEQ2YMDH@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): sequential hours since 1801/01/01
 *              -> year, month, day, hour
 */
void $1(int *py, int *pm, int *pd, int *ph, const int *iseq)
{
    NWP_seq2ymdh(py, pm, pd, ph, *iseq);
} /* $1() */
@:>@)dnl
D_NWP_SEQ2YMDH(NWPC_SEQ2YMDH)
D_NWP_SEQ2YMDH(NWPC_SEQ2YMDH_)
D_NWP_SEQ2YMDH(nwpc_seq2ymdh)
D_NWP_SEQ2YMDH(nwpc_seq2ymdh_)
D_NWP_SEQ2YMDH(nwpc_seq2ymdh__)dnl

dnl
define(@<:@D_NWP_SEQ2YMDHM@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): sequential minutes since 1801/01/01
 *              -> year, month, day, hour, minute
 */
void $1(int *py, int *pm, int *pd, int *ph, int *pmn, const int *iseq)
{
    NWP_seq2ymdhm(py, pm, pd, ph, pmn, *iseq);
} /* $1() */
@:>@)dnl
D_NWP_SEQ2YMDHM(NWPC_SEQ2YMDHM)
D_NWP_SEQ2YMDHM(NWPC_SEQ2YMDHM_)
D_NWP_SEQ2YMDHM(nwpc_seq2ymdhm)
D_NWP_SEQ2YMDHM(nwpc_seq2ymdhm_)
D_NWP_SEQ2YMDHM(nwpc_seq2ymdhm__)dnl

dnl
define(@<:@D_NWP_GETTIME@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): get year, month, day, hour, minute, and sequential miutes
 *                from a specified time card file
 */
void $1(const char *tcname, int *py, int *pm, int *pd, int *ph, int *pmn,
        int *iseq, size_t len_tcname)
{
    char *c_tcname;

    c_tcname = (char *)malloc(len_tcname + 1);
    if (c_tcname == NULL) {
        fprintf(stderr, "Error: cannot allocate %u bytes\n", len_tcname + 1);
        *iseq = -5;
        return;
    }
    strcnv_f2c(c_tcname, tcname, len_tcname);

    *iseq = NWP_gettime(c_tcname, py, pm, pd, ph, pmn);

    free(c_tcname);
} /* $1() */
@:>@)dnl
D_NWP_GETTIME(NWPC_GETTIME)
D_NWP_GETTIME(NWPC_GETTIME_)
D_NWP_GETTIME(nwpc_gettime)
D_NWP_GETTIME(nwpc_gettime_)
D_NWP_GETTIME(nwpc_gettime__)dnl

dnl
define(@<:@D_NWP_SYSTIME@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): get system-clock through environmental variable, time card,
 *                or time(2) library function
 */
void $1(int *py, int *pm, int *pd, int *ph, int *pmn, int *ps, int *iseq)
{
    *iseq = NWP_systime(py, pm, pd, ph, pmn, ps);
} /* $1() */
@:>@)dnl
D_NWP_SYSTIME(NWPC_SYSTIME)
D_NWP_SYSTIME(NWPC_SYSTIME_)
D_NWP_SYSTIME(nwpc_systime)
D_NWP_SYSTIME(nwpc_systime_)
D_NWP_SYSTIME(nwpc_systime__)dnl

dnl
define(@<:@D_NWP_FILESTAT@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): check File Exists
 */
void $1(char *fname, int *rc, int fname_length)
{
    NWP_filestat(fname, rc, fname_length);
} /* $1() */
@:>@)dnl
D_NWP_FILESTAT(NWPC_FILESTAT)
D_NWP_FILESTAT(NWPC_FILESTAT_)
D_NWP_FILESTAT(nwpc_filestat)
D_NWP_FILESTAT(nwpc_filestat_)
D_NWP_FILESTAT(nwpc_filestat__)dnl

dnl
define(@<:@D_NWP_LUSTRE_RECOVER@:>@, dnl
@<:@dnl
/* ============================================================================
 * $1(): recover lustre missing tail
 */
void $1(char *fname, int *rc, int fname_length)
{
    NWP_lustre_recover(fname, rc, fname_length);
} /* $1() */
@:>@)dnl
D_NWP_LUSTRE_RECOVER(NWPC_LUSTRE_RECOVER)
D_NWP_LUSTRE_RECOVER(NWPC_LUSTRE_RECOVER_)
D_NWP_LUSTRE_RECOVER(nwpc_lustre_recover)
D_NWP_LUSTRE_RECOVER(nwpc_lustre_recover_)
D_NWP_LUSTRE_RECOVER(nwpc_lustre_recover__)dnl

/* nwpl_fortapi.c */
dnl nwpl_fortapi.m4
