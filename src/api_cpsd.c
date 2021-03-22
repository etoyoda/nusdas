#include "config.h"
#include "nusdas.h"
#include "sys_kwd.h"
#include "internal_types.h"
#include <stddef.h>
#include <string.h>
# define NEED_MEM2SYM4
#include "sys_sym.h"
# define NEED_PEEK_N_UI4
#include "sys_endian.h"
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_err.h"
#include "dfile.h"
#include "ndf_codec.h"

/** @brief 2UPP����DATA�쥳���ɤ�2UPC���Ѵ������������ΥХ��ȿ������
 *
 * nusdas_inq_data() ���䤤��碌 N_DATA_CONTENT ��������2UPP�Х������
 * 2UPC���Ѵ��������ΥХ��ȿ����������
 *
 * @retval �� ���ｪλ���ͤ�Ÿ����Х��ȿ�
 * @retval -5 ���Ϥ�2UPP�ǤϤʤ�
 *
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.4 ���ɲä��줿��
 */
N_SI4
NuSDaS_uncpsd_nbytes(const void *pdata /**< �ѥå����줿�Х����� */)
{
	const N_UI1 *rawdata = pdata;
	N_UI4 grid_size, header;
	grid_size = PEEK_N_UI4(rawdata) * PEEK_N_UI4(rawdata + 4);
	
	if ( SYM4_2UPP != MEM2SYM4(rawdata + 8) ) { /* packing */
		return nus_err((-5, "UnCPSD input is not 2UPP"));
	}
	switch( MEM2SYM4(rawdata + 12) ) { /* missing */
		case SYM4_NONE: header = 24; break;
		case SYM4_UDFV: header = 26; break;
		case SYM4_MASK: header = 24 + (grid_size + 7) / 8; break;
	}
	return 2 * grid_size + header;
}

/** @brief 2UPP����DATA�쥳���ɤ�2UPC���Ѵ�
 *
 * nusdas_inq_data() ���䤤��碌 N_DATA_CONTENT ��������2UPP�Х������
 * ���ɤ���2UPC�Х�������Ѵ����롣
 *
 * @retval �� ���ｪλ���ͤ�Ÿ����Х��ȿ�
 * @retval -4 Ÿ������礭�� @p usize ���ǡ����쥳���ɤ����ǿ���꾯�ʤ�
 * @retval -5 ���Ϥ�2UPP�ǤϤʤ�
 * @retval -6 2UPP��Ÿ���˼���
 * 
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.4 ���ɲä��줿��
 */
N_SI4
NuSDaS_uncpsd(const void *pdata /**< �ѥå����줿�Х����� */,
		void *cdata, /**< Ÿ�������� */
		N_SI4 csize /**< Ÿ������������ǿ� */)
{
	const N_UI1 *rawdata = pdata;
	const sym4_t packing = SYM4_2UPC;
	N_UI4 grid_size, header, expand_size;
	
	grid_size = PEEK_N_UI4(rawdata) * PEEK_N_UI4(rawdata + 4);
	if ( SYM4_2UPP != MEM2SYM4(rawdata + 8) ) { /* packing */
		return nus_err((-5, "UnCPSD input is not 2UPP"));
	}
	switch( MEM2SYM4(rawdata + 12) ) { /* missing */
		case SYM4_NONE: header = 24; break;
		case SYM4_UDFV: header = 26; break;
		case SYM4_MASK: header = 24 + (grid_size + 7) / 8; break;
	}
	expand_size = 2 * grid_size + header;
	if ((csize < 0) || (expand_size > csize)) {
		return nus_err((-4, "user buffer %Pu < %Pu elements required",
			csize, expand_size));
	}
	/* nus_decode_cpsd ����3�����ϼ¤��������Ѥ���ʤ� */
	if ( sizeof(N_UI2) * grid_size != nus_decode_cpsd(rawdata + header, (N_UI1 *)cdata + header, grid_size) ){
		return nus_err((-6, "UnCPSD failure"));
	}
	/* copy header */
	memcpy(cdata, pdata, header);
	memcpy4((char *)cdata + 8, (const char *)&packing);
	return expand_size;
}
