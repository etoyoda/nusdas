/** \file
 * \brief void * ��ñ�����ꥹ��
 * \li (�����оݤ��ݥ��󥿤ξ��) ������δ����Ϥ��ʤ���
 * \li ��������Ƭ�����ʤΤ������������֤ȵս�˽ФƤ��롣
 *     (���������Τ��Ȥ򤢤Ƥˤ��ʤ��褦��)
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include "sys_container.h"
#include "sys_mem.h"

/** ���Υꥹ�Ȥ��� */
listp_t *
listp_ini(void)
{
	listp_t		*c;
	c = nus_malloc(sizeof(listp_t));
	c->num_entries = 0;
	c->next = NULL;
	c->last = NULL;
	return c;
}

/** �ꥹ�Ȥ����� @p obj ���������롣  */
int
listp_push(listp_t *c, void *obj)
{
	struct listp_entry	*e;
	e = nus_malloc(sizeof(struct listp_entry));
	if (e == NULL) {
		return -1;
	}
	e->obj = obj;
	e->next = NULL;
	if (c->next == NULL) {
		c->next = e;
	} else {
		c->last->next = e;
	}
	c->last = e;
	return ++(c->num_entries);
}

/** �ꥹ�Ȥ����äƤ������Ǥ��줾��ˤĤ��ƴؿ� @p callback ��ƤӽФ���
 * �ؿ��ˤ����� @p obj �� @p arg ���Ϥ���롣
 * callback ��������֤������õ�������Ǥ���롣
 */
int
listp_each(listp_t *c, int (*callback)(void *obj, void *arg), void *arg)
{
	struct listp_entry *cur;
	int	r;
	for (cur = c->next; cur; cur = cur->next) {
		r = callback(cur->obj, arg);
		if (r != 0)
			return r;
	}
	return 0;
}
