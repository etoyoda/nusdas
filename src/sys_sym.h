/** \file
 * \brief 文字列を sym4_t または sym8_t で扱う操作
 * \todo inline 関数指定子をもたない処理系への対応
 */

#ifndef INTERNAL_TYPES_H
#  error please include "internal_types.h"
#endif
#ifndef NUSDAS_CONFIG_H
# error "include config.h"
#endif

/** 8文字パック整数のうち左の4文字を4文字パックとして取り出す
 * @note 定義はバイトオーダー依存
 */
#if HAVE_SI8_TYPE
# ifdef WORDS_BIGENDIAN
#  define SYM8_LEFT4(s8)	((sym4_t)((s8) >> 32))
# else
#  define SYM8_LEFT4(s8)	((sym4_t)((s8) & 0xFFFFFFFFuL))
# endif
#else
# define SYM8_LEFT4(s8)		((s8).loaddr)
#endif

/** 8文字パック整数のうち右の4文字を4文字パックとして取り出す
 * @note 定義はバイトオーダー依存
 */
#if HAVE_SI8_TYPE
# ifdef WORDS_BIGENDIAN
#  define SYM8_RIGHT4(s8)	((sym4_t)((s8) & 0xFFFFFFFFuL))
# else
#  define SYM8_RIGHT4(s8)	((sym4_t)((s8) >> 32))
# endif
#else
# define SYM8_RIGHT4(s8)	((s8).hiaddr)
#endif

/** 8文字パック整数のうち左から5文字目と6文字目を4文字パックとして
 * 取り出す。
 * @note 定義はバイトオーダー依存
 */
#ifdef WORDS_BIGENDIAN
# define SYM8_MID56(s8)	\
	((SYM8_RIGHT4((s8)) & 0xFFFF0000uL) | 0x00002020uL)
#else
# define SYM8_MID56(s8)	\
	((SYM8_RIGHT4((s8)) & 0xFFFF) | 0x20200000uL)
#endif


#if defined(WORDS_BIGENDIAN) && (NEED_ALIGN & (4 | 2))
/** 4バイト固定長バイト列から4文字パックを作成する.
 * \todo リトルエンディアンのアラインメント対応（もしそんな環境があれば）
 */
# define MEM2SYM4(str) (str != 0 ? ( \
     (((unsigned char*)(str))[0] << 24) \
    |(((unsigned char*)(str))[1] << 16) \
    |(((unsigned char*)(str))[2] << 8) \
    |(((unsigned char*)(str))[3]) \
    ) : 0)
#elif (NEED_ALIGN & (4 | 2))
# define MEM2SYM4(str) (str != 0 ? ( \
     (((unsigned char*)(str))[0]) \
    |(((unsigned char*)(str))[1] << 8) \
    |(((unsigned char*)(str))[2] << 16) \
    |(((unsigned char*)(str))[3] << 24) \
    ) : 0)
#else
# define MEM2SYM4(str) (str != 0 ? *(sym4_t *)(str) : 0)
#endif

#if defined(NEED_PACK2NUSDIMS)
# define NEED_MEM6SYM8
#endif

#if defined(NEED_MEM6SYM8) || defined(NEED_STR2SYM8) || defined(NEED_PACK2NUSTYPE)
# define NEED_MEM2SYM8
#endif

#ifdef NEED_MEM2SYM8
#if defined(WORDS_BIGENDIAN) && (NEED_ALIGN & (8 | 4 | 2)) && HAVE_SI8_TYPE
/** 8バイト固定長バイト列から8文字パックを作成する.
 * \todo long long のない環境、またはリトルエンディアンの
 *       アラインメント対応（もしそんな環境があれば）
 */
# define MEM2SYM8(str) (str != 0 ? ( \
    (((sym8_t)((unsigned char*)(str))[0]) << 56) \
   |(((sym8_t)((unsigned char*)(str))[1]) << 48) \
   |(((sym8_t)((unsigned char*)(str))[2]) << 40) \
   |(((sym8_t)((unsigned char*)(str))[3]) << 32) \
   |(((sym8_t)((unsigned char*)(str))[4]) << 24) \
   |(((sym8_t)((unsigned char*)(str))[5]) << 16) \
   |(((sym8_t)((unsigned char*)(str))[6]) << 8) \
   |(((sym8_t)((unsigned char*)(str))[7])) \
   ) : 0)
#elif (NEED_ALIGN & (8 | 4 | 2)) && HAVE_SI8_TYPE
# define MEM2SYM8(str) (str != 0 ? ( \
    (((sym8_t)((unsigned char*)(str))[0])) \
   |(((sym8_t)((unsigned char*)(str))[1]) << 8) \
   |(((sym8_t)((unsigned char*)(str))[2]) << 16) \
   |(((sym8_t)((unsigned char*)(str))[3]) << 24) \
   |(((sym8_t)((unsigned char*)(str))[4]) << 32) \
   |(((sym8_t)((unsigned char*)(str))[5]) << 40) \
   |(((sym8_t)((unsigned char*)(str))[6]) << 48) \
   |(((sym8_t)((unsigned char*)(str))[7]) << 56) \
   ) : 0)
#elif (NEED_ALIGN & (8 | 4 | 2))
INLINE
sym8_t
mem_to_sym8(const char *mem)
{
  sym8_t sym;
  if (mem == 0) {
    sym.hiaddr = sym.loaddr = 0;
  } else {
    sym.loaddr = MEM2SYM4(mem);
    sym.hiaddr = MEM2SYM4(mem + 4);
  }
  return sym;
}
# define MEM2SYM8(str) mem_to_sym8((str))
#else
# define MEM2SYM8(str) (str != 0 ? *(sym8_t *)(str) : 0)
#endif
#endif

#if defined(NEED_MEM6SYM8) || defined(NEED_PACK2NUSDIMS)
/** 固定長6バイト列から８文字パックを作成する。
 * 余った２文字は空白が埋められる。
 * 6バイト中に誤ってヌル終端が入っている場合は以降が空白とみなされる。
 */
INLINE
sym8_t
mem6sym8(const char *mem)
{
	sym8_t	val;
	char buf[8];
	int i;
	for (i = 0; i < 6; i++) {
		if (mem[i] == 0)
			break;
		buf[i] = mem[i];
	}
	while (i < 8) {
		buf[i++] = ' ';
	}
	val = MEM2SYM8(buf);
	return val;
}
#endif

#ifdef NEED_MEM2SYM4
/** 固定長2文字の文字列から4文字パックを作成する。
 * 余った２文字は空白が埋められる。
 * 2バイト中に誤ってヌル終端が入っている場合は以降が空白とみなされる。
 */
INLINE
sym4_t
mem2sym4(const char *mem)
{
	char buf[4];
	int i;
	for (i = 0; i < 2; i++) {
		if (mem[i] == 0)
			break;
		buf[i] = mem[i];
	}
	while (i < 4) {
		buf[i++] = ' ';
	}
	return MEM2SYM4(buf);
}
#endif

#ifdef NEED_STR2SYM8
/** ヌル終端文字列から８文字パックを作成する。
 */
INLINE
sym8_t
str2sym8(const char *str)
{
	char buf[8];
	const char *p = str;
	int i;
	for (i = 0; i < 8; i++) {
		if (!*p)
			break;
		buf[i] = *p++;
	}
	while (i < 8) {
		buf[i++] = ' ';
	}
	return MEM2SYM8(buf);
}
#endif

#if defined(NEED_STR2SYM4) || defined(NEED_PACK2NUSBMV) \
	|| defined(NEED_PACK2NUSDIMS)
/** ヌル終端文字列から4文字パックを作成する。
 */
INLINE
sym4_t
str2sym4(const char *str)
{
	char buf[4];
	const char *p = str;
	int i;
	for (i = 0; i < 4; i++) {
		if (!*p)
			break;
		buf[i] = *p++;
	}
	while (i < 4) {
		buf[i++] = ' ';
	}
	return MEM2SYM4(buf);
}
#endif

#ifdef NEED_STR3SYM4UPCASE
/** 固定長3文字列から4文字パックを作成する。
 * このとき英小文字は英大文字へ変換される。
 * @note ctype.h をインクルードすること。なくても動くが性能劣るおそれあり。
 */
INLINE
sym4_t
str3sym4upcase(const char *str)
{
	char buf[4];
	buf[0] = toupper(str[0]);
	buf[1] = toupper(str[1]);
	buf[2] = toupper(str[2]);
	buf[3] = ' ';
	return MEM2SYM4(buf);
}
#endif

#ifdef NEED_PACK2NUSTYPE
INLINE void
pack2nustype(const char *type1, const char *type2, const char *type3,
		nustype_t *buf)
{
	buf->type1 = MEM2SYM8(type1);
	buf->type2 = MEM2SYM4(type2);
	buf->type3 = MEM2SYM4(type3);
}
#endif

#ifdef NEED_PACK2NUSDIMS
INLINE void
pack2nusdims(N_SI4 basetime, const char *member, N_SI4 validtime1,
	N_SI4 validtime2, const char *plane1, const char *plane2,
	const char *element, nusdims_t *pdims)
{
	pdims->basetime = basetime;
	pdims->member = str2sym4(member);
	pdims->validtime1 = validtime1;
	pdims->validtime2 = validtime2;
	pdims->plane1 = mem6sym8(plane1);
	pdims->plane2 = mem6sym8(plane2 ? plane2 : plane1);
	pdims->element = mem6sym8(element);
}
#endif

#ifdef NEED_PACK2NUSBMV
INLINE void
pack2nusbmv(N_SI4 basetime, const char *member, N_SI4 validtime1,
	N_SI4 validtime2, nusdims_t *pdims)
{
	pdims->basetime = basetime;
	pdims->member = str2sym4(member);
	pdims->validtime1 = validtime1;
	pdims->validtime2 = validtime2;
#ifdef USE_NUS_DEBUG
#if HAVE_SI8_TYPE
	pdims->plane1 = 0;
	pdims->plane2 = 0;
	pdims->element = 0;
#endif
#endif
}
#endif
