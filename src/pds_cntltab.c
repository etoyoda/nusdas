#line 1 "sys_hash.txt"
#line 2 "sys_hash.txt"
#line 3 "sys_hash.txt"
/** @file
 * @brief char ���� char ��Ϳ����ϥå���ơ��֥�
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

/** @brief �ϥå���ơ��֥�Υ���ȥ�
 */
struct pds_cntltab_entry {
	/** ���Υ���ȥ�ؤΥݥ��� */
	struct pds_cntltab_entry *next;
#line 33 "sys_hash.txt"
	/** ����. pds_cntltab_put() ��ʣ���� pds_cntltab_delete() ���˴� */
#line 35 "sys_hash.txt"
	char *key;
#line 37 "sys_hash.txt"
	/** ��. ��������Ϥ��ʤ� */
	char *val;
#line 40 "sys_hash.txt"
};

/** @brief �ϥå���
 */
typedef struct pds_cntltab_t {
	/** ���ǿ� */
	unsigned num_entries;
	/** ����Ȣ�ο� */
	unsigned num_bins;
	/** num_bins �ĤΥݥ��󥿤����� pds_cntltab_entry
	 * ����������󥯥ꥹ�Ȥ�ؤ��Ƥ���.
         * ��¤�ΤȰ���Ū�� pds_cntltab_ini() �ǳ���դ������.
	 */
	struct pds_cntltab_entry **bins;
} pds_cntltab_t;

/** @brief �ϥå����������
 */
pds_cntltab_t *
pds_cntltab_ini(unsigned nbins /** ����Ȣ�ο� */)
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

/** @brief �ϥå�����˴�����
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

/** @brief ��������ϥå����ͤ�׻�����
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

/** @brief �ϥå�����ͤ���������.
 *
 * @warning ���ͤ�������ȵ��ͤ��˴�����롣
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
	/* ���դ���ʤ���п������� */
	/** @note ����: ent ���������Ƥ���ɽ�˺ܤ��ʤ���
	 * malloc �� GC ��Ƥ������Ⱦü��ɽ�򸫤��������
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

/** @brief �ϥå��夫���ͤ�����
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
