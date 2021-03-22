/*
 * vi: set sw=4:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "nusdas.h"
#include "nwpl_capi.h"

#define EPS_DEFAULT 1.e-3

#define STARS63 "********************" "********************" \
    "********************" "***"
#define DASH54 "------------------------------------------------------"

#define memeq(p, q, n)	(memcmp((p), (q), (n)) == 0)
#define streq(p, q)	(strcmp((p), (q)) == 0)

char ox[] = "OX";

int Hetero13 = 0;

	int
index_special(unsigned name1, int len1, unsigned name2, int len2)
{
	if (memeq(&name1, "INDY", 4) && memeq(&name2, "INDX", 4)
			 && (len2 - 12) * 4 == (len1 - 12)) {
		return 1;
	}
	if (memeq(&name1, "END ", 4) && memeq(&name2, "END ", 4)
			&& (len2 != len1)) {
		return 1;
	}
	return 0;
}

	int
main(int argc, char *argv[])
{
    unsigned char *ud[2];
    N_BIGFILE *fp[2];
    int rlens[2], rlene[2], alen[2], stime[2], version[2];
    unsigned int rname[2];
    int y, m, d, h, f, size[2], vt[2], asize[2];
    int i, vFlag = 0, ErrorStatus = 0, ContentDiff = 0, cFlag = 0, 
	    sf = 0, nf = 0, pf = 0, cc = 0, ss = 0, nn = 0, df = 0, rf = 0;
    char srname[5], memb[2][5], levl[2][7], elem[2][7];
    struct tm *tp;
    time_t systime_time_t;
    float eps = EPS_DEFAULT;
    int alen_diff;

    memb[0][4] = '\0';
    levl[0][6] = '\0';
    elem[0][6] = '\0';
    memb[1][4] = '\0';
    levl[1][6] = '\0';
    elem[1][6] = '\0';

    if (argc < 3) {
	fprintf(stderr, DASH54 "\n");
	fprintf(stderr,
		"usage: nuscmp file1 file2 (-v,-V) (-c) (-s) (-n)\n");
	fprintf(stderr, DASH54 "\n");
	fprintf(stderr, "nuscmp compare file1 and file2.\n");
	fprintf(stderr, "same file case -> return 0,"
		" and different file case -> return 1\n");
	fprintf(stderr, "If -v or -V is specified,"
		" difference of create time is ignored.\n");
	fprintf(stderr, "In case of -V, the comparison is continued"
		" to the end of file\n");
	fprintf(stderr, "except for the fatal difference and record"
		" detail information are printted out.\n");
	fprintf(stderr, "If -n is specified,"
		" difference of NuSDaS NUSD record is ignored.\n");
	fprintf(stderr, "If -c is specified,"
		" difference of NuSDaS CNTL record is ignored.\n");
	fprintf(stderr, "If -s is specified,"
		" difference of NuSDaS SUBC record is ignored.\n");
	fprintf(stderr, "If -p is specified,"
		" difference of padding of each record is ignored.\n");
	fprintf(stderr, "If -d is specified,"
		" DATA records are compared with unpacked values.\n");
	fprintf(stderr, "If -r is specified,"
		" difference of record length is ignored.\n");
	return 9;
    }
    if ((fp[0] = bfopen(argv[1], "r")) == NULL) {
	fprintf(stderr, "nuscmp: can not open file <%s>\n", argv[1]);
	return 9;
    }
    if ((fp[1] = bfopen(argv[2], "r")) == NULL) {
	fprintf(stderr, "nuscmp: can not open file <%s>\n", argv[2]);
	return 9;
    }
    if (argc > 3) {
	if (streq(argv[3], "-v")) {
	    vFlag = 1;
	} else if (streq(argv[3], "-V")) {
	    vFlag = 2;
	} else {
	    vFlag = 0;
	}
    }
    i = 3;
    while (i < argc) {
	if (streq(argv[i], "-v")) {
	    vFlag = 1;
	} else if (streq(argv[i], "-V")) {
	    vFlag = 2;
	} else if (streq(argv[i], "-c")) {
	    cFlag = 1;
	} else if (streq(argv[i], "-s")) {
	    sf = 1;
	} else if (streq(argv[i], "-n")) {
	    nf = 1;
	} else if (streq(argv[i], "-p")) {
	    pf = 1;
	} else if (streq(argv[i], "-r")) {
	    rf = 1;
	} else if (streq(argv[i], "-d")) {
	    char *epsstr, *endptr;
	    df = 1;
	    if (i + 1 != argc && *(argv[i + 1]) != '-' ) {
		    eps = strtod(argv[++i], &endptr);
		    if (eps == 0.0 && argv[i] == endptr) {
			    fprintf(stderr, "Invalid limit value.\n");
			    return -9;
		    }
	    }
	}
	i++;
    }

    if (vFlag == 2) {
	puts(STARS63);
	printf("nuscmp: file1=%s\n", argv[1]);
	printf("        file2=%s\n", argv[2]);
	if (df){
		printf("eps = %f\n", eps);
	}
	puts(STARS63);
	printf("?       length createT1/createT2 valid-T1/valid-T2"
	       " level1/level2 elem-1/elem-2\n");
    }

    /* read NUSD record */
    for (i = 0; i < 2; i++) {
	bfread_native(&rlens, 4, 1, fp[i]);
	bfseek(fp[i], 92, SEEK_CUR);
	bfread_native(&version[i], 4, 1, fp[i]);
	bfseek(fp[i], 0L, SEEK_SET);
	if (version[i] == 1) {
	    version[i] = 10;
	}
    }
    Hetero13 = ((version[0] >= 13) && (version[1] < 13))
	    || ((version[0] < 13) && (version[1] >= 13));
    while (1) {

	if (bfread_native(&rlens[0], 4, 1, fp[0]) != 1) {
	    break;
	}
	if (bfread_native(&rlens[1], 4, 1, fp[1]) != 1) {
	    break;
	}
	for (i = 0; i < 2; i++) {
	    if (version[i] == 10) {
		rlens[i] -= 8;
	    }
	}
	bfread(&rname[0], 4, fp[0]);
	bfread(&rname[1], 4, fp[1]);
	bfread_native(&alen[0], 4, 1, fp[0]);
	bfread_native(&alen[1], 4, 1, fp[1]);
	bfread_native(&stime[0], 4, 1, fp[0]);
	bfread_native(&stime[1], 4, 1, fp[1]);
	size[0] = rlens[0] - 12;
	size[1] = rlens[1] - 12;
	if (pf == 1) {
		asize[0] = alen[0] - 12;
		asize[1] = alen[1] - 12;
	} else {
		asize[0] = size[0];
		asize[1] = size[1];
	}
	ud[0] = (unsigned char *) malloc(sizeof(unsigned char) * size[0]);
	ud[1] = (unsigned char *) malloc(sizeof(unsigned char) * size[1]);
	bfread_native(ud[0], 1, size[0], fp[0]);
	bfread_native(ud[1], 1, size[1], fp[1]);
	bfread_native(&rlene[0], 4, 1, fp[0]);
	bfread_native(&rlene[1], 4, 1, fp[1]);
	for (i = 0; i < 2; i++) {
	    if (version[i] == 10) {
		rlene[i] -= 8;
	    }
	}

	if (!rf & !Hetero13 && (rlens[0] != rlens[1])) {
	    fprintf(stderr, "different record length %d:%d!\n", rlens[0],
		    rlens[1]);
	    return 1;
	}

	alen_diff = 0;
	if (index_special(rname[0], alen[0], rname[1], alen[1])
		|| index_special(rname[1], alen[1], rname[0], alen[0])) {
		ContentDiff = 0;
		goto IndexSpecial;
	}
	if (rname[0] != rname[1]) {
		if (Hetero13) {
			alen_diff = 1;
		} else {
			fprintf(stderr, "different record name %.4s:%.4s!\n",
				(char *)&rname[0], (char *)&rname[1]);
			return 1;
		}
	}
	if (alen[0] != alen[1]) {
		if (!rf & !Hetero13) {
			fprintf(stderr, "different available length %d:%d!\n",
				alen[0], alen[1]);
			return 1;
		} else {
			alen_diff = 1;
		}
	}
	if (stime[0] != stime[1]) {
	    if (ErrorStatus == 0) {
		ErrorStatus = 1;
	    }
	    if (vFlag == 0) {
		fprintf(stderr, "different files(create time)!\n");
		return 1;
	    }
	}
	for (i = 0; i < 2; i++)
	    if (!alen_diff && rlens[i] != rlene[i]) {
		fprintf(stderr, "nuscmp: inconsistent record length"
			" <s:%d,e:%d> on file%1.1d\n",
			rlens[i], rlene[i], i);
		return 2;
	    }
	if (!alen_diff && memcmp(&rname[0], "NUSD", 4) == 0) {	/* in case of "NUSD" */
	    if (vFlag != 0) {
		ContentDiff = (memcmp(ud[0] + 84, ud[1] + 84, size[0] - 84) !=
		      0) ? 1 : 0;
	    } else {
		ContentDiff = (memcmp(ud[0], ud[1], 80) != 0
		      || memcmp(ud[0] + 84, ud[1] + 84,
				asize[0] - 84) != 0) ? 1 : 0;
	    }
	} else if (!alen_diff && memcmp(&rname[0], "CNTL", 4) == 0) {	/* in case of "CNTL" */
	    if (vFlag != 0) {
		memset(ud[0] + 16, 0x20, 12);
		memset(ud[1] + 16, 0x20, 12);
	    }
	    ContentDiff = memcmp(ud[0], ud[1], asize[0]);
	} else if (memcmp(&rname[0], "DATA", 4) == 0) {
		if (df == 0) {
			ContentDiff = memcmp(ud[0], ud[1], asize[0]);
		} else {
			int gsize[2];
			N_SI4 dnum;
			float *data[2];
			int r;
			if ((ContentDiff = 
			     memcmp(ud[0] + 32, ud[1] + 32, 8)) == 0) {
				memcpy(gsize, ud[0] + 32, 8);
				endian_swab4(gsize, 2);
				dnum = gsize[0] * gsize[1];
				if ((data[0] =
				     (float*)malloc(dnum * sizeof(float))) 
				    == NULL || 
				    (data[1] = 
				     (float*)malloc(dnum * sizeof(float))) 
				    == NULL){
					fprintf(stderr, "malloc error\n");
					return -10;
				}
				r = nusdas_unpack(ud[0] + 32, 
						  data[0], "R4", dnum);
				/* todo error check */
				r = nusdas_unpack(ud[1] + 32, 
						  data[1], "R4", dnum);
				/* todo error check */
				ContentDiff = 0;
				for (i = 0; i < dnum; i++) {
					float mx;
					mx = fabs(data[0][i]) 
						- fabs(data[1][i]) > 0 
						? fabs(data[0][i]) 
						: fabs(data[1][i]);
					if (mx == 0.0) continue;
					mx = mx < 1.e-6 ? 1.e-6 : mx;
					if (fabs((data[0][i] - data[1][i])) 
					    >= eps * fabs(mx)) {
						ContentDiff = 1;
						break;
					}
				}
				free(data[0]);
				free(data[1]);
			}
		}
	} else if (!alen_diff) {
	    ContentDiff = memcmp(ud[0], ud[1], asize[0]);
	}
IndexSpecial:

	if (ContentDiff != 0) {
	    ContentDiff = 1;
	    if (memeq(&rname[0], "CNTL", 4) && cFlag) {
		cc = 1;
	    } else if (memeq(&rname[0], "SUBC", 4) && sf == 1) {
		ss = 1;
	    } else if (memeq(&rname[0], "NUSD", 4) && nf == 1) {
		nn = 1;
	    } else {
		ErrorStatus = 2;
	    }
	}
	if (vFlag <= 1 && ErrorStatus == 2) {
	    fprintf(stderr, "different files!\n");
	    return 1;
	} else if (vFlag == 2) {
	    memcpy(srname, &rname[0], 4);
	    srname[4] = '\0';
	    systime_time_t = (time_t) stime[0];
	    tp = gmtime((time_t *) & systime_time_t);
	    printf("%c<%4s> %6d %2.2d%2.2d%2.2d%2.2d", ox[ContentDiff], srname,
		   rlens[0], tp->tm_mon + 1, tp->tm_mday, tp->tm_hour,
		   tp->tm_min);
	    systime_time_t = (time_t) stime[1];
	    tp = gmtime((time_t *) & systime_time_t);
	    printf("/%2.2d%2.2d%2.2d%2.2d",
		   tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min);
	    if (strcmp(srname, "DATA") == 0) {
		for (i = 0; i < 2; i++) {
		    memcpy(memb[i], (ud[i] + 0), 4);
		    memcpy(&vt[i], (ud[i] + 4), 4);
		    memcpy(levl[i], (ud[i] + 12), 6);
		    memcpy(elem[i], (ud[i] + 24), 6);
		}
		endian_swab4(vt, 2);
		nwp_seq2ymdhm(&y, &m, &d, &h, &f, vt[0]);
		printf(" %2.2d%2.2d%2.2d%2.2d", m, d, h, f);
		nwp_seq2ymdhm(&y, &m, &d, &h, &f, vt[1]);
		printf("/%2.2d%2.2d%2.2d%2.2d", m, d, h, f);
		printf(" %6s/%6s %6s/%6s\n", levl[0], levl[1], elem[0],
		       elem[1]);
	    } else {
		printf("\n");
	    }
	}
	free(ud[0]);
	free(ud[1]);
    }

    bfclose(fp[0]);
    bfclose(fp[1]);
    if (ErrorStatus == 0) {
	fprintf(stderr, "Same files!\n");
	if (version[0] != version[1]) {
	    fprintf(stderr,
		    "difference of NuSDaS Version Number is ignored."
		    " version: file1 = %d, file2 = %d\n", version[0],
		    version[1]);
	}
	return 0;
    } else if (ErrorStatus == 1) {
	fprintf(stderr, "Same files except for"
		" create time and comment in NUSD!\n");
	if (nn == 1) {
	    fprintf(stderr, "Same files except for NUSD record!\n");
	}
	if (cc == 1) {
	    fprintf(stderr, "Same files except for CNTL record!\n");
	}
	if (ss == 1) {
	    fprintf(stderr, "Same files except for SUBC record!\n");
	}
	if (version[0] != version[1]) {
	    fprintf(stderr,
		    "difference of NuSDaS Version Number is ignored."
		    " version: file1 = %d, file2 = %d\n", version[0],
		    version[1]);
	}
	return 0;
    } else {
	fprintf(stderr, "Above X record(s) is(are) different!\n");
	if (version[0] != version[1]) {
	    fprintf(stderr,
		    "difference of NuSDaS Version Number is ignored."
		    " version: file1 = %d, file2 = %d\n", version[0],
		    version[1]);
	}
	return 1;
    }
}
