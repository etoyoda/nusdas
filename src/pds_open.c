/** @file
 * @brief パンドラデータセット処理ルーチン
 */
#include "config.h"
#include "nusdas.h"
#include <stdlib.h>
#include "internal_types.h"
#include <string.h>
#include "sys_mem.h"
# define NEED_STRING_DUP
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_file.h"
# define NEED_MEMCPY_HTON4
# define NEED_PEEK_N_UI4
# define NEED_PEEK_SYM4_T
# define NEED_PEEK_FLOAT
# define NEED_MEMCPY_HTON4
#include "sys_endian.h"
#include "sys_err.h"
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
# define NEED_MEM6SYM8
#define NEED_STR2SYM8
#include "sys_sym.h"
#include "sys_time.h"
#include "dset.h"
#include "pds.h"
#include "dfile.h"
#include "glb.h"
#include "ndf_codec.h"
#include "sys_container.h"
#include "sys_err.h"
#include "nus_gzlib.h"
# define NEED_SI8_CMP
# define NEED_SYM8OP
#include "sys_int.h"

#ifndef  PANDORA_SERVER_LIST
# define PANDORA_SERVER_LIST "PANDORA_SERVER_LIST"
#endif

#include "pandora_lib.h"

#if USE_NET

static pandora_data *pData = NULL;

	static int
pds_init(void)
{
	if (pData) {
		return 0;
	}
	if ((pData = pdr_new()) == NULL) {
		return nus_err((NUSERR_MemShort,
					"cannot initialize pandora library"));
	}
	return 0;
}

	static int
pds_finalize(void)
{
	if (pData == NULL) {
		return 0;
	}
	return 1;
}

	static int
PDS_SetPath(nustype_t *nustype, const nusdims_t *dim, 
	    struct cut_rectangle *ib_cut)
{
	/* 配列長の見積り:
	 * (type:18) + 1 + 3 * ((time:15) + 1) + (member:4) + 1
	 * + 3 * (plane-or-elem:6 + 1) + NUL = 94
	 */
	static char buffer[128];
	N_SI4 basetime = dim->basetime;
	N_SI4 validtime1 = dim->validtime1;
	N_SI4 validtime2 = dim->validtime2;
	sym4_t member = dim->member;
	sym8_t plane1 = dim->plane1;
	sym8_t plane2 = dim->plane2;
	sym8_t element = dim->element;

	if (member == SYM4_ALLSPACE) {
		memcpy4((char *)&member, "none");
	}
	if (sym8_is_allspace(plane1)) {
		memcpy4((char *)&plane1, "none");
	}
	if (sym8_is_allspace(plane2)) {
		memcpy4((char *)&plane2, "none");
	}
	if (sym8_is_allspace(element)) {
		memcpy4((char *)&element, "none");
	}
	if(dim->basetime == INT_MAX){
		basetime = 0;
		validtime1 = 0;
		validtime2 = 1;
	}
	if(dim->basetime == INT_MIN){ /* for basetime index */
		nusdas_snprintf(buffer, sizeof buffer,
				"%#ys",
				nustype);
	}
	else if(dim->validtime1 == INT_MIN){ /* for validtime index */
		nusdas_snprintf(buffer, sizeof buffer,
				"%#ys/%#PT/%Ps",
				nustype,
				basetime,
				member);
	}
	else if(dim->validtime2 == INT_MIN){
		nusdas_snprintf(buffer, sizeof buffer,
				"%#ys/%#PT/%Ps/%#PT",
				nustype,
				basetime,
				member,
				validtime1);
	}
	else{
		int ixst, ixen, jyst, jyen;
		if(ib_cut != NULL && (! cut_rectangle_disabled(ib_cut))){
			ixst = ib_cut->cr_xofs + 1;
			ixen = ib_cut->cr_xnelems + ixst - 1;
			jyst = ib_cut->cr_yofs + 1;
			jyen = ib_cut->cr_ynelems + jyst - 1;
			nusdas_snprintf(buffer, sizeof buffer,
					"%#ys/%#PT/%Ps/%#PT/%#PT"
					"/%#Qs/%#Qs/%#Qs/%Pd,%Pd/%Pd,%Pd",
					nustype,
					basetime,
					member,
					validtime1,
					validtime2,
					plane1,
					plane2,
					element,
					jyst,
					jyen,
					ixst,
					ixen);
			
		}
		else{

			nusdas_snprintf(buffer, sizeof buffer,
					"%#ys/%#PT/%Ps/%#PT/%#PT"
					"/%#Qs/%#Qs/%#Qs",
					nustype,
					basetime,
					member,
					validtime1,
					validtime2,
					plane1,
					plane2,
					element);
		}
	}
	pdr_set_path(pData, buffer);
	return 0;
}

/** 所属のデータファイルを閉じる
 */
	static int
pds_close(nusdset_t *ds UNUSED, int flags UNUSED)
{
	pds_finalize();
	return 0;
}

/** 今後二度とこの pds を参照しない前提で (他の pds は使われるかもしれない)
 * 全リソースを廃棄する
 */
	static int
pds_delete(nusdset_t *ds UNUSED)
{
	pds_finalize();
	return 0;
}

/** 今後の動作に支障がない範囲で
 * キャッシュなど不要リソースを破棄する
 */
	static int
pds_compact(nusdset_t *ds UNUSED)
{
	return 0;
}

	static int
pds_close_file(nusdset_t *ds UNUSED, const nusdims_t *dim UNUSED)
{
	pds_finalize();
	return 0;
}

	static union nusdfile_t *
pds_findfile(nusdset_t *ds UNUSED,
		const nusdims_t *dim UNUSED, int open_flags UNUSED)
{
	nus_err((NUSERR_NotImplemented, "findfile unimplemented for PDS"));
	return NULL;
}

static int
PDS_set_gzip_encoding()
{
#ifdef USE_ZLIB
	pdr_set_accept_encoding_header(pData, "gzip");	
#endif
	return 0;
}

	static int
PDSSetAccept(sym4_t wanted_fmt, char *rtype)
{
	switch (wanted_fmt) {
		case SYM4_R4:
			pdr_set_accept_header(pData,
					"application/x-nusdas_rawdata, "
					"application/x-float32-stream;q=0.5");
			PDS_set_gzip_encoding();
			strcpy(rtype, "data");
			break;
		case SYM4_I1:
			pdr_set_accept_header(pData,
					"application/x-nusdas_rawdata, "
					"application/x-uint8-stream;q=0.6, "
					"application/x-int32-stream;q=0.5, "
					"application/x-float32-stream;q=0.3");
			PDS_set_gzip_encoding();
			strcpy(rtype, "data");
			break;
		case SYM4_I2:
		case SYM4_I4:
			pdr_set_accept_header(pData,
					"application/x-nusdas_rawdata, "
					"application/x-int32-stream;q=0.5, "
					"application/x-float32-stream;q=0.3");
			PDS_set_gzip_encoding();
			strcpy(rtype, "data");
			break;
		case SYM4_R8:
			pdr_set_accept_header(pData,
					"application/x-nusdas_rawdata, "
					"application/x-float32-stream;q=0.5");
			PDS_set_gzip_encoding();
			strcpy(rtype, "data");
			break;
	        case SYM4_BTLS:
			pdr_set_accept_header(pData,
					      "text/plain");
			strcpy(rtype, "index");
			break;
	        case SYM4_VTLS:
			pdr_set_accept_header(pData,
					      "text/plain");
			strcpy(rtype, "index");
			break;
                case SYM4_RGAU:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-nusdas_rgau;q=0.5");
			strcpy(rtype, "meta_rgau");
			break;
                case SYM4_ZHYB:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-nusdas_zhyb;q=0.5");
			strcpy(rtype, "meta_zhyb");
			break;
	        case SYM4_ETA:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-nusdas_eta;q=0.5");
			strcpy(rtype, "meta_eta");
			break;
	        case SYM4_SIGM:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-nusdas_sigm;q=0.5");
			strcpy(rtype, "meta_sigm");
			break;
	        case SYM4_INFO:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-nusdas_info;q=0.5");
			strcpy(rtype, "meta_info");
			break;
	         case SYM4_TDIF:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-nusdas_tdif;q=0.5");
			strcpy(rtype, "meta_tdif");
			break;
	         case SYM4_LOCA:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo");
			strcpy(rtype, "meta_loca");
			break;
	         case SYM4_RADR:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-int32-stream;q=0.5");
			strcpy(rtype, "meta_radr");
			break;
	         case SYM4_RADS:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-int32-stream;q=0.5");
			strcpy(rtype, "meta_rads");
			break;
	         case SYM4_ISPC:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-int32-stream;q=0.5");
			strcpy(rtype, "meta_ispc");
			break;
	         case SYM4_DPRD:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
 					      "application/x-int32-stream;q=0.5");
			strcpy(rtype, "meta_dprd");
			break;
	         case SYM4_THUN:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo, "
					      "application/x-int32-stream;q=0.5");
			strcpy(rtype, "meta_thun");
			break;
	         case SYM4_DELT:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_subcinfo");
			strcpy(rtype, "meta_delt");
			break;
	         case SYM4_CNTL:
			pdr_set_accept_header(pData, 
					      "text/x-rd");
			strcpy(rtype, "meta_cntl.rd");
			break;
	         case SYM4_DEF:
			pdr_set_accept_header(pData, 
					      "text/x-rd");
			strcpy(rtype, "meta_def.rd");
			break;
	         case SYM4_DATA:
			pdr_set_accept_header(pData, 
					      "text/x-rd");
			strcpy(rtype, "meta_data.rd");
			break;
	        case SYM4_GRID:
			pdr_set_accept_header(pData, 
					      "application/x-nusdas_grid");
			strcpy(rtype, "meta_");
			break;
	        case SYM4_CEMP:
			pdr_set_accept_header(pData, 
					      "application/x-uint8-stream");
			strcpy(rtype, "meta_cntlemap");
			break;
	        case SYM4_CDMP:
			pdr_set_accept_header(pData, 
					      "application/x-uint8-stream");
			strcpy(rtype, "meta_cntldmap");
			break;
	        case SYM4_DEMP:
			pdr_set_accept_header(pData, 
					      "application/x-uint8-stream");
			strcpy(rtype, "meta_defemap");
			break;
		default:
			return nus_err((NUSERR_RD_NoCodec,
					"buffer type %Ps not supported",
					wanted_fmt));
			break;
	}
	return 0;
}
#ifdef USE_ZLIB
static int
PDS_decmp_gzip(char *cmp_data, int cmp_size, 
	       char **decmp_data, int *decmp_size)
{
	int alloc_size;
	int r;
	alloc_size = nusgz_inq_decompressed_size((unsigned char*)cmp_data, 
						 cmp_size) + 100;
	if((*decmp_data = (char*)nus_malloc(alloc_size)) == NULL){
		return nus_err((NUSERR_MemShort, "PDS_decmp_gzip"));
	}
	r = nusgz_decompress((unsigned char*)cmp_data, cmp_size, 
			     (unsigned char*)*decmp_data, alloc_size);    
	if(r < 0){
		nus_free(*decmp_data);
		*decmp_data = NULL;
		return nus_err((NUSERR_IO, "nusgz_decompress error"));
	}
	*decmp_size = r;
	return r;
}
#endif

	static int
PDSLoadData(struct pds_t *pds, const nusdims_t *dim, sym4_t wanted_fmt,
	    struct cut_rectangle *ib_cut,
	    void **get_data, char **ctype, int *gzip_decoded)
{
	int r;
	char rtype[80];
	char *encoding;
	int  get_size;
	static int saved_rc = -99;
	char *server_port;
	unsigned alloc_siz;
	
	if ((r = pds_init()) != 0) {
		return r;
	}
	if (pds->comm.dead_flag == 1){
		nus_debug(("dead_flag has been already set for %ys, %s", 
			  &pds->comm.nustype, pds->server));
		return saved_rc;
	}
	*gzip_decoded = 0;
	PDS_SetPath(&pds->comm.nustype, dim, ib_cut);
	pdr_req_hdr_init(pData);
	alloc_siz = strlen(pds->server) + 20;
	if ((server_port = (char*)nus_malloc(alloc_siz)) == NULL) {
		return nus_err((NUSERR_MemShort, "malloc error"));
	}
	nusdas_snprintf(server_port, alloc_siz, 
			"%s:%d", pds->server, pds->port);
	pdr_set_server(pData, server_port);
	pdr_set_host_header(pData, server_port);
	free(server_port);
	pdr_set_root(pData, pds->path);
	if ((r = PDSSetAccept(wanted_fmt, rtype)) != 0) {
		return r;
        }
	pdr_data_free(pData);
	pdr_set_resource_type(pData, rtype);
	r = pdr_process(pData);
	nus_debug(("Request URI=http://%s:%d%s",
		  pdr_get_host(pData), pdr_get_port(pData),
		  pdr_get_request_path(pData)));
	if (r < 0) {
		char *hret;
		int remote_rc;
		hret = pdr_header_find(pData, "X-Nusdas-Return-Code");
		if (hret) {
			remote_rc = atoi(hret);
			if (remote_rc >= 0) {
				remote_rc = -99;
			}
		} else {
			remote_rc = -99;
		}
		if (r <= -100) {
			nus_err((remote_rc, "Pandora server error %d", -r));
		} else {
			nus_err((remote_rc, "Pandora request error %d", r));
		}
		nus_warn(("Request URI=http://%s:%d%s",
			pdr_get_host(pData), pdr_get_port(pData),
			pdr_get_request_path(pData)));
		/** データセットの死亡マーク */
		if (r == PDR_ERR_CONNECT){
			pds->comm.dead_flag = 1;
			saved_rc = remote_rc;
			nus_debug(("dead_flag is set for %ys, %s", 
				   &pds->comm.nustype, pds->server));
		}
		return remote_rc;
	}
	get_size = r;
	if ((*get_data = pdr_get_data(pData)) == NULL) {
		return nus_err((-99, "No response body"));
	}
	if ((*ctype = pdr_header_find(pData, "Content-Type")) == NULL) {
		return nus_err((-99, "Content-Type missing"));
	}
	encoding = pdr_header_find(pData,"Content-Encoding");
#ifdef USE_ZLIB
	/** gzip 対応 */
	if(encoding != NULL && strstr(encoding, "gzip") != NULL){
		int  decmp_size;
		char *decmp_data;
		r = PDS_decmp_gzip(*get_data, get_size, 
				   &decmp_data, &decmp_size);
		if(r < 0){
			return r;
		}
		r = decmp_size;
		*get_data = decmp_data;
		/* *gzip_decode == 1 のときは、
		 *get_data 使用後に nus_free が必要 */
		*gzip_decoded = 1;
	}
#endif
	return r;
}
	INLINE int
PDSSetCodec(struct pds_t *pds, sym4_t packing, sym4_t missing,
		sym4_t wanted_type)
{
	if (pds->codec && pds->codec->decode
			&& pds->codec->packing == packing
			&& pds->codec->missing == missing
			&& pds->codec->bffm == wanted_type) {
		return 0;
	}
	pds->codec = ndf_get_codec(packing, missing, wanted_type);
	if (pds->codec == NULL || pds->codec->decode == NULL) {
		return nus_err((NUSERR_RD_NoCodec,
					"missing codec <%Ps|%Ps|%Ps>",
					packing, missing, wanted_type));
	}
	return 0;
}

	INLINE int
PDSEvalContentType(struct pds_t *pds, const char *ctype,
		const N_UI1 *get_data, int nbytes,
		sym4_t wanted_type,
                N_UI4 gridsize[2],
                sym4_t *packing, sym4_t *missing, int *offset)
{
	if (streq(ctype, "application/x-float32-stream")) {
		*packing = SYM4_R4;
		*missing = SYM4_NONE;
		gridsize[0] = 1;
		gridsize[1] = nbytes / 4;
		*offset = 0;
	} else if (streq(ctype, "application/x-int32-stream")) {
		*packing = SYM4_I4;
		*missing = SYM4_NONE;
		gridsize[0] = 1;
		gridsize[1] = nbytes / 4;
		*offset = 0;
	} else if (streq(ctype, "application/x-uint8-stream")) {
		*packing = SYM4_I1;
		*missing = SYM4_NONE;
		gridsize[0] = 1;
		gridsize[1] = nbytes;
		*offset = 0;
	} else if (streq(ctype, "application/x-double64-stream")) {
		*packing = SYM4_R8;
		*missing = SYM4_NONE;
		gridsize[0] = 1;
		gridsize[1] = nbytes / 8;
		*offset = 0;
	} else if (streq(ctype, "application/x-nusdas_rawdata")) {
		gridsize[0] = PEEK_N_UI4(get_data + 0);
		gridsize[1] = PEEK_N_UI4(get_data + 4);
		*packing = PEEK_sym4_t(get_data + 8);
		*missing = PEEK_sym4_t(get_data + 12);
		*offset = 16;
	} else {
		return nus_err((NUSERR_RD_NoCodec,
					"unidentified content type %s",
					ctype));
	}
	if (PDSSetCodec(pds, *packing, *missing, wanted_type) != 0) {
		return NUS_ERRNO;
	}
	return 0;
}

	INLINE size_t
PDS_CutWidth(sym4_t buftype)
{
	switch (buftype) {
		case SYM4_I1:
			return 1;
		case SYM4_I2:
			return 2;
		case SYM4_I4:
		case SYM4_R4:
			return 4;
		case SYM4_R8:
			return 8;
		default:
			nus_err((NUSERR_RD_NoCodec,
				"nusdas_cut: N_%Ps not supported", buftype));
			return 0;
	}
}

	INLINE void
PDS_ArrayCut(char *dest, char *src, size_t elemsize, N_UI4 src_nx,
		const struct cut_rectangle *cut)
{
	int iy, linewidth;
	char *srcline;
	linewidth = cut->cr_xnelems * elemsize;
	srcline = src + (src_nx * cut->cr_yofs + cut->cr_xofs) * elemsize;
	for (iy = cut->cr_yofs; iy < cut->cr_yofs + cut->cr_ynelems; iy++) {
		memcpy(dest, srcline, linewidth);
		srcline += elemsize * src_nx;
		dest += linewidth;
	}
}

	static int
pds_readdata(nusdset_t *ds, nusdims_t *dim,
		struct ibuffer_t *buf)
{
	struct pds_t *pds = &ds->pds;
	void *get_data;
	char *src_data = NULL;
	char *ctype;
	N_UI4 gridsize[2];
	int nbytes;
	int offset;
	int r;
	int gzip_decoded;
	sym4_t pack, miss;

	if ((r = PDSLoadData(pds, dim, buf->ib_fmt, &(buf->ib_cut), 
			     &get_data, &ctype, &gzip_decoded)) < 0) {
		return r;
	}
	
	nbytes = r;
	r = PDSEvalContentType(pds, ctype, get_data, nbytes, buf->ib_fmt,
			gridsize, &pack, &miss, &offset);
	if (r != 0) {
		goto End;
	}
	/* JPEG 2000 の -48バイト目読み込み対応 */
	if (pack == SYM4_2UPJ) {
		if((src_data = (char*)nus_malloc(nbytes + 48)) == NULL){
			r = nus_err((NUSERR_MemShort, "nus_malloc failed"));
			goto End;
		}
		POKE_N_SI4(src_data, nbytes + 44);
		memset(src_data + 4, 0, 44);
		memcpy(src_data + 48, get_data, nbytes);
		src_data += 48;
	} else {
		src_data = get_data;
	}
	if (offset != 0 && (! cut_rectangle_disabled(&buf->ib_cut)) &&
	    (pack == SYM4_RLEN || pack == SYM4_2UPJ || pack == SYM4_2UPP || miss == SYM4_MASK)) {
		struct ibuffer_t fullbuf;
		size_t elemwidth;
		N_UI4  nxd, nyd;
		if ((elemwidth = PDS_CutWidth(buf->ib_fmt)) == 0) {
			return NUS_ERR_CODE();
		}
		nxd = gridsize[0];
		nyd = gridsize[1];
		if ((fullbuf.ib_ptr = nus_malloc(nxd * nyd * elemwidth)) == NULL) {
			return nus_err((NUSERR_MemShort, "nus_malloc"));
		}
		fullbuf.ib_fmt = buf->ib_fmt;
		fullbuf.nelems = nxd * nyd;
		cut_rectangle_disable(&fullbuf.ib_cut);
		r = pds->codec->decode(&fullbuf, 
				       (const N_UI1 *)src_data + offset,
				       nxd, nyd);
		if ((r < 0) || ((N_UI4)r < nxd * nyd)) {
			return r;
		}
		PDS_ArrayCut(buf->ib_ptr, fullbuf.ib_ptr, elemwidth, nxd,
				&buf->ib_cut);
		nus_free(fullbuf.ib_ptr);
		r =  cut_rectangle_size(&buf->ib_cut);
	} else {
		r = pds->codec->decode(buf, (const N_UI1 *)src_data + offset, 
				       gridsize[0], gridsize[1]);
	}
End:
	if(src_data && src_data != get_data){
		nus_free(src_data - 48);
	}
	if(gzip_decoded){
		nus_free(get_data);
	}
	return r;
}

	static int
pds_writedata(nusdset_t *ds UNUSED, nusdims_t *dim UNUSED,
		struct obuffer_t *buf UNUSED)
{
	return nus_err((NUSERR_NotImplemented, "writedata unimplemented for PDS"));
}

static int 
check_strtol(long val){
   if(errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)){
	   return 1;
   }else {
	   return 0;
   }
}

	static int
pds_read_aux(nusdset_t *ds,
		const nusdims_t *dim,
		sym4_t rectitle,
		sym4_t recgroup,
		int (*callback)(const void *rec, N_UI4 siz,
			void *arg, union nusdset_t *ds, N_SI4 ofs_flg),
		void *arg)
{
	struct pds_t *pds = &ds->pds;
	void *get_data;
	char *ctype;
	nusdims_t dim_in;
	int r;
	N_SI4 siz, target_siz = 0, nz;
	int gzip_decoded;
	char *saved_data, *target = NULL, *nzstr, *endptr;
	sym4_t group;
	N_SI4 ofs_flg;
	long lnz;

	dim_in.basetime = dim->basetime;
	dim_in.member = dim->member;
	dim_in.validtime1 = dim->validtime1;
	dim_in.validtime2 = 1;
	if (rectitle == SYM4_INFO){
		char grp_str[8];
		group = SYM4_INFO;
		dim_in.plane1 = SYM8_ALLSPACE;
		dim_in.plane2 = SYM8_ALLSPACE;
		nusdas_snprintf(grp_str, 8, "%Ps", recgroup);
		dim_in.element = str2sym8(grp_str);
	}else {
		group = recgroup;
		switch (group){
		case SYM4_RADR:
		case SYM4_RADS:
		case SYM4_ISPC:
		case SYM4_DPRD:
		case SYM4_THUN:
			dim_in.plane1 = dim->plane1;
			dim_in.plane2 = dim->plane2;
			dim_in.element = dim->element;
			break;
		default:
			dim_in.plane1 = SYM8_ALLSPACE;
			dim_in.plane2 = SYM8_ALLSPACE;
			dim_in.element = SYM8_ALLSPACE;
			break;
		}
	}
	if ((r = PDSLoadData(pds, &dim_in, group, NULL,
			     &get_data, &ctype, &gzip_decoded)) < 0) {
		return r;
	}
	siz = r;
	if((saved_data = (char*)nus_malloc(siz)) == NULL){
		return nus_err((NUSERR_MemShort, "nus_malloc failed"));
	}
	memcpy(saved_data, get_data, siz);
	if(gzip_decoded){
		nus_free(get_data);
	}
	if(streq(ctype, "application/x-nusdas_subcinfo")){
		target = saved_data;
		target_siz = siz;
		ofs_flg = 1;
	}else{
		switch(group){
                case SYM4_RGAU:
                case SYM4_ZHYB:
		case SYM4_TDIF:
		case SYM4_RADR:
		case SYM4_RADS:
		case SYM4_ISPC:
		case SYM4_DPRD:
		case SYM4_THUN:
		case SYM4_INFO:
			target = saved_data;
			target_siz = siz;
			break;
	        case SYM4_ETA:
	        case SYM4_SIGM:
			if((target = (char*)nus_malloc(siz + 4)) == NULL){
				return nus_err((NUSERR_MemShort, 
						"nus_malloc failed"));
			}
			nzstr = pdr_header_find(pData,"X-PLANE-NUM");
			if(nzstr == NULL){
				return nus_err((NUSERR_IO, 
						"X-PLANE-NUM is not included "
						"in response header"));
			}
			lnz = strtol(nzstr, &endptr, 10);
			if(check_strtol(lnz)){
				return nus_err((NUSERR_IO, 
						"Bad response for X-PLANE-NUM"));
			}
			nz = (N_SI4) lnz;
			memcpy_ntoh4(target, &nz, 1);
			memcpy(target + 4, saved_data, siz);
			target_siz = siz + 4;
			nus_free(saved_data);
			break;
		default:
			break;
		}
		ofs_flg = 0;
	}
	r = callback(target, target_siz, arg, ds, ofs_flg);
	nus_free(target);
	return r;
}

	static int
pds_write_aux(nusdset_t *ds UNUSED,
		const nusdims_t *dim UNUSED,
		sym4_t rectitle UNUSED,
		sym4_t recgroup UNUSED,
		size_t nbytes UNUSED,
		int (*callback)(void *rec, N_UI4 siz,
			void *arg, union nusdset_t *ds) UNUSED,
		void *arg UNUSED)
{
	return nus_err((NUSERR_NotImplemented, 
			"write_aux unimplemented for PDS"));
}

	static int
pds_subc_namecheck(nusdset_t *ds UNUSED,
		sym4_t grp UNUSED,
		size_t nbytes UNUSED)
{
	return nus_err((NUSERR_NotImplemented, 
			"namecheck unimplemented for PDS"));
}

static int
PDS_dec_bvlist(char* data, array4v_t *list)
{
	char *p_st = data;
	char *p;
	int r;
	N_SI4 dest;

	p = p_st;
	while(*p){
		if(*p == '\n' || *p == '\r'){
			if(panstring_to_time(&dest, p_st, p - p_st) != 0){
				p++;
				while(*p == '\n' || *p == '\r'){
					p++;
				}
				p_st = p;
			}
			else{
				r = array4v_push(list, dest);
				if(r){
					return r;
				}
				p++;
				while(*p == '\n' || *p == '\r'){
					p++;
				}
				p_st = p;
			}
		}
		p++;
	}
	return 0;

}
	static int
BtimeCompare(const void *va, const void *vb)
{
	return *(N_SI4 *)va - *(N_SI4 *)vb;
}

static int
PDS_get_bvlist(struct pds_t *pds, nusdims_t *dim, int query, 
	       N_SI4 data[], N_SI4 data_nelems)
{
	void *get_data;
	char *ctype;
	int gzip_decoded;
	array4v_t *btlist;
	int r;

	btlist = array4v_ini(1000, BtimeCompare);

	if ((r = PDSLoadData(pds, dim, query, NULL, 
			     &get_data, &ctype, &gzip_decoded)) < 0) {
		return r;
	}
	r = PDS_dec_bvlist(get_data, btlist);
	if(gzip_decoded){
		nus_free(get_data);
	}
	if(r){
		return r;
	}
	r = array4v_size(btlist);
	memcpy(data, btlist->list,
			((data_nelems >= r) ? r : data_nelems) * 4);
	array4v_delete(btlist);
	
	return r;
}

	static int
pds_btlist(nusdset_t *ds,
		N_SI4 data[],
		N_SI4 data_nelems,
		int verbose UNUSED)
{
	struct pds_t *pds = &ds->pds;
	nusdims_t dim;

	dim.basetime = INT_MIN;
	dim.member = SYM4_ALLSPACE;
	dim.validtime1 = INT_MIN;
	dim.validtime2 = 1;
	dim.plane1 = SYM8_ALLSPACE;
	dim.plane2 = SYM8_ALLSPACE;
	dim.element = SYM8_ALLSPACE;

	return PDS_get_bvlist(pds, &dim, SYM4_BTLS, data, data_nelems);
}

	static int
pds_vtlist(nusdset_t *ds,
		N_SI4 data[],
		N_SI4 data_nelems,
		N_SI4 basetime,
		int verbose UNUSED)
{
	struct pds_t *pds = &ds->pds;
	nusdims_t dim;

	dim.basetime = basetime;
	dim.member = SYM4_ALLSPACE;
	dim.validtime1 = INT_MIN;
	dim.validtime2 = 1;
	dim.plane1 = SYM8_ALLSPACE;
	dim.plane2 = SYM8_ALLSPACE;
	dim.element = SYM8_ALLSPACE;

	return PDS_get_bvlist(pds, &dim, SYM4_VTLS, data, data_nelems);
}

	static int
pds_inq_aux(nusdset_t *ds UNUSED,
		const nusdims_t *dims UNUSED,
		int query UNUSED,
		sym4_t grp UNUSED,
		void *buf UNUSED,
		N_UI4 bufnelems  UNUSED)
{
	return nus_err((NUSERR_NotImplemented, 
			"inq_aux unimplemented for PDS"));
}

static int
parse_rd(char *cont, int siz, pds_cntltab_t *hp)
{
	char *pcont, *key, *value, *tail, *p;
	int keynum;
	
	pcont = cont;
	keynum = 0;
	
	do {
		key = pcont;
		while(*pcont != '\r' && *pcont != '\n'){
			pcont++;
		}
		tail = pcont;
		*tail = '\0';
		
		/* remove space before key */
		while(*key == ' '){
			key++;
		}
		p = key;
		while(p < tail && *p != ','){
			p++;
		}
		value = p;
		if(value != tail){ /* found ',' */
			/* remove space after key */
			p = value;  /* 'value' indicates ',' */
			do {
				*p = '\0';
				p--;
			}while(*p == ' ');
			
			/* remove space before value */
			do{
				value++;
			}while(*value == ' ');
			
			/* remove space after value */
			p = tail;
			do{
				*p = '\0';
				p--;
			}while(*p == ' ');
			pds_cntltab_put(hp, key, value);
			keynum++;
		}
		/* set pointer to next line */
		do{
			pcont++;
		}while(pcont < cont + siz && (*pcont == '\n' || *pcont == '\r'));
		
	} while(pcont < cont + siz);
	return keynum;
	
}

typedef struct pds_cntl_cache_t{
	nustype_t nustype;
	nusdims_t dims;
	pds_cntltab_t *table;
	char *valstr;
} pds_cntl_cache_t;

static int
PDS_cntl_cache_ini(pds_cntl_cache_t *cache)
{
	if(cache == NULL){
		return 0;
	}
	cache->nustype.type1 = SYM8_ALLSPACE;
	cache->nustype.type2 = SYM4_ALLSPACE;
	cache->nustype.type3 = SYM4_ALLSPACE;
	cache->dims.basetime = INT_MIN;
	cache->dims.member = SYM4_ALLSPACE;
	cache->dims.validtime1 = INT_MIN;
	cache->dims.validtime2 = INT_MIN;
	cache->table = NULL;
	return 0;

}

static int 
parse_space_separated(char* cont, arrayp_t *ary)
{
    char *pbuf = cont, *p;
    int flag_quote_in = 0;
    int n_list;
    int ii;
    char quote = 0;

    while(*pbuf){
        if(*pbuf == ' '){
            pbuf++;
            continue;
        }
	p = pbuf;
        while(*pbuf){
            if(*pbuf == '\'' || *pbuf == '"'){
                if(flag_quote_in == 0){
                    flag_quote_in = 1;
                    quote = *pbuf;
                }
                else if(*pbuf == quote){
                    flag_quote_in = 0;
                }
            }
            else if(flag_quote_in == 0 && *pbuf == ' '){
                *pbuf = '\0';
                pbuf++;
                break;
            }
            pbuf++;
        }
	arrayp_push(ary, p);

    }
    if(flag_quote_in == 1){
        return nus_err((-2, "unbalanced quote mark %c\n", quote));
    }
    n_list = arrayp_size(ary);

    for (ii = 0; ii < n_list; ii++){
	    p = (char*)arrayp_get_value(ary, ii);
	    if(*p == '\'' || *p == '"'){
		    quote = *p;
		    p++;
		    arrayp_set(ary, ii, p);
		    while(*p && *p != quote){
			    p++;
		    }
		    *p = '\0';
	    }
    }

    return n_list;
}


#define CNTLTYPE_SI4 1
#define CNTLTYPE_RL4 2
#define CNTLTYPE_C4  3
#define CNTLTYPE_C4L 4
#define CNTLTYPE_C6L 5
#define CNTLTYPE_SI4L 6
#define CNTLTYPE_MUL 7

static int
PDS_get_cntl_label(int query, char *label[], int *lsize, int *type)
{
	switch(query){
	case N_MEMBER_NUM:
		strcpy(label[0], "number of member");
		*lsize = 1;
		*type = CNTLTYPE_SI4;
		break;
	case N_MEMBER_LIST:
	case SYM4_MIDX:
		strcpy(label[0], "member list");
		*lsize = 1;
		*type = CNTLTYPE_C4L;
		break;
	case N_VALIDTIME_NUM:
		strcpy(label[0], "number of validtime");
		*lsize = 1;
		*type = CNTLTYPE_SI4;
		break;
	case N_VALIDTIME_LIST:
	case SYM4_VIDX:
		strcpy(label[0], "validtime list");
		*lsize = 1;
		*type = CNTLTYPE_SI4L;
		break;
	case N_PLANE_NUM:
		strcpy(label[0], "number of plane");
		*lsize = 1;
		*type = CNTLTYPE_SI4;
		break;
	case N_PLANE_LIST:
	case SYM4_ZIDX:
		strcpy(label[0], "plane list");
		*lsize = 1;
		*type = CNTLTYPE_C6L;
		break;
	case N_ELEMENT_NUM:
		strcpy(label[0], "number of element");
		*lsize = 1;
		*type = CNTLTYPE_SI4;
		break;
	case N_ELEMENT_LIST:
	case SYM4_EIDX:
		strcpy(label[0], "element list");
		*lsize = 1;
		*type = CNTLTYPE_C6L;
		break;
	case N_VALIDTIME_UNIT:
		strcpy(label[0], "validtime unit");
		*lsize = 1;
		*type = CNTLTYPE_C4;
		break;
	case N_PROJECTION:
		strcpy(label[0], "projection type");
		*lsize = 1;
		*type = CNTLTYPE_C4;
		break;
	case N_GRID_SIZE:
		strcpy(label[0], "number of x grids");
		strcpy(label[1], "number of y grids");
		*lsize = 2;
		*type = CNTLTYPE_SI4;
		break;
	case N_GRID_DISTANCE:
		strcpy(label[0], "grid interval x");
		strcpy(label[1], "grid interval y");
		*lsize = 2;
		*type = CNTLTYPE_RL4;
		break;
	case N_GRID_BASEPOINT:
		strcpy(label[0], "base point x");
		strcpy(label[1], "base point y");
		strcpy(label[2], "base point lat");
		strcpy(label[3], "base point lon");
		*lsize = 4;
		*type = CNTLTYPE_RL4;
		break;
	case N_STAND_LATLON:
		strcpy(label[0], "standard lat 1");
		strcpy(label[1], "standard lon 1");
		strcpy(label[2], "standard lat 2");
		strcpy(label[3], "standard lon 1");
		*lsize = 4;
		*type = CNTLTYPE_RL4;
		break;
	case N_SPARE_LATLON:
		strcpy(label[0], "latitude 1");
		strcpy(label[1], "longitude 1");
		strcpy(label[2], "latitude 2");
		strcpy(label[3], "longitude 2");
		*lsize = 4;
		*type = CNTLTYPE_RL4;
		break;
	case SYM4_VALU:
		strcpy(label[0], "representation");
		*lsize = 1;
		*type = CNTLTYPE_C4;
		break;
	case N_RECORD_TIME:
		strcpy(label[0], "record time");
		*lsize = 1;
		*type = CNTLTYPE_SI4;
		break;
	case N_SUBC_NUM:
		strcpy(label[0], "number of subcntl");
		*lsize = 1;
		*type = CNTLTYPE_SI4;
		break;
	case N_SUBC_LIST:
		strcpy(label[0], "subcntl list");
		*lsize = 1;
		*type = CNTLTYPE_C4L;
		break;
	case N_INFO_NUM:
		strcpy(label[0], "number of info");
		*lsize = 1;
		*type = CNTLTYPE_SI4;
		break;
	case N_INFO_LIST:
		strcpy(label[0], "info list");
		*lsize = 1;
		*type = CNTLTYPE_C4L;
		break;
	case N_INDX_SIZE:
		strcpy(label[0], "number of member");
		strcpy(label[1], "number of validtime");
		strcpy(label[2], "number of plane");
		strcpy(label[3], "number of element");
		*lsize = 4;
		*type = CNTLTYPE_MUL;
		break;
	default:
		return nus_err((NUSERR_IQ_BadParam, "unknown query"));
	}
	return 0;
}
static int
PDS_cntl_set(pds_cntltab_t *cntl_tab, int query, char *label[], 
	     int lsize, int type, void *buf, N_UI4 bufnelems, void *params)
{
	N_SI4* si4ptr;
	float rl4getvalue;
	float *rl4ptr;
	N_SI4 si4getvalue;
	N_SI4 csiz, ii, len;
	char *p;
	char *stptr, *endptr, *cptr, *valuestr;
	int r;
	N_UI4 nelems;
	arrayp_t *ary;

	switch(type){
	case CNTLTYPE_SI4:
		if(bufnelems < (N_UI4)lsize){
			return nus_err((NUSERR_IQ_ShortBuf, 
					"bufnelems %Pd < nelems %Pd", 
					bufnelems, lsize));
		}
		si4ptr = (N_SI4*)buf;
		for(ii = 0; ii < lsize; ii++){
			if((stptr = pds_cntltab_get(cntl_tab, 
						label[ii])) == NULL){
				return nus_err((-1, "Cannot find key:%s", label[ii]));
			}
			si4getvalue = strtol(stptr, &endptr, 10);
			if(check_strtol(si4getvalue)){
				return nus_err((NUSERR_IO, 
					"Bad response for %s.", label[ii]));
			}
			*(si4ptr + ii) = si4getvalue;
		}
		r = lsize;
		break;
	case CNTLTYPE_C4:
		if(bufnelems < (N_UI4)lsize){
			return nus_err((NUSERR_IQ_ShortBuf, 
					"bufnelems %Pd < nelems %Pd", 
					bufnelems, lsize));
		}
		cptr = (char*)buf;
		for(ii = 0; ii < lsize; ii++){
			if((stptr = pds_cntltab_get(cntl_tab, 
					label[ii])) == NULL){
				return nus_err((-1, "Cannot find key:%s", label[ii]));
			}
			len = strlen(stptr);
			memset(cptr + ii * 4, 0x20, 4);
			memcpy(cptr + ii * 4, stptr, len > 4 ? 4 : len);
		}
		r = lsize;
		break;
	case CNTLTYPE_RL4:
		if(bufnelems < (N_UI4)lsize){
			return nus_err((NUSERR_IQ_ShortBuf, 
					"bufnelems %Pd < nelems %Pd", 
					bufnelems, lsize));
		}
		rl4ptr = (float *)buf;
		for(ii = 0; ii < lsize; ii++){
			if((stptr = pds_cntltab_get(cntl_tab, 
					label[ii])) == NULL){
				return nus_err((-1, "Cannot find key:%s", label[ii]));
			}
			rl4getvalue = strtod(stptr, &endptr);
			if(errno == ERANGE){
				return nus_err((NUSERR_IO, 
					"Bad response for %s.", label[ii]));
			}
			*(rl4ptr + ii) = rl4getvalue;
		}
		r = lsize;
		break;
	case CNTLTYPE_C4L:
	case CNTLTYPE_C6L:
		if(type == CNTLTYPE_C4L){
			csiz = 4;
		}else{
			csiz = 6;
		}
		if((stptr = pds_cntltab_get(cntl_tab, 
						   label[0])) == NULL){
			return nus_err((-1, "Cannot find key:%s", label[0]));
		}
		if((valuestr = (char*)nus_malloc(strlen(stptr) + 1)) == NULL){
			return nus_err((NUSERR_IQ_ShortBuf, "inq_cntl error"));
		}
		strncpy(valuestr, stptr, strlen(stptr) + 1);
		ary = arrayp_ini(1000, BtimeCompare);
		if((r = parse_space_separated(valuestr, ary)) < 0){
			nus_free(valuestr);
			return NUSERR_IO;
		}
		nelems = r;
		if(params == NULL && bufnelems < nelems){
			return nus_err((NUSERR_IQ_ShortBuf, 
					"bufnelems %Pd < nelems %Pd", 
					bufnelems, nelems));
		}
		if(params != NULL){
			si4ptr = (N_SI4*)buf;
			for(ii = 0; ii < (N_SI4)nelems; ii++){
				if(strncmp(arrayp_get_value(ary, ii), 
					   (char*)params, csiz) == 0){
					break;
				}
			}
			if(ii == (N_SI4)nelems){
				*si4ptr = INT_MIN;
				return nus_err((-1, 
						"Cannot find params for %Ps", 
						query));
			}else{
				*si4ptr = ii;
				r = 1;
			}
		}else{
			cptr = (char*)buf;
			for(ii = 0; ii < (N_SI4)nelems; ii++){
				p = arrayp_get_value(ary, ii);
				len = strlen(p);
				memset(cptr + ii * csiz, 0x20, csiz);
				memcpy(cptr + ii * csiz, p, 
				       len > csiz ? csiz : len);
			}
			r = nelems;
		}
		arrayp_delete(ary);
		nus_free(valuestr);
		break;
	case CNTLTYPE_SI4L:
		si4ptr = (N_SI4*)buf;
		if((stptr = pds_cntltab_get(cntl_tab, 
						   label[0])) == NULL){
			return nus_err((-1, "Cannot find key:%s", label[0]));
		}
		if((valuestr = (char*)nus_malloc(strlen(stptr) + 1)) == NULL){
			return nus_err((NUSERR_IQ_ShortBuf, "inq_cntl error"));
		}
		strncpy(valuestr, stptr, strlen(stptr) + 1);
		ary = arrayp_ini(1000, BtimeCompare);
		if((r = parse_space_separated(valuestr, ary)) < 0){
			nus_free(valuestr);
			return NUSERR_IO;
		}
		nelems = r;
		if(params == NULL && bufnelems < nelems){
			return nus_err((NUSERR_IQ_ShortBuf, "inq_cntl error"));
		}
		if(params != NULL && query == SYM4_VIDX){
			for(ii = 0; ii < (N_SI4)nelems; ii++){
				if(strtol(arrayp_get_value(ary, ii), 
					  &endptr, 10) 
				   == (N_SI4)I8_HI(*((N_UI8*)params))){
					break;
				}
			}
			if(ii == (N_SI4)nelems){
				*si4ptr = INT_MIN;
				return nus_err((-1, 
						"Cannot find params"));
			}else{
				*si4ptr = ii;
				r = 1;
			}
		}else{
			for(ii = 0; ii < (N_SI4)nelems; ii++){
				si4getvalue = strtol(arrayp_get_value(ary, ii),
						     &endptr, 10);
				if(check_strtol(si4getvalue)){
					return nus_err((NUSERR_IO, 
							"Bad response for %s.",
							label[ii]));
				}
				*(si4ptr + ii) = si4getvalue;
			}
			r = nelems;
		}
		arrayp_delete(ary);
		nus_free(valuestr);
		break;
	case CNTLTYPE_MUL:
		if(bufnelems < 1){
			return nus_err((NUSERR_IQ_ShortBuf, 
					"bufnelems %Pd < nelems %Pd", 
					bufnelems, 1));
		}
		si4ptr = (N_SI4*)buf;
		*si4ptr = 1;
		for(ii = 0; ii < lsize; ii++){
			if((stptr = pds_cntltab_get(cntl_tab, 
						label[ii])) == NULL){
				return nus_err((-1, "Cannot find key:%s", label[ii]));
			}
			si4getvalue = strtol(stptr, &endptr, 10);
			if(check_strtol(si4getvalue)){
				return nus_err((NUSERR_IO, 
					"Bad response for %s.", label[ii]));
			}
			*si4ptr *= si4getvalue;
		}
		r = 1;
		break;
	default:
		return nus_err((NUSERR_Internal, 
				"Not supported CNTLTYPE"));
	}
	return r;

}

static int
PDS_cntl_inq_cached(struct pds_t *pds, const nusdims_t *dims, 
		    pds_cntl_cache_t *cache)
{
	if(cache == NULL){
		return 0;
	}
	if(si8_eq(cache->nustype.type1, pds->comm.nustype.type1)
	   && cache->nustype.type2 == pds->comm.nustype.type2 
	   && cache->nustype.type3 == pds->comm.nustype.type3 
	   && cache->dims.basetime == dims->basetime 
	   && cache->dims.member == dims->member 
	   && cache->dims.validtime1 == dims->validtime1 
	   && cache->dims.validtime2 == dims->validtime2){
		return 1;
	}else{
		return 0;
	}
}

static int
PDS_cntl_cache_set(struct pds_t *pds, const nusdims_t *dims, 
		   pds_cntltab_t *table, char* valstr, pds_cntl_cache_t *cache)
{
	if(cache == NULL){
		nus_free(table);
		nus_free(valstr);
		return 0;
	}
	cache->nustype.type1 = pds->comm.nustype.type1;
	cache->nustype.type2 = pds->comm.nustype.type2;
	cache->nustype.type3 = pds->comm.nustype.type3;
	cache->dims.basetime = dims->basetime;
	cache->dims.member = dims->member;
	cache->dims.validtime1 = dims->validtime1;
	cache->dims.validtime2 = dims->validtime2;
	cache->table = table;
	cache->valstr = valstr;
	return 0;
}

static int
PDS_cntl_cache_delete(pds_cntl_cache_t *cache)
{
	if(cache == NULL){
		return 0;
	}
	pds_cntltab_delete(cache->table);
	nus_free(cache->valstr);
	cache->table = NULL;
	cache->valstr = NULL;
	return 0;
}

#define NUM_LABEL_PTR 10
#define LEN_LABEL_UNIT 80
static int
PDS_get_cntldata(nusdset_t *ds,
		 nusdims_t *dims,
		 int query,
		 void *buf,
		 void *arg, 
		 pds_cntl_cache_t *cache, 
		 int *cache_init)
{
	pds_cntltab_t *cntl_tab;
	void *get_data;
	struct pds_t *pds = &ds->pds;
	char *ctype;
	int r;
	N_UI4 ii;
	N_UI4 group;
	int lsize, type, siz;
	char *label[NUM_LABEL_PTR], label0[NUM_LABEL_PTR * LEN_LABEL_UNIT];
	int gzip_decoded;
	int bufnelems;
	void *params;

	switch(query){
	case SYM4_MIDX:
	case SYM4_VIDX:
	case SYM4_ZIDX:
	case SYM4_EIDX:
		bufnelems = 1;
		params = arg;
		break;
	default:
		if(arg == NULL)
			bufnelems=1;
		else
			bufnelems = *((N_SI4*)arg);
		params = NULL;
		break;
	}

	if(*cache_init == 1){
		PDS_cntl_cache_ini(cache);
		*cache_init = 0;
	}
	if(PDS_cntl_inq_cached(pds, dims, cache)){
		cntl_tab = cache->table;
	}else{
		char *valstr;
		if(cache != NULL && cache->table != NULL){
			PDS_cntl_cache_delete(cache);
		}
		if((cntl_tab = (pds_cntltab_t*)pds_cntltab_ini(10000)) == NULL){
			return nus_err((NUSERR_MemShort, "pds_inq_cntl error"));
		}
		if(dims->basetime == INT_MAX){
			group = SYM4_DEF;
		}else if(sym8_is_allspace(dims->element)){
			group = SYM4_CNTL;
		}else{
			group = SYM4_DATA;
		}
		if ((r = PDSLoadData(pds, dims, group, NULL, 
				     &get_data, &ctype, &gzip_decoded)) < 0) {
			return r;
		}
		siz = r;
		if((valstr = (char*)nus_malloc(siz)) == NULL){
 			return nus_err((NUSERR_MemShort, "pds_inq_cntl error"));
		}
		memcpy(valstr, get_data, siz);
		parse_rd(valstr, siz, cntl_tab);
		PDS_cntl_cache_set(pds, dims, cntl_tab, valstr, cache);
		if(gzip_decoded){
			nus_free(get_data);
		}
	}
	for (ii = 0; ii < NUM_LABEL_PTR; ii++){
		label[ii] = label0 + ii * LEN_LABEL_UNIT;
	}
	if((r = PDS_get_cntl_label(query, label, &lsize, &type)) < 0){
		return r;
	}
	r = PDS_cntl_set(cntl_tab, query, label, 
			 lsize, type, buf, bufnelems, params);
	return r;

}

static int
PDS_get_edmap(nusdset_t *ds, nusdims_t *dims, int query, void *buf, 
	      N_UI4 bufnelems)
{
	void *get_data;
	char *ctype, *cptr;
	struct pds_t *pds = &ds->pds;
	int  gzip_decoded, siz;
	int r;

	if ((r = PDSLoadData(pds, dims, query, NULL, 
			     &get_data, &ctype, &gzip_decoded)) < 0) {
		return r;
	}
	siz = r;
	if (bufnelems < (N_UI4)siz){
		if (gzip_decoded){
			nus_free(get_data);
		}
		return nus_err((NUSERR_IQ_ShortBuf, 
				"bufnelems %Pd < nelems %Pd", 
				bufnelems, siz));
	}
	cptr = (char*)buf;
	memcpy(cptr, get_data, siz);
	r = siz;
	if (gzip_decoded){
		nus_free(get_data);
	}
	return r;
}

	static int
pds_inq_data(nusdset_t *ds,
		nusdims_t *dims,
		int query,
		void *buf,
		N_UI4 bufnelems)
{
	static pds_cntl_cache_t data_cache;
	static int data_cache_init = 1;

	N_UI4* arg = &bufnelems;
	return PDS_get_cntldata(ds, dims, query, buf, arg, 
				&data_cache, &data_cache_init);
}

	static int
pds_inq_cntl(nusdset_t *ds,
		const nusdims_t *dims,
		int query,
		void *buf,
		void *arg,
		int keep_open UNUSED)
{
	nusdims_t dims_in;
	int query_in = 0, r;
	N_UI4 bufnelems;
	
	static pds_cntl_cache_t cntl_cache;
	static int cntl_cache_init = 1;

	dims_in.basetime = dims->basetime;
	dims_in.member = dims->member;
	dims_in.validtime1 = dims->validtime1;
	dims_in.validtime2 = dims->validtime2;
	dims_in.plane1 = SYM8_ALLSPACE;
	dims_in.plane2 = SYM8_ALLSPACE;
	dims_in.element = SYM8_ALLSPACE;
	switch (query){
	case N_ELEMENT_MAP:
	case N_DATA_MAP:
		bufnelems = *((N_SI4*)arg);
		if (query == N_ELEMENT_MAP){
			query_in = SYM4_CEMP;
		} else if (query == N_DATA_MAP){
			query_in = SYM4_CDMP;
		}
		r = PDS_get_edmap(ds, &dims_in, query_in, buf, bufnelems);
		break;
	default:
		r = PDS_get_cntldata(ds, &dims_in, query, buf, arg, 
				     &cntl_cache, &cntl_cache_init);
		break;
	}
	return r;
}

	static int
pds_inq_grid(union nusdset_t *ds,
		struct inq_grid_info *info)
{
	
	struct pds_t *pds = &ds->pds;
	nusdims_t *dims = &info->nusdims;
	nusdims_t dims_in;
	void *get_data;
	char *p;
	char *ctype;
	int gzip_decoded;
	int i, r;

	dims_in.basetime = dims->basetime;
	dims_in.member = dims->member;
	dims_in.validtime1 = dims->validtime1;
	dims_in.validtime2 = dims->validtime2;
	dims_in.plane1 = SYM8_ALLSPACE;
	dims_in.plane2 = SYM8_ALLSPACE;
	dims_in.element = SYM8_ALLSPACE;

	if ((r = PDSLoadData(pds, &dims_in, SYM4_GRID, NULL, 
			     &get_data, &ctype, &gzip_decoded)) < 0) {
		return r;
	}
	if(gzip_decoded){
		nus_free(get_data);
	}
	p = get_data;
	memcpy4(info->proj, p);
	p += 4;
	for (i = 0; i < 2; i++){
		info->gridsize[i] = NTOH4(*((N_SI4*)(p)));
		p += 4;
	}
	for (i = 0; i < 14; i++){
		PEEK_float(info->gridinfo + i, p);
		p += 4;
	}
	memcpy4(info->value, p);
	return 1;
}

	static int
pds_inq_def(nusdset_t *ds,
		N_SI4 query,
		void *buf,
		N_SI4 buflen)
{
	static pds_cntl_cache_t inqdef_cache;
	static int inqdef_cache_init = 1;
	N_SI4* arg = &buflen;
	int r;

	nusdims_t dims_in;
	dims_in.basetime = INT_MAX;
	dims_in.member = SYM4_ALLSPACE;
	dims_in.validtime1 = INT_MAX;
	dims_in.validtime2 = INT_MAX;
	dims_in.plane1 = SYM8_ALLSPACE;
	dims_in.plane2 = SYM8_ALLSPACE;
	dims_in.element = SYM8_ALLSPACE;
	
	if (query == N_ELEMENT_MAP) {
		r = PDS_get_edmap(ds, &dims_in, SYM4_DEMP, buf, buflen);
	} else {
		r = PDS_get_cntldata(ds, &dims_in, query, buf, arg, 
				&inqdef_cache, &inqdef_cache_init);
	}
	return r;
}

static struct ds_functab pds_methods = {
	pds_close,
	pds_delete,
	pds_compact,
	pds_findfile,
	pds_readdata,
	pds_inq_data,
	pds_writedata,
	pds_read_aux,
	pds_subc_namecheck,
	pds_btlist,
	pds_vtlist,
	pds_inq_aux,
	pds_inq_cntl,
	pds_write_aux,
	pds_close_file,
	pds_inq_def,
	pds_inq_grid
};

	static union nusdset_t *
pds_open(int nrd, char nustype[16],
		const char *server, unsigned port, const char *path)
{
	struct pds_t *pds;
	pds = nus_malloc(sizeof(struct pds_t));
	if (pds == NULL)
		return NULL;
	pack2nustype(nustype, nustype + 8, nustype + 12, &pds->comm.nustype);
	pds->comm.nrd = nrd;
	pds->comm.methods = pds_methods;
	pds->comm.sc_eta = pds->comm.sc_sigm = NULL;
	pds->comm.sc_rgau = pds->comm.sc_zhyb = NULL;
	pds->comm.sc_delt = NULL;
	pds->server = string_dup(server);
	pds->port = port;
	pds->path = string_dup(path);
	pds->comm.param = GlobalConfig(ds_param);
	pds->codec = NULL;
	pds->comm.dead_flag = 0;
	return (union nusdset_t *)pds;
}

#endif /* if USE_NET */

	int
nuspds_dsfound(int nrd UNUSED, char nustype[16] UNUSED,
		const char *server UNUSED, unsigned port UNUSED,
		const char *path UNUSED)
{
#if USE_NET
	union nusdset_t *ds;
	ds = pds_open(nrd, nustype, server, port, path);
	if (ds == NULL) {
		return -1;
	}
	nus_debug(("push pds:%s,%s,%d, %d", server, path, port, nrd));
	nusglb_pushdset(ds, nrd);
#else
	static int first = 1;
	if (first) {
		first = 0;
		nus_warn(("network NuSDaS disabled by configure"));
	}
#endif
	return 0;
}

/** @brief パンドラデータセットの探索
 */
	int
nuspds_scan(void)
{
	const char *filename;
	unsigned char *text;
	size_t textsize;
	int r;
	if ((filename = getenv(PANDORA_SERVER_LIST)) == NULL) {
		nus_debug(("$" PANDORA_SERVER_LIST " unset"));
		return 0;
	}
	if ((text = file_read(filename, &textsize)) == NULL) {
		return -1;
	}
	r = nuspds_cfglex(text, textsize);
	nus_free(text);
	return r;
}
