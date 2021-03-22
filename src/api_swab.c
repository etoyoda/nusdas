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

/** @brief 2�Х��������ΥХ��ȥ��������Ѵ�
 *
 * ��ȥ륨��ǥ����󵡤Ǥϡ�2�Х������������� @p ary ��
 * �Х��ȥ���������ս�ˤ��롣
 * �ӥå�����ǥ�����Υǡ������ɤ���������Ȥ��Ʋ�᤹������
 * �ޤ��������Ȥ����ͤ��Ǽ������ӥå�����ǥ�����ǽ񤭽Ф����˸Ƥ֡�
 *
 * �ӥå�����ǥ����󵡤ǤϤʤˤ⤷�ʤ���
 * */
	void
NuSDaS_swab2(void *ary UNUSED_IF_BIGENDIAN, /**< ���� */
		const N_UI4 count UNUSED_IF_BIGENDIAN) /**< ��������ǿ� */
{
#ifndef WORDS_BIGENDIAN
	N_UI1 (*array)[4] = ary;
	N_UI4 tmp;
	N_UI4 i;
	/* NUSDAS_INIT; ����ޤǤ⤢��ޤ� */
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

/** @brief 4�Х��������ΥХ��ȥ��������Ѵ�
 *
 * ��ȥ륨��ǥ����󵡤Ǥϡ�4�Х��������ޤ��ϼ¿������� @p ary ��
 * �Х��ȥ���������ս�ˤ��롣
 * �ӥå�����ǥ�����Υǡ������ɤ���������Ȥ��Ʋ�᤹������
 * �ޤ��������Ȥ����ͤ��Ǽ������ӥå�����ǥ�����ǽ񤭽Ф����˸Ƥ֡�
 *
 * �ӥå�����ǥ����󵡤ǤϤʤˤ⤷�ʤ���
 */
	void
NuSDaS_swab4(void *ary UNUSED_IF_BIGENDIAN, /**< ���� */
		const N_UI4 count UNUSED_IF_BIGENDIAN) /**< ��������ǿ� */
{
#ifndef WORDS_BIGENDIAN
	N_UI4	*array = ary;
	N_UI4	i;
	/* NUSDAS_INIT; ����ޤǤ⤢��ޤ� */
	for (i = 0; i < count; i++) {
		array[i] = NTOH4(array[i]);
	}
#endif
}

/** @brief 8�Х��������ΥХ��ȥ��������Ѵ�
 *
 * ��ȥ륨��ǥ����󵡤Ǥϡ�8�Х��������ޤ��ϼ¿������� @p ary ��
 * �Х��ȥ���������ս�ˤ��롣
 * �ӥå�����ǥ�����Υǡ������ɤ���������Ȥ��Ʋ�᤹������
 * �ޤ��������Ȥ����ͤ��Ǽ������ӥå�����ǥ�����ǽ񤭽Ф����˸Ƥ֡�
 *
 * �ӥå�����ǥ����󵡤ǤϤʤˤ⤷�ʤ���
 */
	void
NuSDaS_swab8(void *ary UNUSED_IF_BIGENDIAN, /**< ���� */
		const N_UI4 count UNUSED_IF_BIGENDIAN) /**< ��������ǿ� */
{
#ifndef WORDS_BIGENDIAN
	N_UI8	*array = ary;
	N_UI4	i;
	/* NUSDAS_INIT; ����ޤǤ⤢��ޤ� */
	for (i = 0; i < count; i++) {
		array[i] = NTOH8(array[i]);
	}
#endif
}

/** @brief Ǥ�չ�¤�ΥХ��ȥ��������Ѵ�
 *
 * ��ȥ륨��ǥ����󵡤Ǥϡ�
 * ���ޤ��ޤ�Ĺ���Υǡ��������ߤ�������ΰ� @p ptr ��
 * �Х��ȥ���������ս�ˤ��롣
 * �ӥå�����ǥ�����Υǡ������ɤ���������Ȥ��Ʋ�᤹������
 * �ޤ��������Ȥ����ͤ��Ǽ������ӥå�����ǥ�����ǽ񤭽Ф����˸Ƥ֡�
 *
 * �ӥå�����ǥ����󵡤ǤϤʤˤ⤷�ʤ���
 *
 * ����Υ쥤�����Ȥ�ʸ���� @p fmt �ǻ��ꤵ��롣
 * ʸ����ϰʲ��˼�������ɽ�魯ʸ��������Ǥ��롣
 * <DL>
 * <DT>D, d, L, l<DD>8�Х���
 * <DT>F, f, I, i<DD>4�Х���
 * <DT>H, h<DD>2�Х���
 * <DT>B, b, N, n<DD>1�Х��� (�ʤˤ⤷�ʤ�)
 * </DL>
 * ʸ�������˿�����Ĥ���ȷ����֤����򤢤�魯��
 * ���Ȥ��� ``<TT>4c8i</TT>'' �Ϻǽ�� 4 �Х��Ȥ�̵�Ѵ���
 * ���� 4 �Х���ñ�̤� 8 ���Ѵ���Ԥ����Ȥ򼨤���
 *
 * <H3>���</H3>
 * <UL>
 * <LI>������ strtoul(3) �ǲ�ᤷ�Ƥ���Τǽ��ʤ����ǤϤʤ�Ȭ�ʤ佽ϻ��
 * ��Ȥ��롣
 * ���Ȥ��� ``<TT>0xFFi</TT>'' �� 4 �Х���ñ�̤� 255 ���Ѵ����뤳�Ȥ򼨤���
 * ``<TT>0100h</TT>'' �� 2 �Х���ñ�̤� 64 ���Ѵ����뤳�Ȥ򼨤���
 * </UL>
 *
 * <H3>����</H3>
 * �ܴؿ��� pnusdas ����¸�ߤ���NuSDaS 1.3 �� Fortran ��åѡ���ȼ��
 * �����ӥ����֥롼����Ȥ��ƥɥ�����Ȥ��줿��
 */
	void
NuSDaS_swab_fmt(void *ptr UNUSED_IF_BIGENDIAN, /**< �Ѵ��о� */
		const char *fmt UNUSED_IF_BIGENDIAN /**< �� */)
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
