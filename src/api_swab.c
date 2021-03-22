#include "config.h"
#include "nusdas.h"
#include <stdlib.h>
# define NEED_MAKE_UI8
#include "sys_int.h"
#include "sys_endian.h"

#ifdef WORDS_BIGENDIAN
# define UNUSED_IF_BIGENDIAN UNUSED
#else
# define UNUSED_IF_BIGENDIAN
#endif

/** @brief 2バイト整数のバイトオーダー変換
 *
 * リトルエンディアン機では、2バイト整数の配列 @p ary の
 * バイトオーダーを逆順にする。
 * ビッグエンディアンのデータを読んだ後整数として解釈する前、
 * または整数として値を格納した後ビッグエンディアンで書き出す前に呼ぶ。
 *
 * ビッグエンディアン機ではなにもしない。
 * */
	void
NuSDaS_swab2(void *ary UNUSED_IF_BIGENDIAN, /**< 配列 */
		const N_UI4 count UNUSED_IF_BIGENDIAN) /**< 配列の要素数 */
{
#ifndef WORDS_BIGENDIAN
	N_UI1 (*array)[4] = ary;
	N_UI4 tmp;
	N_UI4 i;
	/* NUSDAS_INIT; するまでもあるまい */
	for (i = 0; i < ((N_UI4)count / 2); i++) {
		tmp = array[i][0];
		array[i][0] = array[i][1];
		array[i][1] = tmp;
		tmp = array[i][2];
		array[i][2] = array[i][3];
		array[i][3] = tmp;
	}
	if ((N_UI4)count % 2) {
		tmp = array[i][0];
		array[i][0] = array[i][1];
		array[i][1] = tmp;
	}
#endif
}

/** @brief 4バイト整数のバイトオーダー変換
 *
 * リトルエンディアン機では、4バイト整数または実数の配列 @p ary の
 * バイトオーダーを逆順にする。
 * ビッグエンディアンのデータを読んだ後整数として解釈する前、
 * または整数として値を格納した後ビッグエンディアンで書き出す前に呼ぶ。
 *
 * ビッグエンディアン機ではなにもしない。
 */
	void
NuSDaS_swab4(void *ary UNUSED_IF_BIGENDIAN, /**< 配列 */
		const N_UI4 count UNUSED_IF_BIGENDIAN) /**< 配列の要素数 */
{
#ifndef WORDS_BIGENDIAN
	N_UI4	*array = ary;
	N_UI4	i;
	/* NUSDAS_INIT; するまでもあるまい */
	for (i = 0; i < count; i++) {
		array[i] = NTOH4(array[i]);
	}
#endif
}

/** @brief 8バイト整数のバイトオーダー変換
 *
 * リトルエンディアン機では、8バイト整数または実数の配列 @p ary の
 * バイトオーダーを逆順にする。
 * ビッグエンディアンのデータを読んだ後整数として解釈する前、
 * または整数として値を格納した後ビッグエンディアンで書き出す前に呼ぶ。
 *
 * ビッグエンディアン機ではなにもしない。
 */
	void
NuSDaS_swab8(void *ary UNUSED_IF_BIGENDIAN, /**< 配列 */
		const N_UI4 count UNUSED_IF_BIGENDIAN) /**< 配列の要素数 */
{
#ifndef WORDS_BIGENDIAN
	N_UI8	*array = ary;
	N_UI4	i;
	/* NUSDAS_INIT; するまでもあるまい */
	for (i = 0; i < count; i++) {
		array[i] = NTOH8(array[i]);
	}
#endif
}

/** @brief 任意構造のバイトオーダー変換
 *
 * リトルエンディアン機では、
 * さまざまな長さのデータが混在するメモリ領域 @p ptr の
 * バイトオーダーを逆順にする。
 * ビッグエンディアンのデータを読んだ後整数として解釈する前、
 * または整数として値を格納した後ビッグエンディアンで書き出す前に呼ぶ。
 *
 * ビッグエンディアン機ではなにもしない。
 *
 * メモリのレイアウトは文字列 @p fmt で指定される。
 * 文字列は以下に示す型を表わす文字の羅列である。
 * <DL>
 * <DT>D, d, L, l<DD>8バイト
 * <DT>F, f, I, i<DD>4バイト
 * <DT>H, h<DD>2バイト
 * <DT>B, b, N, n<DD>1バイト (なにもしない)
 * </DL>
 * 文字の前に数字をつけると繰り返し数をあらわす。
 * たとえば ``<TT>4c8i</TT>'' は最初の 4 バイトが無変換、
 * 次に 4 バイト単位で 8 個変換を行うことを示す。
 *
 * <H3>注意</H3>
 * <UL>
 * <LI>数字は strtoul(3) で解釈しているので十進だけではなく八進や十六進
 * も使える。
 * たとえば ``<TT>0xFFi</TT>'' は 4 バイト単位で 255 個変換することを示し、
 * ``<TT>0100h</TT>'' は 2 バイト単位で 64 個変換することを示す。
 * </UL>
 *
 * <H3>履歴</H3>
 * 本関数は pnusdas から存在し、NuSDaS 1.3 で Fortran ラッパーを伴う
 * サービスサブルーチンとしてドキュメントされた。
 */
	void
NuSDaS_swab_fmt(void *ptr UNUSED_IF_BIGENDIAN, /**< 変換対象 */
		const char *fmt UNUSED_IF_BIGENDIAN /**< 書式 */)
{
#ifndef WORDS_BIGENDIAN
	char *p, *end_of_num;
	const char *f;
	int conv;
	unsigned long count;

	p = (char *)ptr;
	for (f = fmt; *f; f++) {
		count = strtoul(f, &end_of_num, 0);
		conv = *end_of_num;
		if (end_of_num == f) {
			count = 1;
		}
		switch (conv) {
			case 'd':
			case 'D':
			case 'l':
			case 'L':
				endian_swab8(p, count);
				p += 8 * count;
				break;
			case 'f':
			case 'F':
			case 'i':
			case 'I':
				endian_swab4(p, count);
				p += 4 * count;
				break;
			case 'h':
			case 'H':
				endian_swab2(p, count);
				p += 2 * count;
				break;
			default:
				/* do nothing */
				break;
		}
	}
#endif
}
