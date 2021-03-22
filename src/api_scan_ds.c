/** @file
 * @brief API nusdas_scan_ds() �μ���
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
# define NEED_MEMCPY4
# define NEED_MEMCPY8
#include "sys_string.h"
#include "sys_err.h"
#include "sys_time.h"
#include "dset.h"
#include "glb.h"

/** @brief �ǡ������åȤΰ���
 *
 * �ֵ��ͤ���ˤʤ�ޤǸƽФ��򷫤��֤��ȡ��饤�֥�꤬ǧ�����Ƥ���
 * �ǡ������åȤΰ����������롣
 *
 * @retval 0 ����������˥ǡ������åȤξ��󤬳�Ǽ���줿��
 * @retval -1 �⤦����ʾ�ǡ������åȤ�ǧ������Ƥ��ʤ���
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 * pnusdas �ˤ�������� nusdas_list_type �Ȥ����ؿ�����������ε�ǽ����ġ�
 */
	N_SI4
NuSDaS_scan_ds(char type1[8], /**< INTENT(OUT) ����1 */
		char type2[4], /**< INTENT(OUT) ����2 */
		char type3[4], /**< INTENT(OUT) ����3 */
		N_SI4 *nrd) /**< INTENT(OUT) NRD�ֹ� */
{
	union nusdset_t *ds;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	ds = nusglb_dsscan2();
	if (ds == NULL) {
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return -1;
	}
	memcpy8(type1, (const char *)&ds->comm.nustype.type1);
	memcpy4(type2, (const char *)&ds->comm.nustype.type2);
	memcpy4(type3, (const char *)&ds->comm.nustype.type3);
	*nrd = ds->comm.nrd;
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return 0;
}
