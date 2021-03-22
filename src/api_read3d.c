#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_kwd.h"
#include "sys_err.h"
#include "glb.h"

/** @brief �⼡���ɤ߹���
 */
	N_SI4
NuSDaS_read_3d(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������ */
		const char member[][4], /**< ����̾������ */
		const N_SI4 validtime[], /**< �оݻ�������� */
		const char plane[][6], /**< ��̾������ */
		const char element[][6], /**< ����̾������ */
		const N_SI4 *nrecs, /**< �쥳���ɿ� */
		void *udata, /**< INTENT(OUT) �ǡ������� */
		const char utype[2], /**< �ǡ�������η� */
		const N_SI4 *usize) /**< �쥳���ɤ��������ǿ� */
{
	N_SI4 i, rsum;
	size_t recsize;
	NUSDAS_INIT;
	switch (mem2sym4(utype)) {
		case SYM4_I1:
			recsize = *usize;
			break;
		case SYM4_I2:
			recsize = 2 * *usize;
			break;
		case SYM4_I4:
		case SYM4_R4:
			recsize = 4 * *usize;
			break;
		case SYM4_R8:
			recsize = 8 * *usize;
			break;
		default:
			i = nus_err((NUSERR_RD_NoCodec,
				"unsupported array type %.2s", utype));
			NUSDAS_CLEANUP;
			return i;
			break;
	}
	rsum = 0;
	for (i = 0; i < *nrecs; i++) {
		N_SI4 r;
		r = nusdas_read(type1, type2, type3, basetime,
				member[i], &validtime[i], plane[i],
				element[i], (char *)udata + i * recsize,
				utype, usize);
		if (r != *usize) {
			NUSDAS_CLEANUP;
			return r;
		}
		rsum += r;
	}
	NUSDAS_CLEANUP;
	return rsum;
}
