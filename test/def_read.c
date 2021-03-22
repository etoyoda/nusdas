
#include "config.h"
#include "nusdas.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "internal_types.h"
#include "sys_kwd.h"
# define NEED_STR2SYM8
#include "sys_sym.h"
#include "sys_err.h"
#include "sys_string.h"
#include "sys_file.h"
#include "dset.h"
#include "dfile.h"
#include "ndf.h"
# define NEED_SI8_CMP
#include "sys_int.h"

void checkgrid(nusdef_t *def)
{
	N_UI4 cntl[43];
	int r;
	memcpy(cntl + 20, &def->projparam, sizeof(float) * 14);
	endian_swab4(cntl + 20, 14);
	cntl[17] = nusdef_projcode(def);
	r = ndf_grid_check(cntl);
	if (r) {
		fprintf(stderr, "Error: %.16s\n", (char *)&def->nustype);
		NUSDAS_CLEANUP;
	}
}

void dump_element(const char *element, nusdef_t *def)
{
	int im, ie;
	sym8_t elem8;
	elem8 = str2sym8(element);
	for (ie = 0; ie < def->n_el; ie++) {
		if (si8_eq(def->el[ie], elem8)) {
			goto found;
		}
	}
	fprintf(stderr, "element %s not found\n", element);
	return;
found:
	for (im = 0; im < def->n_mb; im++) {
		int iv, iz, i;
		printf("==== member %.4s ====\n", (char *)&(def->mb[im]));
		for (iz = 0; iz < def->n_lv; iz++) {
			printf("%.6s: ", (char *)&(def->lv1[iz]));
			for (iv = 0; iv < def->n_vt; iv++) {
				i = im;
				i *= def->n_vt;
				i += iv;
				i *= def->n_lv;
				i += iz;
				i *= def->n_el;
				i += ie;
				switch (def->elementmap[i]) {
					case ~(N_UI4)0:
						putchar('1');
						break;
					case 0:
						putchar('0');
						break;
					default:
						putchar('?');
				}
			}
			putchar('\n');
		}
	}
}

extern int nusglb_config(void);

int main(int argc, char **argv)
{
	int i, r;
	char *element = NULL;
	int grid_flag = 0;

	if (argc < 2) {
		printf("usage: %s [-eELEM] def_files ...\n", argv[0]);
		return 1;
	}
	r = 0;
	for (i = 1; argv[i]; i++) {
		nusdef_t def;
		if (strncmp(argv[i], "-e", 2) == 0) {
			element = argv[i] + 2;
			continue;
		}
		if (strncmp(argv[i], "-g", 2) == 0) {
			grid_flag = 1;
			nusglb_config();
			continue;
		}
		if (argc > 2) {
			fprintf(stderr, "--- file %s ---\n", argv[i]);
		}
		nusdef_init(&def);
		if ((r = nusdef_readfile(argv[i], &def)) != 0) {
			printf("nusdef_endparse(%s) => %d\n", argv[i], r);
			break;
		}
		if ((r = nusdef_endparse(&def)) != 0) {
			printf("nusdef_endparse(%s) => %d\n", argv[i], r);
			break;
		}
		NUSDAS_CLEANUP;

		printf("dimsize %3d %3d %3d %3d\n",
		      (int)def.n_mb, (int)def.n_vt,
		      (int)def.n_lv, (int)def.n_el);
		printf("packmiss %.4s %.4s\n",
				(char *)&def.packing,
				(char *)&def.missing);
		if (element) {
			dump_element(element, &def);
		}
		if (grid_flag) {
			checkgrid(&def);
		}
	}
	NUSDAS_CLEANUP;
	return r;
}
