/** @file
 * @brief N_UI4 �ѡָ��������
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include "sys_mem.h"
#line 14 "sys_ary.txt"
#include "sys_container.h"

#line 30 "sys_ary.txt"

/** @brief �������ǿ���������롣 */
#line 33 "sys_ary.txt"
 /**
 * @note ��������� array4v_push() �ǳ��礹�뤳�Ȥ��Ǥ��롣
 * ����ˤ����󥵥����϶��Ǥ��롣
 */
#line 43 "sys_ary.txt"
array4v_t *
#line 45 "sys_ary.txt"
array4v_ini(
	unsigned tsize, /**< ����Ĺ */
	/** ����������Ӵؿ� */
	int (*cmpfunc)(const void *a, const void *b))
#line 52 "sys_ary.txt"
{
	array4v_t	*array;
	size_t		size;
#line 58 "sys_ary.txt"
	char		*buf;
#line 64 "sys_ary.txt"
	size = sizeof(array4v_t);
#line 68 "sys_ary.txt"
	buf = nus_malloc(size);
	if (buf == NULL)
		return NULL;
	array = (array4v_t *)buf;
#line 73 "sys_ary.txt"
	array->cmpfunc = cmpfunc;
#line 75 "sys_ary.txt"
#line 76 "sys_ary.txt"
	/* ��Ĺ��ǽ�ΤȤ��ϸ�� nus_realloc �Ǥ���褦���� nus_malloc */
	array->num_entries = 0;
	array->num_allocated = tsize;
	buf = nus_malloc(tsize * sizeof(N_UI4));
	if (buf == NULL) {
		nus_free(array);
		return NULL;
	}
	array->list = (N_UI4 *)buf;
#line 91 "sys_ary.txt"
#line 100 "sys_ary.txt"
	return array;
}

#line 104 "sys_ary.txt"
/** @brief ������˴����񸻳���
 * @param ary �˴����褦�Ȥ�������
 */
void
array4v_delete(array4v_t *ary)
{
	nus_free(ary->list);
	nus_free(ary);
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
array4v_set(array4v_t *ary, unsigned idx, N_UI4 val)
{
#line 128 "sys_ary.txt"
#line 129 "sys_ary.txt"
#if 0
	if (idx >= ary->num_allocated) {
		return -1;
	}
#endif
	ary->list[idx] = val;
#line 141 "sys_ary.txt"
	return 0;
}

#line 145 "sys_ary.txt"
/** @brief ����������ñ���Ǥ��ɲ�
 * @note ����Ĺ����­�ʤ��2�ܤ˳�ĥ����롣
 * @param ary ����
 * @param val ��
 * @retval 0 ���ｪλ
 * @retval -1 �����ĥ����
 */
int
array4v_push(array4v_t *ary, N_UI4 val)
{
	if (ary->num_entries >= ary->num_allocated) {
		char *ptr;
		ptr = nus_realloc(ary->list,
			2 * ary->num_allocated * sizeof(N_UI4 *));
		if (ptr == NULL) {
			return -1;
		}
		ary->list = (N_UI4 *)ptr;
		ary->num_allocated *= 2;
	}
	ary->list[ary->num_entries] = val;
	ary->num_entries++;
	return 0;
}
#line 170 "sys_ary.txt"

/** @brief ����λ����ֹ����Ǥ�����
 * @param ary ����
 * @param idx ź�� (����Ϥޤ�)
 * @return �����ֹ������
 */
N_UI4
array4v_get_value(array4v_t *ary, unsigned idx)
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
array4v_get_index(array4v_t *ary, N_UI4 val)
{
	unsigned idx;
#line 195 "sys_ary.txt"
	idx = 0;
#line 197 "sys_ary.txt"
	while (idx < ary->num_entries) {
		if (ary->list[idx] == val)
			return idx;
		idx++;
	}
	return -1;
}

#line 206 "sys_ary.txt"

/** @brief ����򥽡��Ȥ���
 * @param ary ����
 * @return qsort ��Ʊ�ͤ��ͤ��֤��ʤ�
 */
void
array4v_sort(array4v_t *ary)
{
	qsort(ary->list, ary->num_entries, sizeof(N_UI4), ary->cmpfunc);
}

#line 218 "sys_ary.txt"

#line 246 "sys_ary.txt"
