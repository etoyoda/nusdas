#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <nusdas.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

static void
fill_lowercase_upto4(char dest[5], const char *source)
{
    int i;
    for (i = 0; i < 4; i++) {
	int c = ((unsigned char *) source)[i];
	dest[i] = tolower(c);
	if (c == '\0') {
	    break;
	}
    }
    dest[4] = '\0';		/* failsafe */
}

/** @brief �������烌�x���l�ւ̕ϊ�
 *
 * �z�� @p fdat �̎����f�[�^�����x���l @p idat �ɕϊ����A
 * ISPC �z�� @p ispec �Ƀ��x����\�l���Z�b�g����B
 * �ϊ��e�[�u���Ƃ��ăt�@�C�� ./SRF_LV_TABLE/param.ltb ��ǂށB
 * ������ @p param �͕ϊ��e�[�u���� (�Œ� 4 ��) �ł���B
 *
 * @retval -1 �ϊ��e�[�u�����J�����Ƃ��ł��Ȃ�
 * @retval -2 �ϊ��e�[�u���� 256 �ȏ�̃��x�����w�肳��Ă���
 * @retval �� �ϊ��ɐ��������B�ԋp�l�̓��x����
 * 
 * <H3>��</H3>
 * <UL>
 * <LI>
 * NAPS7 �ł͕ϊ��e�[�u���Ƃ���
 * her ier prr pmf srr srf pr2
 * ��p�ӂ��Ă����B
 * NAPS8 �ł� <BR>
 * /grpK/nwp/Open/Const/Vsrf/Comm/lvtbl.txd �ȉ���
 * her ie2 ier kor pft pm2 pmf pr2 prr sr1 sr2 sr3 srf srj srr yar yrr
 * ���u����Ă���B
 * ���[�`���W���u�ł͂��̃f�B���N�g���փV���{���b�N�����N SRF_LV_TABLE
 * �𒣂��ė��p����B
 * <LI>
 * �ϊ��e�[�u������ ie2, kor, pft �̂Ƃ�,
 * ISPEC �ɂ͕ϊ��e�[�u���ɏ����ꂽ��\�l�� 1/10 ���������B
 * ����ȊO�̏ꍇ�͕ϊ��e�[�u���̑�\�l�����̂܂܏������B
 * <LI>
 * �ϊ��e�[�u������ sr2 �܂��� srj �̂Ƃ��͎����f�[�^�� -900.0 ��菬����
 * ���̂������l�Ƃ݂Ȃ����B�����łȂ���΁A���l�������l�Ƃ݂Ȃ����
 * (NAPS7 �̃}�j���A���ł͌����l�� -1 ���w�肷�邱�ƂƂ���Ă���)�B
 * <LI>
 * �ϊ��e�[�u������ srj �̂Ƃ��́A�����f�[�^���ϊ��e�[�u���̉����l��
 * ���m�Ɉ�v���Ȃ��ƍł���̊K�� (��̓I�ɂ� 42 �� 21.0���Ӗ�����)
 * �Ɋ��蓖�Ă���B���̋����̓o�O��������Ȃ��B
 * <LI>
 * �ϊ��e�[�u���� 191 �s�ȏ㏑����Ă���Ƃ��A
 * �ŏ��� 190 �s�������p�����A���x���l�� 0..190 �ƂȂ邪�A
 * �ԋp�l�ɂ͎��ۂ̃��x���� (�ϊ��e�[�u���̍s�� + 1) ���Ԃ����B
 * ����� ispec �̔z����I�[�o�[�t���[���Ȃ����߂ł���B
 * </UL>
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ���ォ�瑶�݂����B
 * Fortran ���b�p�[��������̒�����`���Ȃ��o�O�� NuSDaS 1.3 �ŉ��������B
 */
int
srf_lv_set(N_SI4 idat[],		/**< INTENT(OUT) ���x���l�i�[�z�� */
	   const float fdat[],	/**< �ϊ����f�[�^�z�� */
	   int dnum,	  	/**< �f�[�^�z��v�f�� */
	   N_SI4 ispec[],	        /**< �V ISPC */
	   const char *param    /**< �ϊ��e�[�u���� */)
{
    char fname[256], s1[80], s2[80];
    char line[256], s3[80];
    char param_lc[5];
    short rep_v[256], nlevs, nlevs_real, lv0, *sp;
    int i, j, order;
    float low_v[256], w;
    FILE *fp;
    fill_lowercase_upto4(param_lc, param);
    sprintf(fname, "./SRF_LV_TABLE/%s.ltb", param_lc);
    if ((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr,
		"srf_lv_set: level table file <%s> can not be opened\n",
		fname);
	return (-1);
    }
    if (strcmp(param, "ie2") == 0 || strcmp(param, "kor") == 0
		    || strcmp(param, "pft") == 0) {
	order = 1;
    } else {
	order = 10;
    }
    nlevs = 0;
    while (fgets(line, sizeof line, fp)) {
	/* scanf �̎g����
	 * %s �͋󔒋�؂�̕������ǂގw���Ȃ̂�
	 * "%s %s" �̂悤�ɊԂɃX�y�[�X�����Ȃ��Ă悢�B
	 * %79s �Ə����Ȃ��� 80 �����ȏ㏑����Ă���Ɣz��N�Q���N�����B
	 * */
	if (sscanf(line, "%79s%79s%79s", s1, s2, s3) < 3) {
   	    continue;
	}
	if (s1[0] == '#') {
	    continue;
	}
	low_v[nlevs] = (float)atoi(s2);
	rep_v[nlevs] = atoi(s3) / order;
	if (++nlevs >= 256) {
	    fprintf(stderr,
		    "srf_lv_set: number of level for %s exceeds 256\n",
		    param);
	    return (-2);
	}
    }
    fclose(fp);
    /* nlevs_real �̓t�@�C������ǂݎ�������x�������̂���
     * ispec[33] ���� ispec[127] �ɂ� (128 - 33) * 4 / 2 = 190 ����
     * �����Ȃ��̂� nlevs �� 191 ������Ƃ��� (1 �� lv0 �p)
     */
    nlevs++;
    nlevs_real = nlevs;
    if (nlevs > 191) {
	nlevs = 191;
    }

    if (strcmp(param, "sr2") == 0) {
	sp = (short *) (&ispec[32]);
	memcpy(sp, &nlevs, sizeof(short));
	lv0 = -1;
	memcpy((short *) (sp + 1), &lv0, sizeof(short));
	memcpy((short *) (sp + 2), rep_v, sizeof(short) * (nlevs - 1));
	for (i = 0; i < dnum; i++) {
	    w = fdat[i] * 100;
	    if (w < -90000)
		idat[i] = 0;
	    else {
		for (j = 0; j < 63; j++)
		    if ((w == low_v[j] && w == low_v[j + 1]) ||
			(w >= low_v[j] && w < low_v[j + 1])) {
			idat[i] = j + 1;
			break;
		    }
		if (j == 63) {
		    if (w >= low_v[j])
			idat[i] = j + 1;
		    else {
			for (j = 63; j < 127; j++)
			    if ((w == low_v[j] && w == low_v[j + 1]) ||
				(w >= low_v[j + 1] && w < low_v[j])) {
				idat[i] = j + 2;
				break;
			    }
			if (j == 127)
			    idat[i] = 127;
		    }
		}
	    }
	}
    } else if (strcmp(param, "srj") == 0) {
	sp = (short *) (&ispec[32]);
	memcpy(sp, &nlevs, sizeof(short));
	lv0 = -1;
	memcpy((short *) (sp + 1), &lv0, sizeof(short));
	memcpy((short *) (sp + 2), rep_v, sizeof(short) * (nlevs - 1));
	for (i = 0; i < dnum; i++) {
	    w = fdat[i] * 100;
	    if (w < -90000)
		idat[i] = 0;
	    else {
		for (j = 0; j < nlevs - 1; j++)
		    if (w == low_v[j]) {
			idat[i] = j + 1;
			break;
		    }
		if (j == nlevs - 1) {
		    idat[i] = j;
		}
	    }
	}
    } else {
	low_v[nlevs - 1] = FLT_MAX;
	sp = (short *) (&ispec[32]);
	memcpy(sp, &nlevs, sizeof(short));
	lv0 = -1;
	memcpy((short *) (sp + 1), &lv0, sizeof(short));
	memcpy((short *) (sp + 2), rep_v, sizeof(short) * (nlevs - 1));
	for (i = 0; i < dnum; i++) {
	    w = fdat[i] * 100;
	    if (w < 0)
		idat[i] = 0;
	    else {
		for (j = 0; j < nlevs - 1; j++) {
		    if ((w == low_v[j] && w == low_v[j + 1]) ||
			(w >= low_v[j] && w < low_v[j + 1])) {
			idat[i] = j + 1;
			break;
		    }
		}
		/* ���� if �߂̓��[�`���łł̓R�����g�A�E�g����Ă��邪�A
		 * w == FLT_MAX �̎��͂���ς肱���ɗ����� */
		if (j == nlevs - 1) {
		    idat[i] = j;
		}
	    }
	}
    }
    endian_swab2(&ispec[32], (128 - 32) * 2);
    endian_swab4(&ispec[32], (128 - 32));
    return nlevs_real;
}

/** @brief ���x���l�������f�[�^ (��\�l) �ɕϊ�
 *
 * �V ISPEC �z�� @p ispec �ɂ���������
 * �z�� @p idat �̃��x���l���\�l @p fdat �ɕϊ�����B
 *
 * <H3>�ԋp�l</H3>
 * �s���l�ȊO�ƂȂ����f�[�^�̗v�f��
 *
 * <H3>��</H3>
 * <UL>
 * <LI>
 * �s���l�� -1 �ƂȂ�B
 * �������AISPEC �̃f�[�^��� (�擪4�o�C�g) ��
 * SRR2, SRF2, SRRR, SRFR �̏ꍇ�Ɍ��� -9999.0 �ƂȂ�B
 * <LI>
 * ISPEC �̃��x���\�͒ʏ� 0.1mm �P�ʂƉ��߂����B
 * �������AISPEC �̐擪 3 �o�C�g�� `<TT>IER</TT>' �ł��邩�A
 * ���邢�� ISPEC �̐擪���� 4 �o�C�g�ڂ� `<TT>1</TT>' �̂Ƃ���
 * 0.01mm �P�ʂƉ��߂����B
 * </UL>
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ���ォ�瑶�݂����B
 */
int
srf_lv_trans(const N_SI4 idat[], /**< ���̓f�[�^ */
	     float fdat[],     /**< INTENT(OUT) ���ʊi�[�z�� */
	     N_SI4 dnum,	       /**< �f�[�^�v�f�� */
	     const N_SI4 ispec[] /**< ISPEC �z�� */)
{
    int i, lnum, rt = 0;
    short *sdat;
    float fdat_0, rev_order;
    unsigned char *cdat;
    int tmp_ispec[128];

    memcpy(tmp_ispec, ispec, 128 * sizeof(int));
    endian_swab4(&tmp_ispec[32], (128 - 32));
    endian_swab2(&tmp_ispec[32], (128 - 32) * 2);
    endian_swab4(&tmp_ispec[0], 1);
    sdat = (short *) (&tmp_ispec[32]);
    lnum = *sdat;
    cdat = (unsigned char *) (&tmp_ispec[0]);
    /* "1" in 4th byte of ispec[0] means order=100 */
    rev_order = ((memcmp(cdat, "IER", 3) == 0 && lnum > 20) || (cdat[3] == '1')) ? 0.01f : 0.1f;
    fdat_0 = (memcmp(cdat, "SRR2", 4) == 0 ||
	      memcmp(cdat, "SRF2", 4) == 0 ||
	      memcmp(cdat, "SRRR", 4) == 0 ||
	      memcmp(cdat, "SRFR", 4) == 0) ? -9999.0f : -1.0f;
    for (i = 0; i < dnum; i++) {
	if (idat[i] == 0) {
		fdat[i] = fdat_0;
	        rt++;
	} else if (idat[i] > 0 && idat[i] < lnum) {
	    fdat[i] = (float) (*(sdat + idat[i] + 1)) * rev_order;
	    rt++;
	} else {
	    fprintf(stderr,
		    "srf_lv_trans: input data level <%d> is out of range\n",
		    idat[i]);
	    fprintf(stderr,
		    "No.%-d output data is set to missing value -1\n", i);
	    fdat[i] = -1.0f;
	}
    }
    return (rt);
}

/** @brief ���x���l�����\�l�ւ̕ϊ�
 *
 * �z�� @p idat �̃��x���l���\�l @p fdat �ɕϊ�����B
 * �ϊ��e�[�u���Ƃ��ăt�@�C�� ./SRF_LV_TABLE/param.ltb ��ǂށB
 * ������ @p param �͕ϊ��e�[�u���� (�Œ� 4 ��) �ł���B
 *
 * @retval -1 �ϊ��e�[�u�����J�����Ƃ��ł��Ȃ�
 * @retval -2 �ϊ��e�[�u���� 256 �ȏ�̃��x�����w�肳��Ă���
 * @retval �� �ϊ��ɐ����B�ԋp�l�͕s���l�ȊO�ƂȂ����f�[�^�̗v�f��
 *
 * <H3>��</H3>
 * <UL>
 * <LI>�s���l�� -1 �ƂȂ�B
 * <LI>
 * NAPS8 �ł͕ϊ��e�[�u���Ƃ���
 * /grpK/nwp/Open/Const/Vsrf/Comm/lvtbl.txd �ȉ���
 * her ie2 ier kor pft pi10 pm2 pmf pr2 prr rr60
 * sr1 sr2 sr3 srf srj srr yar yrr
 * ���u����Ă���B
 * ���[�`���W���u�ł͂��̃f�B���N�g���փV���{���b�N�����N SRF_LV_TABLE
 * �𒣂��ė��p����B
 * <LI>
 * ��L�ϊ��e�[�u���̂����A
 * pi10 �� rr60 ��1�s�Ƀ��x���l�Ƒ�\�l��2�񂪏�����Ă���A
 * ���̑��̓��x���l�A�ŏ��l�A��\�l��3�񂪏�����Ă��邪�A
 * �{�T�u���[�`���͂ǂ���ɂ��Ή����Ă���B
 * </UL>
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ����ɂ͑��݂��Ȃ������悤�ł���B
 * ���[�_�[���쐬���u�Ɋ֘A���ĊJ�����ꂽ�ƍl�����Ă��邪�A
 * NuSDaS 1.3 �ȑO�ɂ͂�����ƃ����e�i���X����Ă��Ȃ������B
 */
int
rdr_lv_trans(N_SI4 idat[], /**< ���̓f�[�^ */
		float fdat[], /**< INTENT(OUT) ���ʊi�[�z�� */
		int dnum, /**< �f�[�^�v�f�� */
		const char *param /**< �e�[�u���� */)
{
    int i, rt = 0;
    short rep_v[256], k;
    char fname[256], s1[80], s2[80];
    char line[256], s3[80];
    char param_lc[5];
    FILE *fp;
    fill_lowercase_upto4(param_lc, param);
    sprintf(fname, "./SRF_LV_TABLE/%s.ltb", param_lc);
    if ((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr,
		"rdr_lv_set: level table file <%s> can not be opened\n",
		fname);
	return (-1);
    }
    k = 0;
    while (fgets(line, sizeof line, fp)) {
	int nwords;
	nwords = sscanf(line, "%79s%79s%79s", s1, s2, s3);
	if (nwords < 2 || s1[0] == '#') {
	    continue;
	}
	if (nwords >= 3) {
	    rep_v[k] = atoi(s3);
	} else {
	    rep_v[k] = atoi(s2);
	}
	if (++k >= 256) {
	    fprintf(stderr,
		    "srf_lv_set: number of level for %s exceeds 256\n",
		    param);
	    return (-2);
	}
    }
    fclose(fp);

    for (i = 0; i < dnum; i++) {
	if (idat[i] < 0 || idat[i] > k - 1) {
	    if (idat[i] != 255) {
		fprintf(stderr,
			"srf_lv_trans: input data level <%d> is out of range\n",
			idat[i]);
		fprintf(stderr,
			"No.%-d output data is set to missing value -1\n",
			i);
	    }
	    fdat[i] = -1;
	} else {
	    fdat[i] = (float) rep_v[idat[i]] / 100;
	    rt++;
	}
    }
    return (rt);
}
