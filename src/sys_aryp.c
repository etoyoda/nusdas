/** @file
 * @brief void * 用「賢い配列」
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include "sys_mem.h"
#line 14 "sys_ary.txt"
#include "sys_container.h"

#line 30 "sys_ary.txt"

/** @brief 指定要素数の配列を作る。 */
#line 33 "sys_ary.txt"
 /**
 * @note この配列は arrayp_push() で拡大することができる。
 * 初期には配列サイズは空である。
 */
#line 43 "sys_ary.txt"
arrayp_t *
#line 45 "sys_ary.txt"
arrayp_ini(
	unsigned tsize, /**< 配列長 */
	/** ソート用比較関数 */
	int (*cmpfunc)(const void *a, const void *b))
#line 52 "sys_ary.txt"
{
	arrayp_t	*array;
	size_t		size;
#line 58 "sys_ary.txt"
	char		*buf;
#line 64 "sys_ary.txt"
	size = sizeof(arrayp_t);
#line 68 "sys_ary.txt"
	buf = nus_malloc(size);
	if (buf == NULL)
		return NULL;
	array = (arrayp_t *)buf;
#line 73 "sys_ary.txt"
	array->cmpfunc = cmpfunc;
#line 75 "sys_ary.txt"
#line 76 "sys_ary.txt"
	/* 伸長可能のときは後で nus_realloc できるように別 nus_malloc */
	array->num_entries = 0;
	array->num_allocated = tsize;
	buf = nus_malloc(tsize * sizeof(void *));
	if (buf == NULL) {
		nus_free(array);
		return NULL;
	}
	array->list = (void * *)buf;
#line 91 "sys_ary.txt"
#line 100 "sys_ary.txt"
	return array;
}

#line 104 "sys_ary.txt"
/** @brief 配列の破棄、資源開放
 * @param ary 破棄しようとする配列
 */
void
arrayp_delete(arrayp_t *ary)
{
	nus_free(ary->list);
	nus_free(ary);
}
#line 114 "sys_ary.txt"

/** @brief 配列の単要素を設定
 * @param ary 配列
 * @param idx 要素の位置 (ゼロ始まり)
 * @param val 値
 * @retval 0 設定成功
 * @retval -1 配列長を超えた添字が指定された (未実装)
 */
int
arrayp_set(arrayp_t *ary, unsigned idx, void * val)
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
/** @brief 配列末尾に単要素を追加
 * @note 配列長が不足ならば2倍に拡張される。
 * @param ary 配列
 * @param val 値
 * @retval 0 正常終了
 * @retval -1 配列拡張失敗
 */
int
arrayp_push(arrayp_t *ary, void * val)
{
	if (ary->num_entries >= ary->num_allocated) {
		char *ptr;
		ptr = nus_realloc(ary->list,
			2 * ary->num_allocated * sizeof(void * *));
		if (ptr == NULL) {
			return -1;
		}
		ary->list = (void * *)ptr;
		ary->num_allocated *= 2;
	}
	ary->list[ary->num_entries] = val;
	ary->num_entries++;
	return 0;
}
#line 170 "sys_ary.txt"

/** @brief 配列の指定番号要素を得る
 * @param ary 配列
 * @param idx 添字 (ゼロ始まり)
 * @return 指定番号の要素
 */
void *
arrayp_get_value(arrayp_t *ary, unsigned idx)
{
	return ary->list[idx];
}

/** @brief 配列中に値が存在する場所の添字を得る
 * @param ary 配列
 * @param val 値
 * @retval 0以上 みつかった。
 * @retval 負 みつからなかった。
 */
int
arrayp_get_index(arrayp_t *ary, void * val)
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

/** @brief 配列をソートする
 * @param ary 配列
 * @return qsort と同様に値を返さない
 */
void
arrayp_sort(arrayp_t *ary)
{
	qsort(ary->list, ary->num_entries, sizeof(void *), ary->cmpfunc);
}

#line 218 "sys_ary.txt"

#line 220 "sys_ary.txt"

/** @brief 指定した要素を先頭に移動する。
 * @param ary 配列
 * @param target 先頭に移動するポインタ
 * @return 0: 正常
 * @return -1: 指定した要素が配列に含まれていなかった
 */
int
arrayp_movehead(arrayp_t *ary, void *target)
{
	unsigned idx, i;
	int r;
	
	r = arrayp_get_index(ary, target);
	if (r < 0) {
		return r;
	}
	idx = r;
	for (i = idx; i > 0; i--) {
		ary->list[i] = ary->list[i - 1];
	}
	ary->list[0] = target;
	return 0;
}

#line 246 "sys_ary.txt"
