#include "nusdas.h"
#include <stdio.h>
#include <string.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief 2�z��̃N�C�b�N�\�[�g
 *
 * �v�f�� @p n_st_n �̔z�� @p st_n ����єz�� @p d_n
 * (��҂̌^�� NuSDaS �Ɠ��l�� @p t_n �Ŏw������� N_R4, N_I4, �܂��� N_I2)
 * �� @p st_n �̒l�̏��Ƀ\�[�g����B
 *
 * <H3>����</H3>
 * ��L 3 ��ވȊO�̌^���w������ƁA�ق��ĉ������Ȃ��B
 * NuSDaS 1.3 ���O�́AN_I2 ���w��ł��Ȃ������B
 */

void
srf_q_sort(N_SI4 *st_n, int n_st_n, void *d_n, const char *t_n)
{
    int gap, i, j, temp, *id, tid;
    short *sd, tsd;
    float *fd, tfd;
    sd = (short *) d_n;
    id = (int *) d_n;
    fd = (float *) d_n;
    if (strncmp(t_n, N_R4, 2) == 0) {
	for (gap = n_st_n / 2; gap > 0; gap /= 2)
	    for (i = gap; i < n_st_n; i++)
		for (j = i - gap; j >= 0 && st_n[j] > st_n[j + gap];
		     j -= gap) {
		    temp = st_n[j];
		    st_n[j] = st_n[j + gap];
		    st_n[j + gap] = temp;
		    tfd = fd[j];
		    fd[j] = fd[j + gap];
		    fd[j + gap] = tfd;
		}
    } else if (strncmp(t_n, N_I4, 2) == 0) {
	for (gap = n_st_n / 2; gap > 0; gap /= 2)
	    for (i = gap; i < n_st_n; i++)
		for (j = i - gap; j >= 0 && st_n[j] > st_n[j + gap];
		     j -= gap) {
		    temp = st_n[j];
		    st_n[j] = st_n[j + gap];
		    st_n[j + gap] = temp;
		    tid = id[j];
		    id[j] = id[j + gap];
		    id[j + gap] = tid;
		}
    } else if (strncmp(t_n, N_I2, 2) == 0) {
	for (gap = n_st_n / 2; gap > 0; gap /= 2)
	    for (i = gap; i < n_st_n; i++)
		for (j = i - gap; j >= 0 && st_n[j] > st_n[j + gap];
		     j -= gap) {
		    temp = st_n[j];
		    st_n[j] = st_n[j + gap];
		    st_n[j + gap] = temp;
		    tsd = sd[j];
		    sd[j] = sd[j + gap];
		    sd[j + gap] = tsd;
		}
    }
}

/** @brief �����z��̓񕪒T��
 *
 * ���� @p n_st_n �̔z�� @p st_n (�\�[�g����Ă�����̂Ɖ��肷��) ����
 * �l�� @p st_r �Ɉ�v����v�f�����o�����̓Y�� (0 �n�܂�) ��Ԃ��B
 *
 * @retval �� ���t�������v�f�̓Y��
 * @retval -1 �݂���Ȃ�����
 */

int
srf_amd_slct_sub(int st_r, const N_SI4 *st_n, int n_st_n)
{
    int i, j, k;
    i = (n_st_n + 1) / 2;
    j = 0;
    k = (n_st_n + 1) / 2;
    while (1) {
	if (st_r == st_n[i])
	    break;
	if (j == 2) {
	    i = -1;
	    break;
	} else
	    k = (k + 1) / 2;
	if (k == 1)
	    j++;
	if (st_r > st_n[i])
	    i += k;
	else
	    i -= k;
	if (i >= n_st_n)
	    i = n_st_n - 1;
	else if (i < 0)
	    i = 0;
    }
    return (i);
}

/** @brief �A���_�X�f�[�^���w��̒n�_�ԍ����ɕ��ׂ�
 *
 * ���� @p n_st_n �̒n�_�ԍ��z�� @p st_n ��
 * �Ή����鏇�ɕ��� @p t_n �^�̔z�� @p d_n ����A
 * �ʂ̒n�_�ԍ��z�� @p st_r (�v�f�� @p n_st_r ��) �̏��ɕ���
 * @p t_r �^�̔z�� @p d_r (�v�f�� @p n_st_r ��) �����B
 *
 * �z�� @p st_n �� @p d_n �����炩���߃\�[�g����Ă���ꍇ @p sort_f ��
 * N_OFF (nusdas.h �Œ�`�����) ���w�肷��B
 * �����łȂ��ꍇ @p sort_f �� N_ON ���w�肷��ƃ\�[�g�����B
 *
 * @retval 0 �z�� @p st_r �̑S�n�_�����t������
 * @retval �� �݂���Ȃ������n�_��
 *
 * <UL>
 * <LI>�^�� nusdas.h �Œ�`����� N_R4, N_I4, N_I2 �̂����ꂩ�Ŏw�肷��B
 * <LI>�z�� @p st_r �Ɋ܂܂��n�_�ԍ��� @p st_n �Ō��t����Ȃ��ꍇ��
 * nusdas.h �Œ�`����錇���l N_MV_R4, N_MV_SI4, N_MV_SI2 ������B
 * </UL>
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ���ォ�瑶�݂����B
 */

int
srf_amd_slct(const N_SI4 *st_r, /**< ���ʂ̏����w������n�_�ԍ��\ */
		int n_st_r, /**< ���ʔz�� */
		void *d_r, /**< INTENT(OUT) ���ʔz�� */
		const char *t_r, /**< ���ʔz��̌^ */
		N_SI4 *st_n, /**< ���f�[�^�n�_�ԍ��z�� */
		int n_st_n, /**< ���f�[�^�z�� */
		void *d_n, /**< ���f�[�^�z�� */
		const char *t_n, /**< ���f�[�^�z��̌^ */
		int sort_f /**< ���\�[�g�t���O */)
{
    int i, j, k = 0;
    short *sd_r, *sd_n;
    int *id_r, *id_n;
    float *fd_r, *fd_n;

    if (sort_f == N_ON)
	srf_q_sort(st_n, n_st_n, d_n, t_n);

    sd_r = (short *) d_r;
    id_r = (int *) d_r;
    fd_r = (float *) d_r;
    sd_n = (short *) d_n;
    id_n = (int *) d_n;
    fd_n = (float *) d_n;

    if (strncmp(t_r, N_R4, 2) == 0) {
	if (strncmp(t_n, N_R4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		fd_r[j] = (i == -1) ? N_MV_R4 : fd_n[i];
	    }
	} else if (strncmp(t_n, N_I4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		fd_r[j] = (i == -1) ? N_MV_R4 : (float) id_n[i];
	    }
	} else if (strncmp(t_n, N_I2, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		fd_r[j] = (i == -1) ? N_MV_R4 : (float) sd_n[i];
	    }
	}
    } else if (strncmp(t_r, N_I4, 2) == 0) {
	if (strncmp(t_n, N_R4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		id_r[j] = (i == -1) ? N_MV_SI4 : (int) fd_n[i];
	    }
	} else if (strncmp(t_n, N_I4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		id_r[j] = (i == -1) ? N_MV_SI4 : id_n[i];
	    }
	} else if (strncmp(t_n, N_I2, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		id_r[j] = (i == -1) ? N_MV_SI4 : (int) sd_n[i];
	    }
	}
    } else if (strncmp(t_r, N_I2, 2) == 0) {
	if (strncmp(t_n, N_R4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		sd_r[j] = (i == -1) ? N_MV_SI2 : (short) fd_n[i];
	    }
	} else if (strncmp(t_n, N_I4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		sd_r[j] = (i == -1) ? N_MV_SI2 : (short) id_n[i];
	    }
	} else if (strncmp(t_n, N_I2, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		sd_r[j] = (i == -1) ? N_MV_SI2 : sd_n[i];
	    }
	}
    }
    return (k);
}
