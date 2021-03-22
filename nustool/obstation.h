
    struct  stn_joho_s {
        int                  bgn, fin;
        int                  high;
        float                lat, lon;
        struct  stn_joho_s  *next;
    };

    struct  station_s {
        int                  no;
        struct  stn_joho_s  *joho;
    };


int amd_search(int, int, float *, float *, int *);
int obs_search(int, int, float *, float *, int *);
int aws_search(int, int, float *, float *, int *);
int pub_search(int, int, float *, float *, int *);
