#!/usr/bin/ruby
# ex: set sw=4 :
$stdout.reopen("ndf_rlen.c", "w")

print <<EOF
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
EOF

types = [:UI1, :SI2, :SI4, :R4, :R8]

for type in types

    case type
    when :UI1
	ctype, neg, suf = 'N_UI1', '', 'i1'
	negchk = ''
	vtype, uref = ctype, 'udata[iu]'
	vdecl = ''; vref = uref
    when :SI2
	ctype, neg, suf = 'N_SI2', '���ͤޤ���', 'i2'
	negchk = 'udata[iu] < 0 || '
	vtype, uref = 'int', 'udata[iu]'
	vdecl = ''; vref = uref
    when :SI4
	ctype, neg, suf = 'N_SI4', '���ͤޤ���', 'i4'
	negchk = 'udata[iu] < 0 || '
	vtype, uref = 'int', 'udata[iu]'
	vdecl = ''; vref = uref
    when :R4
	ctype, neg, suf = 'float', '���ͤޤ���', 'r4'
	negchk = 'udata[iu] < 0.0 || '
	vtype, uref = 'long', 'ROUND(udata[iu])'
	vdecl = "int v;\n\tv = ROUND(udata[iu]);"; vref = 'v'
    when :R8
	ctype, neg, suf = 'double', '���ͤޤ���', 'r8'
	negchk = 'udata[iu] < 0.0 || '
	vtype, uref = 'long', 'ROUND(udata[iu])'
	vdecl = "int v;\n\tv = ROUND(udata[iu]);"; vref = 'v'
    end

    print <<EOF
/** @brief #{ctype} �����ϢĹ���̤���
 * @retval ���� ���̷�̤ΥХ��ȿ�
 * @retval -1 ���̷�̤� cbuf_nbytes �Х��Ȥ�Ķ����
 * @retval -2 ���ϥǡ�����#{neg}�礭�������ͤ�����
 */
	N_SI4
nus_compress_rlen_#{suf}(const #{ctype} *udata, /**< ���ϥǡ��� */
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
    for (iu = 0; iu < udata_nelems; iu++) {#{vdecl}
	if (#{negchk}#{vref} > max_acceptable_data) {
	    overflow = 1;
	} else if (#{vref} > *maxv) {
	    *maxv = #{vref};
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
	#{vtype} value;
	value = #{uref.sub(/\biu\b/, 'iu++')};
	if (ic == cbuf_nbytes) {
	    return -1;
	}
	cbuf[ic++] = value;
	if (iu == udata_nelems) {
	    break;
	}
	seq_count = 0;
	while (1) {
	    if (value != #{uref}) {
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
EOF
end

for type in types

    case type
    when :UI1
	ctype, neg, suf = 'N_UI1', '', 'i1'
    when :SI2
	ctype, neg, suf = 'N_SI2', '���ͤޤ���', 'i2'
    when :SI4
	ctype, neg, suf = 'N_SI4', '���ͤޤ���', 'i4'
    when :R4
	ctype, neg, suf = 'float', '���ͤޤ���', 'r4'
    when :R8
	ctype, neg, suf = 'double', '���ͤޤ���', 'r8'
    end

    print <<EOF

/** @brief ϢĹ���̥ǡ������� #{ctype} �����Ÿ������
 * @retval ���� Ÿ��������ǿ�
 * @retval -4 �ǡ�����Ĺ�᤮�ƽ�����˳�Ǽ�Ǥ��ʤ�
 * @retval -6 ���̥ǡ���������Ƥ��� (�ǽ�ΥХ����ͤ��ǡ��������ͤ�Ķ����)
 */
	N_SI4
nus_decompress_rlen_#{suf}(const N_UI1 *compressed, /**< ���̥ǡ��� */
		N_UI4 maxv, /**< �ǡ����κ����� */
		N_UI4 cbytes, /**< ���̥ǡ����ΥХ��ȿ� */
		#{ctype} *udata, /**< ������Хåե� */
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
EOF
end

