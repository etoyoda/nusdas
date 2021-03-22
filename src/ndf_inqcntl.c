#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
#include "sys_kwd.h"
#include "sys_err.h"
# define NEED_LONG_TO_SI8
# define NEED_MAKE_UI8
#include "sys_int.h"
#include <string.h>
# define NEED_MEMCPY6
#include "sys_string.h"
#include "sys_container.h"
#include "sys_endian.h"
#include "dfile.h"
#include "ndf.h"

	INLINE sym4_t
ndf_projname2(sym4_t projname3)
{
	switch (projname3) {
		case SYM4_MER:
			return SYM4_MR;
		default:
			return projname3;
	}
}

	int
nusndf_inq_cntl(union nusdfile_t *df, sym4_t query, void *dest, void *arg)
{
	int (**puttime)(N_SI4 time, void *arg);
	struct ndf_t *ndf = &df->ndf;
	N_SI8 *arg_si8;
	N_SI4 *si4ptr, *arg_si4;
	N_UI4 *sizptr;
	char *charptr;
	N_UI4 *cntl;
	unsigned i, imax;
	N_UI8 ofs;
	int r;
	if (dest == NULL) {
		return nus_err((NUSERR_IQ_DestNull, "can't write to NULL"));
	}
	switch (query) {
		/** nusdas_inq_nrdbtime() のため */
		case SYM4_BTLS:
			puttime = dest;
			r = (*puttime)(array4_get_value(ndf->btime, 0), arg);
			break;
		/** nusdas_inq_nrdvtime() のため */
		case SYM4_BTIM:
			si4ptr = dest;
			*si4ptr = array4_get_value(ndf->btime, 0);
			r = 1;
			break;
		case SYM4_VTLS:
			puttime = dest;
			r = 0;
			for (i = 0; i < ndf->nv; i++) {
				N_SI8 vt;
				vt = array8_get_value(ndf->vtime, i);
				r = (*puttime)(I8_HI(vt), arg);
				if (r) {
					break;
				}
			}
			break;
		/** nusdas_subc_{srf,tdif} のため */
		case SYM4_VIDX:
			arg_si8 = arg;
			si4ptr = dest;
			*si4ptr = array8_get_index(ndf->vtime, *arg_si8);
			r = 1;
			break;
		case SYM4_MIDX:
			arg_si4 = arg;
			si4ptr = dest;
			*si4ptr = array4_get_index(ndf->member, *arg_si4);
			r = 1;
			break;
		case SYM4_ZIDX:
			arg_si8 = arg;
			si4ptr = dest;
			*si4ptr = array8_get_index(ndf->plane, *arg_si8);
			r = 1;
			break;
		case SYM4_EIDX:
			arg_si8 = arg;
			si4ptr = dest;
			*si4ptr = array8_get_index(ndf->element, *arg_si8);
			r = 1;
			break;
		/** nusdas_inq_cntl() のため */
		case N_MEMBER_NUM:
			si4ptr = dest;
			*si4ptr = ndf->nm;
			r = 1;
			break;
		case N_MEMBER_LIST:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < ndf->nm) {
				return nus_err((NUSERR_IQ_ShortBuf, "mbl"));
			}
			for (i = 0; i < ndf->nm; i++) {
				si4ptr[i] = array4_get_value(ndf->member, i);
			}
			r = ndf->nm;
			break;
		case N_VALIDTIME_NUM:
			si4ptr = dest;
			*si4ptr = ndf->nv;
			r = 1;
			break;
		case N_VALIDTIME_LIST:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < ndf->nv) {
				return nus_err((NUSERR_IQ_ShortBuf, "vtl"));
			}
			for (i = 0; i < ndf->nv; i++) {
				N_SI8 vt;
				vt = array8_get_value(ndf->vtime, i);
				si4ptr[i] = I8_HI(vt);
			}
			r = ndf->nv;
			break;
		case N_VALIDTIME2_LIST:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < ndf->nv) {
				return nus_err((NUSERR_IQ_ShortBuf, "vtl"));
			}
			for (i = 0; i < ndf->nv; i++) {
				N_SI8 vt;
				vt = array8_get_value(ndf->vtime, i);
				si4ptr[i] = I8_LO(vt);
			}
			r = ndf->nv;
			break;
		case N_PLANE_NUM:
			si4ptr = dest;
			*si4ptr = ndf->nz;
			r = 1;
			break;
		case N_PLANE_LIST:
		case N_PLANE2_LIST: /* fake */
			charptr = dest;
			sizptr = arg;
			if (*sizptr < ndf->nz) {
				return nus_err((NUSERR_IQ_ShortBuf, "pll"));
			}
			for (i = 0; i < ndf->nz; i++) {
				N_SI8 pl;
				pl = array8_get_value(ndf->plane, i);
				memcpy6(charptr, (char *)&pl);
				charptr += 6;
			}
			r = ndf->nz;
			break;
		case N_ELEMENT_NUM:
			si4ptr = dest;
			*si4ptr = ndf->ne;
			r = 1;
			break;
		case N_ELEMENT_LIST:
			charptr = dest;
			sizptr = arg;
			if (*sizptr < ndf->ne) {
				return nus_err((NUSERR_IQ_ShortBuf, "ell"));
			}
			for (i = 0; i < ndf->ne; i++) {
				N_SI8 el;
				el = array8_get_value(ndf->element, i);
				memcpy6(charptr, (char *)&el);
				charptr += 6;
			}
			r = ndf->ne;
			break;
		/* 1.3 で nusdas_inq_cntl() に追加された項目 */
		case N_NUSD_NBYTES:
			si4ptr = dest;
			nusndf_load_cntl(df);
			cntl = (N_UI4 *)df->ndf.nusd;
			*si4ptr = NTOH4(cntl[2]) + 8;
			r = 1;
			break;
		case N_NUSD_CONTENT:
			sizptr = arg;
			nusndf_load_cntl(df);
			cntl = (N_UI4 *)df->ndf.nusd;
			r = NTOH4(cntl[2]) + 8;
			if (*sizptr < (unsigned)r) {
				if (*sizptr > 0) {
					memcpy(dest, cntl, *sizptr);
				}
				return nus_err((NUSERR_IQ_ShortBuf, "nc"));
			}
			memcpy(dest, cntl, r);
			break;
		case N_CNTL_NBYTES:
			si4ptr = dest;
			cntl = nusndf_load_cntl(df);
			*si4ptr = NTOH4(cntl[2]) + 8;
			r = 1;
			break;
		case N_CNTL_CONTENT:
			sizptr = arg;
			cntl = nusndf_load_cntl(df);
			r = NTOH4(cntl[2]) + 8;
			if (*sizptr < (unsigned)r) {
				if (*sizptr > 0) {
					memcpy(dest, cntl, *sizptr);
				}
				return nus_err((NUSERR_IQ_ShortBuf, "nc"));
			}
			memcpy(dest, cntl, r);
			break;
		/** nusdas_grid() のため */
		case N_PROJECTION:
			si4ptr = dest;
			cntl = nusndf_load_cntl(df);
			*si4ptr = ndf_projname2(cntl[17]);
			r = 1;
			break;
		case N_GRID_SIZE:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < 2) {
				return nus_err((NUSERR_IQ_ShortBuf, "sz"));
			}
			cntl = nusndf_load_cntl(df);
			si4ptr[0] = NTOH4(cntl[18]);
			si4ptr[1] = NTOH4(cntl[19]);
			r = 2;
			break;
		case N_GRID_BASEPOINT:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < 4) {
				return nus_err((NUSERR_IQ_ShortBuf, "bp"));
			}
			cntl = nusndf_load_cntl(df);
			si4ptr[0] = NTOH4(cntl[20]);
			si4ptr[1] = NTOH4(cntl[21]);
			si4ptr[2] = NTOH4(cntl[22]);
			si4ptr[3] = NTOH4(cntl[23]);
			r = 4;
			break;
		case N_GRID_DISTANCE:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < 2) {
				return nus_err((NUSERR_IQ_ShortBuf, "dxy"));
			}
			cntl = nusndf_load_cntl(df);
			si4ptr[0] = NTOH4(cntl[24]);
			si4ptr[1] = NTOH4(cntl[25]);
			r = 2;
			break;
		case N_STAND_LATLON:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < 4) {
				return nus_err((NUSERR_IQ_ShortBuf, "std"));
			}
			cntl = nusndf_load_cntl(df);
			si4ptr[0] = NTOH4(cntl[26]);
			si4ptr[1] = NTOH4(cntl[27]);
			si4ptr[2] = NTOH4(cntl[28]);
			si4ptr[3] = NTOH4(cntl[29]);
			r = 4;
			break;
		case N_SPARE_LATLON:
			si4ptr = dest;
			sizptr = arg;
			if (*sizptr < 4) {
				return nus_err((NUSERR_IQ_ShortBuf, "oth"));
			}
			cntl = nusndf_load_cntl(df);
			si4ptr[0] = NTOH4(cntl[30]);
			si4ptr[1] = NTOH4(cntl[31]);
			si4ptr[2] = NTOH4(cntl[32]);
			si4ptr[3] = NTOH4(cntl[33]);
			r = 4;
			break;
		case SYM4_VALU:
			si4ptr = dest;
			cntl = nusndf_load_cntl(df);
			*si4ptr = ndf_projname2(cntl[34]);
			r = 1;
			break;
	        case N_INDX_SIZE:
			sizptr = arg;
			if(*sizptr < 1){
				return nus_err((NUSERR_IQ_ShortBuf, "idsize"));
			}
			imax = ndf->nm * ndf->nz
				* ndf->nv * ndf->ne;
			si4ptr = dest;
			*si4ptr = imax;
			r = 1;
			break;
	        case N_ELEMENT_MAP:
			imax = ndf->nm * ndf->nz
				* ndf->nv * ndf->ne;
			sizptr = arg;
			if(*sizptr < imax){
				return nus_err((NUSERR_IQ_ShortBuf, "emap"));
			}
			charptr = dest;
			if (ndf->indx4) {
				for (i = 0; i < imax; i++) {
					charptr[i] = 1;
					if (~ndf->indx4[i] == 0) {
						charptr[i] = 0;
					}
				}
			} else {
				for (i = 0; i < imax; i++) {
					N_UI8 *headtab = 
						(N_UI8 *)(ndf->indx + 16);
					charptr[i] = 1;
					if (ui8_maxp(headtab[i])) {
						charptr[i] = 0;
					}
				}
			}
			r = imax;
			break;
	        case N_DATA_MAP:
			imax = ndf->nm * ndf->nz
				* ndf->nv * ndf->ne;
			sizptr = arg;
			if(*sizptr < imax){
				return nus_err((NUSERR_IQ_ShortBuf, "dmap"));
			}
			charptr = dest;
			if (ndf->indx4) {
				for (i = 0; i < imax; i++) {
					charptr[i] = 1;
					if (~ndf->indx4[i] == 0) {
						charptr[i] = 0;
					} else{
						ofs = long_to_si8(
							NTOH4(ndf->indx4[i]));
						if (i8_zerop(ofs)) {
							charptr[i] = 0;
						}
					}
				}
			} else {
				for (i = 0; i < imax; i++) {
					N_UI8 *headtab = 
						(N_UI8 *)(ndf->indx + 16);
					charptr[i] = 1;
					if (ui8_maxp(headtab[i])) {
						charptr[i] = 0;
					} else{
						ofs = NTOH8(headtab[i]);
						if (i8_zerop(ofs)) {
							charptr[i] = 0;
						}
					}
				}
			}
			r = imax;
			break;
		default:
			r = nus_err((NUSERR_IQ_BadParam,
				"BUG: bad query %Ps (%d)", query, query));
	}
	return r;
}

