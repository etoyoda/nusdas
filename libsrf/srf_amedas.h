/* -------------------------------------------------------------------------- */
/* �A���_�X�n�_�����Ǎ��T�u���[�`���p                                         */
/* -------------------------------------------------------------------------- */
/* �w�b�_�[�t�@�C��                               Ver.1 2000.11.09 N.Fuji     */
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>

typedef struct amd_data {
    N_SI4 snum, type;		/* �n�_�ԍ��A�n�_��� */
    float lat, lon;		/* �ܓx�A�o�x         */
    float hh, wh;		/* �n�_�W���A�����v�̍��� */
    char name1[10], name2[14];	/* �n�_���J�^�J�i�A���� */
} SRF_AMD_SINFO;		/* �P�n�_�f�[�^�\���� */

#define SRF_KANS  0
#define SRF_ELM4  1
#define SRF_AMEL  2
#define SRF_AMEN  3
#define SRF_AIRP  4
#define SRF_YUKI  5
#define SRF_ALL  10

#ifdef __GNUC__
# define UNUSED __attribute__((unused))
#else
# define UNUSED
#endif
