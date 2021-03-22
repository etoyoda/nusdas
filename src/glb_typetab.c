#line 1 "sys_hash.txt"
#line 2 "sys_hash.txt"
#line 3 "sys_hash.txt"
/** @file
 * @brief nustype_t ���� struct nustype_dstab ��Ϳ����ϥå���ơ��֥�
 */
#include "config.h" /* for INLINE */
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
# define NEED_UI8_MOD_UI2
# define NEED_SI8_CMP
#include "sys_int.h" /* for ui8_mod_ui2 */
#include "internal_types.h" /* for nustype_t */
#include "sys_mem.h"
# define NEED_STRING_DUP
#line 19 "sys_hash.txt"

#line 24 "sys_hash.txt"

/** @brief �ϥå���ơ��֥�Υ���ȥ�
 */
struct glb_typetab_entry {
	/** ���Υ���ȥ�ؤΥݥ��� */
	struct glb_typetab_entry *next;
#line 31 "sys_hash.txt"
	/** ���� */
#line 35 "sys_hash.txt"
	nustype_t key;
#line 37 "sys_hash.txt"
	/** ��. ��������Ϥ��ʤ� */
	struct nustype_dstab *val;
#line 40 "sys_hash.txt"
};

/** @brief �ϥå���
 */
typedef struct glb_typetab_t {
	/** ���ǿ� */
	unsigned num_entries;
	/** ����Ȣ�ο� */
	unsigned num_bins;
	/** num_bins �ĤΥݥ��󥿤����� glb_typetab_entry
	 * ����������󥯥ꥹ�Ȥ�ؤ��Ƥ���.
         * ��¤�ΤȰ���Ū�� glb_typetab_ini() �ǳ���դ������.
	 */
	struct glb_typetab_entry **bins;
} glb_typetab_t;

/** @brief �ϥå����������
 */
glb_typetab_t *
glb_typetab_ini(unsigned nbins /** ����Ȣ�ο� */)
{
	glb_typetab_t	*hp;
	unsigned	i;
	hp = (glb_typetab_t *)nus_malloc(sizeof(glb_typetab_t));
	if (hp == NULL) {
		return NULL;
	}
	hp->bins = (struct glb_typetab_entry **)nus_malloc(nbins *
		sizeof(struct glb_typetab_entry *));
	hp->num_entries = 0;
	hp->num_bins = nbins;
	for (i = 0; i < nbins; i++) {
		hp->bins[i] = NULL;
	}
	return hp;
}

/** @brief �ϥå�����˴�����
 */
void
glb_typetab_delete(glb_typetab_t *hp)
{
	unsigned	i;
	for (i = 0; i < hp->num_bins; i++) {
		struct glb_typetab_entry *p;
		struct glb_typetab_entry *p_next;
		for (p = hp->bins[i]; p; p = p_next) {
#line 90 "sys_hash.txt"
			p_next = p->next;
			nus_free(p);
		}
	}
	nus_free(hp->bins);
	nus_free(hp);
}

/** @brief ��������ϥå����ͤ�׻�����
 */
INLINE unsigned
glb_typetab_hashval(const nustype_t *key, unsigned nbins)
{
#line 104 "sys_hash.txt"
	return (ui8_mod_ui2(key->type1, nbins) + key->type2 + key->type3)
		% nbins;
#line 123 "sys_hash.txt"
}

/** @brief �ϥå�����ͤ���������.
 *
 * @warning ���ͤ�������ȵ��ͤ��˴�����롣
 */
int
#line 131 "sys_hash.txt"
glb_typetab_put(glb_typetab_t *hp, nustype_t *key, struct nustype_dstab *val)
#line 135 "sys_hash.txt"
{
	unsigned hashval;
	struct glb_typetab_entry	**entp;
	struct glb_typetab_entry	*ent;
	hashval = glb_typetab_hashval(key, hp->num_bins);
	for (entp = hp->bins + hashval; *entp; entp = &((*entp)->next)) {
		if (nustype_p_eq(&((*entp)->key), key)) {
			goto Found;
		}
	}
	/* ���դ���ʤ���п������� */
	/** @note ����: ent ���������Ƥ���ɽ�˺ܤ��ʤ���
	 * malloc �� GC ��Ƥ������Ⱦü��ɽ�򸫤��������
	 */
	ent = nus_malloc(sizeof(struct glb_typetab_entry));
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

/** @brief �ϥå��夫���ͤ�����
 */
#line 170 "sys_hash.txt"
	struct nustype_dstab *
#line 174 "sys_hash.txt"
glb_typetab_get(glb_typetab_t *hp, const nustype_t *key)
{
	unsigned hashval;
	struct glb_typetab_entry	**entp;
	hashval = glb_typetab_hashval(key, hp->num_bins);
	if (hp->bins[hashval] == NULL) {
#line 181 "sys_hash.txt"
		return NULL;
#line 185 "sys_hash.txt"
	}
	for (entp = hp->bins + hashval; *entp; entp = &((*entp)->next)) {
		if (nustype_p_eq(&((*entp)->key), key)) {
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
