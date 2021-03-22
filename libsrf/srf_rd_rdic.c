/* ************************************************************************** */
/* subroutine for read radar dictionary                                       */
/* ************************************************************************** */
#include <nusdas.h>
#include <nwpl_capi.h>
#include <stdio.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief ���s�܂œǂݔ�΂�
 *
 * �X�g���[�� @p fp ������s�������ǂݎ����܂�1�o�C�g�Âǂݔ�΂��B
 * 
 */

char
srf_new_line(FILE * fp)
{
    /* C ���ꋳ��: fgetc �̌��ʂ������� char �Ŏ󂯂Ă͂Ȃ�Ȃ��B
     * char �������t���Ȃ當���R�[�h 0xFF (DEL) �� EOF �̋�ʂ����Ȃ�
     * �Ȃ邵�Achar �������Ȃ��Ȃ� EOF �����m�ł��Ȃ��Ȃ��Ė������[�v��
     * �Ȃ邩��ł���B
     */
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n')
	continue;
    return c;
}

/** @brief �x���b����x�ւ̊��Z
 *
 * �x���b�`���ŕ\�����ꂽ���� (��\�̌����b�A�S��̌������A���̌����x)
 * ����x�P�ʂ̎�����^����B
 */

float
srf_i2f_latlon(int ll)
{
    int i, j, k;
    i = ll / 10000;
    j = (ll - i * 10000) / 100;
    k = (ll - i * 10000 - j * 100);
    return i + j / 60.0f + k / 3600.0f;
}

/** @brief ���[�_�[�T�C�g���̖₢���킹
 *
 * �t�@�C���� RADAR_DIC �̃��[�_�[�n�_��������
 * ���� @p iseq (�ʎZ���ł����ĒʎZ���łȂ����Ƃɒ���)
 * �ɂ�����n�_�ԍ� @p stnum �̃��[�_�[�T�C�g�̏���Ǐo���B
 * 
 * @retval 1 ����I��
 * @retval 0 �w�肳�ꂽ���[�_�[�T�C�g���݂���Ȃ�����
 * @retval -1 ���[�_�[�n�_�������J���Ȃ�����
 *
 * <H3>��</H3>
 * <UL>
 * <LI>���[�_�[�n�_������ NAPS8 �ł�
 * /grpK/nwp/Open/Const/Vsrf/Dcd/rdrdic.txt �ɒu����Ă���B
 * <LI>NAPS8 ������ libsrf.a �ɂ͌o�x�̂����Ɍ���Ĉܓx��Ԃ��o�O������B
 * </UL>
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ���ォ�烋�[�`�����ɂ͑��݂������A
 * pnusdas ���� NuSDaS 1.1 �Ɏ��� CVS �Ń\�[�X�ɂ͊܂܂�Ă��Ȃ������B
 * NuSDaS 1.3 �ŗ��҂��������ꂽ�B
 */

int
srf_rd_rdic(int stnum, /**< �n�_�ԍ� */
		int iseq, /**< ����(�ʎZ��) */
		float *lat, /**< INTENT(OUT) �ܓx */
		float *lon, /**< INTENT(OUT) �o�x */
		float *hh, /**< INTENT(OUT) ���x */
		N_SI4 *offx, /**< INTENT(OUT) ���S�̃I�t�Z�b�g */
		N_SI4 *offy, /**< INTENT(OUT) ���S�̃I�t�Z�b�g */
		N_SI4 *type1, /**< INTENT(OUT) �f�W�^�����^�C�v */
		N_SI4 *type2 /**< INTENT(OUT) �f�W�^�����^�C�v */)
{
    FILE *fp;
    char line[256], stime[16], etime[16];
    char name[80];
    int num, iseqs, iseqe;
    int iy, im, id, ih, ilat, ilon, ihh;
    int set_flag = 0;

    if ((fp = fopen("RADAR_DIC", "r")) == NULL) {
	fprintf(stderr,
		"srf_rd_rddic: "
		"Can not open radar dictionary file <RADAR_DIC>\n");
	return (-1);
    }
    /* ���[�_�[�n�_�����̏���

....v....1....v....2....v....3....v....4....v....5....v....6....v....7....v
#### stnum      start         end       lat     lon  height offx offy type
SAPP 47415 1999.03.01.00 2999.12.31.23 d30820 1410035  7490   40  140 1 3
%s-> %d--> %s----------> %s----------> %d---> %d---->  %d->   %d  %d> %d%d

     */
    while (fgets(line, sizeof line, fp) != NULL) {
        int nfields, ox, oy, t1, t2;
	nfields = sscanf(line, "%79s%d%15s%15s%d%d%d%d%d%d%d",
				name, &num, stime, etime,
				&ilat, &ilon, &ihh, &ox, &oy, &t1, &t2);
	if (nfields != 11 || name[0] == '#') {
	    continue;
	}
	if (stnum != num) {
	    continue;
	}
	if (sscanf(stime, "%d.%d.%d.%d", &iy, &im, &id, &ih) != 4)
	    break;
	iseqs = nwp_ymdh2seq(iy, im, id, ih);
	if (sscanf(etime, "%d.%d.%d.%d", &iy, &im, &id, &ih) != 4)
	    break;
	iseqe = nwp_ymdh2seq(iy, im, id, ih);
	if (iseq < iseqs || iseq > iseqe) {
	    continue;
	}
	*lat = srf_i2f_latlon(ilat);
	*lon = srf_i2f_latlon(ilon);
	*hh = ihh / 10.0f;
	*offx = ox;
	*offy = oy;
	*type1 = t1;
	*type2 = t2;
	set_flag = 1;
	break;
    }
    fclose(fp);
    return set_flag;
}
