/** \file
 * \brief ����Ū�ǡ�����¤�����
 *
 * ���Υإå������Ѥ�����ѥץ�����¾�ΥС������ؤΰܿ����Ϥʤ���
 */

#ifndef NUSDAS_H
#include "nusdas.h"
#endif

#define INTERNAL_TYPES_H

/** @brief 4ʸ�����ݻ����뤿���������
 *
 * ʸ����ϥޥ���Х��ȥ��������ǳ�Ǽ����롣�ե�����ؤν�жػߡ�
 */
typedef N_UI4	sym4_t;

/** @brief 8ʸ�����ݻ����뤿���������
 *
 * ʸ����ϥޥ���Х��ȥ��������ǳ�Ǽ����롣�ե�����ؤν�жػߡ�
 */
typedef N_UI8	sym8_t;

/** @brief NuSDaS ����
 */
typedef struct {
	sym8_t	type1; /**< ����1 */
	sym4_t	type2; /**< ����2 */
	sym4_t	type3; /**< ����3 */
} nustype_t;

#define nustype_eq(a, b) \
	(  ui8_eq((a).type1, (b).type1) \
	&& ((a).type2 == (b).type2) \
	&& ((a).type3 == (b).type3) )

#define nustype_p_eq(a, b) \
	(  ui8_eq((a)->type1, (b)->type1) \
	&& ((a)->type2 == (b)->type2) \
	&& ((a)->type3 == (b)->type3) )

/** @brief ���̰ʳ��� NuSDaS ���� (basetime..element)
 */
typedef struct nusdims_t {
	N_SI4	basetime;
	sym4_t	member;
	N_SI4	validtime1;
	N_SI4	validtime2;
	sym8_t	plane1;
	sym8_t	plane2;
	sym8_t	element;
} nusdims_t;

/** @brief �ǡ������å�����
 *
 * �ǡ������å�������ꤵ�줦������. �ǡ������å���˥��ԡ��������.
 */
struct nusxds_param_t {
	const N_UI1	*dsp_pc_mask_bit;
	sym4_t	dsp_pc_packing;
	N_SI4	dsp_pc_sizex;
	N_SI4	dsp_pc_sizey;
	/** @brief �񤭹��ߥХåե�Ĺ (kbyte) */
	N_UI4	dsp_pc_wbuffer;
	/** @brief �ɤ߹��ߥХåե�Ĺ (�Х��ȿ������뤿��� 1 �򥷥եȤ����) */
	N_UI4	dsp_pc_rbuffer;
	/** @brief �ե�����򳫤�
	 *
	 * �ǥե���Ȥ� pio_open() �Ǥ��롣
	 * ���ץ��������ˤ�� sio_open(), pio_open(), eio_open() ����
	 * ���ؤ����롣
	 * @note �����ϴؿ��ݥ��󥿤ʤΤ��� io_comm.h ��¸�����ӽ����뤿��
	 * ���ä��Ƥ��롣
	 */
	union nusio_t *(*dsp_io_open)(const char *filename, int flags);
};

#define DynParam(param, name)	((param)->dsp_ ## name)
