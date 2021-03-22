/** @file
 * @brief N_UI4 用「賢い配列」
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include "sys_mem.h"
#line 14 "sys_ary.txt"
#include "sys_container.h"

#line 17 "sys_ary.txt"
/** @brief ハッシュ検索表の大きさを決定する */
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

/** @brief 指定要素数の配列を作る。 */
#line 38 "sys_ary.txt"
 /**
 * @note この配列は後で拡大することができない。
 * 初期には配列には不定値が入っている。
 */
#line 43 "sys_ary.txt"
array4_t *
#line 50 "sys_ary.txt"
array4_ini(unsigned tsize /**< 配列長 */)
#line 52 "sys_ary.txt"
{
	array4_t	*array;
	size_t		size;
#line 56 "sys_ary.txt"
	unsigned	hsize, i;
#line 58 "sys_ary.txt"
	char		*buf;
#line 60 "sys_ary.txt"
#line 61 "sys_ary.txt"
	hsize = nus_hashsize(tsize);
	size = sizeof(array4_t) + tsize * sizeof(N_UI4) + hsize * sizeof(unsigned);
#line 68 "sys_ary.txt"
	buf = nus_malloc(size);
	if (buf == NULL)
		return NULL;
	array = (array4_t *)buf;
#line 75 "sys_ary.txt"
#line 86 "sys_ary.txt"
	/* 伸長不可のときは array と一体的に nus_malloc */
	array->num_entries = tsize;
	buf += sizeof(array4_t);
	array->list = (N_UI4 *)buf;
#line 91 "sys_ary.txt"
#line 92 "sys_ary.txt"
	/* ハッシュ利用時は常に伸長不可 */
	array->num_hashtab = hsize;
	buf += (tsize * sizeof(N_UI4));
	array->hash = (unsigned *)buf;
	for (i = 0u; i < array->num_hashtab; i++) {
		array->hash[i] = ~0u;
	}
#line 100 "sys_ary.txt"
	return array;
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
array4_set(array4_t *ary, unsigned idx, N_UI4 val)
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
	ihash = (val % ary->num_hashtab);
	if (ary->hash[ihash] > idx) {
		ary->hash[ihash] = idx;
	}
#line 141 "sys_ary.txt"
	return 0;
}

#line 170 "sys_ary.txt"

/** @brief 配列の指定番号要素を得る
 * @param ary 配列
 * @param idx 添字 (ゼロ始まり)
 * @return 指定番号の要素
 */
N_UI4
array4_get_value(array4_t *ary, unsigned idx)
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
array4_get_index(array4_t *ary, N_UI4 val)
{
	unsigned idx;
#line 193 "sys_ary.txt"
	idx = ary->hash[val % ary->num_hashtab];
#line 197 "sys_ary.txt"
	while (idx < ary->num_entries) {
		if (ary->list[idx] == val)
			return idx;
		idx++;
	}
	return -1;
}

#line 218 "sys_ary.txt"

#line 246 "sys_ary.txt"
