#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "stringplus.h"
#include <nusdas.h>
#include "nusdim.h"
#include "textout.h"

#if 0
static char rcsid[] = "$Id: listtype.c,v 1.6 2007-02-26 05:46:06 suuchi43 Exp $";
#endif

	int
nusls_type(const char *pattern, const int longe) {
	char type1[8], type2[4], type3[4];
	N_SI4   nrd, r;
	while ((r = nusdas_scan_ds(type1, type2, type3, &nrd)) >= 0) {
		char	type[32];
		sprintf(type, "%.8s.%.4s.%.4s", type1, type2, type3);
		if (pattern != NULL) {
			if (!fmatch(pattern, type)) 
				goto nextline;
		}
		tprintf(0, "%s", type);
		if (longe)
			tprintf(0, " %d", nrd);
		tprintf(0, "\n");
nextline:
		;
	}
	return (r == -1) ? 0 : r;
}
