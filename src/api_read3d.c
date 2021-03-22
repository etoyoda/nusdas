#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_kwd.h"
#include "sys_err.h"
#include "glb.h"

/** @brief 高次元読み込み
 */
	N_SI4
NuSDaS_read_3d(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻 */
		const char member[][4], /**< メンバ名の配列 */
		const N_SI4 validtime[], /**< 対象時刻の配列 */
		const char plane[][6], /**< 面名の配列 */
		const char element[][6], /**< 要素名の配列 */
		const N_SI4 *nrecs, /**< レコード数 */
		void *udata, /**< INTENT(OUT) データ配列 */
		const char utype[2], /**< データ配列の型 */
		const N_SI4 *usize) /**< レコードあたり要素数 */
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
