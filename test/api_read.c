
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> /* for debug printf */
#include "nusdas.h"
#include "config.h"
#include "internal_types.h"
#include "sys_sym.h"
#include "sys_time.h"
#include "dset.h"
#include "glb.h"

#include "sys_string.h"

char *
string_to_chars(char * const dest, const char *src, const size_t n)
{
	char *p = dest;
	while (p < dest + n) {
		if (*src == '\0')
			break;
		*p++ = *src++;
	}
	while (p < dest + n) {
		*p++ = ' ';
	}
	return dest;
}

int main(int argc, char **argv)
{
	char	type[16], m[4], z[6], e[6];
	N_SI4	bt, vt, r, nelems, i; 
	float	*buf;
	if (argc < 8) {
		puts("usage: api_read type1type2type3 bt m vt z e nelems");
		return 1;
	}
	string_to_chars(type, argv[1], 16);
	string_to_time(&bt, argv[2]);
	if (streq(argv[3], "none")) {
		string_to_chars(m, "    ", 4);
	} else {
		string_to_chars(m, argv[3], 4);
	}

	string_to_time(&vt, argv[4]);
	string_to_chars(z, argv[5], 6);
	string_to_chars(e, argv[6], 7);
	nelems = atol(argv[7]);
	buf = (float *)malloc(nelems * sizeof(float));
	if (buf == NULL) {
		perror("malloc");
		return 1;
	}
	r = nusdas_read(type, type + 8, type + 12, &bt, m, &vt, z, e, buf,
			N_R4, &nelems);
	printf("nusdas_read %d\n", (int)r);
	nelems = (r >= 20) ? 10 : ((r - 1) / 2 + 1);
	for (i = 0; i < nelems; i++) {
		printf("%06d: %12.5g\n", i, buf[i]);
	}
	nelems = (r >= 20) ? (r - 10) : (r - ((r - 1) / 2 + 1));
	for (i = nelems; i < r; i++) {
		printf("%06d: %12.5g\n", i, buf[i]);
	}
	return 0;
}
