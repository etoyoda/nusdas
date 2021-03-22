#ifndef  _INCLUDE_RBR_H 
#define  _INCLUDE_RBR_H 


#include   <time.h>


#define    RBR_IM        2560
#define    RBR_JM        3360
#define    RBR_LAT0      48.0
#define    RBR_LON0     118.0
#define    RBR_DLAT_INV   120
#define    RBR_DLON_INV    80


struct rbr_grid_t {
            int     im, jm;
            float   lat0, lon0;
        };


char   *rbr_time (time_t, int);
time_t  rbr_to_utc (char *, int);
char   *pandora_time (time_t);
time_t  pandora_to_time_t (char *);


void  set_rbr_dir (char *);


int  rbr_header (void *, int, int *, char *, int *, int *, char **);
int  rbr_block  (void *, float *, float *, int *, unsigned char **);
int  rbr_ll_grid(void *, int, time_t, unsigned char *);

int  rp13_read (char *, char *, char *, int *, char *,  int *,
                char *, char *, void *, char *, int *);

int  rp13_size (char *, char *, char *, int *, char *,  int *,
                char *, char *, void *, char *, int *);

int  rbr_read (char *, char *, void *);


int  rbr_file_valid (time_t);
int  rbr_file_stat ();
int  nusdir_rbr (char *);

#endif
