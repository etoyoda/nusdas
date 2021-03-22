#ifndef NUSDAS_H
typedef unsigned short N_UI2;
typedef short N_SI2;
typedef int N_SI4;
#endif

#ifndef SRF_KANS
typedef struct {
	N_SI4 snum;
	N_SI4 type;
	float lat;
	float lon;
	float hh;
	float wh;
	char name[10];
	char name2[24];
} SRF_AMD_SINFO;
#endif

void srf_amd_aqc(const N_UI2 aqc_in[],
		int num,
		N_SI2 aqc_out[],
		const char *param);

int srf_amd_rdic(SRF_AMD_SINFO amd[],
		int amdnum,
		int btime,
		int amd_type);

int srf_search_amdstn(const SRF_AMD_SINFO *amd,
		int ac,
		int stn,
		int amd_type);

int srf_amd_slct(const N_SI4 st_r[],
		int n_st_r,
		void *d_r,
		const char t_r[2],
		N_SI4 st_n[],
		int n_st_n,
		void *d_n,
		const char t_n[2],
		int sort_f);

int srf_lv_set(N_SI4 idat[],
		const float fdat[],
		int dnum,
		N_SI4 ispec[],
		const char *param);

int srf_lv_trans(const N_SI4 idat[],
		float fdat[],
		N_SI4 dnum,
		const N_SI4 ispec[]);

int rdr_lv_trans(N_SI4 idat[],
		float fdat[],
		int dnum,
		const char *param);

int srf_rd_rdic(int stnum,
		int iseq,
		float *lat,
		float *lon,
		float *hh,
		N_SI4 *offx,
		N_SI4 *offy,
		N_SI4 *type1,
		N_SI4 *type2);
