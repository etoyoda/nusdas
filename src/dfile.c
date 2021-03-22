#include "config.h"
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>
#include "internal_types.h"
#include "sys_err.h"
#include "sys_kwd.h"
#include "dset.h"
#include "dfile.h"
#include "glb.h"
#include "sys_endian.h"
# define NEED_LONG_TO_SI8
#include "sys_int.h"
#include "io_comm.h"

/** @brief データファイルを開く */
	nusdfile_t *
df_open(const char *filename, int flags, struct dds_t *dds, const nusdims_t *dim)
{
	nusio_t	*io;
	N_SI8 ofs, size;
	net4_t	*nusd;
	io = (DynamicParam(&dds->comm.param, io_open))(filename, flags);
	if (io == NULL) {
		if(!(flags & IO_ERRMSG_OFF)){
			nus_err((((flags & IO_WRITABLE)
				? NUSERR_CreatFailed : NUSERR_OpenRFailed),
				"open(%s) failed", filename));
		}
		return NULL;
	}
	/* 先頭部ロード */
	ofs = long_to_si8(0);
	size = long_to_si8(30 * 4);
	io_load(io, ofs, size);
	/* 先頭部読んでみる */
	nusd = io_peek(io, ofs, size);
	if (nusd == NULL) {
		return ndf_open(io, filename, flags | IO_CREATED,
				dds, dim);
	}
	if (nusd[1] != SYM4_NUSD) {
		nus_err((NUSERR_NUSDReadFail, "bad magic number %Ps for NUSD",
					nusd[1]));
		return NULL;
	}
	switch (NTOH4(nusd[24])) {
		case 1:
		case 10:
			return ndf_open(io, filename, flags | IO_OLDSEQF,
					dds, dim);
		case 11:
		case 13:
		case 14:
			return ndf_open(io, filename, flags, dds, dim);
		default:
			nus_err((NUSERR_BadVersion,
				"bad datafile version %x",
				NTOH4(nusd[24])));
			return NULL;
	}
}
