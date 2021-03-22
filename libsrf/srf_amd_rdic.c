/* -------------------------------------------------------------------------- */
/* �A���_�X�a�t�e�q�d����f�R�[�h����v���O����                               */
/* -------------------------------------------------------------------------- */
										/* �T�u���[�`���W                                 Ver.1 1999.09.30 N.Fuji     *//* -------------------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include "nusdas.h"
#include "srf_amedas.h"
#include "nwpl_capi.h"

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief �n�_�ԍ��̎������ʔԂ�T��
 *
 * SRF_AMD_SINFO �\���^�̔z�� @p amd (�n�_�� @p ac ��) ����
 * �n�_�ԍ� @p stn �̒n�_�������߂��Y�� (1�n�܂�) ��Ԃ��B
 *
 * @retval �� �n�_�̎������i�[���� (1�n�܂�)
 * @retval -1 �n�_���݂���Ȃ�
 *
 * <H3>����</H3>
 * <UL>
 * <LI>�n�_��� @p amd_type �͖��������B
 * <LI>�z�񂪒n�_�ԍ����Ƀ\�[�g����Ă��邱�Ƃ�O��ɓ񕪒T�����g���Ă���B
 * </UL>
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ���ォ�瑶�݂����悤�ł��邪
 * �h�L�������g����Ă��Ȃ������B
 * NuSDaS 1.3 �����[�X�ɍۂ��ăh�L�������g�����悤�ɂȂ����B
 */

int
srf_search_amdstn(const SRF_AMD_SINFO * amd, /**< �n�_�����z�� */
		int ac, /**< �n�_�����z��̒��� */
		int stn, /**< �n�_�ԍ� */
		int amd_type UNUSED /**< �n�_��� */)
{
    int i, j, k;
    i = ac / 2;
    j = (i + 1) / 2;
    k = 0;
    while (1) {
	if (stn == (amd + i)->snum)
	    break;
	else if (stn > (amd + i)->snum)
	    i += j;
	else if (stn < (amd + i)->snum)
	    i -= j;
	if (k > 3)
	    return (-1);
	j = (j + 1) / 2;
	if (j == 1)
	    k++;
	if (i > ac - 1)
	    i = ac - 1;
	else if (i < 0)
	    i = 0;
    }
    return (i + 1);
}

/** @brief �A���_�X�n�_�����̓ǂݍ���
 *
 * ���ϐ� NWP_AMDDCD_STDICT ���w���n�_�����t�@�C��
 * (���ϐ�����`���� amddic.txt �ƂȂ�) ����
 * SRF_AMD_SINFO �\���^�̔z�� @p amd �ɃA���_�X�n�_����Ǐo���B
 * �Ǐo�����n�_�͎��� @p btime �ɑ��݂�����̂��I�΂�A
 * ����Ɉ��� @p amd_type �ɂ���Ď��̂悤�Ɍ��肳���B
 * �z�� @p amdnum ���z���ď����o�����Ƃ͂Ȃ��B
 *
 * <DL>
 * <DT>SRF_KANS<DD>����
 * <DT>SRF_ELM4<DD>4�v�f���ϑ����Ă���n�_
 * <DT>SRF_AMEL<DD>���{�b�g�J�ʌv
 * <DT>SRF_AIRP<DD>�q�󊯏�
 * <DT>SRF_YUKI<DD>�ϐ�ϑ��n�_
 * <DT>SRF_ALL<DD>�S�n�_
 * </DL>
 *
 * @retval �� �n�_��
 * @retval -1 �n�_��ʂ��s��
 * @retval -2 ���ʊi�[�z��̒����s��
 * @retval -3 �n�_�����t�@�C�����J���Ȃ�
 *
 * <H3>�Q�l</H3>
 * NAPS8 �ɂ����Ă͒n�_������
 * /grpK/nwp/Open/Const/Pre/Dcd/amddic.txt �ɒu����Ă���B
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ���ォ�瑶�݂����B
 */
int
srf_amd_rdic(SRF_AMD_SINFO * amd /**< �n�_�����i�[�z�� */,
	     int amdnum, /**< �n�_�����z�� */
	     int btime, /**< �T������ (�ʎZ��) */
	     int amd_type /**< �n�_��� */)
{
    FILE *fp;
    char s[100], *env, fname[200], atype, name1[11], name2[15], ws[80];
    int i, cnt, num, ilat[3], ilon[3], height;
    int sy, sm, sd, sh, sf, ey, em, ed, eh, ef, st, et;

    switch (amd_type) {
	    case SRF_KANS:
	    case SRF_ELM4:
	    case SRF_AMEL:
	    case SRF_AMEN:
	    case SRF_AIRP:
	    case SRF_YUKI:
	    case SRF_ALL:
		    break;
	    default:
		    fprintf(stderr, "srf_amd_rdic: your selection type "
				    "<%d> is not defined!\n", amd_type);
		    return -1;
    }
    /* �����t�@�C���̓ǂݍ��� */
    if ((env = getenv("NWP_AMDDCD_STDICT")) != NULL) {
	strcpy(fname, env);
    } else {
	strcpy(fname, "amddic.txt");
    }
    if ((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr, "srf_amd_rdic: amedas dictionary "
		"file (%s) open failed\n", fname);
	return -3;
    }

    cnt = 0;
    while (1) {
	if (fgets(s, 100, fp) == NULL) {
	    break;
	}
	if (s[0] == '#') {
	    continue;
	}
	if (sscanf(s, " %5d %2d.%2d.%1d %3d.%2d.%1d %d  %c %79s",
		   &num, &ilat[0], &ilat[1], &ilat[2], &ilon[0], &ilon[1],
		   &ilon[2], &height, &atype, ws) != 10
	    || sscanf(&s[40], "%10s%14s", name1, name2) != 2) {
	    fprintf(stderr, "amedas dictionary file line error!\n");
	    fprintf(stderr, "%s\n", s);
	    continue;
	}
	if (amd_type != SRF_ALL) {
	    if (amd_type == SRF_KANS && atype != '@')
		continue;
	    if (amd_type == SRF_ELM4 && atype != '#')
		continue;
	    if (amd_type == SRF_AMEL && atype != '\xDB')
		continue;
	    if (amd_type == SRF_AMEN && atype != '!')
		continue;
	    if (amd_type == SRF_AIRP && atype != 'A')
		continue;
	    if (amd_type == SRF_YUKI && atype != '*')
		continue;
	}
	if (sscanf(&s[65], "%4d%2d%2d%2d%2d %4d%2d%2d%2d%2d",
		   &sy, &sm, &sd, &sh, &sf, &ey, &em, &ed, &eh,
		   &ef) != 10) {
	    fprintf(stderr,
		    "srf_amd_rdic: amedas dictionary file line error!\n");
	    fprintf(stderr, ">>>>> %s\n", s);
	    continue;
	}
	st = nwp_ymdhm2seq(sy, sm, sd, sh, sf);
	et = nwp_ymdhm2seq(ey, em, ed, eh, ef);
	if (btime < st || btime > et) {
	    continue;
	}

	(amd + cnt)->snum = num;
	(amd + cnt)->lat =
	    ilat[0] + ((float) ilat[1] + (float) ilat[2] / 10) / 60;
	(amd + cnt)->lon =
	    ilon[0] + ((float) ilon[1] + (float) ilon[2] / 10) / 60;
	if (atype == '@')
	    (amd + cnt)->type = SRF_KANS;
	else if (atype == '#')
	    (amd + cnt)->type = SRF_ELM4;
	else if (atype == '\xDB')
	    (amd + cnt)->type = SRF_AMEL;
	else if (atype == '!')
	    (amd + cnt)->type = SRF_AMEN;
	else if (atype == 'A')
	    (amd + cnt)->type = SRF_AIRP;
	else if (atype == '*')
	    (amd + cnt)->type = SRF_YUKI;
	(amd + cnt)->hh = (float) height;
	if (strncmp(ws, "     ", 5) == 0)
	    (amd + cnt)->wh = 6.4;
	else
	    sscanf(ws, "%f", &((amd + cnt)->wh));
	strcpy((amd + cnt)->name1, name1);
	for (i = strlen(name1); i < 10; i++)
	    (amd + cnt)->name1[i] = ' ';
	strcpy((amd + cnt)->name2, name2);
	for (i = strlen(name2); i < 14; i++)
	    (amd + cnt)->name2[i] = ' ';
	if (++cnt >= amdnum) {
	    fprintf(stderr,
		    "srf_amd_rdic : your srf_amd_sinfo dimension <%d> is insufficient!\n",
		    amdnum);
	    return -2;
	}
    }
    fclose(fp);

    return cnt;
}
