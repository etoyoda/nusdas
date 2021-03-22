/** @file
 * @brief ごみ拾い
 */
#include "config.h"
#include <stddef.h>
#include "internal_types.h"
#include "dset.h"
#include "glb.h"
#include "sys_err.h"

/** @brief ひとつのデータセットの不要リソースを開放
 *
 * 無理にやらなければならないわけではないので、
 * コンパクションでエラーが起こればデータセット探索を中止させる
 */
static int
Compact_Dataset(union nusdset_t *ds, void *arg UNUSED)
{
	return ds_compact(ds);
}

/** @brief いらない資源を開放する
 */
	int
nusglb_garbage_collect(void)
{
	static volatile int active = 0;
	int r;
	if (active++) {
		nus_debug(("looping"));
		return -1;
	}
	nus_warn(("garbage collecting"));
	r = nusglb_dsscan(Compact_Dataset, 0);
	active = 0;
	return r;
}
