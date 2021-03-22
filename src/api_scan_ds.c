/** @file
 * @brief API nusdas_scan_ds() の実装
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

/** @brief データセットの一覧
 *
 * 返却値が負になるまで呼出しを繰り返すと、ライブラリが認識している
 * データセットの一覧が得られる。
 *
 * @retval 0 引数の配列にデータセットの情報が格納された。
 * @retval -1 もうこれ以上データセットは認識されていない。
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 * pnusdas には非公開の nusdas_list_type という関数があり類似の機能を持つ。
 */
	N_SI4
NuSDaS_scan_ds(char type1[8], /**< INTENT(OUT) 種別1 */
		char type2[4], /**< INTENT(OUT) 種別2 */
		char type3[4], /**< INTENT(OUT) 種別3 */
		N_SI4 *nrd) /**< INTENT(OUT) NRD番号 */
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
