/** @file
 * @brief グローバル設定
 */
#include "config.h"
#include "nusdas.h"
#include <sys/types.h>
#include <ctype.h>		/* for isalpha(3) */
#include <stddef.h>
#include <stdlib.h>		/* for nus_free(3) */
#include "internal_types.h"
#include "glb.h"
#include "sys_file.h"
#include "sys_err.h"
#include "sys_kwd.h"
# define NEED_STR2SYM4
#include "sys_sym.h"
#include <string.h>		/* for sys_string.h */
#include "sys_mem.h"
# define NEED_STRING_DUP
# define NEED_STRING_COPY
#include "sys_string.h"
#include "sys_time.h"
# define NEED_SI8_ADD
#include "sys_int.h"
#include "io_comm.h"

#ifndef NUS_DFVER
# define NUS_DFVER 11
#endif

#define DEFAULT_SHIFT_NUMBER	17

struct nusglb_param_t nusglb_param = {
	1,	/* io_mark_end */
	1,	/* io_w_fclose */
	1,	/* io_r_fclose */
	0,	/* io_badgrid */
#if SIZEOF_INT == 4	/* pc_missing_si4 */
	INT_MIN,
#elif SIZEOF_LONG == 4
	LONG_MIN,
#else
	0x80000000L,	/* よくわからないのでわかりやすい表記に */
#endif
	FLT_MAX,	/* pc_missing_r4 */
	DBL_MAX,	/* pc_missing_r8 */
	NRD_UNFIX,	/* nrd_override */
	NUS_DFVER, 	/* pc_filever */
	(NEED_ALIGN ? 3u : 0u), /* pc_alignment */
	NULL, /* saved_mask */
	0, /* saved_mask_size */
	-1, /* pc_keep_closed_file */
	{ /* ds_param */
		NULL,		/* pc_mask_bit */
		0,		/* pc_packing */
		0,		/* pc_sizex */
		0,		/* pc_sizey */
		0,		/* pc_wbuffer */
#ifdef SIO_DEFAULT
		0,		/* pc_rbuffer */
		sio_open	/* io_open */
#else
		DEFAULT_SHIFT_NUMBER,		/* pc_rbuffer */
		pio_open	/* io_open */
#endif
	},
	512,		/* io_setvbuf */
	"",		/* dds_forcedpath */
	256u,		/* eio_psize */
	256u,		/* eio_ssize */
	0,		/* no_timecard */
	-32768,		/* pc_missing_si2 */
	UCHAR_MAX	/* pc_missing_ui1 */
};

int nusglb_cfgdone = 0;

	int
nusglb_opt(sym4_t key, const char *val)
{
	int	ver;
	switch (key) {
		case SYM4_GFCL:
			GlobalConfig(io_w_fclose) = 0;
			GlobalConfig(io_r_fclose) = 0;
			break;
		case SYM4_GDBG:
			nus_dbg_enabled = 1;
			nus_wrn_enabled = 1;
			break;
		case SYM4_GWRN:
			nus_wrn_enabled = 1;
			break;
		case SYM4_GRPF:
			NUSPROF_INI(val);
			NUSPROF_MARK(NP_API);
			break;
		case SYM4_GALG:
			ver = strtoul(val, NULL, 0);
			if (ver) {
				GlobalConfig(pc_alignment) = ver - 1;
			} else {
				GlobalConfig(pc_alignment) = 0;
			}
			break;
		case SYM4_FVER:
			ver = atoi(val);
			if (ver == 1) {
				GlobalConfig(pc_filever) = 10;
			} else if (ver == 10 || ver == 11
			|| ver == 13 || ver == 14) {
				GlobalConfig(pc_filever) = ver;
			} else {
				nus_warn(("bad FVER=%d; 14 assumed", ver));
				GlobalConfig(pc_filever) = 14;
			}
			break;
		case SYM4_GKCF:
			GlobalConfig(pc_keep_closed_file) = atoi(val);
			break;
		case SYM4_GRCK:
			GlobalConfig(io_badgrid) = 0;
			break;
		case SYM4_GBAD:
			GlobalConfig(io_badgrid) = 1;
			break;
		case SYM4_GSVB:
			GlobalConfig(io_setvbuf) = strtoul(val, NULL, 0);
			break;
		case SYM4_GPTH:
			string_copy(GlobalConfig(dds_forcedpath), val,
				sizeof GlobalConfig(dds_forcedpath));
			break;
		case SYM4_GESP:
			GlobalConfig(eio_psize) = strtoul(val, NULL, 0);
			break;
		case SYM4_GESS:
			GlobalConfig(eio_ssize) = strtoul(val, NULL, 0);
			break;
		case SYM4_GNTS:
			GlobalConfig(no_timestamp) = 1;
			break;
		default:
			return -1;
	}
	return 0;
}

	void
nusdas_opt(struct nusxds_param_t *param, sym4_t key, const unsigned char *val)
{
	struct nusxds_param_t *xparam;
	xparam = param ? param : &GlobalConfig(ds_param);
	nus_debug(("%s option %Ps=%s",
				param ? "dataset" : "global",
				key,
				val ? (char *)val : "(nil)"));
	if (nusdset_opt(xparam, key, (char *)val) == 0)
		return;
	if (nusio_opt(xparam, key, (char *)val) == 0)
		return;
	if (param)
		return;
	nusglb_opt(key, (char *)val);
}

#define OPT_ARGLEN	128

	int
nusdas_opts(struct nusxds_param_t *param, const unsigned char *str)
{
	enum OptStatus { stINI, stN1, stN2, stN3, stN4, stA, stAB,
		stAQ, stAQB } stat;
	enum OptSymbol { syW, syE, syD, syQ, syB, syx };
	const enum OptSymbol symtab[256] = {
	syD,syx,syx,syx, syx,syx,syx,syx, syx,syD,syD,syD, syD,syD,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syD,syx,syQ,syx, syx,syx,syx,syx, syx,syx,syx,syx, syD,syx,syx,syx,
	syW,syW,syW,syW, syW,syW,syW,syW, syW,syW,syE,syD, syx,syE,syx,syx,
	syD,syW,syW,syW, syW,syW,syW,syW, syW,syW,syW,syW, syW,syW,syW,syW,
	syW,syW,syW,syW, syW,syW,syW,syW, syW,syW,syW,syx, syB,syx,syx,syW,
	syx,syW,syW,syW, syW,syW,syW,syW, syW,syW,syW,syW, syW,syW,syW,syW,
	syW,syW,syW,syW, syW,syW,syW,syW, syW,syW,syW,syx, syx,syx,syx,syW,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx,
	syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx, syx,syx,syx,syx
	};
	enum OptSymbol sym;
	const unsigned char *p;
	unsigned char *q;
	unsigned char name[4];
	unsigned char arg[OPT_ARGLEN];
	stat = stINI;
	p = str;
	q = arg;
	while (1) {
		sym = symtab[*p];
#define SENDNAME	nusdas_opt(param, MEM2SYM4(name), NULL)
		switch (stat) {
			case stINI:
				if (sym == syW) {
					name[0] = toupper(*p);
					stat = stN1;
				}
				break;
			case stN1:
				if (sym == syW) {
					name[1] = toupper(*p);
					stat = stN2;
				} else if (sym == syE) {
					name[1] = name[2] = name[3] = ' ';
					stat = stA;
					q = arg;
				} else {
					name[1] = name[2] = name[3] = ' ';
					SENDNAME;
					stat = stINI;
				}
				break;
			case stN2:
				if (sym == syW) {
					name[2] = toupper(*p);
					stat = stN3;
				} else if (sym == syE) {
					name[2] = name[3] = ' ';
					stat = stA;
					q = arg;
				} else {
					name[2] = name[3] = ' ';
					SENDNAME;
					stat = stINI;
				}
				break;
			case stN3:
				if (sym == syW) {
					name[3] = toupper(*p);
					stat = stN4;
				} else if (sym == syE) {
					name[3] = ' ';
					stat = stA;
					q = arg;
				} else {
					name[3] = ' ';
					SENDNAME;
					stat = stINI;
				}
				break;
			case stN4:
				if (sym == syE) {
					stat = stA;
					q = arg;
				} else if (sym != syW) {
					SENDNAME;
					stat = stINI;
				}
				break;
#define SENDARG \
		*q = 0; \
		nusdas_opt(param, MEM2SYM4(name), arg)
#define SAVEARG(c) \
		if (q <= (arg + OPT_ARGLEN - 2)) { \
			*q++ = (c); \
		}
			case stA:
				if (sym == syD) {
					SENDARG;
					stat = stINI;
				} else if (sym == syQ) {
					stat = stAQ;
				} else if (sym == syB) {
					stat = stAB;
				} else {
					SAVEARG(*p);
				}
				break;
			case stAB:
				SAVEARG(*p);
				stat = stA;
				break;
			case stAQ:
				if (sym == syQ) {
					stat = stA;
				} else if (sym == syB) {
					stat = stAQB;
				} else {
					SAVEARG(*p);
				}
				break;
			case stAQB:
				SAVEARG(*p);
				stat = stAQ;
				break;
			default:
				stat = stINI;
		}
		if (*p == 0) {
			break;
		}
		p++;
	}
	return 0;
}

	int
nusglb_config(void)
{
	unsigned char *file;
	const char *env;
	size_t	dummy;
	int r;
	/* 重複起動回避 */
	if (nusglb_cfgdone) {
		return 0;
	}
	/* 設定読み込み */
	if ((file = file_read("nusdas.ini", &dummy)) != NULL) {
		nusdas_opts(NULL, file);
		nus_free(file);
	}
	if ((env = getenv("NUSDAS_OPTS")) != NULL) {
		nusdas_opts(NULL, (const unsigned char *)env);
	}
	/* データセット探索 */
	if ((r = nusdset_scan()) != 0)
		return r;
	nusglb_cfgdone = 1;
	return 1;
}
