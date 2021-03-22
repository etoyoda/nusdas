#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "nusdas.h"
#include "nwpl_capi.h"
#include "stringplus.h"
#include "nusdim.h"

#if 0
static char rcsid[] = "$Id: glue.c,v 1.1 2007-02-26 02:46:55 suuchi43 Exp $";
#endif

#define LIMIT(var, lim) ((var) = ((var) > (lim)) ? (lim) : (var))

	int
split_type(const char *type, struct nus_type_t *result)
{
	char		*p, *type2, *type3;
	size_t		len1, len2, len3, i;

	memset(result, ' ', sizeof *result);

	p = strchr(type, '.');
	if (p == NULL) return 1;
	len1 = p - type;
	LIMIT(len1, sizeof result->type1);
	memcpy(result->type1, type, len1);

	type2 = p + 1;
	p = strchr(type2, '.');
	if (p == NULL) return 2;
	len2 = p - type2;
	LIMIT(len2, sizeof result->type2);
	memcpy(result->type2, type2, len2);

	type3 = p + 1;
	len3 = strlen(type3);
	LIMIT(len3, sizeof result->type3);
	memcpy(result->type3, type3, len3);

	for (i = 0; i < sizeof(result->type3); i++) {
		if (result->type3[i] == ':')
			result->type3[i] = '\0';
	}

	return 0;
}

	int
str_to_month(const char *mon_buf)
{
	switch (toupper(mon_buf[0])) {
	case 'J':
		if (toupper(mon_buf[1]) == 'A') {
			return 1;
		} else if (toupper(mon_buf[2]) == 'N') {
			return 6;
		} else {
			return 7;
		}
	case 'F':
			return 2;
	case 'M':
		if (toupper(mon_buf[2]) == 'R') {
			return 3;
		} else {
			return 5;
		}
	case 'A':
		if (toupper(mon_buf[1]) == 'P') {
			return 4;
		} else {
			return 8;
		}
	case 'S':	return 9;
	case 'O':	return 10;
	case 'N':	return 11; 
	case 'D':
	default:
			return 12;
	}
}

	int
str_to_minute(const char *text)
{
	extern int nwp_ymdhm2seq(int year, int mon, int day,
		int hour, int min);
	int	year, mon, day, hour, min;
	int	r;
	char	z_buf[2], mon_buf[4];

	if (streq(text, "today")) {
		/* fake */
		year = 2001, mon = 8, day = 30, hour = min = 0;
		goto Convert;
	}

	if ((r = sscanf(text, "min%d", &min)) == 1)
		return min;

	if ((r = sscanf(text, "%4d-%2d-%2d%1[-Tt]%2d:%2d",
		&year, &mon, &day, z_buf, &hour, &min)) == 6) {
		goto Convert;
	}

	if ((r = sscanf(text, "%4d-%2d-%2d%1[-Tt]%2d-%2d",
		&year, &mon, &day, z_buf, &hour, &min)) == 6) {
		goto Convert;
	}

	if ((r = sscanf(text, "%4d-%2d-%2d%1[-Tt]%4d",
		&year, &mon, &day, z_buf, &min)) >= 4) {
		if (r <= 4) min = 0;
		if (min >= 100) {
			hour = min / 100;
			min -= hour * 100;
		} else {
			hour = 0;
		}
		goto Convert;
	}

	if ((r = sscanf(text, "%4d%2d%2d%1[-Tt]%2d:%2d",
		&year, &mon, &day, z_buf, &hour, &min)) >= 3) {
		goto no_separator;
	}

	if ((r = sscanf(text, "%4d%2d%2d%1[-Tt]%2d%2d",
		&year, &mon, &day, z_buf, &hour, &min)) >= 3) {
		goto no_separator;
	}

	if ((r = sscanf(text, "%4d%2d%2d%2d%2d",
		&year, &mon, &day, &hour, &min)) >= 3) {
no_separator:
		if (r <= 3) hour = 0;
		if (r <= 4) min = 0;
		goto Convert;
	}

	if ((r = sscanf(text, "%2d%1[Zz]%2d%3[A-Za-z]%4d",
		&hour, z_buf, &day, mon_buf, &year)) == 5) {
		min = 0;
		mon = str_to_month(mon_buf);
		goto Convert;
	}
	return 0;

Convert:
	return (long)nwp_ymdhm2seq(year, mon, day, hour, min);
}

	const char *
minute_to_str(const int minute, char *buf, size_t len)
{
	extern void nwp_seq2ymdhm(int *year, int *mon, int *day,
		int *hour, int *min, int seq);
	static char	cbuf[32];
	int	year, mon, day, hour, min;

	nwp_seq2ymdhm(&year, &mon, &day, &hour, &min, minute);
	if (minute > 100) {
		sprintf(cbuf, "%04d-%02d-%02dt%02d%02d",
			year, mon, day, hour, min);
	} else {
		sprintf(cbuf, "min%d", minute);
	}
	if (buf == NULL) {
		return cbuf;
	}
	strncpy(buf, cbuf, len);
	return buf;
}

	int
split_dims(const char *dimsspec, const char *opt_base,
	const char *opt_member, const char *opt_valid,
	struct nus_dims_t *result)
{
	int	r;
	if (!dimsspec) {
		r = 0;
	} else {
		char		*p;
		const char	*cursor;
		size_t	len;

		memset(result->member, ' ', sizeof result->member);
		result->base = 0;
		result->valid = 0;

		cursor = dimsspec;
		r = split_type(cursor, &(result->type));
		if ((p = strchr(cursor, ':')) == NULL)
			goto no_more_fields;

		cursor = p + 1;
		result->base = str_to_minute(cursor);
		if ((p = strchr(cursor, ':')) == NULL)
			goto no_more_fields;

		cursor = p + 1;
		if ((p = strchr(cursor, ':')) == NULL) {
			len = strlen(cursor);
			LIMIT(len, sizeof(result->member));
			memcpy(result->member, cursor, len);
			goto no_more_fields;
		} else {
			len = p - cursor;
			LIMIT(len, sizeof(result->member));
			memcpy(result->member, cursor, len);
		}
		cursor = p + 1;

		result->valid = str_to_minute(cursor);

no_more_fields:
		;
	}
	if (opt_base && *opt_base)
		result->base = str_to_minute(opt_base);
	if (opt_valid && *opt_valid)
		result->valid = str_to_minute(opt_valid);
	if (opt_member && *opt_member) {
		size_t	len;
		memset(result->member, ' ', sizeof result->member);
		len = strlen(opt_member);
		LIMIT(len, sizeof(result->member));
		memcpy(result->member, opt_member, len);
	}
	return 0;
}

