/** @file
 * @brief 時間、とくに数値予報通算分に関する処理
 */
extern int string_to_time(N_SI4 *dest, const char *src);
extern int panstring_to_time(N_SI4 *dest, const char *src, int len);

extern int time_to_chars(char *dest, N_SI4 src);
extern N_SI4 time_add(N_SI4 mseq, N_SI4 offset, sym4_t vtunits);

#ifndef NUSDAS_CONFIG_H
# error requre config.h
#endif

enum nusprof_sectype_t {
	NP_USER,
	NP_API,
	NP_ENCODE,
	NP_WRITE,
	NP_FLUSH,
	NP_SYSWRITE,
	NP_COUNT
};

extern int nusprof_ini(const char *filename);
extern enum nusprof_sectype_t nusprof_state(void);
extern void nusprof_mark(enum nusprof_sectype_t ipart);

#ifdef USE_NUS_PROFILE
# define NUSPROF_INI(filename)	nusprof_ini((filename))
# define NUSPROF_MARK(i)	nusprof_mark((i))
# define NUSPROF_BUF_DECL	int npstat
# define NUSPROF_BACKUP		(npstat = nusprof_state())
# define NUSPROF_RESTORE	nusprof_mark(npstat)
#else
# define NUSPROF_INI(filename)
# define NUSPROF_MARK(i)
# define NUSPROF_BUF_DECL
# define NUSPROF_BACKUP
# define NUSPROF_RESTORE
#endif

#ifdef NEED_DECIMAL_UINT
INLINE int
decimal_uint(const char *str, int length, unsigned *result)
{
	int	c;
	*result = 0;
	while (length) {
		*result *= 10;
		c = *str;
		if (!isdigit(c))
			return -1;
		*result += (c - '0');
		length--, str++;
	}
	return 0;
}
#endif
