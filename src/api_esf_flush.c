/** @file
 * @brief nusdas_esf_flush() �μ���
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
#include "sys_time.h"
#include "sys_err.h"
#include "glb.h"

/** @brief NAPS7��ES�ե�����ν��ϴ�λ
 *
 * <H3>����</H3> nusdas_esf_flush() �� NuSDaS 1.0 ����¸�ߤ��롣
 * @bug ���� NuSDaS 1.3 �Ǥ� ES �򥵥ݡ��Ȥ��Ƥ��ʤ����ᡢ
 * ���δؿ��ϥ��ߡ��Ǥ��롣
 * */
	N_SI4
NuSDaS_esf_flush(const char type1[8] UNUSED, /**< ����1 */
		const char type2[4] UNUSED, /**< ����2 */
		const char type3[4] UNUSED, /**< ����3 */
		const N_SI4 *basetime UNUSED, /**< ������ */
		const char member[4] UNUSED, /**< ���С�̾ */
		const N_SI4 *validtime UNUSED) /**< �оݻ��� */
{
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return 1;
}
