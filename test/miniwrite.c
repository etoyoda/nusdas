#include <stdio.h>
#include <nwpl_capi.h>
#include <nusdas.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ctype.h>

#define VERBOSE 0

static int rm_rf(const char *path)
{
	char buf[1024];
	size_t pathlen = strlen(path);
	DIR *dp;
	struct dirent *leaf;
	dp = opendir(path);
	if (dp == NULL) {
		perror(path);
		return -1;
	}
	while ((leaf = readdir(dp)) != NULL) {
		struct stat stb;
		if (leaf->d_name[0] == '.') {
			if (leaf->d_name[1] == '\0') {
				continue;
			} else if (leaf->d_name[1] == '.' && !leaf->d_name[2]) {
				continue;
			}
		}
		if (strlen(leaf->d_name) + pathlen + 2 > sizeof(buf)) {
			fputs("too long pathname\n", stderr);
			closedir(dp);
			return -1;
		}
		strcpy(buf, path);
		strcat(buf, "/");
		strcat(buf, leaf->d_name);
		if (stat(buf, &stb) < 0) {
			perror(buf);
			closedir(dp);
			return -1;
		}
		if (S_ISDIR(stb.st_mode)) {
			if (rm_rf(buf) < 0) {
				closedir(dp);
				return -1;
			}
		} else {
			if (unlink(buf) < 0) {
				perror(buf);
				closedir(dp);
				return -1;
			}
		}
	}
	if (closedir(dp) == -1) {
		perror(path);
		return -1;
	}
	if (rmdir(path) == -1) {
		perror(path);
		return -1;
	}
	return 0;
}

int mkdir_f(const char *pathname, int mode)
{
	struct stat stb;
	int r;
	r = stat(pathname, &stb);
	if (r == 0) {
		if (S_ISDIR(stb.st_mode)) {
			fprintf(stderr, "W !rm -r %s\n", pathname);
			r = rm_rf(pathname);
			if (r) {
				return -1;
			}
		} else {
			fprintf(stderr, "W !rm %s\n", pathname);
			r = unlink(pathname);
			if (r) {
				perror(pathname);
				return -1;
			}
		}
	}
	r = mkdir(pathname, mode);
	if (r < 0) {
		perror(pathname);
		return -1;
	}
	return 0;
}

FILE *defopen(int nrd)
{
	char buf[32];
	FILE *fp;

	sprintf(buf, "NUSDAS%02u", nrd);
	if (mkdir_f(buf, 0755) != 0) {
		return NULL;
	}

	sprintf(buf, "NUSDAS%02u/nusdas_def", nrd);
	if (mkdir_f(buf, 0755) != 0) {
		return NULL;
	}

	sprintf(buf, "NUSDAS%02u/nusdas_def/auto.def", nrd);
	fp = fopen(buf, "w");
	if (fp == NULL)
		perror(buf);
	return fp;
}

int newnus(int nrd, const char *line)
{
	static int nrd_prev = -1;
	static FILE *fp = NULL;
	if (nrd < 1 || nrd >= 100) {
		fprintf(stderr, "invalid nrd %d\n", nrd);
		return -1;
	}
	if (line == NULL) {
		fclose(fp);
		fp = NULL;
		return 0;
	}
	if (fp) {
		if (nrd_prev != nrd) {
			fclose(fp);
			fp = defopen(nrd);
		}
	} else {
		fp = defopen(nrd);
	}
	if (fp == NULL)
		return -1;
	nrd_prev = nrd;
	fputs(line, fp);
	return 0;
}

int rmdset(int nrd)
{
	char buf[32];
	sprintf(buf, "NUSDAS%02d", nrd);
	return rm_rf(buf);
}

void mkdset(void)
{
	newnus(10, "path nwp_path_bs\n");
	newnus(10, "type1 _GSM LL PP\n");
	newnus(10, "type2 FC SV\n");
	newnus(10, "type3 STD1\n");
	newnus(10, "validtime 3 HOUR in\n");
	newnus(10, "validtime1 ARITHMETIC 0 6\n");
	newnus(10, "plane 3\n");
	newnus(10, "plane1 SURF 1000 500\n");
	newnus(10, "element 4\n");
	newnus(10, "elementmap T      0\n");
	newnus(10, "elementmap Z      1 0 1 1\n");
	newnus(10, "elementmap PSEA   1 1 0 0\n");
	newnus(10, "elementmap RAIN 2 1 0 0 0\n");
	newnus(10, "                  2 1 0 0\n");
	newnus(10, "size 36 19\n");
	newnus(10, "basepoint 1 1 0E 90N\n");
	newnus(10, "distance 1.25 1.25\n");
	newnus(10, "packing 2UPC\n");
	newnus(10, NULL);
}

#define BUFNELEMS (36 * 19)

int main()
{
	float buf[BUFNELEMS + 2];
	int i;
	N_SI4 btime, buflen, r;

	mkdset();

	for (i = 0; i < BUFNELEMS; i++) {
		buf[i] = i;
	}
	btime = nwp_ymdhm2seq(2006, 7, 24, 12, 0);

	buflen = BUFNELEMS;
	r = nusdas_write("_GSMLLPP", "FCSV", "STD1", &btime, "    ",
			&btime, "SURF  ", "PSEA  ", buf, N_R4,
			&buflen);
	printf("nusdas_write(..., %d) => %d\n", (int)buflen, (int)r);
	if (r < buflen)
		return -1;

	r = nusdas_allfile_close(N_FOPEN_ALL);
	printf("nusdas_allfile_close => %d\n", (int)r);
#if 0
	if (r < 0)
		return -1;
#endif

	memset(buf, 0xFF, sizeof(buf));

	buflen = BUFNELEMS;
	r = nusdas_read("_GSMLLPP", "FCSV", "STD1", &btime, "    ",
			&btime, "SURF  ", "PSEA  ", buf, N_R4,
			&buflen);
	printf("nusdas_read(..., %d) => %d\n", (int)buflen, (int)r);
	if (r < 0) {
		return -1;
	}

	r = 0;
	for (i = 0; i < BUFNELEMS; i++) {
		if (fabs(buf[i] - i) < 0.011)
			continue;
		fprintf(stderr, "broken value %d: %12.3f\n", i, buf[i]);
		r++;
	}

	if (r == 0 && getenv("KEEP_NUSDIR") == NULL) {
		r |= rmdset(10);
	}

	if (r == 0)
		puts("ok");
	return r;
}
