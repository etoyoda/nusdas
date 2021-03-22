#line 1 "sys_hash.txt"
#line 2 "sys_hash.txt"
#line 3 "sys_hash.txt"
/** @file
 * @brief char から char を与えるハッシュテーブル
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
#line 19 "sys_hash.txt"

#line 24 "sys_hash.txt"

/** @brief ハッシュテーブルのエントリ
 */
struct pds_cntltab_entry {
	/** 次のエントリへのポインタ */
	struct pds_cntltab_entry *next;
#line 33 "sys_hash.txt"
	/** キー. pds_cntltab_put() で複製、 pds_cntltab_delete() で破棄 */
#line 35 "sys_hash.txt"
	char *key;
#line 37 "sys_hash.txt"
	/** 値. メモリ管理はしない */
	char *val;
#line 40 "sys_hash.txt"
};

/** @brief ハッシュ
 */
typedef struct pds_cntltab_t {
	/** 要素数 */
	unsigned num_entries;
	/** 仕訳箱の数 */
	unsigned num_bins;
	/** num_bins 個のポインタの配列が pds_cntltab_entry
	 * の片方向リンクリストを指している.
         * 構造体と一体的に pds_cntltab_ini() で割り付けされる.
	 */
	struct pds_cntltab_entry **bins;
} pds_cntltab_t;

/** @brief ハッシュを構成する
 */
pds_cntltab_t *
pds_cntltab_ini(unsigned nbins /** 仕訳箱の数 */)
{
	pds_cntltab_t	*hp;
	unsigned	i;
	hp = (pds_cntltab_t *)nus_malloc(sizeof(pds_cntltab_t));
	if (hp == NULL) {
		return NULL;
	}
	hp->bins = (struct pds_cntltab_entry **)nus_malloc(nbins *
		sizeof(struct pds_cntltab_entry *));
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
pds_cntltab_delete(pds_cntltab_t *hp)
{
	unsigned	i;
	for (i = 0; i < hp->num_bins; i++) {
		struct pds_cntltab_entry *p;
		struct pds_cntltab_entry *p_next;
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
pds_cntltab_hashval(const char *key, unsigned nbins)
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
pds_cntltab_put(pds_cntltab_t *hp, char *key, char *val)
#line 135 "sys_hash.txt"
{
	unsigned hashval;
	struct pds_cntltab_entry	**entp;
	struct pds_cntltab_entry	*ent;
	hashval = pds_cntltab_hashval(key, hp->num_bins);
	for (entp = hp->bins + hashval; *entp; entp = &((*entp)->next)) {
		if (!strcmp(((*entp)->key), key)) {
			goto Found;
		}
	}
	/* 見付からなければ新規割当 */
	/** @note 重要: ent を完成させてから表に載せないと
	 * malloc が GC を呼んで中途半端な表を見る危険がある
	 */
	ent = nus_malloc(sizeof(struct pds_cntltab_entry));
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
	char *
#line 174 "sys_hash.txt"
pds_cntltab_get(pds_cntltab_t *hp, const char *key)
{
	unsigned hashval;
	struct pds_cntltab_entry	**entp;
	hashval = pds_cntltab_hashval(key, hp->num_bins);
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

#line 224 "sys_hash.txt"

#line 261 "sys_hash.txt"
