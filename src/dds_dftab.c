#line 1 "sys_hash.txt"
#line 2 "sys_hash.txt"
#line 3 "sys_hash.txt"
/** @file
 * @brief char から union nusdfile_t を与えるハッシュテーブル
 */
#include "config.h" /* for INLINE */
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
# define NEED_UI8_MOD_UI2
# define NEED_SI8_CMP
#include "sys_int.h" /* for ui8_mod_ui2 */
#include "internal_types.h" /* for char */
#include "sys_mem.h"
# define NEED_STRING_DUP
#line 17 "sys_hash.txt"
#include <string.h>
#line 17 "sys_hash.txt"
#include "sys_string.h"
#line 17 "sys_hash.txt"
#include "dset.h"
#line 19 "sys_hash.txt"

#line 24 "sys_hash.txt"

/** @brief ハッシュテーブルのエントリ
 */
struct dds_dftab_entry {
	/** 次のエントリへのポインタ */
	struct dds_dftab_entry *next;
#line 33 "sys_hash.txt"
	/** キー. dds_dftab_put() で複製、 dds_dftab_delete() で破棄 */
#line 35 "sys_hash.txt"
	char *key;
#line 37 "sys_hash.txt"
	/** 値. メモリ管理はしない */
	union nusdfile_t *val;
#line 40 "sys_hash.txt"
};

/** @brief ハッシュ
 */
typedef struct dds_dftab_t {
	/** 要素数 */
	unsigned num_entries;
	/** 仕訳箱の数 */
	unsigned num_bins;
	/** num_bins 個のポインタの配列が dds_dftab_entry
	 * の片方向リンクリストを指している.
         * 構造体と一体的に dds_dftab_ini() で割り付けされる.
	 */
	struct dds_dftab_entry **bins;
} dds_dftab_t;

/** @brief ハッシュを構成する
 */
dds_dftab_t *
dds_dftab_ini(unsigned nbins /** 仕訳箱の数 */)
{
	dds_dftab_t	*hp;
	unsigned	i;
	hp = (dds_dftab_t *)nus_malloc(sizeof(dds_dftab_t));
	if (hp == NULL) {
		return NULL;
	}
	hp->bins = (struct dds_dftab_entry **)nus_malloc(nbins *
		sizeof(struct dds_dftab_entry *));
	hp->num_entries = 0;
	hp->num_bins = nbins;
	for (i = 0; i < nbins; i++) {
		hp->bins[i] = NULL;
	}
	return hp;
}

/** @brief ハッシュを破棄する
 */
void
dds_dftab_delete(dds_dftab_t *hp)
{
	unsigned	i;
	for (i = 0; i < hp->num_bins; i++) {
		struct dds_dftab_entry *p;
		struct dds_dftab_entry *p_next;
		for (p = hp->bins[i]; p; p = p_next) {
#line 88 "sys_hash.txt"
			nus_free(p->key);
#line 90 "sys_hash.txt"
			p_next = p->next;
			nus_free(p);
		}
	}
	nus_free(hp->bins);
	nus_free(hp);
}

/** @brief キーからハッシュ値を計算する
 */
INLINE unsigned
dds_dftab_hashval(const char *key, unsigned nbins)
{
#line 112 "sys_hash.txt"
	unsigned char *p;
	unsigned sum = 0;
	for (p = (unsigned char *)key; *p; p++) {
		sum += (*p ^ ' ');
	}
	return (sum % nbins);
#line 123 "sys_hash.txt"
}

/** @brief ハッシュに値を投入する.
 *
 * @warning 衝突が起こると旧値が破棄される。
 */
int
#line 131 "sys_hash.txt"
dds_dftab_put(dds_dftab_t *hp, char *key, union nusdfile_t *val)
#line 135 "sys_hash.txt"
{
	unsigned hashval;
	struct dds_dftab_entry	**entp;
	struct dds_dftab_entry	*ent;
	hashval = dds_dftab_hashval(key, hp->num_bins);
	for (entp = hp->bins + hashval; *entp; entp = &((*entp)->next)) {
		if (!strcmp(((*entp)->key), key)) {
			goto Found;
		}
	}
	/* 見付からなければ新規割当 */
	/** @note 重要: ent を完成させてから表に載せないと
	 * malloc が GC を呼んで中途半端な表を見る危険がある
	 */
	ent = nus_malloc(sizeof(struct dds_dftab_entry));
	if (ent == NULL) {
		return -1;
	}
	ent->val = val;
	ent->next = NULL;
	ent->key = string_dup(key);
	hp->num_entries++;
	*entp = ent;
	return 0;

Found:
#line 162 "sys_hash.txt"
	(*entp)->val = val;
#line 164 "sys_hash.txt"
	return 0;
}

/** @brief ハッシュから値を得る
 */
#line 170 "sys_hash.txt"
	union nusdfile_t *
#line 174 "sys_hash.txt"
dds_dftab_get(dds_dftab_t *hp, const char *key)
{
	unsigned hashval;
	struct dds_dftab_entry	**entp;
	hashval = dds_dftab_hashval(key, hp->num_bins);
	if (hp->bins[hashval] == NULL) {
#line 181 "sys_hash.txt"
		return NULL;
#line 185 "sys_hash.txt"
	}
	for (entp = hp->bins + hashval; *entp; entp = &((*entp)->next)) {
		if (!strcmp(((*entp)->key), key)) {
#line 189 "sys_hash.txt"
			return (*entp)->val;
#line 193 "sys_hash.txt"
		}
	}
#line 196 "sys_hash.txt"
	return NULL;
#line 200 "sys_hash.txt"
}

#line 203 "sys_hash.txt"
/** @brief ハッシュに投入された (キー, 値) の組を順にコールバックする
 */
int
dds_dftab_each(dds_dftab_t *hp,
	int (*callback)(char *key, union nusdfile_t *val, void *arg),
	void *arg)
{
	unsigned hashval;
	struct dds_dftab_entry **entp;
	int r;
	for (hashval = 0; hashval < hp->num_bins; hashval++) {
		for (entp = hp->bins + hashval; *entp;
			entp = &((*entp)->next)) {
			r = callback((*entp)->key, (*entp)->val, arg);
			if (r)
				return r;
		}
	}
	return 0;
}
#line 224 "sys_hash.txt"

#line 226 "sys_hash.txt"
/** @brief ハッシュの値をチェックして一部だけ除去する
 *
 * ハッシュの値を次々に引数として @p callback を呼出し、
 * 非零が返ったときだけハッシュから除去する。
 */
void
dds_dftab_reject(dds_dftab_t *hp, int (*callback)(union nusdfile_t *val))
{
	unsigned	i;
	for (i = 0; i < hp->num_bins; i++) {
		struct dds_dftab_entry *p;
		struct dds_dftab_entry *p_next;
		struct dds_dftab_entry *prev = NULL;
		for (p = hp->bins[i]; p; p = p_next) {
			if (callback(p->val) == 0) {
				prev = p;
				p_next = p->next;
				continue;
			}
			hp->num_entries--;
#line 247 "sys_hash.txt"
			nus_free(p->key);
#line 249 "sys_hash.txt"
			if (prev) {
				prev->next = p->next;
			} else {
				hp->bins[i] = p->next;
			}
			p->val = 0;
			p_next = p->next;
			nus_free(p);
		}
	}
}
#line 261 "sys_hash.txt"
