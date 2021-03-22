/*
 * NWP LIB: Numerical Weather Prediction Library
 *
 * nwpl_timecnv.h: prototypes of function dealing with time conversion
 */

#ifndef NWPL_TIMECNV_H_INCLUDED
#define NWPL_TIMECNV_H_INCLUDED

/* nwpl_jdsqcv.c */
int NWP_ymd2seq(int iy, int im, int id);
int NWP_ymdh2seq(int iy, int im, int id, int ih);
int NWP_ymdhm2seq(int iy, int im, int id, int ih, int imn);
void NWP_seq2ymd(int *py, int *pm, int *pd, int iseq);
void NWP_seq2ymdh(int *py, int *pm, int *pd, int *ph, int iseq);
void NWP_seq2ymdhm(int *py, int *pm, int *pd, int *ph, int *pmn, int iseq);

/* nwpl_systime.c */
int NWP_gettime(const char *tcname, int *py, int *pm, int *pd, int *ph,
                int *pmn);
int NWP_systime(int *py, int *pm, int *pd, int *ph, int *pmn, int *ps);

#endif /* NWPL_TIMECNV_H_INCLUDED */

/* nwpl_timecnv.h */
