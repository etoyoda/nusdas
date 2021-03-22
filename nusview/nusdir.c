/* nusdir.c */

#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "stringplus.h"
#include <nusdas.h>
#include "nusdir.h"
#include "textout.h"

#if 0
static char rcsid[] = "$Id: nusdir.c,v 1.5 2007-02-26 05:46:06 suuchi43 Exp $";
#endif

	void
opt_clear(struct nusdir_opts *op)
{
	op->longe = 0;
	op->debug = 0;
}

	void
option(struct nusdir_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;

	switch (*str) {
	case 'd':
		op->debug = 1;
		tset_content_type(NULL);
		break;

	case 'r':
		if (str[1]) {
			nusdas_addroot(str+1);
		}
		break;

	case 'l':
		op->longe = 1;
		break;
	
	case 'h':
		puts("nusdir [options]");
		puts("-d\tdebug");
		puts("-r\tobsolete");
		puts("-rdir\tuse dir as NUSDAS91");
		puts("-l\tlist NRD number in addition to nustype");
		puts("-h\tthis help");
		exit(1);
		break;

	default:	
		break;
	}
}

	int
nusdir(const char *path, struct nusdir_opts *op)
{
	struct nus_dims_t dims;
	int	flags;

	if (path == NULL || !*path || streq(path, "/")) {
		if (op->debug)
			tprintf(0, "# null -> ls\n");
		return nusls_type(NULL, op->longe);
	}
	if (strchr(path, '*') || strchr(path, '?')) {
		if (op->debug)
			tprintf(0, "# sqeeze-slash-ls {%s}\n", path);
		while (*path == '/')
			path++;
		return nusls_type(path, op->longe);
	}
	
	path_to_nusdims(path, &dims);
	if (op->debug)
		tprintf(0, "# %d %s\n", dims.stat, nusdims_to_path(&dims));
	flags = dims.stat;

	if (flags == 0) {
		while (*path == '/') path++;
		return nusls_type(path, op->longe);
	}

#if 0
	if (op->version == 2) {
		return nusdir_generic(&dims, op);
	}
#endif

	if ((flags &= ~NUSDIM_TYPE) == 0) {
		return nusdir_base(&dims, op);
	}
	if ((flags &= ~NUSDIM_BASE) == 0) {
		return nusdir_member(&dims, op);
	}
	if ((flags &= ~NUSDIM_MEMBER) == 0) {
		return nusdir_valid(&dims, op);
	}
	if ((flags &= ~NUSDIM_VALID) == 0) {
		return nusdir_valid2(&dims, op);
	}
	if ((flags &= ~NUSDIM_VALID2) == 0) {
		return nusdir_plane(&dims, op);
	}
	if ((flags &= ~NUSDIM_PLANE) == 0) {
		return nusdir_plane2(&dims, op);
	}
	if ((flags &= ~NUSDIM_PLANE2) == 0) {
		return nusdir_element(&dims, op);
	}
	if ((flags &= ~NUSDIM_ELEMENT) == 0) {
		return nusdir_element(&dims, op);
	}
	eprintf(HTTP_ERR, "bad spec <%s>\n", path);
	return 4;
}

    int
main(int argc, char **argv)
{
	struct nusdir_opts opt;
	int	argind, r, files;
	files = 0;
	text_init("text/plain");
	opt_clear(&opt);
	for (argind = 1; argind < argc; argind++) {
		if (argv[argind][0] == '-') {
			option(&opt, argv[argind]);
			continue;
		}
		files++;
		r = nusdir(argv[argind], &opt);
		if (r) break;
	}
	if (files == 0) {
		r = nusdir("/", &opt);
	}
	text_end();
	return r;
}
