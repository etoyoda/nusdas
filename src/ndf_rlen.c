/** @file
 * @brief ϢĹ���̥��å�
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
/** @brief �������Ѿ�
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
/** @brief N_UI1 �����ϢĹ���̤���
 * @retval ���� ���̷�̤ΥХ��ȿ�
 * @retval -1 ���̷�̤� cbuf_nbytes �Х��Ȥ�Ķ����
 * @retval -2 ���ϥǡ������礭�������ͤ�����
 */
	N_SI4
nus_compress_rlen_i1(const N_UI1 *udata, /**< ���ϥǡ��� */
		N_UI4 udata_nelems, /**< ���ϥǡ��������ǿ� */
		N_SI4 *maxv, /**< ���ϥǡ����κ����ͤ��֤��ݥ��� */
		N_UI1 *cbuf, /**< ���̷�̤��Ǽ����Хåե� */
		N_UI4 cbuf_nbytes) /**< ���̷�̥Хåե��ΥХ��ȿ� */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* ���̽����η��� */
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
    /* ���̼»� */
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
/** @brief N_SI2 �����ϢĹ���̤���
 * @retval ���� ���̷�̤ΥХ��ȿ�
 * @retval -1 ���̷�̤� cbuf_nbytes �Х��Ȥ�Ķ����
 * @retval -2 ���ϥǡ��������ͤޤ����礭�������ͤ�����
 */
	N_SI4
nus_compress_rlen_i2(const N_SI2 *udata, /**< ���ϥǡ��� */
		N_UI4 udata_nelems, /**< ���ϥǡ��������ǿ� */
		N_SI4 *maxv, /**< ���ϥǡ����κ����ͤ��֤��ݥ��� */
		N_UI1 *cbuf, /**< ���̷�̤��Ǽ����Хåե� */
		N_UI4 cbuf_nbytes) /**< ���̷�̥Хåե��ΥХ��ȿ� */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* ���̽����η��� */
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
    /* ���̼»� */
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
/** @brief N_SI4 �����ϢĹ���̤���
 * @retval ���� ���̷�̤ΥХ��ȿ�
 * @retval -1 ���̷�̤� cbuf_nbytes �Х��Ȥ�Ķ����
 * @retval -2 ���ϥǡ��������ͤޤ����礭�������ͤ�����
 */
	N_SI4
nus_compress_rlen_i4(const N_SI4 *udata, /**< ���ϥǡ��� */
		N_UI4 udata_nelems, /**< ���ϥǡ��������ǿ� */
		N_SI4 *maxv, /**< ���ϥǡ����κ����ͤ��֤��ݥ��� */
		N_UI1 *cbuf, /**< ���̷�̤��Ǽ����Хåե� */
		N_UI4 cbuf_nbytes) /**< ���̷�̥Хåե��ΥХ��ȿ� */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* ���̽����η��� */
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
    /* ���̼»� */
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
/** @brief float �����ϢĹ���̤���
 * @retval ���� ���̷�̤ΥХ��ȿ�
 * @retval -1 ���̷�̤� cbuf_nbytes �Х��Ȥ�Ķ����
 * @retval -2 ���ϥǡ��������ͤޤ����礭�������ͤ�����
 */
	N_SI4
nus_compress_rlen_r4(const float *udata, /**< ���ϥǡ��� */
		N_UI4 udata_nelems, /**< ���ϥǡ��������ǿ� */
		N_SI4 *maxv, /**< ���ϥǡ����κ����ͤ��֤��ݥ��� */
		N_UI1 *cbuf, /**< ���̷�̤��Ǽ����Хåե� */
		N_UI4 cbuf_nbytes) /**< ���̷�̥Хåե��ΥХ��ȿ� */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* ���̽����η��� */
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
    /* ���̼»� */
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
/** @brief double �����ϢĹ���̤���
 * @retval ���� ���̷�̤ΥХ��ȿ�
 * @retval -1 ���̷�̤� cbuf_nbytes �Х��Ȥ�Ķ����
 * @retval -2 ���ϥǡ��������ͤޤ����礭�������ͤ�����
 */
	N_SI4
nus_compress_rlen_r8(const double *udata, /**< ���ϥǡ��� */
		N_UI4 udata_nelems, /**< ���ϥǡ��������ǿ� */
		N_SI4 *maxv, /**< ���ϥǡ����κ����ͤ��֤��ݥ��� */
		N_UI1 *cbuf, /**< ���̷�̤��Ǽ����Хåե� */
		N_UI4 cbuf_nbytes) /**< ���̷�̥Хåե��ΥХ��ȿ� */
{
    N_UI4 seq_count, radix;
    N_UI4 iu, ic;
    const int max_acceptable_data = (1 << MAX_DATA_BITS) - 4;
    int overflow;
    /* ���̽����η��� */
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
    /* ���̼»� */
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

/** @brief ϢĹ���̥ǡ������� N_UI1 �����Ÿ������
 * @retval ���� Ÿ��������ǿ�
 * @retval -4 �ǡ�����Ĺ�᤮�ƽ�����˳�Ǽ�Ǥ��ʤ�
 * @retval -6 ���̥ǡ���������Ƥ��� (�ǽ�ΥХ����ͤ��ǡ��������ͤ�Ķ����)
 */
	N_SI4
nus_decompress_rlen_i1(const N_UI1 *compressed, /**< ���̥ǡ��� */
		N_UI4 maxv, /**< �ǡ����κ����� */
		N_UI4 cbytes, /**< ���̥ǡ����ΥХ��ȿ� */
		N_UI1 *udata, /**< ������Хåե� */
		N_UI4 udata_nelems) /**< ������Хåե������ǿ� */
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

/** @brief ϢĹ���̥ǡ������� N_SI2 �����Ÿ������
 * @retval ���� Ÿ��������ǿ�
 * @retval -4 �ǡ�����Ĺ�᤮�ƽ�����˳�Ǽ�Ǥ��ʤ�
 * @retval -6 ���̥ǡ���������Ƥ��� (�ǽ�ΥХ����ͤ��ǡ��������ͤ�Ķ����)
 */
	N_SI4
nus_decompress_rlen_i2(const N_UI1 *compressed, /**< ���̥ǡ��� */
		N_UI4 maxv, /**< �ǡ����κ����� */
		N_UI4 cbytes, /**< ���̥ǡ����ΥХ��ȿ� */
		N_SI2 *udata, /**< ������Хåե� */
		N_UI4 udata_nelems) /**< ������Хåե������ǿ� */
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

/** @brief ϢĹ���̥ǡ������� N_SI4 �����Ÿ������
 * @retval ���� Ÿ��������ǿ�
 * @retval -4 �ǡ�����Ĺ�᤮�ƽ�����˳�Ǽ�Ǥ��ʤ�
 * @retval -6 ���̥ǡ���������Ƥ��� (�ǽ�ΥХ����ͤ��ǡ��������ͤ�Ķ����)
 */
	N_SI4
nus_decompress_rlen_i4(const N_UI1 *compressed, /**< ���̥ǡ��� */
		N_UI4 maxv, /**< �ǡ����κ����� */
		N_UI4 cbytes, /**< ���̥ǡ����ΥХ��ȿ� */
		N_SI4 *udata, /**< ������Хåե� */
		N_UI4 udata_nelems) /**< ������Хåե������ǿ� */
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

/** @brief ϢĹ���̥ǡ������� float �����Ÿ������
 * @retval ���� Ÿ��������ǿ�
 * @retval -4 �ǡ�����Ĺ�᤮�ƽ�����˳�Ǽ�Ǥ��ʤ�
 * @retval -6 ���̥ǡ���������Ƥ��� (�ǽ�ΥХ����ͤ��ǡ��������ͤ�Ķ����)
 */
	N_SI4
nus_decompress_rlen_r4(const N_UI1 *compressed, /**< ���̥ǡ��� */
		N_UI4 maxv, /**< �ǡ����κ����� */
		N_UI4 cbytes, /**< ���̥ǡ����ΥХ��ȿ� */
		float *udata, /**< ������Хåե� */
		N_UI4 udata_nelems) /**< ������Хåե������ǿ� */
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

/** @brief ϢĹ���̥ǡ������� double �����Ÿ������
 * @retval ���� Ÿ��������ǿ�
 * @retval -4 �ǡ�����Ĺ�᤮�ƽ�����˳�Ǽ�Ǥ��ʤ�
 * @retval -6 ���̥ǡ���������Ƥ��� (�ǽ�ΥХ����ͤ��ǡ��������ͤ�Ķ����)
 */
	N_SI4
nus_decompress_rlen_r8(const N_UI1 *compressed, /**< ���̥ǡ��� */
		N_UI4 maxv, /**< �ǡ����κ����� */
		N_UI4 cbytes, /**< ���̥ǡ����ΥХ��ȿ� */
		double *udata, /**< ������Хåե� */
		N_UI4 udata_nelems) /**< ������Хåե������ǿ� */
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
