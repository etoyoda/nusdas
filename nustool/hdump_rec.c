#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "nusdas.h"
#include "nwpl_capi.h"

static struct hdump_opts {
	int long_output;
	long forcedrlen;
} Opts = {
	0,	/* long_output */
	0	/* forcedrlen */
};

struct file_stat {
	N_SI8	ni_filesize;
	N_UI4	ni_filesize4;
	N_UI4	ni_rec;
	N_UI4	ni_subc;
	N_UI4	ni_info;
};

	int
str_is_all_digits(const char *str)
{
	while (*str) {
		if (!isdigit(*str)) {
			return 0;
		}
		str++;
	}
	return 1;
}

	void
filestat_ini(struct file_stat *st)
{
	st->ni_rec = 0;
	st->ni_subc = 0;
	st->ni_info = 0;
}

	int
dump_data(N_BIGFILE *fp, int *ladd)
{
    double dbase, damp;
    float base, amp;
    char member[4], pack[4];
    char level1[6], level2[6], element[6];
    int y, m, d, h, f, vt1, vt2, xs, ys, miss;
    bfread_native(member, 1, 4, fp);
    bfread_native(&vt1, 4, 1, fp);
    bfread_native(&vt2, 4, 1, fp);
    nwp_seq2ymdhm(&y, &m, &d, &h, &f, vt1);
    bfread_native(level1, 1, 6, fp);
    bfread_native(level2, 1, 6, fp);
    bfread_native(element, 1, 6, fp);
    bfseek(fp, 2, SEEK_CUR);
    bfread_native(&xs, 4, 1, fp);
    bfread_native(&ys, 4, 1, fp);
    bfread_native(pack, 1, 4, fp);
    bfread_native(&miss, 1, 4, fp);
    if (Opts.long_output) {
	*ladd = 0;
	if (strncmp(pack, "1PAC", 4) == 0
	    || strncmp(pack, "2PAC", 4) == 0
	    || memcmp(pack, "2UPC", 4) == 0) {
	    if (memcmp(&miss, "UDFV", 4) == 0) {
		if (strncmp(pack, "1PAC", 4) == 0) {
		    bfseek(fp, 1, SEEK_CUR);
		    *ladd = 1;
		} else if (strncmp(pack, "2UPC", 4) == 0
			   || strncmp(pack, "2PAC", 4) == 0) {
		    bfseek(fp, 2, SEEK_CUR);
		    *ladd = 2;
		}
	    } else if (strncmp((char *) &miss, "MASK", 4) == 0) {
		bfseek(fp, (xs * ys - 1) / 8 + 1, SEEK_CUR);
		*ladd = (xs * ys - 1) / 8 + 1;
	    }
	    bfread_native(&base, 4, 1, fp);
	    bfread_native(&amp, 4, 1, fp);
	    *ladd += 8;
	} else if (strncmp(pack, "4PAC", 4) == 0) {
	    if (strncmp((char *) &miss, "UDFV", 4) == 0) {
		bfseek(fp, 4, SEEK_CUR);
		*ladd = 4;
	    } else if (strncmp((char *) &miss, "MASK", 4) == 0) {
		bfseek(fp, (xs * ys - 1) / 8 + 1, SEEK_CUR);
		*ladd = (xs * ys - 1) / 8 + 1;
	    }
	    bfread_native(&dbase, 8, 1, fp);
	    base = dbase;
	    bfread_native(&damp, 8, 1, fp);
	    amp = damp;
	    *ladd += 16;
	} else {
	    *ladd = 0;
	}
    } else {
	*ladd = 0;
    }
    if (memcmp(&miss, "UDFV", 4) == 0
	|| memcmp(&miss, "MASK", 4) == 0
	|| memcmp(&miss, "NONE", 4) == 0) {
	printf
	    (" <%4.4s> %4.4d%2.2d%2.2d%2.2d%2.2d <%6.6s:%6.6s> "
	     "<%4.4s> <%-d,%-d>",
	     member, y, m, d, h, f, level1, element, pack, xs, ys);
    } else {
	memcpy(pack, &xs, 4);
	printf(" <%4.4s> %4.4d%2.2d%2.2d%2.2d%2.2d <%6.6s:%6.6s> <%4.4s>",
	       member, y, m, d, h, f, level1, element, pack);
    }
    if (Opts.long_output && (strncmp(&pack[1], "PAC", 3) == 0 ||
		       strncmp(&pack[1], "UPC", 3) == 0)) {
	printf(" <base:%f, amp:%f>", base, amp);
    }
    return 0;
}

	int
dump_cntl(N_BIGFILE *fp)
{
    char type[16];
    char cbase[13];
    int ibase;
    bfread_native(type, 1, 16, fp);
    bfread_native(cbase, 1, 12, fp);
    cbase[12] = '\0';
    bfread_native(&ibase, 4, 1, fp);
    if (ibase != 0) {
	printf(" <%16.16s>, <%12.12s>", type, cbase);
    } else {
	printf(" <%16.16s>, <%12.12s>", type, "180101010000");
    }
    return 0;
}

	int
dump_subc(N_BIGFILE *fp)
{
    N_SI4 size;
    char sname[4];
    bfread_native(sname, 1, 4, fp);
    printf(" <%4.4s>", sname);
    bfread_native(&size, 4, 1, fp);
    printf(" %d", size);
    bfread_native(&size, 4, 1, fp);
    printf(" %d", size);
    return 0;
}

	int
dump_info(N_BIGFILE *fp)
{
    char sname[4];
    char info[20];
    bfread_native(sname, 1, 4, fp);
    printf(" <%4.4s>", sname);
    bfread_native(info, 1, 20, fp);
    printf(" %20.20s", info);
    return 0;
}

	int
dump_end(N_BIGFILE *fp, struct file_stat *st)
{
	bfread_native(&st->ni_filesize4, 4, 1, fp);
	bfread_native(&st->ni_rec, 4, 1, fp);
	return 0;
}

	int
dump_nusd(N_BIGFILE *fp, N_UI4 version, struct file_stat *st)
{
	if (version <= 12) {
		bfseek(fp, 84, SEEK_CUR);
		bfread_native(&st->ni_filesize4, 4, 1, fp);
		memset(&st->ni_filesize, '\0', 4);
		memcpy((char *)&st->ni_filesize + 4, &st->ni_filesize4, 4);
		endian_swab4((char *)&st->ni_filesize + 4, 1);
		endian_swab8(&st->ni_filesize, 1);
	} else {
		bfseek(fp, 72, SEEK_CUR);
		bfread_native(&st->ni_filesize, 8, 1, fp);
		bfseek(fp, 4, SEEK_CUR);
		bfread_native(&st->ni_filesize4, 4, 1, fp);
	}
	bfread_native(&st->ni_rec, 4, 1, fp);
	bfread_native(&st->ni_info, 4, 1, fp);
	bfread_native(&st->ni_subc, 4, 1, fp);
	return 0;
}

	int
dumpfile(const char *filename)
{
    N_BIGFILE *fp;
    struct file_stat nusd_stat, end_stat, real_stat;
    int rlens, rlene, alen, systime, offset, ladd, version;
    int n_data = 0, frlen = 0, eflag = 0;
    char rname[5], level1[7], level2[7], element[7], pack[5];
    struct tm *tp;
    time_t systime_time_t;

    filestat_ini(&real_stat);
    rname[4] = '\0';
    pack[4] = '\0';
    level1[6] = '\0';
    level2[6] = '\0';
    element[6] = '\0';
    if ((fp = bfopen(filename, "r")) == NULL) {
	fprintf(stderr, "Can not open file <%s>\n", filename);
	return 1;
    }
    /* read NUSD record */
    bfseek(fp, 4, SEEK_SET);
    bfread(rname, 4, fp);
    if (memcmp(rname, "NUSD", 4) == 0) {
	bfseek(fp, 88, SEEK_CUR);
	bfread_native(&version, 4, 1, fp);
	printf("NuSDaS Version: %d\n", version);
	if (version == 1) {
	    version = 10;
	}
    } else {
	version = 11;
    }
    bfseek(fp, 0, SEEK_SET);

    printf
	(" name  length create-clock member  valid-time   level1:element  pack  size-x,y\n");
    offset = 0;
    while (1) {
	if (bfread_native(&rlens, 4, 1, fp) != 1)
	    break;
	bfread_native(rname, 1, 4, fp);
	real_stat.ni_rec++;
	if (frlen != 0 && rlens == 0) {
	    rlens = frlen;
	    strcpy(rname, "DATA");
	    eflag = 1;
	}
	bfread_native(&alen, 4, 1, fp);
	bfread_native(&systime, 4, 1, fp);
	systime_time_t = (time_t) systime;
	tp = gmtime((time_t *) & systime_time_t);
	printf("<%4s> %6d %4.4d%2.2d%2.2d%2.2d%2.2d", rname, rlens,
	       tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
	       tp->tm_hour, tp->tm_min);
	if (strcmp(rname, "DATA") == 0) {
	    n_data++;
	    dump_data(fp, &ladd);
	    /* for N_VERSION_11 */
	    if (version == 10) {
		offset = rlens - 32 - 36 - ladd;
	    } else {
		offset = rlens + 8 - 32 - 36 - ladd;
	    }
	} else if (strcmp(rname, "CNTL") == 0) {
	    dump_cntl(fp);
	    /* for N_VERSION_11 */
	    if (version == 10) {
		offset = rlens - 20 - 16 - 16;
	    } else {
		offset = rlens + 8 - 20 - 16 - 16;
	    }
	} else if (strcmp(rname, "SUBC") == 0) {
	    real_stat.ni_subc++;
	    dump_subc(fp);
	    /* for N_VERSION_11 */
	    if (version == 10) {
		offset = rlens - 20 - 4 * 3;
	    } else {
		offset = rlens + 8 - 20 - 4 * 3;
	    }
	} else if (strcmp(rname, "INFO") == 0) {
	    real_stat.ni_info++;
	    dump_info(fp);
	    /* for N_VERSION_11 */
	    if (version == 10) {
		offset = rlens - 20 - 4 - 20;
	    } else {
		offset = rlens + 8 - 20 - 4 - 20;
	    }
	} else if (strcmp(rname, "END ") == 0) {
	    dump_end(fp, &end_stat);
	    /* for N_VERSION_11 */
	    if (version == 10) {
		offset = rlens - 20 - 8;
	    } else {
		offset = rlens + 8 - 20 - 8;
	    }
	} else if (strcmp(rname, "NUSD") == 0) {
	    dump_nusd(fp, version, &nusd_stat);
	    /* for N_VERSION_11 */
	    if (version == 10) {
		offset = rlens - 20 - 100;
	    } else {
		offset = rlens + 8 - 20 - 100;
	    }
	} else {
	    /* for N_VERSION_11 */
	    if (version == 10) {
		offset = rlens - 20;
	    } else {
		offset = rlens + 8 - 20;
	    }
	}
	bfseek(fp, offset, SEEK_CUR);
	bfread_native(&rlene, 4, 1, fp);
	if (frlen == 0 && rlens != rlene) {
	    fprintf(stderr,
		    " !!!! inconsistent record length <s:%d,e:%d>\n",
		    rlens, rlene);
	    bfclose(fp);
	    return 0;
	} else {
	    printf("\n");
	}
    }
    printf("<<<  total DATA record = %d  >>>\n", n_data);
    if (real_stat.ni_rec != nusd_stat.ni_rec
		    || real_stat.ni_rec != end_stat.ni_rec) {
	fprintf(stderr,
		"inconsistent record number: NUSD: %lu, END :%lu, n_rec: %lu\n",
		(unsigned long)nusd_stat.ni_rec,
		(unsigned long)end_stat.ni_rec,
		(unsigned long)real_stat.ni_rec);
    } else {
	printf("<<<  total record = %lu  >>>\n",
			(unsigned long)real_stat.ni_rec);
    }
    if (real_stat.ni_info != nusd_stat.ni_info) {
	fprintf(stderr,
		"inconsistent info number: NUSD: %lu, n_info: %lu\n",
		(unsigned long)nusd_stat.ni_info,
		(unsigned long)real_stat.ni_info);
    } else {
	printf("<<<  total INFO record = %lu  >>>\n",
			(unsigned long)real_stat.ni_rec);
    }
    if (real_stat.ni_subc != nusd_stat.ni_subc) {
	fprintf(stderr,
		"inconsistent subc number: NUSD: %lu, n_subc: %lu\n",
		(unsigned long)nusd_stat.ni_subc,
		(unsigned long)real_stat.ni_subc);
    } else {
	printf("<<<  total SUBC record = %lu  >>>\n",
			(unsigned long)real_stat.ni_subc);
    }

    bfgetpos(fp, &real_stat.ni_filesize);
    if (memcmp(&real_stat.ni_filesize, &nusd_stat.ni_filesize, 8)
		    || (nusd_stat.ni_filesize4 != end_stat.ni_filesize4)) {
	char message[256];
	nusdas_snprintf(message, sizeof message,
		"inconsistent filesize: NUSD: 0x%QX 0x%PX, END :0x%PX, "
		"real filesize: 0x%QX (%Qu)\n",
		nusd_stat.ni_filesize,
		nusd_stat.ni_filesize4,
		end_stat.ni_filesize4,
		real_stat.ni_filesize,
		real_stat.ni_filesize);
	fputs(message, stderr);
    } else {
	char message[256];
	nusdas_snprintf(message, sizeof message,
		"<<<  filesize   = %Qu  >>>\n",
		real_stat.ni_filesize);
	fputs(message, stdout);
    }

    bfclose(fp);
    return 0;
}

	int
main(int argc, const char **argv)
{
	const char *filename = NULL;
	int i;
	if (argc <= 1) {
		fputs(
		"usage: nusdas_dump_rec filename (-l) (forced_rlen)\n"
		"    if -l specified and the record in the file is ?PAC\n"
		"    or ?UPC, base and amp values are printed!!\n",
		stderr);
		return 1;
	}
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'l') {
				Opts.long_output = 1;
			} else {
				fprintf(stderr, "unknown option %s\n",
						argv[i]);
				return 1;
			}
		} else if (filename == NULL) {
			filename = argv[i];
		} else if (str_is_all_digits(argv[i])) {
			Opts.forcedrlen = strtoul(argv[i], NULL, 0);
			printf(" frlen : %ld\n", Opts.forcedrlen);
		} else {
			fprintf(stderr, "extra argument <%s> ignored\n",
					argv[i]);
		}
	}
	return dumpfile(filename);
}
