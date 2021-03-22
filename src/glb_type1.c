#include "config.h"
#include "nusdas.h"
#include <string.h>
#include "internal_types.h"
#include "sys_sym.h"
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_err.h"

static struct modeltab_t {
	char model[4];
	char rest[4];
} modeltab[] = {
	{ "_GSM", "LLPP" },
	{ "_WFM", "LLPP" },
	{ "_SF1", "LLPP" },
	{ "_SF4", "LLPP" },
	{ "_RSM", "LLPP" },
	{ "_MSM", "LLPP" },
	{ "_TYM", "LLPP" },
	{ "_CA1", "LLPP" },
	{ "_ELM", "FGZZ" },
	{ "_OAM", "FGZZ" },
	{ "_WV1", "LLZZ" },
	{ "_WV2", "LLZZ" },
	{ "_SRF", "OLZZ" },
	{ "_DDD", "LLPP" },
	{ "", "" }
};

	int
nusglb_intp_type1(sym8_t *sym)
{
	char *type1 = (char *)sym;
	struct modeltab_t *mp;
	if (!(type1[4] == ' ' && type1[5] == ' '
			&& type1[6] == ' ' && type1[7] == ' ')) {
		return 0;
	}
	for (mp = modeltab; mp->model[0]; mp++) {
		if (memcmp(type1, mp->model, 4) == 0) {
			memcpy4(type1 + 4, mp->rest);
			nus_debug(("type1=<%.8s>", type1));
			return 0;
		}
	}
	return nus_err((NUSERR_BadType1,
			"cannot interpolate semi-blank type1=<%.8s>", type1));
}
