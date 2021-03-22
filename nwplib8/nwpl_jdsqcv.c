/*
 * NWP LIB: Numerical Weather Prediction Library
 *
 * nwpl_jdsqcv.c: conversion between year, month, day, hour, minute
 *                and sequential time since 1801/01/01
 *
 *  iseq = NWP_ymd2seq  (iy, im, id)
 *  iseq = NWP_ymdh2seq (iy, im, id, ih)
 *  iseq = NWP_ymdhm2seq(iy, im, id, ih, imn)
 *  void   NWP_seq2ymd  (py, pm, pd,          iseq)
 *  void   NWP_seq2ymdh (py, pm, pd, ph,      iseq)
 *  void   NWP_seq2ymdhm(py, pm, pd, ph, pmn, iseq)
 */

#include "nwpl_timecnv.h"

#define MINS_PER_DAY      (24 * 60)
#define DAYS_PER_OLYMPIAD (365 * 4 + 1)
#define DAYS_PER_CENTURY  (DAYS_PER_OLYMPIAD * 25 - 1)
#define DAYS_PER_400YRS   (DAYS_PER_CENTURY * 4 + 1)
#define ORIGIN2000        72743  /* days since 1801 of 2000-03-01T00:00:00Z */
#define ORIGIN1600        (ORIGIN2000 - DAYS_PER_400YRS)
#define YEAR_ZERO         -657742

	static int
month_offset(int year, int month)
{
	int dseq;
	while (month <= 2) {
		month += 12;
		year--;
	}
	while (month > (2+12)) {
		month -= 12;
		year++;
	}
	dseq = (306 * (month + 1)) / 10 - 122 + YEAR_ZERO;
/* --- WHY THIS WORKS? ---
 * The C89 Standard requires that (year / 400) be either floor(year / 400.0)
 * or ceil(year / 400.0), and that ((year / 400) * 400 + year % 400 == year).
 *
 * What we want here is shift by floor(year / 400.0) times 400 years.
 * That is achieved in case of downward truncation.
 * In case of truncation toward zero [that is the C99 Standard],
 * (year / 400) == ceil(year / 400.0) and -399 <= (year % 400) <= 0.
 * The result is different if (year % 400 != 0) and it is adjusted by
 * an if branch (year < 0) below.
 */
	dseq += (year / 400) * DAYS_PER_400YRS;
	year %= 400;
	if (year < 0) {
		dseq -= DAYS_PER_400YRS;
		year += 400;
	}
	dseq += (year / 100) * DAYS_PER_CENTURY;
	year %= 100;
	dseq += (year / 4) * DAYS_PER_OLYMPIAD;
	year %= 4;
	dseq += year * 365;
	return dseq;
}

/* ============================================================================
 * NWP_ymd2seq(): year, month, day
 *                -> sequential days since 1801/01/01
 *
 *  iy, im, id:  [IN] year, month, day
 *  return value: sequential days (1801/01/01 = 1)
 */
	int
NWP_ymd2seq(int iy, int im, int id)
{
	return month_offset(iy, im) + id;
}

/* ============================================================================
 * NWP_ymdh2seq(): year, month, day, hour
 *                 -> sequential hours since 1801/01/01
 *
 *  iy, im, id, ih: [IN] year, month, day, hour
 *  return value:    sequential hours (1801/01/01 00Z = 0)
 */
	int
NWP_ymdh2seq(int iy, int im, int id, int ih)
{
	return (month_offset(iy, im) + (id - 1)) * 24 + ih;
}

/* ============================================================================
 * NWP_ymdhm2seq(): year, month, day, hour, minute
 *                  -> sequential minutes since 1801/01/01
 *
 *  iy, im, id, ih, imn: [IN] year, month, day, hour
 *  return value:         sequential minutes (1801/01/01 00:00Z = 0)
 */
	int
NWP_ymdhm2seq(int iy, int im, int id, int ih, int imn)
{
	return ((month_offset(iy, im) + (id - 1)) * 24 + ih) * 60 + imn;
}

/** 
 * determine year and month from sequential day
 * seqday: [IN] sequential day (1801/01/01 = 1)
 */
	static int
make_ymd(int *py, int *pm, int seqday)
{
	int year, century, olympic, month;
	/*
	 * decompose given seqday to quad-century and remainder in days
	 */
	if (seqday >= ORIGIN2000) {
		seqday -= ORIGIN2000;
		year = 2000 + (seqday / DAYS_PER_400YRS) * 400;
		seqday %= DAYS_PER_400YRS;
	} else if (seqday >= ORIGIN1600) {
		seqday -= ORIGIN1600;
		year = 1600;
	} else {
		int stepback = (ORIGIN1600 - 1 - seqday) / DAYS_PER_400YRS;
		year = 1200 - stepback * 400;
		seqday -= (ORIGIN1600 - DAYS_PER_400YRS);
		seqday += stepback * DAYS_PER_400YRS;
	}
	/*
	 * decompose the remainder to century (added to year) and
	 * remainder in days
	 * Note: the last (3rd) century in a quad-century is LONGER by 1 day.
	 */
	century = (seqday / DAYS_PER_CENTURY) * 100;
	seqday %= DAYS_PER_CENTURY;
	if (century == 400) {
		century -= 100;
		seqday += DAYS_PER_CENTURY;
	}
	year += century;
	/*
	 * decompose the remainder to olympiad and remainder in days
	 * Note: the last olympiad in a century is SHORTER by 1 day.
	 */
	year += (seqday / DAYS_PER_OLYMPIAD) * 4;
	seqday %= DAYS_PER_OLYMPIAD;
	/*
	 * decompose the remainder to year and remainder in days
	 * Note: the last year in an olympiad is LONGER by 1 day.
	 */
	olympic = seqday / 365;
	seqday %= 365;
	if (olympic == 4) {
		olympic--;
		seqday += 365;
	}
	year += olympic;
	/*
	 * get month from days in a year
	 */
	month = (seqday * 10 + 4) / 306 + 3;
	seqday -= (306 * month + 6) / 10 - 93;
	if (month > 12) {
		/* switch to Circumcision Style:
		 * a year begins in January instead of March */
		month -= 12;
		year++;
	}
	/* result */
	*py = year;
	*pm = month;
	return seqday;
}

/* ============================================================================
 * NWP_seq2ymd(): sequential days since 1801/01/01
 *                -> year, month, day
 *
 *  py, pm, pd: [OUT] year, month, day
 *  iseq:       [IN]  sequential days since 1801/01/01
 */
	void
NWP_seq2ymd(int *py, int *pm, int *pd, int seqday)
{
	*pd = make_ymd(py, pm, seqday - 1);
}

/* ============================================================================
 * NWP_seq2ymdh(): sequential hours since 1801/01/01
 *                 -> year, month, day, hour
 *
 *  py, pm, pd, ph: [OUT] year, month, day, hour
 *  iseq:           [IN]  sequential hours since 1801/01/01
 */
	void
NWP_seq2ymdh(int *py, int *pm, int *pd, int *ph, int seqhour)
{
	int seqday;
	seqday = seqhour / 24;
	seqhour %= 24;
	if (seqhour < 0) {
		seqday--;
		seqhour += 24;
	}
	*pd = make_ymd(py, pm, seqday);
	*ph = seqhour;
}

/* ============================================================================
 * NWP_seq2ymdhm(): sequential minutes since 1801/01/01
 *                  -> year, month, day, hour, minute
 *
 *  py, pm, pd, ph, pmn: [OUT] year, month, day, hour, minute
 *  iseq:                [IN]  sequential minutes since 1801/01/01
 */
	void
NWP_seq2ymdhm(int *py, int *pm, int *pd, int *ph, int *pmn, int seqmin)
{
	int seqday;
	seqday = seqmin / MINS_PER_DAY;
	seqmin %= MINS_PER_DAY;
	if (seqmin < 0) {
		seqday--;
		seqmin += MINS_PER_DAY;
	}
	*pd = make_ymd(py, pm, seqday);
	*ph = seqmin / 60;
	*pmn = seqmin % 60;
}

/* nwpl_jdsqcv.c */
