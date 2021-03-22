/** @file
 * @brief バイトオーダーに依存する処理
 */
#ifndef NUSDAS_CONFIG_H
# error "include config.h"
#endif

/** ちょうど 32 ビットの整数。
 * ネットワークバイトオーダーで入っているかもしれないときに
 * この型を用いる。
 * いいかえると四則演算禁止。
 */
typedef N_UI4	net4_t;

/** ちょうど 64 ビットの整数。
 * ネットワークバイトオーダーで入っているかもしれないときに
 * この型を用いる。
 * いいかえると四則演算禁止。
 */
typedef N_UI8	net8_t;

#ifdef WORDS_BIGENDIAN
# define HTON8(i64) (i64)
#else
# if HAVE_SI8_TYPE
#  define HTON8(i64) \
	( (((i64) & 0xFF00000000000000uLL) >> 56) \
	| (((i64) &   0xFF000000000000uLL) >> 40) \
	| (((i64) &     0xFF0000000000uLL) >> 24) \
	| (((i64) &       0xFF00000000uLL) >>  8) \
	| (((i64) &         0xFF000000uLL) <<  8) \
	| (((i64) &           0xFF0000uLL) << 24) \
	| (((i64) &             0xFF00uLL) << 40) \
	| (((i64) &               0xFFuLL) << 56) )
# else
#  define HTON8(i64) make_ui8(HTON4(I8_LO(i64)), HTON4(I8_HI(i64)))
# endif
#endif

#define NTOH8(i64)	HTON8(i64)

/** net4_t または N_UI4 のネットワークバイトオーダー整数値を
 * マシンオーダーにする。
 * @warning ビットマスクが long なので符号付き整数に使う場合注意
 */
#ifdef WORDS_BIGENDIAN
# define NTOH4(i32) (i32)
#else
# define NTOH4(i32) \
	( (((i32) & 0xFF000000uL) >> 24) \
	| (((i32) &   0xFF0000uL) >> 8) \
	| (((i32) &     0xFF00uL) << 8) \
	| (((i32) &       0xFFuL) << 24))
#endif

/** N_UI4 のマシンバイトオーダー整数値を
 * ネットワークバイトオーダーにする。
 * @warning ビットマスクが long なので符号付き整数に使う場合注意
 */
#ifdef WORDS_BIGENDIAN
# define HTON4(i32) (i32)
#else
# define HTON4(i32) \
	( (((i32) & 0xFF000000uL) >> 24) \
	| (((i32) &   0xFF0000uL) >> 8) \
	| (((i32) &     0xFF00uL) << 8) \
	| (((i32) &       0xFFuL) << 24))
#endif

/** N_UI2 のマシンバイトオーダー整数値を
 * ネットワークバイトオーダーにする。
 * @warning 式の型は int なので符号付き整数に使うと(大抵)符号が脱落する
 */
#ifdef WORDS_BIGENDIAN
# define HTON2(i16) (i16)
#else
# define HTON2(i16) \
	( (((i16) & 0xFF00) >> 8) \
	| (((i16) &   0xFF) << 8))
#endif

/** N_UI2 のネットワークバイトオーダー整数値を
 * マシンワークバイトオーダーにする。
 * @warning 式の型は int なので符号付き整数に使うと(大抵)符号が脱落する
 */
#ifdef WORDS_BIGENDIAN
# define NTOH2(i16) (i16)
#else
# define NTOH2(i16) \
	( (((i16) & 0xFF00) >> 8) \
	| (((i16) &   0xFF) << 8))
#endif

/** N_UI2 の数値をネットワークバイトオーダーでメモリに書き出す */
#define POKE_N_UI1(ptr, val) (*(N_UI1 *)(ptr) = (N_UI1)(val))
#if NEED_ALIGN & 2
	INLINE void
poke2(unsigned char *ptr, N_UI2 val)
{
	ptr[0] = ((val >> 8) & 0xFF);
	ptr[1] = (val & 0xFF);
}
#define POKE_N_UI2(ptr, val) poke2((unsigned char *)(ptr), val)
#define POKE_N_SI2(ptr, val) poke2((unsigned char *)(ptr), val)
#else
#define POKE_N_UI2(ptr, val) (*(N_UI2 *)(ptr) = HTON2((N_UI2)(val)))
#define POKE_N_SI2(ptr, val) (*(N_UI2 *)(ptr) = HTON2((N_UI2)(val)))
#endif

#if NEED_ALIGN & 4
	INLINE void
poke4(unsigned char *ptr, N_UI4 val)
{
	ptr[0] = ((val >> 24) & 0xFF);
	ptr[1] = ((val >> 16) & 0xFF);
	ptr[2] = ((val >> 8) & 0xFF);
	ptr[3] = (val & 0xFF);
}
#endif

#if NEED_ALIGN & 4
# define POKE_N_UI4(ptr, val) poke4((unsigned char *)(ptr), val)
#else
# define POKE_N_UI4(ptr, val) (*(N_UI4 *)(ptr) = HTON4((N_UI4)(val)))
#endif

#if NEED_ALIGN & 4
# define POKE_N_SI4(ptr, val) poke4((unsigned char *)(ptr), val)
#else
# define POKE_N_SI4(ptr, val) (*(N_UI4 *)(ptr) = HTON4((N_UI4)(val)))
#endif

#ifdef NEED_POKE_FLOAT
	INLINE void
poke_float(unsigned char *dest, float val)
{
	unsigned char *src = (unsigned char *)&val;
# ifdef WORDS_BIGENDIAN
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
# else
	dest[0] = src[3];
	dest[1] = src[2];
	dest[2] = src[1];
	dest[3] = src[0];
# endif
}
# define POKE_float(ptr, val) poke_float((unsigned char *)(ptr), val)
#endif

#ifdef NEED_POKE_DOUBLE
	INLINE void
poke_double(unsigned char *dest, double val)
{
	unsigned char *src = (unsigned char *)&val;
# ifdef WORDS_BIGENDIAN
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
	dest[4] = src[4];
	dest[5] = src[5];
	dest[6] = src[6];
	dest[7] = src[7];
# else
	dest[0] = src[7];
	dest[1] = src[6];
	dest[2] = src[5];
	dest[3] = src[4];
	dest[4] = src[3];
	dest[5] = src[2];
	dest[6] = src[1];
	dest[7] = src[0];
# endif
}
# define POKE_double(ptr, val) poke_double((unsigned char *)(ptr), val)
#endif

#if defined(NEED_PEEK_N_SI2)
# if NEED_ALIGN & 2
	INLINE N_SI2
peek2s(const unsigned char *ptr)
{
	int r;
	r = ((ptr[0] & 0x7F) << 8) | ptr[1];
	if (ptr[0] & 0x80) r -= 0x8000;
	return r;
}
#  define PEEK_N_SI2(ptr) peek2s((const unsigned char *)(ptr))
# else
#  define PEEK_N_SI2(ptr) (N_SI2)NTOH2(*(N_UI2 *)(ptr))
# endif
#endif

#if defined(NEED_PEEK_N_UI2)
# if NEED_ALIGN & 2
	INLINE N_UI2
peek2(const unsigned char *ptr)
{
	return (N_UI2)ptr[0] << 8 | (N_UI2)ptr[1];
}
#  define PEEK_N_UI2(ptr) peek2((const unsigned char *)(ptr))
# else
#  define PEEK_N_UI2(ptr) NTOH2(*(N_UI2 *)(ptr))
# endif
#endif

#if defined(NEED_PEEK_N_SI4)
# if NEED_ALIGN & 4
	INLINE N_SI4
peek4s(const unsigned char *ptr)
{
	return (N_SI4)ptr[0] << 24
		| (N_SI4)ptr[1] << 16
		| (N_SI4)ptr[2] << 8
		| (N_SI4)ptr[3];
}
#  define PEEK_N_SI4(ptr) peek4((const unsigned char *)(ptr))
# else
#  define PEEK_N_SI4(ptr) NTOH4(*(N_SI4 *)(ptr))
# endif
#endif

#if defined(NEED_PEEK_N_UI4) || defined(NEED_MEMCPY_NTOH4) || defined(NEED_MEMCPY_HTON4)
# if NEED_ALIGN & 4
	INLINE N_UI4
peek4(const unsigned char *ptr)
{
	return (N_UI4)ptr[0] << 24
		| (N_UI4)ptr[1] << 16
		| (N_UI4)ptr[2] << 8
		| (N_UI4)ptr[3];
}
# endif

# if NEED_ALIGN & 4
#  define PEEK_N_UI4(ptr) peek4((const unsigned char *)(ptr))
# else
#  define PEEK_N_UI4(ptr) NTOH4(*(N_UI4 *)(ptr))
# endif
#endif

#ifdef NEED_PEEK_SYM4_T
# if NEED_ALIGN & 4
	INLINE sym4_t
peek_sym4_t(const unsigned char *csrc)
{
	sym4_t r;
	unsigned char *cdest = (unsigned char *)&r;
	cdest[0] = csrc[0];
	cdest[1] = csrc[1];
	cdest[2] = csrc[2];
	cdest[3] = csrc[3];
	return r;
}
# define PEEK_sym4_t(ptr) peek_sym4_t((const unsigned char *)(ptr))
# else
# define PEEK_sym4_t(ptr) (*(N_UI4 *)(ptr))
# endif
#endif

#ifdef NEED_PEEK_FLOAT
# if NEED_ALIGN & 4
	INLINE void
PEEK_float(float *fp, const unsigned char *csrc)
{
	unsigned char *cdest = (unsigned char *)fp;
#  ifdef WORDS_BIGENDIAN
	cdest[0] = csrc[0];
	cdest[1] = csrc[1];
	cdest[2] = csrc[2];
	cdest[3] = csrc[3];
#  else
	cdest[0] = csrc[3];
	cdest[1] = csrc[2];
	cdest[2] = csrc[1];
	cdest[3] = csrc[0];
#  endif
}
# else
#  define PEEK_float(fp, csrc) *(N_UI4 *)(fp) = NTOH4(*(N_UI4 *)(csrc))
# endif
#endif

#ifdef NEED_PEEK_DOUBLE
# if NEED_ALIGN & 8
	INLINE void
PEEK_double(double *fp, const unsigned char *csrc)
{
	unsigned char *cdest = (unsigned char *)fp;
#  ifdef WORDS_BIGENDIAN
	cdest[0] = csrc[0];
	cdest[1] = csrc[1];
	cdest[2] = csrc[2];
	cdest[3] = csrc[3];
	cdest[4] = csrc[4];
	cdest[5] = csrc[5];
	cdest[6] = csrc[6];
	cdest[7] = csrc[7];
#  else
	cdest[0] = csrc[7];
	cdest[1] = csrc[6];
	cdest[2] = csrc[5];
	cdest[3] = csrc[4];
	cdest[4] = csrc[3];
	cdest[5] = csrc[2];
	cdest[6] = csrc[1];
	cdest[7] = csrc[0];
#  endif
}
# else
#  define PEEK_double(fp, csrc) *(N_UI8 *)(fp) = NTOH8(*(N_UI8 *)(csrc))
# endif
#endif

#if defined(NEED_MEMCPY_NTOH4) || defined(NEED_MEMCPY_HTON4)
/** net4_t の配列から N_UI4 の配列へコピーする。
 */
	INLINE void
memcpy_ntoh4(void *dest, const void *src, size_t n)
{
	N_UI4 *idest = dest;
	const N_UI4 *isrc = src;
	unsigned i;
	for (i = 0; i < n; i++) {
# if NEED_ALIGN & 4
                idest[i] = PEEK_N_UI4(isrc + i);
# else
		idest[i] = NTOH4(isrc[i]);
# endif
	}
}
#endif

/** N_UI4 の配列から net4_t の配列にコピーする. */
#ifdef NEED_MEMCPY_HTON4
#define memcpy_hton4(dest, src, n)	memcpy_ntoh4((dest), (src), (n))
#endif
