#define _LARGE_FILES
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "nusdas.h"
#include "nwpl_capi.h"

/* global variables */

/* filelist */
#define MAX_N_NUSFILE 256
#define MAX_PATHLEN 256

char **nusfilepath;
int  nusfilenum = 0;
char nrdname[MAX_PATHLEN];
char elem_from[7];
char elem_to[7];
char type_from[17];
char type_buf[17];
char type_to[17];
char memb_from[4];
char memb_to[4];

/* NuSDaS file */
int  version;
/*int  index[1024];*/
int  n_dc;
int  n_vt;
int  n_lv;
int  n_el;
off_t  offset;
int  btadd;

int convcntl(FILE *fp){
	int rbuf[4];
	off_t offset_tmp;
	int p;
        int ii;
	int p_from = 0;
	int flag = 0;
	char *type_buf;
	char type_tmp[17];
        int  yyyy, mm, dd, hh, ff;
	char ymdhfc[13];
	int  btbuf;
	int  bttmp;
        char *elemlist;
        char *memblist;

	/* type123 */
	type_tmp[16] = '\0';
	type_buf = (char *)malloc(17);
	fseeko(fp, offset + 16, SEEK_SET);
	fread(type_buf, 16, 1, fp); 
	sprintf(type_tmp, "%16s", type_buf);
        if (type_from[0] != 0) {
          if (strncmp(type_tmp, type_from, 16) == 0) {
            fprintf(stderr, "Type %s is changed to %s. \n", type_from, type_to);
            fseeko(fp, offset + 16, SEEK_SET);
            fwrite(type_to, 16, 1, fp); 
          } else {
            fprintf(stderr, "Type 123 you specified (%s) is not match with the type 123 of NuSDaS (%s). \n", type_from, type_tmp);
            exit(-11);
          }
        }

	/* basetime */
	ymdhfc[12] = '\0';
	fseeko(fp, offset + 44, SEEK_SET);
	fread(&btbuf, sizeof(int), 1, fp);
        endian_swab4((char *)&btbuf, 1);
	bttmp = btbuf;
	bttmp = bttmp + btadd;
	nwp_seq2ymdhm(&yyyy, &mm, &dd, &hh, &ff, bttmp);
	sprintf(ymdhfc, "%4.4d%2.2d%2.2d%2.2d%2.2d", yyyy, mm, dd, hh, ff);
	fseeko(fp, offset + 32, SEEK_SET);
	fwrite(ymdhfc, 12, 1, fp); 
        endian_swab4((char *)&bttmp, 1);
	fwrite(&bttmp, sizeof(int), 1, fp);


	/* element */
	fseeko(fp, offset + 52, SEEK_SET);
	fread(&rbuf, sizeof(int), 4, fp); 
	n_dc = rbuf[0];
	n_vt = rbuf[1];
	n_lv = rbuf[2];
	n_el = rbuf[3];
	endian_swab4((char *)&n_dc, 1);
	endian_swab4((char *)&n_vt, 1);
	endian_swab4((char *)&n_lv, 1);
	endian_swab4((char *)&n_el, 1);


        if (memb_from[0] != 0){
          if ((memblist = (char*)malloc(n_dc * 4)) == NULL){
            fprintf(stderr, "malloc error at line %d in %s\n", __LINE__, __FILE__);
            exit(-10);
          }

          offset_tmp = offset + 172;
          fseeko(fp, offset_tmp, SEEK_SET);
          fread(memblist, 4, n_dc, fp);

          flag = 0;
          for (ii = 0; ii < n_dc; ii++ ){
            if (memcmp(memblist + ii * 4, memb_to, 4) == 0){
              fprintf(stderr, "Error! : The member %c%c%c%c is already defined in your NuSDaS file\n", 
                      memb_to[0], memb_to[1], memb_to[2], memb_to[3]);
              exit(-10);
            }
            if (memcmp(memblist + ii * 4, memb_from, 4) == 0){
              flag = 1;
              p_from = ii * 4;
              break;
            }
          }

          if (flag == 1) {
            offset_tmp = offset_tmp + p_from;
            fseeko(fp, offset_tmp, SEEK_SET);
            fwrite(memb_to, 4, 1, fp); 
          }

          free(memblist);
        }
        

        if (elem_from[0] != 0) {
          if ((elemlist = (char *)malloc(n_el * 6 + 1)) == NULL){
            fprintf(stderr, "malloc error at line %d in %s\n", __LINE__, __FILE__);
            exit(-10);
          }
          offset_tmp = offset + 172 + 4 * n_dc + 4 * n_vt * 2 + 6 * n_lv * 2;
          fseeko(fp, offset_tmp, SEEK_SET);
          fread(elemlist, 6 * n_el, 1, fp); 
          
          flag = 0;
          for (ii = 0; ii < n_el; ii++) {
            if (memcmp(elemlist + ii * 6, elem_to, 6) == 0){
              fprintf(stderr, "The element %c%c%c%c%c%c already existsin your NuSDaS file. Use another element name. \n", 
                      elem_to[0], elem_to[1], elem_to[2], elem_to[3], elem_to[4], elem_to[5]);
              exit(-10);
            }
            if (memcmp(elemlist + ii * 6 , elem_from, 6) == 0){
              flag = 1;
              p_from = ii * 6;
              break;
            }
          }
          if (flag == 1) {
            offset_tmp = offset_tmp + p_from;
            fseeko(fp, offset_tmp, SEEK_SET);
            fwrite(elem_to, 6, 1, fp); 
          }
          free(elemlist);
        }
	return 0;
}

int setoffset(char *rec_name, FILE *fp){
	int offset_buf; 
	char rec_buf[4];

	offset = -8;
	offset_buf = 0;

	memset(rec_buf, 0x20, 4);
	while (!strncmp(rec_name, rec_buf, 4) == 0) {
		offset = offset + 8 + offset_buf;
		fseeko(fp, offset, SEEK_SET);
		fread(&offset_buf, 4, 1, fp);
		endian_swab4((char *)&offset_buf, 1);
		fread(&rec_buf, 4, 1, fp);
	}
	fseeko(fp, offset, SEEK_SET);
	return 0;
}

int rewrite_data(int j) {
	FILE *fp;
	int  rec_len;
	char datarecname[4];
	char dataelmname[6];
        char datamemb[4];
	off_t offset_end;
        off_t off_rec;

	fp = fopen(nusfilepath[j], "r");
	setoffset("END ", fp);
	offset_end = offset;
	fclose(fp);

 
	
	fp = fopen(nusfilepath[j], "r+b");
	setoffset("DATA", fp);
	fseeko(fp, offset, SEEK_SET);
	while (offset < offset_end) {
		/* read record length */
		fread(&rec_len, sizeof(int), 1, fp); 
		endian_swab4((char *)&rec_len, 1);
                off_rec = ftello(fp);
		/* read record name */
		fread(datarecname, 4, 1, fp); 
		if (strncmp(datarecname, "DATA", 4) == 0) {
                  if (memb_from[0] != 0) {
			fseeko(fp, off_rec + 12, SEEK_SET);
			fread(datamemb, 4, 1, fp);
			if (strncmp(datamemb, memb_from, 4) == 0) {
				fseeko(fp, -4, SEEK_CUR);
				fwrite(memb_to, 4, 1, fp); 
			}
                  }

                  if (elem_from[0] != 0) {
			fseeko(fp, off_rec + 36, SEEK_SET);
			fread(dataelmname, 6, 1, fp);
			if (strncmp(dataelmname, elem_from, 6) == 0) {
				fseeko(fp, -6, SEEK_CUR);
				fwrite(elem_to, 6, 1, fp); 
			}
                  }
                }
                fseeko(fp, off_rec + (off_t)rec_len + 4, SEEK_SET);
		offset = offset + rec_len + 8;
	}
	fclose(fp);
	return 0;
}

int rewrite_cntl(int j) {
	FILE *fp;

	fp = fopen(nusfilepath[j], "r+b");
	setoffset("CNTL", fp);
	convcntl(fp);
	fclose(fp);
	return 0;
}


int getversion(int j) {
	FILE *fp;
	struct stat sb;
        char permission[7];
        int peri;

    
	fp = fopen(nusfilepath[j], "r");
	fseeko(fp, 96, SEEK_SET);
	fread(&version, sizeof(int), 1, fp);
 	endian_swab4((char *)&version, 1);
	if (version == 1) version = 11;
	if (version != 11 && version != 13) {
		fprintf(stderr, "NuSDaS Version is invalid.");
		return 99;
	}
	fclose(fp);
	stat(nusfilepath[j], &sb);
        sprintf(permission, "%o", sb.st_mode);
        permission[6] = '\0';
        peri = atoi(permission) - 100000;
        if (peri < 600) {
		fprintf(stderr, "Invalid file permission : %d\n", peri);
		return 99;
	}
	return 0;
}

int rewrite(void) {
	int j;
	for (j = 0; j < nusfilenum; j++) {
		fprintf(stderr, "Converting %s \n", nusfilepath[j]);
		if (getversion(j) == 0) {
			rewrite_cntl(j);
			rewrite_data(j);
		}
	}
	return 0;
}

int nrdopen(char *nrdname) {
	FILE *fp;
	DIR *dir;
	struct dirent *dp;
	char nrdname_recursive[MAX_PATHLEN];
	if ((fp = fopen(nrdname, "r")) == NULL) {
			fprintf(stderr, "%s%s\n", nrdname, " : No such file or directory.");
	} else {
		if ((dir = opendir(nrdname)) == NULL) {
			/* check if the file is NUSDAS or not */
			char nusd[4];
			fseeko(fp, 4, SEEK_SET);
			fread(&nusd, 4, 1, fp);
			if (strncmp(nusd, "NUSD", 4) == 0) {
				strcpy(nusfilepath[nusfilenum], nrdname);
				nusfilenum++;
			} else {
                          if (memcmp(nrdname + strlen(nrdname) - 4, ".def", 4) != 0) {
				fprintf(stderr, "%s%s\n", nrdname, " : not NuSDaS file.");
                          }
			}
		} else {
			for(dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
				if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
						strcpy(nrdname_recursive, nrdname);
						strcat(nrdname_recursive, "/");
						strcat(nrdname_recursive, dp->d_name);
						strcat(nrdname_recursive, "\0");
						nrdopen(nrdname_recursive);
				}
			}
		}
		closedir(dir);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int ii, rt, jj, nc;
	char *p;
        size_t len;

	/* */
	if (argc < 2) {
		fprintf(stderr, "Usage : ./nusconv NRD  [ -e ELEM_FROM ELEM_TO ] [ -t TYPE_FROM TYPE_TO ] [-m MEMB_FROM MEMB_TO ][-bt INCR ] \n");
		fprintf(stderr, "       NRD : NuSDaS Root Directory\n");
		fprintf(stderr, "       ELEM_FROM, ELEM_TO : Name of the element ELEM_FROM will be converted to ELEM_TO. \n");
		fprintf(stderr, "       TYPE_FROM, TYPE_TO : The type123 will be converted from TYPE_FROM to TYPE_TO. \n");
		fprintf(stderr, "       MEMB_FROM, MEMB_TO : The name of the member specified as MEMB_FROM will be converted to MEMB_TO. \n");
		fprintf(stderr, "                 Note: You can use '____' (four underscores) to indicate four spaces \n");
		fprintf(stderr, "       INCR : The basetime is added by INCR min. \n");
		exit(-99);
	} 

        memset(elem_from, 0, 6);
        memset(elem_to, 0, 6);
        memset(type_buf, 0, 16);
        memset(type_to, 0, 16);
        memset(memb_from, 0, 4);
        memset(memb_to, 0, 4);

	p = argv[1];
	strncpy(nrdname, p, strlen(p));
	btadd = 0;


	for (ii = 2; argv[ii]; ii++) {
		if (strncmp(argv[ii], "-e", 2) == 0) {
			p = argv[ii + 1];
                        len = strlen(p);
                        if (len > 6) {
                          fprintf(stderr, "Error: elem_from is too long.\n");
                          exit(-1);
                        }
			memcpy(elem_from, p, len);
                        memset(elem_from + len, 0x20, 6 - len);

			p = argv[ii + 2];
                        len = strlen(p);
                        if (len > 6) {
                          fprintf(stderr, "Error: elem_to is too long.\n");
                          exit(-1);
                        }
			memcpy(elem_to, p, len);
                        memset(elem_to + len, 0x20, 6 - len);
			ii = ii + 2;
		}
		else if (strncmp(argv[ii], "-m", 2) == 0) {
			p = argv[++ii];
                        len = strlen(p);
                        if (len == 4 && memcmp(p, "____", len) == 0) {
                          len = 0;
                        } else {
                          if (len > 4) {
                            fprintf(stderr, "Error: memb_from is too long.\n");
                            exit(-1);
                          }
                          memcpy(memb_from, p, len);
                        }
			memset(memb_from + len, 0x20, 4 - len);

			p = argv[++ii];
                        len = strlen(p);
                        if (len == 4 && memcmp(p, "____", len) == 0) {
                          len = 0;
                        } else {
                          if (len > 4) {
                            fprintf(stderr, "Error: memb_to is too long.\n");
                            exit(-1);
                          }
                          memcpy(memb_to, p, len);
                        }
			memset(memb_to + len, 0x20, 4 - len);
		}
		else if (strncmp(argv[ii], "-bt", 3) == 0) {
			btadd = atoi(argv[ii + 1]);
			ii = ii + 1;
		}
		else if (strncmp(argv[ii], "-t", 2) == 0) {
			p = argv[ii + 1];
                        len = strlen(p);
                        if (len > 16) {
                          fprintf(stderr, "Error: type_from is too long.\n");
                          exit(-1);
                        }

			memcpy(type_buf, p, len);
			memset(type_buf + len, 0x20, 16 - len);
			nc = 0;
			jj = 0;
			while (jj < 16) {
				if (type_buf[jj] == 0x20) {
					nc++;
				}
				jj++;
			}
			jj = 0;
			while (jj < nc) {
				type_from[jj] = 0x20;
				jj++;
			}
			while (jj < 16) {
			        type_from[jj] =  type_buf[jj - nc];
				jj++;
			}
			p = argv[ii + 2];
                        len = strlen(p);
                        if (len > 16) {
                          fprintf(stderr, "Error: type_to is too long.\n");
                          exit(-1);
                        }
			memcpy(type_to, p, len);
			memset(type_to + len, 0x20, 16 - len);
			ii = ii + 2;
		}
	}

        if ((nusfilepath = (char**)malloc(MAX_N_NUSFILE * sizeof(char*))) == NULL){
          fprintf(stderr, "malloc error! at line %d in %s\n", __LINE__, __FILE__);
          exit(-10);
        }
        for (ii = 0; ii < MAX_N_NUSFILE; ii++){
          if ((nusfilepath[ii] = (char*)malloc(MAX_PATHLEN)) == NULL){
            fprintf(stderr, "malloc error! at line %d in %s\n", __LINE__, __FILE__);
            exit(-10);
          }
        }


	rt = nrdopen(nrdname);
	rt = rewrite();
	return 0;
}
