#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "nusdas.h"
#include "stringplus.h"

#if 0
static char rcsid[] = "$Id: stringplus.c,v 1.1 2007-02-26 00:10:47 suuchi43 Exp $";
#endif

/* streqi - 大文字小文字を無視した文字列の同定
 */
	int
streqi(const char *a, const char *b)
{
	int	ia, ib;
	for (;;) {
		ia = toupper(*a);
		ib = toupper(*b);
		if ((ia == 0) && (ib == 0))
			return 1;
		if (ia != ib)
			return 0;
		a++;
		b++;
	}
}

/* memeqi - 大文字小文字を無視した文字列の同定
 */
	int
memeqi(const char *a, const char *b, size_t n)
{
	int	ia, ib;
	size_t	i;
	for (i = 0; i < n; i++) {
		ia = toupper(*a);
		ib = toupper(*b);
		if ((ia == 0) && (ib == 0))
			return 1;
		if (ia != ib)
			return 0;
		a++;
		b++;
	}
	return 1;
}

/* strdup3 - 文字列の連結と割り付け
 * 最大3個のC文字列を連結し、malloc で割り付けた領域に格納して返す。
 * ナルポインタは空文字列と同一視される。
 */
	char *
strdup3(const char *a, const char *b, const char *c) {
	char 		*result;
	unsigned	la, lb, lc;
	if (a == NULL) a = "";
	if (b == NULL) b = "";
	if (c == NULL) c = "";
	la = strlen(a);
	lb = strlen(b);
	lc = strlen(c);
	result = malloc(la + lb + lc + 1);
	if (!result) return NULL;
	strncpy(result, a, la);
	strncpy(result + la, b, lb);
	strncpy(result + la + lb, c, lc);
	result[la + lb + lc] = '\0';
	return result;
}

/* indexc - Fortran の INDEX 関数の対応物
 * C 文字列 string の中に文字 c があれば、その最初の出現位置のオフセット
 * を返す。みつからなければ、あるいは string がナルポインタだったら、
 * -1 を返す。
 * Fortran の INDEX と異なり、返却値 0 は「最初の文字が c だ」である。
 */

	int
indexc(const char *string, char c)
{
	char	*p;
	if (string == NULL) return -1;
	p = strchr(string, c);
	if (p == NULL) return -1;
	return p - string;
}

/* string_copy - より安全な strncpy(3)
 * src から size-1 バイトを dest に複写し、必ず NUL 止めする。
 * 本来 C の文字列コピーとして使われるべきインターフェイス。
 */
	char *
string_copy(char *dest, size_t size, const char *src)
{
	size_t	len;
	len = strlen(src);
	if (len > size - 1)
		len = size - 1;
	strncpy(dest, src, len);
	dest[len] = '\0';
	return dest + len;
}

/* string_to_fstring - C 文字列から Fortran 文字列への変換
 * C 文字列 src から長さ size の Fortran 文字列 dest に複写する。
 * Fortran の文字代入文と同様、src が短ければスペースを埋め、
 * src が長ければ右側を切り捨てる。
 */
	void
string_to_fstring(char *dest, size_t size, const char *src)
{
	size_t len;
	len = strlen(src);
	if (len > size)
		len = size;
	strncpy(dest, src, len);
	if (len < size)
		memset(dest + len, ' ', size - len);
}

/* fstring_to_fstring - Fortran 文字列どうしの複写
 * 長さ ssize の Fortran 文字列 src から長さ dsize の領域 dest に複写する。
 * Fortran の文字代入文と同様、src が短ければスペースを埋め、
 * src が長ければ右側を切り捨てる。
 */
	void
fstring_to_fstring(char *dest, const size_t dsize, const char *src, const size_t ssize)
{
	size_t len;
	len = ssize;
	if (len > dsize)
		len = dsize;
	memmove(dest, src, len);
	if (dsize > len)
		memset(dest + len, ' ', dsize - len);
}

/* fstring_to_string - Fortran 文字列から C 文字列への変換
 * 長さ ssize の Fortran 文字列 src から長さ dsize の領域 dest に複写する。
 * src 末尾の空白は切り捨てられ、NUL 止めされて C 文字列となる。
 * src が長ければ右側を切り捨てる。返却値は dest 末端のポインタ。
 */
	char *
fstring_to_string(char *dest, const size_t dsize, const char *src, const size_t ssize)
{
	size_t		len, ofs;
	int		spc;
	len = dsize - 1;
	if (len > ssize)
		len = ssize;
	dest[len] = '\0';
	ofs = len - 1;
	spc = 1;
	while (1) {
		if (!spc) {
			dest[ofs] = src[ofs];
		} else if (src[ofs] == ' ' || src[ofs] == '\0') {
			dest[ofs] = '\0';
			len = ofs;
		} else {
			spc = 0;
			dest[ofs] = src[ofs];
		}
		if (ofs == 0) break;
		ofs--;
	}
	return dest + len;
}

/* replace_space - Fortran 文字列から表示用 C 文字列へのその場変換
 * buf から len 文字の範囲の非図形文字を '_' に変換し、NUL 止めする。
 * buf には len + 1 文字の書き込み可能領域がなければならない。
 */
	int
replace_space(char *buf, unsigned len)
{
	char	*p;
	for (p = buf; p < (buf + len); p++) {
		if (*p == '\0') break;
		if (!isgraph(*p)) *p = '_';
	}
	while (p < buf + len) {
		 *p++ = '_';
	}
	buf[len] = '\0';
	return 0;
}

/* fmatch - シェル風正規表現パッケージ
 * text が pattern にマッチするなら真(非零)を返す
 */

	/* fmatch 用の作業用サブルーチン */
	static int
match_star(const char *pattern, const char *text)
{
	for (;;) {
		if (fmatch(pattern, text))
			return 1;
		if (*text == '\0')
			return 0;
		text++;
	}
}

	int
fmatch(const char *pattern, const char *text)
{
	int	r;
	if (pattern[0] == '\0')
		return *text == '\0';
	if (pattern[0] == '*')
		return match_star(pattern+1, text);
	if (*text == '\0')
		return 0;
	if (*pattern == '?' || *pattern == *text) {
		r = fmatch(pattern+1, text+1);
		return r;
	}
	return 0;
}
