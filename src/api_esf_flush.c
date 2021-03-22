/** @file
 * @brief nusdas_esf_flush() の実装
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
#include "sys_time.h"
#include "sys_err.h"
#include "glb.h"

/** @brief NAPS7型ESファイルの出力完了
 *
 * <H3>履歴</H3> nusdas_esf_flush() は NuSDaS 1.0 から存在する。
 * @bug 現在 NuSDaS 1.3 では ES をサポートしていないため、
 * この関数はダミーである。
 * */
	N_SI4
NuSDaS_esf_flush(const char type1[8] UNUSED, /**< 種別1 */
		const char type2[4] UNUSED, /**< 種別2 */
		const char type3[4] UNUSED, /**< 種別3 */
		const N_SI4 *basetime UNUSED, /**< 基準時刻 */
		const char member[4] UNUSED, /**< メンバー名 */
		const N_SI4 *validtime UNUSED) /**< 対象時刻 */
{
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return 1;
}
