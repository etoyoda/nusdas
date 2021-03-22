/** @file
 * @brief N_UI8 �ѡָ��������
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include "sys_mem.h"
#line 10 "sys_ary.txt"
# define NEED_UI8_MOD_UI2
# define NEED_SI8_CMP
#include "sys_int.h"
#line 14 "sys_ary.txt"
#include "sys_container.h"

#line 17 "sys_ary.txt"
/** @brief �ϥå��帡��ɽ���礭������ꤹ�� */
INLINE unsigned
nus_hashsize(unsigned arysize)
{
	if (arysize < 11u) { return 11u;
	} else if (arysize < 19u) { return 19u;
	} else if (arysize < 37u) { return 37u;
	} else if (arysize < 67u) { return 67u;
	} else if (arysize < 131u) { return 131u;
	} else { return 283u;
	}
}
#line 30 "sys_ary.txt"

/** @brief �������ǿ���������롣 */
#line 38 "sys_ary.txt"
 /**
 * @note ��������ϸ�ǳ��礹�뤳�Ȥ��Ǥ��ʤ���
 * ����ˤ�����ˤ������ͤ����äƤ��롣
 */
#line 43 "sys_ary.txt"
array8_t *
#line 50 "sys_ary.txt"
array8_ini(unsigned tsize /**< ����Ĺ */)
#line 52 "sys_ary.txt"
{
	array8_t	*array;
	size_t		size;
#line 56 "sys_ary.txt"
	unsigned	hsize, i;
#line 58 "sys_ary.txt"
	char		*buf;
#line 60 "sys_ary.txt"
#line 61 "sys_ary.txt"
	hsize = nus_hashsize(tsize);
	size = sizeof(array8_t) + tsize * sizeof(N_UI8) + hsize * sizeof(unsigned);
#line 68 "sys_ary.txt"
	buf = nus_malloc(size);
	if (buf == NULL)
		return NULL;
	array = (array8_t *)buf;
#line 75 "sys_ary.txt"
#line 86 "sys_ary.txt"
	/* ��Ĺ�ԲĤΤȤ��� array �Ȱ���Ū�� nus_malloc */
	array->num_entries = tsize;
	buf += sizeof(array8_t);
	array->list = (N_UI8 *)buf;
#line 91 "sys_ary.txt"
#line 92 "sys_ary.txt"
	/* �ϥå������ѻ��Ͼ�˿�Ĺ�Բ� */
	array->num_hashtab = hsize;
	buf += (tsize * sizeof(N_UI8));
	array->hash = (unsigned *)buf;
	for (i = 0u; i < array->num_hashtab; i++) {
		array->hash[i] = ~0u;
	}
#line 100 "sys_ary.txt"
	return array;
}

#line 114 "sys_ary.txt"

/** @brief �����ñ���Ǥ�����
 * @param ary ����
 * @param idx ���Ǥΰ��� (����Ϥޤ�)
 * @param val ��
 * @retval 0 ��������
 * @retval -1 ����Ĺ��Ķ����ź�������ꤵ�줿 (̤����)
 */
int
array8_set(array8_t *ary, unsigned idx, N_UI8 val)
{
#line 126 "sys_ary.txt"
	unsigned ihash;
#line 128 "sys_ary.txt"
#line 129 "sys_ary.txt"
#if 0
	if (idx >= ary->num_entries) {
		return -1;
	}
#endif
	ary->list[idx] = val;
#line 136 "sys_ary.txt"
	ihash = (ui8_mod_ui2(val, ary->num_hashtab));
	if (ary->hash[ihash] > idx) {
		ary->hash[ihash] = idx;
	}
#line 141 "sys_ary.txt"
	return 0;
}

#line 170 "sys_ary.txt"

/** @brief ����λ����ֹ����Ǥ�����
 * @param ary ����
 * @param idx ź�� (����Ϥޤ�)
 * @return �����ֹ������
 */
N_UI8
array8_get_value(array8_t *ary, unsigned idx)
{
	return ary->list[idx];
}

/** @brief ��������ͤ�¸�ߤ������ź��������
 * @param ary ����
 * @param val ��
 * @retval 0�ʾ� �ߤĤ��ä���
 * @retval �� �ߤĤ���ʤ��ä���
 */
int
array8_get_index(array8_t *ary, N_UI8 val)
{
	unsigned idx;
#line 193 "sys_ary.txt"
	idx = ary->hash[ui8_mod_ui2(val, ary->num_hashtab)];
#line 197 "sys_ary.txt"
	while (idx < ary->num_entries) {
		if (ui8_eq(ary->list[idx], val))
			return idx;
		idx++;
	}
	return -1;
}

#line 218 "sys_ary.txt"

#line 246 "sys_ary.txt"
