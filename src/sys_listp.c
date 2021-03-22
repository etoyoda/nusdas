/** \file
 * \brief void * 用単方向リスト
 * \li (管理対象がポインタの場合) その先の管理はしない。
 * \li 投入は先頭挿入なので投入した順番と逆順に出てくる。
 *     (しかしそのことをあてにしないように)
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include "sys_container.h"
#include "sys_mem.h"

/** 空のリストを作る */
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

/** リストに要素 @p obj を投入する。  */
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

/** リストに入っている要素それぞれについて関数 @p callback を呼び出す。
 * 関数には要素 @p obj と @p arg が渡される。
 * callback が非零を返した場合探索は中断される。
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
