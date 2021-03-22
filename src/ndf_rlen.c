/** @file
 * @brief 連長圧縮ロジック
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include "dfile.h"
#include "ndf_codec.h"

#define USE_BACKWARD_SCAN 1
#define MAX_DATA_BITS 8

#ifdef HAVE_LRINT
# define ROUND(x)	lrint(x)
#else
# define ROUND(x)	floor(x + 0.5)
#endif

#if !USE_BACKWARD_SCAN
/** @brief 整数の冪乗
 */
	INLINE N_UI4
n_ipow(N_UI4 radix, N_UI4 raiseto)
{
	N_UI4 i, r;
	r = 1;
	for (i = 0; i < raiseto; i++) {
		r *= radix;
	}
	return r;
}
#endif
/** @brief N_UI1 配列を連長圧縮する
 * @retval 非負 圧縮結果のバイト数
 * @retval -1 圧縮結果は cbuf_nbytes バイトを超える
 * @retval -2 入力データに大きすぎる値がある
 */
	N_SI4
nus_compress_rlen_i1(const N_UI1 *udata, /**< 入力データ */
		N_UI4 udata_nelems, /**< 入力データの要素数 */
		N_SI4 *maxv, /**< 入力データの最大値を返すポインタ */
		N_UI1 *cbuf, /**< 圧縮結果を格納するバッファ */
		N_UI4 cbuf_nbytes) /**< 圧縮結果バッファのバイト数 */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* 圧縮諸元の決定 */
    *maxv = -1;
    overflow = 0;
    for (iu = 0; iu < udata_nelems; iu++) {
	if (udata[iu] > max_acceptable_data) {
	    overflow = 1;
	} else if (udata[iu] > *maxv) {
	    *maxv = udata[iu];
	}
    }
    if (overflow) {
	return -2;
    }
    radix = (1 << MAX_DATA_BITS) - 1 - *maxv;
    /* 圧縮実施 */
    iu = 0;
    ic = 0;
    while (iu < udata_nelems) {
	N_UI1 value;
	value = udata[iu++];
	if (ic == cbuf_nbytes) {
	    return -1;
	}
	cbuf[ic++] = value;
	if (iu == udata_nelems) {
	    break;
	}
	seq_count = 0;
	while (1) {
	    if (value != udata[iu]) {
		break;
	    }
	    seq_count++;
	    if (++iu == udata_nelems) {
		break;
	    }
	}
	if (seq_count == 0) {
	    continue;
	}
	while (seq_count >= 1) {
	    if (ic == cbuf_nbytes) {
		return -1;
	    }
	    cbuf[ic++] = seq_count % radix + *maxv + 1;
	    seq_count /= radix;
	}
    }
    return ic;
}
/** @brief N_SI2 配列を連長圧縮する
 * @retval 非負 圧縮結果のバイト数
 * @retval -1 圧縮結果は cbuf_nbytes バイトを超える
 * @retval -2 入力データに負値または大きすぎる値がある
 */
	N_SI4
nus_compress_rlen_i2(const N_SI2 *udata, /**< 入力データ */
		N_UI4 udata_nelems, /**< 入力データの要素数 */
		N_SI4 *maxv, /**< 入力データの最大値を返すポインタ */
		N_UI1 *cbuf, /**< 圧縮結果を格納するバッファ */
		N_UI4 cbuf_nbytes) /**< 圧縮結果バッファのバイト数 */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* 圧縮諸元の決定 */
    *maxv = -1;
    overflow = 0;
    for (iu = 0; iu < udata_nelems; iu++) {
	if (udata[iu] < 0 || udata[iu] > max_acceptable_data) {
	    overflow = 1;
	} else if (udata[iu] > *maxv) {
	    *maxv = udata[iu];
	}
    }
    if (overflow) {
	return -2;
    }
    radix = (1 << MAX_DATA_BITS) - 1 - *maxv;
    /* 圧縮実施 */
    iu = 0;
    ic = 0;
    while (iu < udata_nelems) {
	int value;
	value = udata[iu++];
	if (ic == cbuf_nbytes) {
	    return -1;
	}
	cbuf[ic++] = value;
	if (iu == udata_nelems) {
	    break;
	}
	seq_count = 0;
	while (1) {
	    if (value != udata[iu]) {
		break;
	    }
	    seq_count++;
	    if (++iu == udata_nelems) {
		break;
	    }
	}
	if (seq_count == 0) {
	    continue;
	}
	while (seq_count >= 1) {
	    if (ic == cbuf_nbytes) {
		return -1;
	    }
	    cbuf[ic++] = seq_count % radix + *maxv + 1;
	    seq_count /= radix;
	}
    }
    return ic;
}
/** @brief N_SI4 配列を連長圧縮する
 * @retval 非負 圧縮結果のバイト数
 * @retval -1 圧縮結果は cbuf_nbytes バイトを超える
 * @retval -2 入力データに負値または大きすぎる値がある
 */
	N_SI4
nus_compress_rlen_i4(const N_SI4 *udata, /**< 入力データ */
		N_UI4 udata_nelems, /**< 入力データの要素数 */
		N_SI4 *maxv, /**< 入力データの最大値を返すポインタ */
		N_UI1 *cbuf, /**< 圧縮結果を格納するバッファ */
		N_UI4 cbuf_nbytes) /**< 圧縮結果バッファのバイト数 */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* 圧縮諸元の決定 */
    *maxv = -1;
    overflow = 0;
    for (iu = 0; iu < udata_nelems; iu++) {
	if (udata[iu] < 0 || udata[iu] > max_acceptable_data) {
	    overflow = 1;
	} else if (udata[iu] > *maxv) {
	    *maxv = udata[iu];
	}
    }
    if (overflow) {
	return -2;
    }
    radix = (1 << MAX_DATA_BITS) - 1 - *maxv;
    /* 圧縮実施 */
    iu = 0;
    ic = 0;
    while (iu < udata_nelems) {
	int value;
	value = udata[iu++];
	if (ic == cbuf_nbytes) {
	    return -1;
	}
	cbuf[ic++] = value;
	if (iu == udata_nelems) {
	    break;
	}
	seq_count = 0;
	while (1) {
	    if (value != udata[iu]) {
		break;
	    }
	    seq_count++;
	    if (++iu == udata_nelems) {
		break;
	    }
	}
	if (seq_count == 0) {
	    continue;
	}
	while (seq_count >= 1) {
	    if (ic == cbuf_nbytes) {
		return -1;
	    }
	    cbuf[ic++] = seq_count % radix + *maxv + 1;
	    seq_count /= radix;
	}
    }
    return ic;
}
/** @brief float 配列を連長圧縮する
 * @retval 非負 圧縮結果のバイト数
 * @retval -1 圧縮結果は cbuf_nbytes バイトを超える
 * @retval -2 入力データに負値または大きすぎる値がある
 */
	N_SI4
nus_compress_rlen_r4(const float *udata, /**< 入力データ */
		N_UI4 udata_nelems, /**< 入力データの要素数 */
		N_SI4 *maxv, /**< 入力データの最大値を返すポインタ */
		N_UI1 *cbuf, /**< 圧縮結果を格納するバッファ */
		N_UI4 cbuf_nbytes) /**< 圧縮結果バッファのバイト数 */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* 圧縮諸元の決定 */
    *maxv = -1;
    overflow = 0;
    for (iu = 0; iu < udata_nelems; iu++) {int v;
	v = ROUND(udata[iu]);
	if (udata[iu] < 0.0 || v > max_acceptable_data) {
	    overflow = 1;
	} else if (v > *maxv) {
	    *maxv = v;
	}
    }
    if (overflow) {
	return -2;
    }
    radix = (1 << MAX_DATA_BITS) - 1 - *maxv;
    /* 圧縮実施 */
    iu = 0;
    ic = 0;
    while (iu < udata_nelems) {
	long value;
	value = ROUND(udata[iu++]);
	if (ic == cbuf_nbytes) {
	    return -1;
	}
	cbuf[ic++] = value;
	if (iu == udata_nelems) {
	    break;
	}
	seq_count = 0;
	while (1) {
	    if (value != ROUND(udata[iu])) {
		break;
	    }
	    seq_count++;
	    if (++iu == udata_nelems) {
		break;
	    }
	}
	if (seq_count == 0) {
	    continue;
	}
	while (seq_count >= 1) {
	    if (ic == cbuf_nbytes) {
		return -1;
	    }
	    cbuf[ic++] = seq_count % radix + *maxv + 1;
	    seq_count /= radix;
	}
    }
    return ic;
}
/** @brief double 配列を連長圧縮する
 * @retval 非負 圧縮結果のバイト数
 * @retval -1 圧縮結果は cbuf_nbytes バイトを超える
 * @retval -2 入力データに負値または大きすぎる値がある
 */
	N_SI4
nus_compress_rlen_r8(const double *udata, /**< 入力データ */
		N_UI4 udata_nelems, /**< 入力データの要素数 */
		N_SI4 *maxv, /**< 入力データの最大値を返すポインタ */
		N_UI1 *cbuf, /**< 圧縮結果を格納するバッファ */
		N_UI4 cbuf_nbytes) /**< 圧縮結果バッファのバイト数 */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* 圧縮諸元の決定 */
    *maxv = -1;
    overflow = 0;
    for (iu = 0; iu < udata_nelems; iu++) {int v;
	v = ROUND(udata[iu]);
	if (udata[iu] < 0.0 || v > max_acceptable_data) {
	    overflow = 1;
	} else if (v > *maxv) {
	    *maxv = v;
	}
    }
    if (overflow) {
	return -2;
    }
    radix = (1 << MAX_DATA_BITS) - 1 - *maxv;
    /* 圧縮実施 */
    iu = 0;
    ic = 0;
    while (iu < udata_nelems) {
	long value;
	value = ROUND(udata[iu++]);
	if (ic == cbuf_nbytes) {
	    return -1;
	}
	cbuf[ic++] = value;
	if (iu == udata_nelems) {
	    break;
	}
	seq_count = 0;
	while (1) {
	    if (value != ROUND(udata[iu])) {
		break;
	    }
	    seq_count++;
	    if (++iu == udata_nelems) {
		break;
	    }
	}
	if (seq_count == 0) {
	    continue;
	}
	while (seq_count >= 1) {
	    if (ic == cbuf_nbytes) {
		return -1;
	    }
	    cbuf[ic++] = seq_count % radix + *maxv + 1;
	    seq_count /= radix;
	}
    }
    return ic;
}

/** @brief 連長圧縮データから N_UI1 配列に展開する
 * @retval 非負 展開後の要素数
 * @retval -4 データが長過ぎて出力先に格納できない
 * @retval -6 圧縮データが壊れている (最初のバイト値がデータ最大値を超える)
 */
	N_SI4
nus_decompress_rlen_i1(const N_UI1 *compressed, /**< 圧縮データ */
		N_UI4 maxv, /**< データの最大値 */
		N_UI4 cbytes, /**< 圧縮データのバイト数 */
		N_UI1 *udata, /**< 出力先バッファ */
		N_UI4 udata_nelems) /**< 出力先バッファの要素数 */
{
#if USE_BACKWARD_SCAN
    N_UI4 ic, iu, radix, seq_count, iseq;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    ic = cbytes - 1;
    iu = udata_nelems;
    seq_count = 0;
    while (1) {
	if (compressed[ic] > maxv) {
	    seq_count *= radix;
	    seq_count += (compressed[ic] - maxv - 1);
	} else {
	    if (iu < seq_count++) {
		return -4;
	    }
	    for (iseq = 0; iseq < seq_count; iseq++) {
		udata[--iu] = compressed[ic];
	    }
	    seq_count = 0;
	}
	if (ic-- == 0) {
	    break;
	}
    }
    if (iu > 0) {
	memmove(udata, udata + iu, (udata_nelems - iu) * sizeof(*udata));
    }
    return udata_nelems - iu;
#else
    N_UI4 iseq, radix, seq_count = INT_MAX, idx_digit = INT_MAX;
    N_UI4 ic, iu, value, prev_value;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    if (compressed[0] > maxv) {
	return -6;
    }
    ic = 0;
    iu = 0;
    prev_value = UINT_MAX;
    while (ic < cbytes) {
	value = compressed[ic++];
	if (value <= maxv) {
	    if (prev_value != UINT_MAX) {
		for (iseq = 0; iseq < seq_count; iseq++) {
		    if (iu == udata_nelems) {
			return -4;
		    }
		    udata[iu++] = prev_value;
		}
	    }
	    prev_value = value;
	    seq_count = 1;
	    idx_digit = 0;
	} else {
	    seq_count += n_ipow(radix, idx_digit++) * (value - maxv - 1);
	}
    }
    for (iseq = 0; iseq < seq_count; iseq++) {
	if (iu == udata_nelems) {
	    return -4;
	}
	udata[iu++] = prev_value;
    }
    return iu;
#endif
}

/** @brief 連長圧縮データから N_SI2 配列に展開する
 * @retval 非負 展開後の要素数
 * @retval -4 データが長過ぎて出力先に格納できない
 * @retval -6 圧縮データが壊れている (最初のバイト値がデータ最大値を超える)
 */
	N_SI4
nus_decompress_rlen_i2(const N_UI1 *compressed, /**< 圧縮データ */
		N_UI4 maxv, /**< データの最大値 */
		N_UI4 cbytes, /**< 圧縮データのバイト数 */
		N_SI2 *udata, /**< 出力先バッファ */
		N_UI4 udata_nelems) /**< 出力先バッファの要素数 */
{
#if USE_BACKWARD_SCAN
    N_UI4 ic, iu, radix, seq_count, iseq;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    ic = cbytes - 1;
    iu = udata_nelems;
    seq_count = 0;
    while (1) {
	if (compressed[ic] > maxv) {
	    seq_count *= radix;
	    seq_count += (compressed[ic] - maxv - 1);
	} else {
	    if (iu < seq_count++) {
		return -4;
	    }
	    for (iseq = 0; iseq < seq_count; iseq++) {
		udata[--iu] = compressed[ic];
	    }
	    seq_count = 0;
	}
	if (ic-- == 0) {
	    break;
	}
    }
    if (iu > 0) {
	memmove(udata, udata + iu, (udata_nelems - iu) * sizeof(*udata));
    }
    return udata_nelems - iu;
#else
    N_UI4 iseq, radix, seq_count = INT_MAX, idx_digit = INT_MAX;
    N_UI4 ic, iu, value, prev_value;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    if (compressed[0] > maxv) {
	return -6;
    }
    ic = 0;
    iu = 0;
    prev_value = UINT_MAX;
    while (ic < cbytes) {
	value = compressed[ic++];
	if (value <= maxv) {
	    if (prev_value != UINT_MAX) {
		for (iseq = 0; iseq < seq_count; iseq++) {
		    if (iu == udata_nelems) {
			return -4;
		    }
		    udata[iu++] = prev_value;
		}
	    }
	    prev_value = value;
	    seq_count = 1;
	    idx_digit = 0;
	} else {
	    seq_count += n_ipow(radix, idx_digit++) * (value - maxv - 1);
	}
    }
    for (iseq = 0; iseq < seq_count; iseq++) {
	if (iu == udata_nelems) {
	    return -4;
	}
	udata[iu++] = prev_value;
    }
    return iu;
#endif
}

/** @brief 連長圧縮データから N_SI4 配列に展開する
 * @retval 非負 展開後の要素数
 * @retval -4 データが長過ぎて出力先に格納できない
 * @retval -6 圧縮データが壊れている (最初のバイト値がデータ最大値を超える)
 */
	N_SI4
nus_decompress_rlen_i4(const N_UI1 *compressed, /**< 圧縮データ */
		N_UI4 maxv, /**< データの最大値 */
		N_UI4 cbytes, /**< 圧縮データのバイト数 */
		N_SI4 *udata, /**< 出力先バッファ */
		N_UI4 udata_nelems) /**< 出力先バッファの要素数 */
{
#if USE_BACKWARD_SCAN
    N_UI4 ic, iu, radix, seq_count, iseq;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    ic = cbytes - 1;
    iu = udata_nelems;
    seq_count = 0;
    while (1) {
	if (compressed[ic] > maxv) {
	    seq_count *= radix;
	    seq_count += (compressed[ic] - maxv - 1);
	} else {
	    if (iu < seq_count++) {
		return -4;
	    }
	    for (iseq = 0; iseq < seq_count; iseq++) {
		udata[--iu] = compressed[ic];
	    }
	    seq_count = 0;
	}
	if (ic-- == 0) {
	    break;
	}
    }
    if (iu > 0) {
	memmove(udata, udata + iu, (udata_nelems - iu) * sizeof(*udata));
    }
    return udata_nelems - iu;
#else
    N_UI4 iseq, radix, seq_count = INT_MAX, idx_digit = INT_MAX;
    N_UI4 ic, iu, value, prev_value;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    if (compressed[0] > maxv) {
	return -6;
    }
    ic = 0;
    iu = 0;
    prev_value = UINT_MAX;
    while (ic < cbytes) {
	value = compressed[ic++];
	if (value <= maxv) {
	    if (prev_value != UINT_MAX) {
		for (iseq = 0; iseq < seq_count; iseq++) {
		    if (iu == udata_nelems) {
			return -4;
		    }
		    udata[iu++] = prev_value;
		}
	    }
	    prev_value = value;
	    seq_count = 1;
	    idx_digit = 0;
	} else {
	    seq_count += n_ipow(radix, idx_digit++) * (value - maxv - 1);
	}
    }
    for (iseq = 0; iseq < seq_count; iseq++) {
	if (iu == udata_nelems) {
	    return -4;
	}
	udata[iu++] = prev_value;
    }
    return iu;
#endif
}

/** @brief 連長圧縮データから float 配列に展開する
 * @retval 非負 展開後の要素数
 * @retval -4 データが長過ぎて出力先に格納できない
 * @retval -6 圧縮データが壊れている (最初のバイト値がデータ最大値を超える)
 */
	N_SI4
nus_decompress_rlen_r4(const N_UI1 *compressed, /**< 圧縮データ */
		N_UI4 maxv, /**< データの最大値 */
		N_UI4 cbytes, /**< 圧縮データのバイト数 */
		float *udata, /**< 出力先バッファ */
		N_UI4 udata_nelems) /**< 出力先バッファの要素数 */
{
#if USE_BACKWARD_SCAN
    N_UI4 ic, iu, radix, seq_count, iseq;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    ic = cbytes - 1;
    iu = udata_nelems;
    seq_count = 0;
    while (1) {
	if (compressed[ic] > maxv) {
	    seq_count *= radix;
	    seq_count += (compressed[ic] - maxv - 1);
	} else {
	    if (iu < seq_count++) {
		return -4;
	    }
	    for (iseq = 0; iseq < seq_count; iseq++) {
		udata[--iu] = compressed[ic];
	    }
	    seq_count = 0;
	}
	if (ic-- == 0) {
	    break;
	}
    }
    if (iu > 0) {
	memmove(udata, udata + iu, (udata_nelems - iu) * sizeof(*udata));
    }
    return udata_nelems - iu;
#else
    N_UI4 iseq, radix, seq_count = INT_MAX, idx_digit = INT_MAX;
    N_UI4 ic, iu, value, prev_value;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    if (compressed[0] > maxv) {
	return -6;
    }
    ic = 0;
    iu = 0;
    prev_value = UINT_MAX;
    while (ic < cbytes) {
	value = compressed[ic++];
	if (value <= maxv) {
	    if (prev_value != UINT_MAX) {
		for (iseq = 0; iseq < seq_count; iseq++) {
		    if (iu == udata_nelems) {
			return -4;
		    }
		    udata[iu++] = prev_value;
		}
	    }
	    prev_value = value;
	    seq_count = 1;
	    idx_digit = 0;
	} else {
	    seq_count += n_ipow(radix, idx_digit++) * (value - maxv - 1);
	}
    }
    for (iseq = 0; iseq < seq_count; iseq++) {
	if (iu == udata_nelems) {
	    return -4;
	}
	udata[iu++] = prev_value;
    }
    return iu;
#endif
}

/** @brief 連長圧縮データから double 配列に展開する
 * @retval 非負 展開後の要素数
 * @retval -4 データが長過ぎて出力先に格納できない
 * @retval -6 圧縮データが壊れている (最初のバイト値がデータ最大値を超える)
 */
	N_SI4
nus_decompress_rlen_r8(const N_UI1 *compressed, /**< 圧縮データ */
		N_UI4 maxv, /**< データの最大値 */
		N_UI4 cbytes, /**< 圧縮データのバイト数 */
		double *udata, /**< 出力先バッファ */
		N_UI4 udata_nelems) /**< 出力先バッファの要素数 */
{
#if USE_BACKWARD_SCAN
    N_UI4 ic, iu, radix, seq_count, iseq;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    ic = cbytes - 1;
    iu = udata_nelems;
    seq_count = 0;
    while (1) {
	if (compressed[ic] > maxv) {
	    seq_count *= radix;
	    seq_count += (compressed[ic] - maxv - 1);
	} else {
	    if (iu < seq_count++) {
		return -4;
	    }
	    for (iseq = 0; iseq < seq_count; iseq++) {
		udata[--iu] = compressed[ic];
	    }
	    seq_count = 0;
	}
	if (ic-- == 0) {
	    break;
	}
    }
    if (iu > 0) {
	memmove(udata, udata + iu, (udata_nelems - iu) * sizeof(*udata));
    }
    return udata_nelems - iu;
#else
    N_UI4 iseq, radix, seq_count = INT_MAX, idx_digit = INT_MAX;
    N_UI4 ic, iu, value, prev_value;
    radix = (1 << MAX_DATA_BITS) - 1 - maxv;
    if (compressed[0] > maxv) {
	return -6;
    }
    ic = 0;
    iu = 0;
    prev_value = UINT_MAX;
    while (ic < cbytes) {
	value = compressed[ic++];
	if (value <= maxv) {
	    if (prev_value != UINT_MAX) {
		for (iseq = 0; iseq < seq_count; iseq++) {
		    if (iu == udata_nelems) {
			return -4;
		    }
		    udata[iu++] = prev_value;
		}
	    }
	    prev_value = value;
	    seq_count = 1;
	    idx_digit = 0;
	} else {
	    seq_count += n_ipow(radix, idx_digit++) * (value - maxv - 1);
	}
    }
    for (iseq = 0; iseq < seq_count; iseq++) {
	if (iu == udata_nelems) {
	    return -4;
	}
	udata[iu++] = prev_value;
    }
    return iu;
#endif
}
