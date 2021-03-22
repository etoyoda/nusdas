#include <stddef.h>
#include <sys/types.h>
#include "config.h"
#include "internal_types.h"
#include "sys_int.h"
#include "sys_kwd.h"
#include "sys_err.h"
#include "glb.h"
#include "io_comm.h"

	int
nusio_opt(struct nusxds_param_t *param, sym4_t key, const char *val UNUSED)
{
	switch (key) {
		case SYM4_IPSX:
			DynParam(param, io_open) = pio_open;
			break;
		case SYM4_ISTD:
			DynParam(param, io_open) = sio_open;
			break;
		case SYM4_IESF:
			DynParam(param, io_open) = eio_open;
			break;
		case SYM4_IASY:
			nus_warn(("obsolete option IASY: ISTD used instead"));
			DynParam(param, io_open) = sio_open;
			break;
		case SYM4_IMMP:
			nus_warn(("obsolete option IMMP: ISTD used instead"));
			DynParam(param, io_open) = sio_open;
			break;
		case SYM4_IBMS:
			nus_warn(("obsolete option IBMS: ISTD used instead"));
			DynParam(param, io_open) = sio_open;
			break;
		default:
			return -1;
	}
	return 0;
}
