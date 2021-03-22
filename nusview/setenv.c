#include <stdio.h>
#include <stdlib.h>
#include "stringplus.h"
#include <nusdas.h>
#include "nusdim.h"
#include "config.h"

#if 0
static char rcsid[] = "$Id: setenv.c,v 1.6 2007-02-27 10:21:58 suuchi43 Exp $";
#endif

	int
nusdas_addroot(const char *root)
{
	static int	rdno = 91;
	char	rootname[10];
	if (root == NULL || *root == '\0')
		return -1;
	rdno--;
	if (rdno <= 0)
		return -1;
	sprintf(rootname, "NUSDAS%02d", rdno);
	putenv(strdup3(rootname, "=", root));
	return 0;
}
