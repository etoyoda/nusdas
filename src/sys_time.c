/** \file
 * \brief 時間、とくに数値予報通算分に関する処理
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include "internal_types.h"
# define NEED_DECIMAL_UINT
#include "sys_time.h"
#include "sys_file.h"
#include "sys_kwd.h"
# define NEED_MEMCPY12
#include "sys_string.h"
#include "nwpl_capi.h"

/** @brief 数字列から通算分への変換
 *
 * 10 または 12 文字の数字列 yyyymmddhh または yyyymmddhhnn を
 * 年月日時(分)と解釈して数値予報通算分に変換する。
 *
 * @retval 0 変換に成功した
 * @retval -1 書式が不正
 * @bug 西暦紀元前の -001 などは書式エラーとなる。
 * @bug 通算分がオーバーフローする 5884 年以降の年には対応していない。
 */
	int
string_to_time(N_SI4 *dest, const char *src)
{
	unsigned	y, m, d, h, n;
	if (decimal_uint(src + 0, 4, &y) != 0) { return *dest = -1; }
	if (decimal_uint(src + 4, 2, &m) != 0) { return *dest = -1; }
	if (decimal_uint(src + 6, 2, &d) != 0) { return *dest = -1; }
	if (decimal_uint(src + 8, 2, &h) != 0) {
		n = 0;
	} else {
		decimal_uint(src + 10, 2, &n);
	}
	*dest = nwp_ymdhm2seq(y, m, d, h, n);
	return 0;
}

/** @brief pandora 形式数字列から通算分への変換
 *
 * 15 文字の数字列 yyyy-mm-ddthhnn を 
 * 年月日時分と解釈して、または minXX を解釈して数値予報通算分に変換する。
 *
 * @retval 0 変換に成功した
 * @retval -1 書式が不正
 * @note 分を省略することは認めない。
 * @bug 西暦紀元前の -001 などは書式エラーとなる。
 * @bug 通算分がオーバーフローする 5884 年以降の年には対応していない。
 */
	int
panstring_to_time(N_SI4 *dest, const char *src, int len)
{
	if (len == 15 && src[4] == '-' && src[7] == '-' && src[10] == 't') {
		unsigned	y, m, d, h, n;
		/* yyyy-mm-ddthhmm style */
		if (decimal_uint(src + 0, 4, &y) != 0) { return *dest = -1; }
		if (decimal_uint(src + 5, 2, &m) != 0) { return *dest = -1; }
		if (decimal_uint(src + 8, 2, &d) != 0) { return *dest = -1; }
		if (decimal_uint(src + 11, 2, &h) != 0) { return *dest = -1; }
		if (decimal_uint(src + 13, 2, &n) != 0) { return *dest = -1; }
		*dest = nwp_ymdhm2seq(y, m, d, h, n);
	} else if (strncmp(src, "min", 3) == 0) {
		unsigned minute;
		if (src[3] == '-') {
			if (decimal_uint(src + 4, len - 4, &minute) != 0) { 
				return *dest = -1;
			}
			*dest = -(int)minute;
		} else {
			if (decimal_uint(src + 3, len - 3, &minute) != 0) { 
				return *dest = -1;
			}
			*dest = minute;
		}
	} else {
		return *dest = -1;
	}
	return 0;
}

INLINE void
uint_decimal(char *p, unsigned val, int n)
{
	int	i;
	for (i = n - 1; i >= 0; i--) {
		p[i] = (val % 10) + '0';
		val /= 10;
	}
}

/** @brief 通算分から数字列への変換
 *
 * 数値予報通算分を yyyymmddhhnn 形式の 12 文字の文字列に変換する。
 * @bug 0000-01-01t00:00 以前の時刻に対応していない。
 */
	int
time_to_chars(char *dest, N_SI4 src)
{
	int	y, m, d, h, n;
	char	buf[16];
	nwp_seq2ymdhm(&y, &m, &d, &h, &n, src);
	uint_decimal(buf + 0, y, 4);
	uint_decimal(buf + 4, m, 2);
	uint_decimal(buf + 6, d, 2);
	uint_decimal(buf + 8, h, 2);
	uint_decimal(buf + 10, n, 2);
	memcpy12(dest, buf);
	return 0;
}

	INLINE int
leap_year(int y)
{
	if (y % 400 == 0) {
		return 1;
	} else if (y % 100 == 0) {
		return 0;
	} else if (y % 4 == 0) {
		return 1;
	} else {
		return 0;
	}
}

	INLINE int
month_days(int y, int m)
{
	switch (m) {
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
		default:
			return 31;
		case 4: case 6: case 9: case 11:
			return 30;
		case 2:
			return leap_year(y) ? 29 : 28;
	}
}

	N_SI4
time_add(N_SI4 base, N_SI4 ftime, sym4_t ftunits)
{
	int	y, m, d, h, n;
	int	i;
	switch (ftunits) {
		case SYM4_MIN:
			return base + ftime;
		case SYM4_HOUR:
			return base + ftime * 60;
		case SYM4_DAY:
			return base + ftime * 60 * 24;
		case SYM4_WEEK:
			return base + ftime * 60 * 24 * 7;
		case SYM4_PENT:
			goto pentad;
		case SYM4_JUN:
			goto jun;
		case SYM4_MONT:
			nwp_seq2ymdhm(&y, &m, &d, &h, &n, base);
			m += ftime;
			m--;
/**
 * 負の ftime [月数] が与えられた場合、m が負になりうる。
 * C89 は m / 12 が floor(m / 12) または ceil(m / 12) のどちらかであることと
 * (m / 12) * 12 + m % 12 == m であることを要請している
 * [JIS X 3010-1993 §6.3.5]。
 * 負値の m に対して m / 12 == floor(m / 12) のとき、0 <= m % 12 < 12
 * であって、m はそのまま月番号 [ゼロ始まり] と解釈できるため
 * 下の if (m < 0) に入ることはない。
 * 逆に m / 12 == ceil(m / 12) のとき [C99 はこの動作を要求している] の
 * ときは -12 < m % 12 <= 0 となる。m % 12 == 0 ならば m は 12 の
 * 倍数であったので特段の処置はいらない。
 * -11 <= m % 12 <= -1 のときは、下の if (m < 0) のごとき対処をすればよい。
 */
			y += m / 12;
			m %= 12;
			if (m < 0) {
				m += 12;
				y--;
			}
			m++;
			return nwp_ymdhm2seq(y, m, d, h, n);
		case SYM4_YEAR:
			nwp_seq2ymdhm(&y, &m, &d, &h, &n, base);
			y += ftime;
			return nwp_ymdhm2seq(y, m, d, h, n);
		default:
			return (N_SI4)-1;
	}
#define NEXTMONTH	if (++m > 12) { m -= 12; y++; }
pentad:
	nwp_seq2ymdhm(&y, &m, &d, &h, &n, base);
	if (ftime < 0) {
		/** 負の半旬数の予報時間に対しては、
		 * 必要な年数遡った同じ月日から正の半旬数進んだ
		 * 時点をもって答えとする。
		 */
		int years = (71 - ftime) / 72;
		y -= years;
		ftime += years * 72;
	}
	for (i = 0; i < ftime; i++) {
		if (leap_year(y) && m == 2 && d >= 24) {
			/* Feb 25 => Mar 2 */
			d -= 23;
			if (d == 6) {
				d--;
			}
			m = 3;
		} else {
			d += 5;
			if (d > month_days(y, m)) {
				d -= month_days(y, m);
				NEXTMONTH
			}
		}
	}
	return nwp_ymdhm2seq(y, m, d, h, n);
jun:
	/** @todo これで NuSDaS 1.1 互換となる。なぜかはわからない。 */
	nwp_seq2ymdhm(&y, &m, &d, &h, &n, base);
	if (ftime < 0) {
		/* 負の予報時間の対応 */
		int years = (35 - ftime) / 36;
		y -= years;
		ftime += years * 36;
	}
	if (ftime >= 3) {
		int day31 = (d == 31);
		m += ftime / 3;
		m--;
		y += m / 12;
		m %= 12;
		m++;
		if (day31) { d = month_days(y, m); }
		ftime %= 3;
	}
	if (ftime == 1) {
		if (d > 20) {
			d = (d >= 30) ? 10 : (d % 10);
			NEXTMONTH
		} else if (m == 2 && d + 10 > month_days(y, m)) {
			d = d - month_days(y, m) + 10;
			NEXTMONTH
		} else {
			d += 10;
		}
	} else if (ftime == 2) {
		if (d > 10) {
			d = (d >= 30) ? 20 : (d % 10) + 10;
			NEXTMONTH
		} else if (m == 2 && d + 10 > month_days(y, m)) {
			d = d - month_days(y, m) + 20;
			NEXTMONTH
		} else {
			d += 20;
		}
	}
	return nwp_ymdhm2seq(y, m, d, h, n);
}
