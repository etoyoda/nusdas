/** @file
 * @brief ���߽���
 */
#include "config.h"
#include <stddef.h>
#include "internal_types.h"
#include "dset.h"
#include "glb.h"
#include "sys_err.h"

/** @brief �ҤȤĤΥǡ������åȤ����ץ꥽��������
 *
 * ̵���ˤ��ʤ���Фʤ�ʤ��櫓�ǤϤʤ��Τǡ�
 * ����ѥ������ǥ��顼��������Хǡ������å�õ������ߤ�����
 */
static int
Compact_Dataset(union nusdset_t *ds, void *arg UNUSED)
{
	return ds_compact(ds);
}

/** @brief ����ʤ��񸻤�������
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
