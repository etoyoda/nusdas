/** @file
 * @brief 64ビット整数演算を隠蔽するマクロ
 *
 * 64ビット整数型を持たないコンパイラに対応するため64ビット整数演算は
 * 代入以外全てマクロで記述し、ここの定義によって分岐する。
 */

#ifndef NUSDAS_H
# error "include nusdas.h"
#endif
#ifndef NUSDAS_CONFIG_H
# error "include config.h"
#endif

/** 上位32ビット成分を取り出すマクロ @note 定義はバイトオーダー依存 */
#if HAVE_SI8_TYPE
# define I8_HI(i8)	(N_UI4)(((N_UI8)(i8) >> 32) & 0xFFFFFFFF)
#else
# ifdef WORDS_BIGENDIAN
#  define I8_HI(i8)	((i8).loaddr)
# else
#  define I8_HI(i8)	((i8).hiaddr)
# endif
#endif

/** 下位32ビット成分を取り出すマクロ @note 定義はバイトオーダー依存 */
#if HAVE_SI8_TYPE
# define I8_LO(i8)	(N_UI4)((N_UI8)(i8) & 0xFFFFFFFF)
#else
# ifdef WORDS_BIGENDIAN
#  define I8_LO(i8)	((i8).hiaddr)
# else
#  define I8_LO(i8)	((i8).loaddr)
# endif
#endif

#ifdef NEED_MAKE_UI8
/** 32ビット成分2個から N_UI8 値を作る */
#if HAVE_SI8_TYPE
# define make_ui8(hi, lo)	(((N_UI8)(hi) << 32) | (N_UI8)(lo))
#else
INLINE N_UI8 make_ui8(N_UI4 hi, N_UI4 lo)
{
	N_UI8	result;
	I8_HI(result) = hi;
	I8_LO(result) = lo;
	return result;
}
#endif
#endif

#ifdef NEED_MAKE_SI8
/** 32ビット成分2個から N_SI8 値を作る */
#if HAVE_SI8_TYPE
# define make_si8(hi, lo)	(((N_SI8)(hi) << 32) | (N_SI8)(lo))
#else
INLINE N_SI8 make_si8(N_UI4 hi, N_UI4 lo)
{
	N_SI8	result;
	I8_HI(result) = hi;
	I8_LO(result) = lo;
	return result;
}
#endif
#endif

#ifdef NEED_ULONG_TO_UI8
/** 符号無し整数値から N_UI8 への変換 */
#if HAVE_SI8_TYPE
# define ulong_to_ui8(L)		(N_UI8)(L)
#else
INLINE N_UI8 ulong_to_ui8(unsigned long ul) {
	N_UI8	result;
	I8_HI(result) = 0;
	I8_LO(result) = ul;
	return result;
}
#endif
#endif

#ifdef NEED_ULONG_TO_SI8
/** 符号無し整数値から N_SI8 への変換 */
#if HAVE_SI8_TYPE
# define ulong_to_si8(L)		(N_SI8)(L)
#else
INLINE N_SI8 ulong_to_si8(unsigned long ul) {
	N_UI8	result;
	I8_HI(result) = 0;
	I8_LO(result) = ul;
	return result;
}
#endif
#endif

#ifdef NEED_LONG_TO_SI8
/** 符号付き整数値から N_SI8 への変換 */
#if HAVE_SI8_TYPE
# define long_to_si8(L)			(N_SI8)(L)
#else
INLINE N_SI8 long_to_si8(long i) {
	N_UI8	result;
	I8_HI(result) = ((i < 0) ? ~(N_UI4)0 : 0);
	I8_LO(result) = (N_UI4)i;
	return result;
}
#endif
#endif

/** N_SI8 から size_t への変換 */
#if HAVE_SI8_TYPE
# define si8_to_size(si8)		(size_t)(si8)
#else
# define si8_to_size(si8)		(size_t)(I8_LO(si8))
#endif

/** N_SI8 から off_t への変換 */
#if HAVE_SI8_TYPE
# define si8_to_off(si8)		(off_t)(si8)
#else
# define si8_to_off(si8)		(off_t)(I8_LO(si8))
#endif

/** N_SI8 から long への変換 */
#if HAVE_SI8_TYPE
# define si8_to_long(si8)		(long)(si8)
#else
# define si8_to_long(si8)		(long)(I8_LO(si8))
#endif

/** N_SI8 から N_UI4 への高速変換 */
#if HAVE_SI8_TYPE
# define si8_to_ui4(si8)		(N_UI4)(si8)
#else
# define si8_to_ui4(si8)		I8_LO(si8)
#endif

/** off_t から N_SI8 への変換 */
#if HAVE_SI8_TYPE
# define off_to_si8(off)		(N_SI8)(off)
#else
# define off_to_si8(off)		long_to_si8(off)
#endif

#ifdef NEED_SI8_ADD
/** N_SI8 どうしの加算 */
#if HAVE_SI8_TYPE
# define si8_add(a, b)		((a) + (b))
#else
INLINE N_SI8 si8_add(N_SI8 a, N_SI8 b)
{
	N_SI8	result;
	I8_LO(result) = I8_LO(a) + I8_LO(b);
	I8_HI(result) = (I8_LO(result) < I8_LO(b)) + I8_HI(a) + I8_HI(b);
	return result;
}
#endif
#endif

/** N_SI8 どうしの += 演算子 */
#if HAVE_SI8_TYPE
# define si8_addto(a, b)		((a) += (b))
#else
# define si8_addto(a, b)		((a) = si8_add((a), (b)))
#endif

#ifdef NEED_SI8_ADDTO_UI4
/** N_SI8 と N_UI4 の加算 */
#if HAVE_SI8_TYPE
# define si8_addto_ui4(a, b)		((a) += (b))
#else
INLINE N_SI8 si8_addtoptr_ui4(N_SI8 *a, N_UI4 b)
{
	N_SI8	result;
	I8_LO(*a) += b;
	if (I8_LO(*a) < b)
		I8_HI(*a)++;
	return result;
}
# define si8_addto_ui4(a, b)		si8_addtoptr_ui4(&(a), (b))
#endif
#endif

#ifdef NEED_SI8_SUB
/** N_SI8 どうしの減算 */
#if HAVE_SI8_TYPE
# define si8_sub(a, b)		((a) - (b))
#else
INLINE N_SI8 si8_sub(N_SI8 a, N_SI8 b)
{
	N_SI8	result;
	result = a;
	if (I8_LO(result) < I8_LO(b))
		I8_HI(result)--;
	I8_LO(result) -= I8_LO(b);
	I8_HI(result) -= I8_HI(b);
	return result;
}
#endif
#endif

#ifdef NEED_SI8_SUBFROM
/** N_SI8 どうしの減算 */
#if HAVE_SI8_TYPE
# define si8_subfrom(a, b)		((a) -= (b))
#else
INLINE N_SI8 si8_subfromptr(N_SI8 *a, N_SI8 b)
{
	if (I8_LO(*a) < I8_LO(b))
		I8_HI(*a)--;
	I8_LO(*a) -= I8_LO(b);
	I8_HI(*a) -= I8_HI(b);
	return *a;
}
# define si8_subfrom(a, b)		si8_subfromptr(&(a), (b))
#endif
#endif

#ifdef NEED_UI8_MOD_UI2
/** N_UI8 を N_UI2 で割った剰余 */
#if HAVE_SI8_TYPE
#define ui8_mod_ui2(a, b)	((N_UI8)(a) % (N_UI2)(b))
#else
INLINE N_UI2 ui8_mod_ui2(N_UI8 a, N_UI2 b)
{
	N_UI4	m16, m;
	m16 = 0x10000 % b;
	m = I8_HI(a) % b;
	m *= m16;
	m %= b;
	m += (I8_LO(a) >> 16) % b;
	m %= b;
	m *= m16;
	m %= b;
	m += (I8_LO(a) & 0xFFFF) % b;
	m %= b;
	return m;
}
#endif
#endif

#ifdef NEED_UI8_DIV_UI2
/** N_UI8 を N_UI2 で割った商 */
#if HAVE_SI8_TYPE
#define ui8_div_ui2(a, b)	((N_UI8)(a) / (N_UI2)(b))
#else
INLINE N_UI8 ui8_div_ui2(N_UI8 a, N_UI2 b)
{
	N_UI4	res, div[4];
	res = (I8_HI(a) >> 16);
	div[0] = res / b;
	res -= div[0] * b;
	res <<= 16;
	res += (I8_HI(a) & 0xFFFF);
	div[1] = res / b;
	res -= div[1] * b;
	res <<= 16;
	res += (I8_LO(a) >> 16);
	div[2] = res / b;
	res -= div[2] * b;
	res <<= 16;
	res += (I8_LO(a) & 0xFFFF);
	div[3] = res / b;
	return make_ui8(((div[0] << 16) | div[1]),
			((div[2] << 16) | div[3]));
}
#endif
#endif

/** N_SI8 のゼロの判定 */
#if HAVE_SI8_TYPE
# define si8_iszero(a)		((a) == 0)
#else
# define si8_iszero(a)		((I8_HI(a) == 0) && (I8_LO(a) == 0))
#endif

/** N_UI8 のゼロの判定 */
#if HAVE_SI8_TYPE
# define ui8_iszero(a)		((a) == 0)
#else
# define ui8_iszero(a)		((I8_HI(a) == 0) && (I8_LO(a) == 0))
#endif

/** N_SI8 の負値の判定 */
#if HAVE_SI8_TYPE
# define si8_isnegative(a)		((a) < 0)
#else
# define si8_isnegative(a)		(!!(I8_HI(a) & (1ul << 31)))
#endif

#ifdef NEED_UI8_LSHIFT
/** N_UI8 の左シフト */
#if HAVE_SI8_TYPE
#define ui8_lshift(a, b)	((a) << (b))
#else
INLINE N_UI8 ui8_lshift(N_UI8 a, int b)
{
	N_UI8	result;
	if (b == 0) {
		return a;
	}
	I8_LO(result) = (I8_LO(a) << b);
	I8_HI(result) = ((I8_LO(a) >> (32 - b)) | (I8_HI(a) << b));
	return result;
}
#endif
#endif

#ifdef NEED_UI8_RSHIFT
/** N_UI8 の左シフト */
#if HAVE_SI8_TYPE
#define ui8_rshift(a, b)	((a) >> (b))
#else
INLINE N_UI8 ui8_rshift(N_UI8 a, int b)
{
	N_UI8	result;
	if (b == 0) {
		return a;
	}
	I8_LO(result) = ((I8_LO(a) >> b) | (I8_HI(a) << (32 - b)));
	I8_HI(result) = (I8_HI(a) >> b);
	return result;
}
#endif
#endif

#ifdef NEED_UI8_PLUSPLUS
/** N_UI8/N_SI8 のインクリメント */
#if HAVE_SI8_TYPE
#define ui8_plusplus(a)	((a)++)
#else
INLINE N_UI8 ui8ptr_plusplus(N_UI8 *ptr)
{
	N_UI8	result;
	result = *ptr;
	I8_LO(*ptr)++;
	if (I8_LO(*ptr) == 0)
		I8_HI(*ptr)++;
	return result;
}
#define ui8_plusplus(a)	ui8ptr_plusplus(&(a))
#endif
#endif

#ifdef NEED_SI8_UNARYMINUS
/** N_SI8 の符号反転 */
#if HAVE_SI8_TYPE
#define si8_unaryminus(a)	(-(N_SI8)(a))
#else
INLINE N_UI8 si8_unaryminus(N_UI8 a)
{
	N_UI8	result;
	I8_HI(result) = ~I8_HI(a);
	I8_LO(result) = ~I8_LO(a);
	I8_LO(result)++;
	if (I8_LO(result) == 0)
		I8_HI(result)++;
	return result;
}
#endif
#endif

#define SCALAR_CMP(a, b)	(((a) > (b)) ? 1 : (((a) < (b)) ? -1 : 0))

#ifdef NEED_SI8_CMP
/** N_SI8 の三値比較演算子 (比較演算子を作るために使う) */
#if HAVE_SI8_TYPE
# define si8_cmp(a, b)		(((a) > (b)) ? 1 : (((a) < (b)) ? -1 : 0))
#else
INLINE int si8_cmp(N_SI8 a, N_SI8 b)
{
	int	a_nega, b_nega, c;
	a_nega = si8_isnegative(a);
	b_nega = si8_isnegative(b);
	if (a_nega) {
		if (b_nega) {
			c = SCALAR_CMP(I8_HI(a), I8_HI(b));
			if (c) return -c;
			return -SCALAR_CMP(I8_LO(a), I8_LO(b));
		} else {
			return -1;
		}
	} else {
		if (b_nega) {
			return 1;
		} else {
			c = SCALAR_CMP(I8_HI(a), I8_HI(b));
			if (c) return c;
			return SCALAR_CMP(I8_LO(a), I8_LO(b));
		}
	}
	return 0;
}
#endif

/** N_SI8 の < 演算子 */
#if HAVE_SI8_TYPE
# define si8_lessthan(a, b)		((a) < (b))
#else
# define si8_lessthan(a, b)		(si8_cmp((a), (b)) < 0)
#endif

/** N_SI8 の <= 演算子 */
#if HAVE_SI8_TYPE
# define si8_lesseq(a, b)		((a) <= (b))
#else
# define si8_lesseq(a, b)		(si8_cmp((a), (b)) <= 0)
#endif

/** N_SI8 の > 演算子 */
#if HAVE_SI8_TYPE
# define si8_morethan(a, b)		((a) > (b))
#else
# define si8_morethan(a, b)		(si8_cmp((a), (b)) > 0)
#endif

/** N_SI8 の >= 演算子 */
#if HAVE_SI8_TYPE
# define si8_moreeq(a, b)		((a) >= (b))
#else
# define si8_moreeq(a, b)		(si8_cmp((a), (b)) >= 0)
#endif

/** N_SI8 の同値判定 */
#if HAVE_SI8_TYPE
# define si8_eq(a, b)		((a) == (b))
#else
# define si8_eq(a, b)	((I8_HI(a) == I8_HI(b)) && (I8_LO(a) == I8_LO(b)))
#endif

/** N_UI8 の同値判定 */
#if HAVE_SI8_TYPE
# define ui8_eq(a, b)		((a) == (b))
#else
# define ui8_eq(a, b)	((I8_HI(a) == I8_HI(b)) && (I8_LO(a) == I8_LO(b)))
#endif
#endif /* NEED_SI8_CMP */

/** N_UI8/N_SI8 のゼロ判定 */
#if HAVE_SI8_TYPE
# define i8_zerop(a)	((a) == 0)
#else
# define i8_zerop(a)	((I8_HI(a) == 0) && (I8_LO(a) == 0))
#endif

#if HAVE_SI8_TYPE
# define ui8_maxp(a)	(!~(a))
#else
# define ui8_maxp(a)	(!~I8_HI(a) && !~I8_LO(a))
#endif

#ifdef NEED_UI8_CMP
/** N_UI8 の三値比較演算子 (比較演算子を作るために使う) 
 * @note 符号付きの int が返る */
#if HAVE_SI8_TYPE
# define ui8_cmp(a, b)		(((a) > (b)) ? 1 : (((a) < (b)) ? -1 : 0))
#else
INLINE int ui8_cmp(N_UI8 a, N_UI8 b)
{
	int	chi;
	chi = SCALAR_CMP(I8_HI(a), I8_HI(b));
	if (chi > 0) {
		return 1;
	} else if (chi < 0) {
		return -1;
	}
	return SCALAR_CMP(I8_LO(a), I8_LO(b));
}
#endif
#endif

/** N_UI8 の < 演算子 */
#if HAVE_SI8_TYPE
# define ui8_lessthan(a, b)		((a) < (b))
#else
# define ui8_lessthan(a, b)		(ui8_cmp((a), (b)) < 0)
#endif

#ifdef NEED_SYM8OP
#if HAVE_SI8_TYPE
# define sym8_is_allspace(sym) (sym == SYM8_ALLSPACE)
#else
INLINE
int
sym8_is_allspace(sym8_t sym)
{
  char *ptr = (char *)&sym;
  return (
    (ptr[0] == ' ') && (ptr[1] == ' ') && (ptr[2] == ' ') &&
    (ptr[3] == ' ') && (ptr[4] == ' ') && (ptr[5] == ' ') &&
    (ptr[6] == ' ') && (ptr[7] == ' ')
  );
}
#endif
#endif
