#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include <fcntl.h>
#include "dset.h"
#include "dfile.h"
#include "io_comm.h"
#include "sys_err.h"

	int
nusxds_write_grid(union nusdset_t *ds, nusdims_t *dim,
		const char proj[4],
		const N_SI4 size[2],
		const float projparam[14],
		const char value[4])
{
	union nusdfile_t *df;
	int r, rr;
	df = ds_findfile(ds, dim, IO_READWRITE);
	if (df == NULL) {
		return NUS_ERR_CODE();
	}
	r = df_write_grid(df, proj, size, projparam, value);
	rr = df_close(df);
	return r ? r : rr;
}
