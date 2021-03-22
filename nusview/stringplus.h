/*
 * nwpl_string.h - 数値予報標準ライブラリ第2版計画 文字列処理
 * $Id: stringplus.h,v 1.1 2007-02-23 05:33:15 suuchi43 Exp $
 */

#ifndef NWPL_STRING_H
# define NWPL_STRING_H

/* str.c */
# define streq(a, b) (strcmp((a), (b)) == 0)
extern int      streqi(const char *a, const char *b);
extern int	memeqi(const char *a, const char *b, size_t n);
extern char     *strdup3(const char *a, const char *b, const char *c);
extern int      indexc(const char *string, char c);
extern char     *string_copy(char *dest, size_t size, const char *src);
extern void     string_to_fstring(char *dest, size_t size, const char *src);
extern void     fstring_to_fstring(char *dest, const size_t dsize,
	const char *src, const size_t ssize);
extern char     *fstring_to_string(char *dest, const size_t dsize,
	const char *src, const size_t ssize);
extern int      fmatch(const char *pattern, const char *text);
extern int      replace_space(char *buf, unsigned len);

/* regmatch.c */

extern long	regmatch(const char *pattern, const char *text, long *length);
extern char	*regreplace(const char *pattern, const char *replace,
		const char *text) ;
/* glue.c */
int str_to_minute(const char *text);
const char *minute_to_str(const int minute, char *buf, size_t len);

#endif /* ifndef NWPL_STRING_H */
