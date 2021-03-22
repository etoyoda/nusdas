/** \file
 * \brief NuSDaS ����ե�����β���.
 *
 */
#include "config.h"
#include "nusdas.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "internal_types.h"
#include "sys_kwd.h"
# define NEED_STR2SYM4
# define NEED_STR2SYM8
#include "sys_sym.h"
# define NEED_ULONG_TO_UI8
#include "sys_int.h"
#include "sys_mem.h"
# define NEED_STRING_UPCASE
# define NEED_STRING_COPY
# define NEED_STRING_CAT
# define NEED_STRING_DUP
#include "sys_string.h"
#include "dset.h"
#include "glb.h"
#include "sys_err.h"
#include "sys_file.h"  /* for file_read */

#define TEST_ELEMENTMAP 0

#ifndef DEFAULT_CREATOR
# define DEFAULT_CREATOR "Japan Meteorological Agency http://www.jma.go.jp/"
#endif

/**
 * @brief nusdef_parse �ξ��֡�
 * \warning �ѡ����Ϥ���¾�� def ���ͤʤɤ⻲�Ȥ��뤱�ɡ�
 */
enum parse_phase {
	pINIT = 1, pNUSDAS, pPATH, pFILENAME, pCREATOR, pTYPE1,
	pTYPE2, pTYPE3, pMEMBER, pMEMBERLIST, pBASETIME, pVALIDTIME,
	pVALIDTIME1, pVALIDTIME2, pPLANE, pPLANE1, pPLANE2, pELEMENT,
	pELEMENTMAP, pSIZE, pDISTANCE, pBASEPOINT, pSTANDARD, pOTHERS, pVALUE,
	pPACKING, pMISSING, pINFORMATION, pSUBCNTL, pFORCEDRLEN, pOPTIONS,
	pERR
};

/** ��ư��������� token2phase() ���ɤ߹��� */
#include "def_phase.h"

/** @brief ����ե�������ɤ߹������� + ��ʬ���������
 *
 * lex_buffer_ini(), lex_buffer_free() �������Ѵ����롣
 * */
struct lex_buffer {
	unsigned char	*buf;	/**< �Хåե� */
	unsigned char	*stopper; /**< �Хåե������μ��ΥХ��� */
	unsigned char	*cursor; /**< ���� gettoken_raw() �����Ȥ��� */
	unsigned	line;	/**< ���ֹ� */
	int		elemctr; /**< ELEMENTMAP ʸ�Υ����� */
	/** ľ���줬���Ԥǽ�ü����Ƥ����ե饰��
	 * nusdef_skipline() �Τ�����Ѱդ���Ƥ��롣*/
	int		just_hit_nr;
};

/** @brief �����ޤǥ����å�
 *
 * gettoken_raw() �������ɤ߽Ф����֤�����ޤǥ����פ��롣
 */
INLINE void
nusdef_skipline(struct lex_buffer *file)
{
	if (file->just_hit_nr)
		return;
	while (*file->cursor) {
		if (*file->cursor == '\n')
			break;
		file->cursor++;
	}
}

/** @brief 1�ȡ��������
 *
 * �ե�������ɤ߹��ޤ줿����ե����� @p file ��ȡ������ʬ�򤹤롣
 * �ե꡼�ե����ޥåȤǤ�����Ԥ�ۤ���õ�����롣
 *
 * @retval NULL �ե�����������ã����
 * @retval ¾ �ȡ�����
 */
INLINE
char *gettoken_raw(struct lex_buffer *file)
{
	char *token;
	while (file->cursor < file->stopper) {
		switch (*file->cursor) {
			case '\n':
				file->line++;
				/* fallthrough */
			case ' ':
			case '\t':
			case '\r':
			case '\v':
			case '\f':
				file->cursor++;
				break;
			default:
				goto SpaceSkipped;
		}
	}
	if (file->cursor >= file->stopper)
		return NULL;
SpaceSkipped:
	token = (char *)file->cursor;
	while (file->cursor < file->stopper) {
		switch (*file->cursor) {
			case '\n':
				file->line++;
				file->just_hit_nr = 1;
				goto EndToken;
				/* fallthrough */
			case ' ':
			case '\t':
			case '\r':
			case '\v':
			case '\f':
				file->just_hit_nr = 0;
				goto EndToken;
			default:
				file->cursor++;
				break;
		}
	}
	file->just_hit_nr = 0;
EndToken:
	*(file->cursor++) = '\0';
	return token;
}

/** @brief ��̿Ū���顼�μ���
 *
 * nusdef_parse ��λ��������̿Ū���顼�μ���
 */

enum parser_fatal { UnexpectedEOF = 1, MemoryShort };

/** @brief 1�ȡ��������
 *
 * �ե�������ɤ߹��ޤ줿����ե����� @p file ��ȡ������ʬ�򤹤롣
 * gettoken_raw ��Ʊ���������ե����������Ǥ� NULL ���֤�������
 * "" ���֤����ᡢ���δؿ��η�̤Υݥ��󥿤ϰ����˻��ȤǤ��롣
 */
INLINE char *
gettoken_safe(struct lex_buffer *file)
{
	char *p;
	p = gettoken_raw(file);
	if (p == NULL) {
		return "";
	}
	return p;
}

/** @brief PATH ʸ�β���
 * @param do_gettoken ��ˤ���ȥȡ������ɤ߽Ф��򤷤ʤ��ʥ��顼�ˤ�ʤ�ʤ���
 *
 * @retval 0 ���ｪλ
 * @retval ���� ���顼��ͽ�۳��Υե�����������
 */
INLINE int
defstmt_path(char *token, nusdef_t *def, int do_gettoken)
{
	string_upcase(token);
	if (streq(token, "RELATIVE_PATH") && do_gettoken) {
		token = gettoken_raw(def->lexbuffer);
		if (token == NULL) {
			return 1;
		}
		string_copy(def->path, token, sizeof(def->path));
	} else if (*token == '.') {
		string_copy(def->path, token, sizeof(def->path));
	} else if (streq(token, "NWP_PATH_S")) {
		string_copy(def->path, "\0073name/_validtime",
				sizeof(def->path));
	} else if (streq(token, "NWP_PATH_VM")) {
		string_copy(def->path, "\0073name/_member",
				sizeof(def->path));
	} else if (streq(token, "NWP_PATH_M")) {
		string_copy(def->path, "\0073name/_member/_validtime",
				sizeof(def->path));
	} else if (streq(token, "NWP_PATH_BS")) {
		string_copy(def->path, "\0073name/_basetime",
				sizeof(def->path));
	} else if (streq(token, "NWP_ESF")) {
		string_copy(def->path, "\0074esdev/_type",
				sizeof(def->path));
		string_cat(def->options, "IESF,", sizeof def->options);
	} else {
		nus_warn(("invalid path type <%s> ignored (%#ys)", token,
					&def->nustype));
	}
	return 0;
}

/** @brief TYPE1 ʸ�β���
 * ���ޤΤȤ���ͽ�����̥ե����륨��ɤ�̵�뤵��롣
 *
 * @retval 0 ���ｪλ
 */
INLINE int
defstmt_type1(char *token, nusdef_t *def)
{
	char buf[8];

	if (strlen(token) == 4) {
		memcpy(buf, token, 4);
	} else {
		memcpy(buf, "_XXX", 4);
	}

	token = gettoken_safe(def->lexbuffer);
	if (strlen(token) == 2) {
		memcpy(buf + 4, token, 2);
	} else {
		memcpy(buf + 4, "XX", 2);
	}

	token = gettoken_safe(def->lexbuffer);
	if (strlen(token) == 2) {
		memcpy(buf + 6, token, 2);
	} else {
		memcpy(buf + 6, "XX", 2);
	}

	def->nustype.type1 = MEM2SYM8(buf);

	return 0;
}

/** @brief TYPE2 ʸ�β���
 * ���ޤΤȤ���ͽ�����̥ե����륨��ɤ�̵�뤵��롣
 *
 * @retval 0 ���ｪλ
 */
INLINE int
defstmt_type2(char *token, nusdef_t *def)
{
	char buf[4];

	if (strlen(token) == 2) {
		memcpy(buf, token, 2);
	} else {
		memcpy(buf, "XX", 2);
	}

	token = gettoken_safe(def->lexbuffer);
	if (strlen(token) == 2) {
		memcpy(buf + 2, token, 2);
	} else {
		memcpy(buf + 2, "XX", 2);
	}

	def->nustype.type2 = MEM2SYM4(buf);

	return 0;
}

/** @brief in/out ��Ƚ��
 *
 * ����ե������� in/out ���̤�ɽ���Ȥ������ɡ�
 * @retval 0 IN
 * @retval 1 OUT
 * @retval -1 �����ȡ�����
 */
INLINE
int defword_out(char *token)
{
	string_upcase(token);
	if (streq(token, "IN")) {
		return 0;
	} else if (streq(token, "OUT")) {
		return 1;
	} else {
		return -1;
	}
}

/** @brief MEMBER ʸ�β���
 *
 * @retval 0 ���ｪλ
 * @retval �� �Ǹ�� in/out ��ȴ���Ƽ�ʸ�Υ�����ɤ��ɤ�Ǥ��ޤä���� (enum��)
 * @retval NUSERR_EOFinDef ͽ�����̥ե���������
 * @retval �� ����¾���顼�ʥ�å�����ɽ���Ѥߡ�
 */
INLINE
int defstmt_member(char *token, nusdef_t *def)
{
	if (def->n_mb) {
		return nus_err((NUSERR_DefOrder, "duplicated MEMBER"
					" (%#ys)", &def->nustype));
	}
	if ((def->n_mb = atoi(token)) <= 0) {
		return nus_err((NUSERR_Deffile,
					"invalid member number <%s>"
					" (%#ys)", token, &def->nustype));
	}
	token = gettoken_raw(def->lexbuffer);
	if (token == NULL) {
		return NUSERR_EOFinDef;
	}
	if ((def->mb_out = defword_out(token)) < 0) {
		enum parse_phase phase = token2phase(token);
		if (phase != pERR) {
			nus_warn(("in/out missing in MEMBER (%#ys)",
						&def->nustype));
			return phase;
		}
		return nus_err((NUSERR_Deffile,
					"invalid in/out specifier <%s>"
					" (%#ys)",
					token, &def->nustype));
	}
	return 0;
}

/** @brief MEMBERLIST ʸ�β���
 *
 * @retval 0 ���ｪλ
 * @retval �� �ꥹ����˼�ʸ�Υ�����ɤ����줿�Τǽ��������Ǥ��� (enum��)
 * @retval NUSERR_EOFinDef ͽ�����̥ե���������
 * @retval �� ����¾���顼�ʥ�å�����ɽ���Ѥߡ�
 */
INLINE
int defstmt_memberlist(char *token, nusdef_t *def)
{
	enum parse_phase phase;
	int i;
	if (def->n_mb <= 0) {
		return nus_err((NUSERR_DefOrder,
					"MEMBER must be before MEMBERLIST"
					" (%#ys)", &def->nustype));
	}
	if (def->mb) {
		return nus_err((NUSERR_DefOrder, "duplicated MEMBERLIST"
					" (%#ys)", &def->nustype));
	}
	def->mb = nus_malloc(sizeof(sym4_t) * def->n_mb);
	if (def->mb == NULL) {
		return nus_err((NUSERR_MemShort, "memory shortage"));
	}
	def->mb[0] = str2sym4(token);
	for (i = 1; i < def->n_mb; i++) {
		token = gettoken_raw(def->lexbuffer);
		if (token == NULL) {
			return NUSERR_EOFinDef;
		}
		if (strlen(token) > 4) {
			string_upcase(token);
			if ((phase = token2phase(token)) != pERR) {
				goto keyword_found;
			}
			nus_warn(("too long member name <%s>", token));
		}
		def->mb[i] = str2sym4(token);
	}
	return 0;

keyword_found:
	nus_warn(("missing %d names in memberlist for %#ys",
		def->n_mb - i, &def->nustype));
	while (i < def->n_mb) {
		def->mb[i] = SYM4_ALLSPACE;
		i++;
	}
	return phase;
}

/** @brief VALIDTIME1 ʸ�β���
 *
 * @retval 0 ���ｪλ
 * @retval NUSERR_EOFinDef ͽ�����̥ե���������
 * @retval �� ����¾���顼�ʥ�å�����ɽ���Ѥߡ�
 */
INLINE
int defstmt_validtime1(char *token, nusdef_t *def)
{
	int i;
	if (def->n_vt <= 0) {
		return nus_err((NUSERR_DefOrder, "VALIDTIME must be before "
					"VALIDTIME1 (%#ys)",
					&def->nustype));
	}
	if (def->ft1) {
		return nus_err((NUSERR_DefOrder, "duplicated VALIDTIME1"
					" (%#ys)",
					&def->nustype));
	}
	def->ft1 = nus_malloc(sizeof(int) * def->n_vt);
	if (def->ft1 == NULL) {
		return nus_err((NUSERR_MemShort, "memory shortage"));
	}
	string_upcase(token);
	if (streq(token, "ARITHMETIC")) {
		int	base, step;
		base = atoi(gettoken_safe(def->lexbuffer));
		step = atoi(gettoken_safe(def->lexbuffer));
		for (i = 0; i < def->n_vt; i++) {
			def->ft1[i] = base + step * i;
		}
	} else if (streq(token, "ALL_LIST")) {
		for (i = 0; i < def->n_vt; i++) {
			char *stopper;
			token = gettoken_raw(def->lexbuffer);
			if (token == NULL) {
				return NUSERR_EOFinDef;
			}
			def->ft1[i] = strtol(token, &stopper, 10);
			if (stopper == token) {
				return nus_err((NUSERR_DefMissingVL,
					"invalid validtime1 %s;"
					" %d items missing (%#ys)",
					token, def->n_vt - i, &def->nustype));
			}
		}
	} else {
		return nus_err((NUSERR_Deffile,
					"invalid validtime1 style <%s>"
					" (%#ys)", token, &def->nustype));
	}
	return 0;
}


/** @brief ELEMENT ʸ�β��� */
INLINE int
defstmt_element(char *token, nusdef_t *def)
{
	size_t emapsize;

	def->n_el = atoi(token);
	if (def->n_el <= 0) {
		return nus_err((NUSERR_DefMissingNE,
			"invalid element number <%s> (%#ys)", token,
			&def->nustype));
	}
	def->el = nus_malloc(sizeof(sym8_t) * def->n_el);
	if (def->el == NULL) {
		return nus_err((NUSERR_MemShort, "memory shortage"));
	}
	memset(def->el, 0x00, sizeof(sym8_t) * def->n_el);
	if (def->n_mb == 0) {
		/* member ʸ��礯���Υǥե���Ƚ��� */
		def->n_mb = 1;
		def->mb = nus_malloc(sizeof(sym4_t));
		def->mb[0] = SYM4_ALLSPACE;
	}
	emapsize = def->n_el * def->n_lv *def->n_vt * def->n_mb;
	if (emapsize == 0) {
		if (def->n_vt == 0) {
			return nus_err((NUSERR_DefMissingNV,
						"missing VALIDTIME (%#ys)",
						&def->nustype));
		} else {
			return nus_err((NUSERR_DefMissingNZ,
						"missing PLANE (%#ys)",
						&def->nustype));
		}
	}
	def->elementmap = nus_malloc(emapsize * sizeof(N_UI4));
	if (def->elementmap == NULL) {
		return nus_err((NUSERR_MemShort, "memory shortage"));
	}
	memset(def->elementmap, 0xBE, emapsize * sizeof(N_UI4));
	return 0;
}

/** @brief elementmap �γ����Ǥλ��ȡ�
 *
 * ������ (�����о�) �ˤ������ΤǴؿ�����ʤ��ƥޥ���
 */
#define MAPITEM(def, im, iv, iz, ie) \
	((def)->elementmap[ \
	(((im * (def)->n_vt) \
	 + iv) * (def)->n_lv \
	 + iz) * (def)->n_el \
	 + ie ])

/** @brief elementmap ��1�ԥ������
 *
 * ����ե������ ELEMENTMAP ʸ��˸�����̿��֤�� 0/1 ���ɤ߼�롣
 *
 * @retval 0 ���ｪλ
 * @retval �� ������ɤ��ɤ߼�äƤ��ޤä���硢�������ܾ���
 * @retval �� ���顼�ʥ�å�����ɽ���Ѥߡ�
 */
INLINE int
elementmap_readrow(nusdef_t *def, int im, int iv, int ie)
{
	int iz;
	for (iz = 0; iz < def->n_lv; iz++) {
		char *token;
		enum parse_phase phase;
		/* �ե����������ϤҤȤޤ�̵�뤵��� */
		token = gettoken_safe(def->lexbuffer);
		if (token[0] == '0' && token[1] == '\0') {
			MAPITEM(def, im, iv, iz, ie) = ~(N_UI4)0;
		} else if (token[0] == '1' && token[1] == '\0') {
			MAPITEM(def, im, iv, iz, ie) = 0;
		} else if ((phase = token2phase(token)) != pERR) {
			nus_warn(("<%s> for where 1/0 is expected (%#ys)",
						token, &def->nustype));
			while (iz < def->n_lv) {
				MAPITEM(def, im, iv, iz, ie) = 0;
				iz++;
			}
			return phase;
		} else {
			return nus_err((NUSERR_Deffile,
				"<%s> for where 1/0 is expected (%#ys)",
				token, &def->nustype));
		}
	}
	return 0;
}

/** @brief 2������ elementmap ��ʸ�β���
 *
 * ����� elementmap ʸ���ʤ�Ӥ��軰�� elementmap ʸ�Ρ�2����β���
 *
 * @retval 0 ���ｪλ
 * @retval �� �۾�������
 * @retval �� ���顼�ʥ�å�����ɽ���Ѥߡ�
 */
INLINE int
elementmap_read2(nusdef_t *def, int im, int nm, int ie)
{
	struct lex_buffer *file = def->lexbuffer;
	int iv, iz, jm, jv, nv, r;
	char *token;

	r = 0;
	/* �ޤ� im ���ɤ߹��� */
	nv = 0;
	for (iv = 0; iv < def->n_vt; iv += nv) {
		/* �оݻ�������ɤ߼���Ķ������å� */
		/* �ե����������ϤҤȤޤ�̵�뤵��� */
		token = gettoken_safe(file);
		nv = atoi(token);
		if (nv <= 0) {
			string_upcase(token);
			r = token2phase(token);
			if (r == pERR) {
				return nus_err((NUSERR_BadElementmap,
					"<%s> in elementmap (%#ys)",
					token, &def->nustype));
			} else {
				nus_warn(("%d validtimes missing"
				" in elementmap %Qs (%#ys)",
				def->n_vt - iv,
				def->el[file->elemctr],
				&def->nustype));
			}
		}
		if (iv + nv > def->n_vt) {
			nus_warn(("too many valid times (%d + %d >= %d) "
				"in elementmap (%#ys line %d)",
				iv, nv, def->n_vt,
				&def->nustype, file->line));
		}
		if (r == 0) {
			/* �ޤ������Ƥ���ǽ�� iv ���ɤ߹��� */
			r = elementmap_readrow(def, im, iv, ie);
		} else {
			/* ��Ϥ�ȡ�������ɤ�ǤϤʤ�ʤ��Τ�
			 * �����ꥢ���� */
			for (iz = 0; iz < def->n_lv; iz++) {
				MAPITEM(def, im, iv, iz, ie) = 0;
			}
			/* ��­�ʤ� vt ������
			 * ���δؿ���æ�Ф���褦Ĵ�� */
			nv = def->n_vt - iv;
		}
		/* �Ĥ�� iv �˥��ԡ� */
		for (jv = iv + 1; jv < iv + nv; jv++) {
		for (iz = 0; iz < def->n_lv; iz++) {
			MAPITEM(def, im, jv, iz, ie) =
			MAPITEM(def, im, iv, iz, ie);
		}
		}
	}
	/* �Ĥ�Υ��С� (im + 1, im + nm - 1) �˥��ԡ� */
	for (jm = im + 1; jm < im + nm; jm++) {
	for (jv = 0; jv < def->n_vt; jv++) {
	for (iz = 0; iz < def->n_lv; iz++) {
		MAPITEM(def, jm, jv, iz, ie) =
		MAPITEM(def, im, jv, iz, ie);
	}
	}
	}
	return r;
}

/** @brief �軰�� elementmap ʸ�β���
 *
 * @todo elementmap_read2 ��ƤӽФ��Ȥ����Х��äƤ��뤯������
 */
INLINE
void elementmap_read3(nusdef_t *def, int ie)
{
	struct lex_buffer *file = def->lexbuffer;
	int im = 0;

	/* �ޤ� im ���ɤ߹��� */
	while (im < def->n_mb) {
		int	nm, iv, iz, jm;
		char	*token;
		/* ���С������ɤ߼���Ķ������å� */
		nm = atoi(gettoken_safe(file));
		if (im + nm > def->n_mb) {
			nus_err((1,
			"too many members (%d + %d >= %d)"
			"specified in line %d",
			im, nm, def->n_mb, file->line));
			return;
		}
		switch (atoi(token = gettoken_safe(file))) {
			case 1:
				elementmap_readrow(def, im, 0, ie);
				for (iv = 1; iv < def->n_vt; iv++) {
				for (iz = 0; iz < def->n_lv; iz++) {
					MAPITEM(def, im, iv, iz, ie) =
					MAPITEM(def, im,  0, iz, ie);
				}
				}
				break;
			case 2:
				elementmap_read2(def, im, 1, ie);
				break;
			default:
				nus_err((1,
				"bad style %s (should be 1 or 2)"));
				return;
		}
		for (jm = im + 1; jm < im + nm; jm++) {
		for (iv = 0; iv < def->n_vt; iv++) {
		for (iz = 0; iz < def->n_lv; iz++) {
			MAPITEM(def, jm, iv, iz, ie) =
			MAPITEM(def, im, iv, iz, ie);
		}
		}
		}
		im += nm;
	}
}

/** @brief ELEMENTMAP ʸ�β���
 *
 * @retval 0 ���ｪλ
 * @retval �� ���������
 * @retval ¾���� ���顼�ʥ�å�����ɽ���Ѥߡ�
 * */
INLINE
int defstmt_elementmap(char *token, nusdef_t *def)
{
	struct lex_buffer *file = def->lexbuffer;
	int emapstyle, ie, iz, iv, nz, im;
	int r = 0;

	if (def->n_el <= 0) {
		return nus_err((NUSERR_DefOrder,
					"ELEMENT must be before ELEMENTMAP"
					" (%#ys)", &def->nustype));
	}
	if ((ie = file->elemctr) >= def->n_el) {
		return nus_err((NUSERR_BadElementmap,
		"too many ELEMENTMAP statements for %d elements (%#ys)",
		def->n_el, &def->nustype));
	}
	def->el[file->elemctr] = str2sym8(token);
	emapstyle = atoi(token = gettoken_safe(file));
	switch (emapstyle) {
	case 0:
		/* ���С��оݻ���̤ˤ�餺���٤Ƶ��� */
		nz = def->n_mb * def->n_vt * def->n_lv;
		for (iz = 0; iz < nz; iz++) {
			MAPITEM(def, 0, 0, iz, ie) = 0;
		}
		break;
	case 1:
		/* �̥ꥹ�Ȥ��ɤ߹��ߡ����٤ƤΥ���/�оݻ����Ŭ�� */
		/* �ޤ� im = iv = 0 ���ɤ߹��� */
		r = elementmap_readrow(def, 0, 0, ie);
		/* ���٤Ƥ� im �� iv = 0 �˥��ԡ� */
		for (im = 1; im < def->n_mb; im++) {
		for (iz = 0; iz < def->n_lv; iz++) {
			MAPITEM(def, im, 0, iz, ie) = 
			MAPITEM(def,  0, 0, iz, ie);
		}
		}
		/* ���Ƥ� im ���Ф��� iv = 0 ����¾�� iv �˥��ԡ� */
		for (im = 0; im < def->n_mb; im++) {
		for (iv = 1; iv < def->n_vt; iv++) {
		for (iz = 0; iz < def->n_lv; iz++) {
			MAPITEM(def, im, iv, iz, ie) = 
			MAPITEM(def, im,  0, iz, ie);
		}
		}
		}
		break;
	case 2:
		/* �оݻ����̤˰ۤʤä� readrow ��Ԥ� */
		r = elementmap_read2(def, 0, def->n_mb, ie);
		break;
	case 3:
		/* ���п������֤��� case 1 �ޤ��� case 2 ����� */
		elementmap_read3(def, ie);
		break;
	default:
		return nus_err((NUSERR_Deffile, "invalid elementmap style"
					" <%s> (%#ys)",
					token, &def->nustype));
	}
	if (r > pERR) {
		nus_warn(("elementmap for %.6s is incomplete", token));
	}

	file->elemctr++;
	return r;
}

	INLINE double
strtod_dir(const char *nptr, int assumed_dir, int *direction)
{
	char *endptr;
	double r;
	r = strtod(nptr, &endptr);
	if (nptr == endptr) {
		*direction = 0;
		return 0.0;
	}
	switch (*endptr) {
		case 's':
		case 'S':
			r = -r;
			/* fallthru */
		case 'n':
		case 'N':
			*direction = 'N';
			break;
		case 'w':
		case 'W':
			r = -r;
			/* fallthru */
		case 'e':
		case 'E':
			*direction = 'E';
			break;
		default:
			*direction = assumed_dir;
			break;
	}
	return r;
}

/** @brief �а��٤˴ؤ���ȡ������¤��ؤ��Ȳ���
 *
 * �а��٤Ϥɤ��餫����˻��ꤷ�Ƥ�褤��
 * @retval 0 ���ｪλ
 * @retval �� �۾�ȡ�����ˤ��������
 */
	INLINE int
add_latlon(char *token1, nusdef_t *def, int ofs)
{
	char *token2;
	int dim1, dim2;
	double coord1, coord2;
	coord1 = strtod_dir(token1, 'E', &dim1);
	if (dim1 == 0) {
		nus_warn(("suspicious longitude (%s) for %Qs%Ps%Ps",
				token1, def->nustype.type1,
				def->nustype.type2, def->nustype.type3));
		string_upcase(token1);
		return token2phase(token1);
	}
	token2 = gettoken_raw(def->lexbuffer);
	if (token2 == NULL) {
		return NUSERR_MemShort;
	}
	coord2 = strtod_dir(token2, 'N', &dim2);
	if (dim2 == 0) {
		nus_warn(("suspicious latitude (%s) for %Qs%Ps%Ps",
				token2, def->nustype.type1,
				def->nustype.type2, def->nustype.type3));
		string_upcase(token2);
		return token2phase(token2);
	}
	if (dim1 == 'N' && dim2 == 'E') {
		def->projparam[ofs][0] = coord1;
		def->projparam[ofs][1] = coord2;
	} else if (dim1 == dim2) {
		def->projparam[ofs][0] = coord2;
		def->projparam[ofs][1] = coord1;
		nus_warn(("suspicious geogr coord (%s %s) for %Qs%Ps%Ps",
				token1, token2, def->nustype.type1,
				def->nustype.type2, def->nustype.type3));
	} else {
		def->projparam[ofs][0] = coord2;
		def->projparam[ofs][1] = coord1;
	}
	return 0;
}

/** @brief INFORMATION ʸ�β���
 *
 * information ʸ����ɤ��� def->infotab ����Ƭ���������롣
 *
 * @retval 0 ����
 * @retval NUSERR_MemShort ������­
 */
INLINE int
add_info(nusdef_t *def, char *token)
{
	struct nusdef_subcinfo_t *s;
	sym4_t grp;
	grp = str2sym4(token);
	token = gettoken_raw(def->lexbuffer);
	if (token == NULL) {
		/** �ե�����̾��礯 information ʸ��̵��������
		 * �ºݤ˻Ȥ��뤳�Ȥ�����Τǥ��顼�ˤϤ��ʤ���
		 */
		nus_warn(("missing filename for INFO record (%#ys)",
					&def->nustype));
		return 0;
	}
	/* ������� */
	s = nus_malloc(sizeof(*s));
	if (s == NULL) {
		return nus_err((NUSERR_MemShort, "memory short"));
	}
	/* def->subcinfo ����Ƭ������ */
	s->next = def->infotab;
	def->infotab = s;
	def->n_info++;
	
	s->size = 0;
	s->group = grp;
	s->filename = string_dup(token);
	return 0;
}

/** @brief SUBCNTL ʸ�β���
 *
 * subcntl ʸ����ɤ��� def->subcinfo ����Ƭ���������롣
 * @retval 0 ���ｪλ
 * @retval �� �����ʥȡ����󡢥�����ɤȤ��Ʋ����ߤ����������֡�
 * @retval �� ���顼
 */
INLINE int
add_subc(nusdef_t *def, char *token)
{
	struct nusdef_subcinfo_t *s;
	int n_subc;

	for (n_subc = atoi(token); n_subc; n_subc--) {
		/* ������� */
		s = nus_malloc(sizeof(*s));
		if (s == NULL) {
			return nus_err((NUSERR_MemShort, "memory short"));
		}
		token = gettoken_safe(def->lexbuffer);
		if (strlen(token) > 4) {
			nus_warn(("bad subc name %s (%#ys)", token,
						&def->nustype));
			string_upcase(token);
			return token2phase(token);
		}
		s->group = str2sym4(token);
		s->filename = NULL;
		token = gettoken_safe(def->lexbuffer);
		s->size = atoi(token);
		if (s->size == 0) {
			nus_warn(("bad subc size %s (%#ys)", token,
						&def->nustype));
			string_upcase(token);
			return token2phase(token);
		}
		/* def->subcinfo ����Ƭ������ */
		s->next = def->subctab;
		def->subctab = s;
		def->n_subc++;
	}
	return 0;
}

#if TEST_ELEMENTMAP
void test_elementmap(nusdef_t *def) {
	int ie, iz, iv, im;
	char	*buf, *p;
	nus_debug(("--- elementmap ---"));
	buf = nus_malloc(11 /* header */ + def->n_lv + 16 /* NUL plus safety */);
	for (im = 0; im < def->n_mb; im++) {
	for (iv = 0; iv < def->n_vt; iv++) {
	for (ie = 0; ie < def->n_el; ie++) {
		nus_snprintf(buf, 11, "%02d %02d %02d: ", im, iv, ie);
		p = buf + strlen(buf);
		for (iz = 0; iz < def->n_lv; iz++) {
			switch (MAPITEM(def, im, iv, iz, ie)) {
				case 0:
					*p++ = '1';
					break;
				case ~(N_UI4)0:
					*p++ = '0';
					break;
				case 0xBEBEBEBE:
					*p++ = '.';
					break;
				default:
					*p++ = '#';
					break;
			}
		}
		*p = 0;
		nus_debug(("%s", buf));
	}
	}
	}
	nus_free(buf);
}
#endif

/** @brief ����ե�����β���
 *
 * �ե�������ɤ߹��ޤ줿����ե����� @p file ����Ϥ��� @p def ��
 * ��Ǽ���롣
 * @retval 0 ���ｪλ
 * @retval �� ���顼�ʥ��顼�����ɡ�
 */
int nusdef_parse(nusdef_t *def,
		int hush) /**< type123 �����ꤷ���轪λ����ե饰 */
{
	struct lex_buffer *file;
	enum parse_phase phase = pINIT;
	char *token, *kwd = "(none)";
	unsigned typemask = 0;

	file = def->lexbuffer;

	while (1) {
		if (phase == pINIT) {
			token = gettoken_raw(file);
			if (token == NULL) { /* expected EOF */
				break;
			}
			string_upcase(token);
			phase = token2phase(kwd = token);
			if (phase == pERR) {
				if (*token == '#') {
					nusdef_skipline(file);
				} else {
					nus_warn(("unknown "
						"keyword <%s> at line %d",
						token, file->line));
				}
				phase = pINIT;
			}
			continue;
		}
		token = gettoken_raw(file);
		if (token == NULL)
			goto gUnexpectedEOF;
		switch (phase) {
			int i;
	case pNUSDAS:
			i = atoi(token);
			if (i == 1) {
				def->version = 10;
			} else if (i == 10 || i == 11 || i == 13 || i == 14) {
				def->version = i;
			} else {
				nus_warn(("invalid version code "
					"%s (%d assumed)", token,
					GlobalConfig(pc_filever)));
				def->version = GlobalConfig(pc_filever);
			}
			break;
	case pPATH:
			if (defstmt_path(token, def, 1) != 0) {
				goto gUnexpectedEOF;
			}
			break;
	case pFILENAME:
			string_copy(def->filename, token,
					sizeof(def->filename));
			break;
	case pCREATOR:
			/* �ե�����С������ 13 ������Ȥ���
			 * ���� 72 �Х��Ȥ����񤫤ʤ� */
			string_copy(def->creator, token, 72);
			break;
	case pTYPE1:
			defstmt_type1(token, def);
			if (((typemask |= 1) == 7) && hush)
				return 0;
			break;
	case pTYPE2:
			defstmt_type2(token, def);
			if (((typemask |= 2) == 7) && hush)
				return 0;
			break;
	case pTYPE3:
			def->nustype.type3 = str2sym4(token);
			if (((typemask |= 4) == 7) && hush)
				return 0;
			break;
	case pMEMBER:
			i = defstmt_member(token, def);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i == NUSERR_EOFinDef) {
				goto gUnexpectedEOF;
			} else if (i < 0) {
				return i;
			}
			break;
	case pMEMBERLIST:
			i = defstmt_memberlist(token, def);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i == NUSERR_EOFinDef) {
				goto gUnexpectedEOF;
			} else if (i < 0) {
				return i;
			}
			break;
	case pBASETIME:
			/* ��ȴ�� */
			break;
	case pVALIDTIME:
			def->n_vt = atoi(token);
			token = gettoken_raw(file);
			if ((token == NULL) || (strlen(token) > 4)) {
				return nus_err((NUSERR_Deffile,
	"missing \"units in/out\" for VALIDTIME (%#ys)", &def->nustype));
			}
			string_upcase(token);
			def->ftunits = str2sym4(token);
			if (def->ftunits == SYM4_IN
					|| def->ftunits == SYM4_OUT) {
				def->vt_out = defword_out(token);
				token = gettoken_raw(file);
				if ((token == NULL) || (strlen(token) > 4)) {
					return nus_err((NUSERR_Deffile,
	"missing units for VALIDTIME (%#ys)", &def->nustype));
				}
				string_upcase(token);
				def->ftunits = str2sym4(token);
			} else {
				token = gettoken_safe(file);
				i = defword_out(token);
				if (i < 0) {
					def->vt_out = 0;
					phase = token2phase(token);
					if (phase != pERR) {
						nus_warn(("missing in/out "
	"for VALIDTIME (%#ys)", &def->nustype));
						continue;
					} else {
						return nus_err((NUSERR_Deffile,
	"missing in/out for VALIDTIME (%#ys) %s", &def->nustype, token));
					}
				} else {
					def->vt_out = i;
				}
			}
			break;
	case pVALIDTIME1:
			if (defstmt_validtime1(token, def) != 0) {
				return NUS_ERRNO;
			}
			nusdef_skipline(file);
			break;
	case pVALIDTIME2:
			break;
	case pPLANE:
			def->n_lv = atoi(token);
			break;
	case pPLANE1:
			def->lv1 = nus_malloc(sizeof(sym8_t) * def->n_lv);
			if (def->lv1 == NULL) {
				return nus_err((NUSERR_MemShort,
					"memory shortage"));
			}
			def->lv1[0] = str2sym8(token);
			for (i = 1; i < def->n_lv; i++) {
				token = gettoken_safe(file);
				def->lv1[i] = str2sym8(token);
			}
			break;
	case pPLANE2:
			def->lv2 = nus_malloc(sizeof(sym8_t) * def->n_lv);
			if (def->lv2 == NULL) {
				return nus_err((NUSERR_MemShort,
					"memory shortage"));
			}
			def->lv2[0] = str2sym8(token);
			for (i = 1; i < def->n_lv; i++) {
				token = gettoken_safe(file);
				def->lv2[i] = str2sym8(token);
			}
			break;
	case pELEMENT:
			i = defstmt_element(token, def);
			if (i < 0) {
				return i;
			}
			break;
	case pELEMENTMAP:
			i = defstmt_elementmap(token, def);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i < 0) {
				return i;
			}
			nusdef_skipline(file);
			break;
	case pSIZE:
			def->nx = atoi(token);
			def->ny = atoi(gettoken_safe(file));
			break;
			/** @note BASEPOINT, STANDARD, OTHERS ʸ�ˤ�����
			 * �а��٤򤢤�魯������ lon lat �ν�Ǥ���
			 * nusdef_t ��¤�Ρ�CNTL ��Ͽ��nusdas_inq_def
			 * ����������¤ӤεդˤʤäƤ��뤳�Ȥ���ա�
			 */
	case pBASEPOINT:
			def->projparam[0][0] = atof(token);
			def->projparam[0][1] = atof(gettoken_safe(file));
			token = gettoken_safe(file);
			i = add_latlon(token, def, 1);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i < 0) {
				return i;
			}
			break;
	case pDISTANCE:
			def->projparam[2][0] = atof(token);
			def->projparam[2][1] = atof(gettoken_safe(file));
			break;
	case pSTANDARD:
			i = add_latlon(token, def, 3);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i < 0) {
				return i;
			}
			token = gettoken_safe(file);
			i = add_latlon(token, def, 4);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i < 0) {
				return i;
			}
			break;
	case pOTHERS:
			i = add_latlon(token, def, 5);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i < 0) {
				return i;
			}
			token = gettoken_safe(file);
			i = add_latlon(token, def, 6);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i < 0) {
				return i;
			}
			break;
	case pVALUE:
			def->value = str2sym4(token);
			break;
	case pPACKING:
			def->packing = str2sym4(token);
			break;
	case pMISSING:
			def->missing = str2sym4(token);
			break;
	case pINFORMATION:
			i = add_info(def, token);
			if (i < 0) {
				return i;
			}
			break;
	case pSUBCNTL:
			i = add_subc(def, token);
			if (i > pINIT) {
				phase = i;
				continue;
			} else if (i < 0) {
				return i;
			}
			break;
	case pFORCEDRLEN:
			def->forcedrlen = atol(token);
			break;
	case pOPTIONS:
			if (!string_cat(def->options, token,
						sizeof def->options)) {
				nus_warn(("options > %d bytes",
						sizeof def->options - 1));
			}
			string_cat(def->options, ",", sizeof def->options);
			break;
	default:
			nus_warn(("internal error: wrong phase %d line %d"
						" (%#ys)",
						(int)phase, file->line,
						&def->nustype));
			break;
		}
		phase = pINIT;
	}
#if TEST_ELEMENTMAP
	test_elementmap(def);
#endif
	return 0;

gUnexpectedEOF:
	return nus_err((NUSERR_EOFinDef,
		"unexpected EOF in %s statement (line %d)",
		kwd, file->line));
}

/** @brief �ե����뤫�� lex_buffer �ؤ��ɤ߼��
 */
INLINE struct lex_buffer *
lex_buffer_ini(const char *filename)
{
	struct lex_buffer *file;
	size_t size;
	file = nus_malloc(sizeof(struct lex_buffer));
	if (file == NULL)
		return NULL;
	file->buf = file_read(filename, &size);
	if (file->buf == NULL)
		return NULL;
	file->cursor = file->buf;
	file->stopper = file->buf + size;
	file->line = 1;
	file->elemctr = 0;
	file->just_hit_nr = 0;
	return file;
}

/** @brief lex_buffer �λ񸻳���
 */
INLINE void
lex_buffer_free(struct lex_buffer *file)
{
	nus_free(file->buf);
	nus_free(file);
}

/** @brief ����ե������Ǹ�ޤǲ���
 */
int nusdef_endparse(nusdef_t *def)
{
	int r;
	r = nusdef_parse(def, 0);
	lex_buffer_free(def->lexbuffer);
	/** CREATOR ʸ���ʤ����Ŭ�������Ƥ��䤦 */
	if (def->creator[0] == '\0') {
		string_copy(def->creator, DEFAULT_CREATOR,
				sizeof(def->creator));
	}
	/** �¹Ի����ץ���� GPTH �����ꤵ��Ƥ����� PATH ʸ�������
	 * ����˥ǡ������åȥ��ץ���� IESF �����ꤵ��뤳�Ȥ⤢��
	 */
	if (GlobalConfig(dds_forcedpath)[0]) {
		defstmt_path(GlobalConfig(dds_forcedpath), def, 0);
	}
	/** NWP_PATH_x �����ꤵ��Ƥ����� filename ʸ��̵���� */
	if (def->path[0] == '\007') {
		def->filename[0] = '\0';
	}
	/** ���ڤ� PATH ʸ�����ꤵ��Ƥ��ʤ����ν���� */
	if (def->path[0] == '\0') {
		string_copy(def->path,
			"_model/_attribute/_space/_time/_name/_basetime",
			sizeof(def->path));
		if (def->mb_out) {
			string_cat(def->path, "/_member", sizeof def->path);
		}
		if (def->vt_out) {
			string_cat(def->path, "/_validtime", sizeof def->path);
		}
	}
	/** memberlist ���ߤĤ���ʤ���ä�����ե�����α��޽��� */
	if (def->mb == NULL && def->n_mb) {
		r = NUSERR_DeffileHasNoMemberlist;
	}
	/** PATH ʸ��VARIDTIME ʸ��̷���ǧ��PATH ʸͥ��� */
	if ( (strstr(def->path,     "_validtime") != NULL) ||
         (strstr(def->filename, "_validtime") != NULL) ) {
		/** validtime out */
		if (def->vt_out == 0) {
			nus_warn(("conflict between PATH and VALIDTIME(in)."
				" change to VALIDTIME(out) (%#ys)", &def->nustype));
			def->vt_out= 1;
		}
	} else {
		/** validtime in */
		if (def->vt_out != 0) {
			nus_warn(("conflict between PATH and VALIDTIME(out)."
				" change to VALIDTIME(in) (%#ys)", &def->nustype));
			def->vt_out= 0;
		}
	}
	/** PATH ʸ��MEMBER ʸ��̷���ǧ��PATH ʸͥ��� */
	if ( (strstr(def->path,     "_member") != NULL) ||
         (strstr(def->filename, "_member") != NULL) ) {
		/** member out */
		if (def->mb_out == 0) {
			nus_warn(("conflict between PATH and MEMBER(in) and PATH."
				" change to MEMBER(out) (%#ys)", &def->nustype));
			def->mb_out= 1;
		}
	} else {
		/** member in */
		if (def->mb_out != 0) {
			nus_warn(("conflict between PATH and MEMBER(out) and PATH."
				" change to MEMBER(in) (%#ys)", &def->nustype));
			def->mb_out= 0;
		}
	}
	/* �ݽ� */
	def->lexbuffer = NULL;
	return r;
}

/** @brief ����ե�����β���
 *
 * ����ե������ @p filename �����ɤ߼����ɤ��롣
 */
int nusdef_readfile(
		const char *filename,	/**< ����ե������̾�� */
		nusdef_t *def)		/**< ���ɷ�̤���Ǽ����� */
{
	int r;
	def->lexbuffer = lex_buffer_ini(filename);
	if (def->lexbuffer == NULL) {
		return 1;
	}
	r = nusdef_parse(def, 1);
	return r;
}

/** @brief ����ե����빽¤�Τν������
 *
 * \todo �ǥե���ȵ�ư�ˤĤ��������Խ�ʬ����Ĵ��
 */
int nusdef_init(nusdef_t *def)
{
	memset(def, '\0', sizeof(*def));
	def->version = GlobalConfig(pc_filever);
	def->basetime = -1;
	def->value = SYM4_PVAL;
	def->packing = SYM4_2UPC;
	def->missing = SYM4_NONE;
	return 0;
}

/** @brief ���ɺѤ�����ե����뤫�� CNTL ��Ͽ�����ˡ�����ɤ�����
 */
sym4_t nusdef_projcode(nusdef_t *def)
{
	sym4_t	type1_2d = SYM8_MID56(def->nustype.type1);
	switch (type1_2d) {
		case SYM4_LM:
			if (def->projparam[3][0] >= 0.0f) {
				return SYM4_LMN;
			} else {
				return SYM4_LMS;
			}
		case SYM4_PS:
			if (def->projparam[3][0] >= 0.0f) {
				return SYM4_PSN;
			} else {
				return SYM4_PSS;
			}
		case SYM4_MR:
		case SYM4_ME:
			return SYM4_MER;
		default:
			return type1_2d;
	}
}

/** @brief 2ʸ�����ˡ�����ɤ����
 */
sym4_t nusdef_projcode2(nusdef_t *def)
{
	sym4_t projname3 = nusdef_projcode(def);
	switch (projname3) {
		case SYM4_MER:
			return SYM4_MR;
		default:
			return projname3;
	}
}
