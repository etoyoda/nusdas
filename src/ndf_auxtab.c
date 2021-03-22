#line 1 "sys_hash.txt"
#line 2 "sys_hash.txt"
#line 3 "sys_hash.txt"
/** @file
 * @brief sym4_t から struct ndf_aux_t を与えるハッシュテーブル
 */
#include "config.h" /* for INLINE */
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
# define NEED_UI8_MOD_UI2
# define NEED_SI8_CMP
#include "sys_int.h" /* for ui8_mod_ui2 */
#include "internal_types.h" /* for sym4_t */
#include "sys_mem.h"
# define NEED_STRING_DUP
#line 17 "sys_hash.txt"
#include "dfile.h"
#line 17 "sys_hash.txt"
#include "ndf.h"
#line 19 "sys_hash.txt"

#line 24 "sys_hash.txt"

/** @brief ハッシュテーブルのエントリ
 */
struct ndf_auxtab_entry {
	/** 次のエントリへのポインタ */
	struct ndf_auxtab_entry *next;
#line 31 "sys_hash.txt"
	/** キー */
#line 35 "sys_hash.txt"
	sym4_t key;
#line 37 "sys_hash.txt"
	/** 値. メモリ管理はしない */
	struct ndf_aux_t *val;
#line 40 "sys_hash.txt"
};

/** @brief ハッシュ
 */
typedef struct ndf_auxtab_t {
	/** 要素数 */
	unsigned num_entries;
	/** 仕訳箱の数 */
	unsigned num_bins;
	/** num_bins 個のポインタの配列が ndf_auxtab_entry
	 * の片方向リンクリストを指している.
         * 構造体と一体的に ndf_auxtab_ini() で割り付けされる.
	 */
	struct ndf_auxtab_entry **bins;
} ndf_auxtab_t;

/** @brief ハッシュを構成する
 */
ndf_auxtab_t *
ndf_auxtab_ini(unsigned nbins /** 仕訳箱の数 */)
{
	ndf_auxtab_t	*hp;
	unsigned	i;
	hp = (ndf_auxtab_t *)nus_malloc(sizeof(ndf_auxtab_t));
	if (hp == NULL) {
		return NULL;
	}
	hp->bins = (struct ndf_auxtab_entry **)nus_malloc(nbins *
		sizeof(struct ndf_auxtab_entry *));
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
ndf_auxtab_delete(ndf_auxtab_t *hp)
{
	unsigned	i;
	for (i = 0; i < hp->num_bins; i++) {
		struct ndf_auxtab_entry *p;
		struct ndf_auxtab_entry *p_next;
		for (p = hp->bins[i]; p; p = p_next) {
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
ndf_auxtab_hashval(const sym4_t *key, unsigned nbins)
{
#line 119 "sys_hash.txt"
        return *key % nbins;
#line 123 "sys_hash.txt"
}

/** @brief ハッシュに値を投入する.
 *
 * @warning 衝突が起こると旧値が破棄される。
 */
int
#line 131 "sys_hash.txt"
ndf_auxtab_put(ndf_auxtab_t *hp, sym4_t *key, struct ndf_aux_t *val)
#line 135 "sys_hash.txt"
{
	unsigned hashval;
	struct ndf_auxtab_entry	**entp;
	struct ndf_auxtab_entry	*ent;
	hashval = ndf_auxtab_hashval(key, hp->num_bins);
	for (entp = hp->bins + hashval; *entp; entp = &((*entp)->next)) {
		if (nusndf_aux_eq(&((*entp)->key), key)) {
			goto Found;
		}
	}
	/* 見付からなければ新規割当 */
	/** @note 重要: ent を完成させてから表に載せないと
	 * malloc が GC を呼んで中途半端な表を見る危険がある
	 */
	ent = nus_malloc(sizeof(struct ndf_auxtab_entry));
	if (ent == NULL) {
		return -1;
	}
	ent->val = val;
	ent->next = NULL;
	ent->key = *(key);
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
	struct ndf_aux_t *
#line 174 "sys_hash.txt"
ndf_auxtab_get(ndf_auxtab_t *hp, const sym4_t *key)
{
	unsigned hashval;
	struct ndf_auxtab_entry	**entp;
	hashval = ndf_auxtab_hashval(key, hp->num_bins);
	if (hp->bins[hashval] == NULL) {
#line 181 "sys_hash.txt"
		return NULL;
#line 185 "sys_hash.txt"
	}
	for (entp = hp->bins + hashval; *entp; entp = &((*entp)->next)) {
		if (nusndf_aux_eq(&((*entp)->key), key)) {
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
ndf_auxtab_each(ndf_auxtab_t *hp,
	int (*callback)(sym4_t key, struct ndf_aux_t *val, void *arg),
	void *arg)
{
	unsigned hashval;
	struct ndf_auxtab_entry **entp;
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

#line 261 "sys_hash.txt"
