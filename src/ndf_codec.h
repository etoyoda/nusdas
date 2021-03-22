/** @file
 * @brief ビットパック又は圧縮のエンコード・デコード
 */

struct ndf_codec_t {
	sym4_t	packing;
	sym4_t	missing;
	sym4_t	bffm;
	size_t	size_fixed;
	int	size_factor;
	long (*encode)(unsigned char *drec64, 
			struct obuffer_t *buf,
			N_UI4 nxd,
			N_UI4 nyd);
	long (*decode)(struct ibuffer_t *userbuf,
			const unsigned char *drec,
			N_UI4 nxd,
			N_UI4 nyd);
};

extern struct ndf_codec_t *ndf_get_codec(sym4_t packing,
		sym4_t missing, sym4_t buffmt);

#define CODEC_MAXRECL(codec, nelems) \
	((codec)->size_fixed + (codec)->size_factor * (((nelems) >> 3) + 1))

	N_SI4
nus_compress_rlen_i1(const N_UI1 *udata, 
		N_UI4 udata_nelems, 
		N_SI4 *maxv, 
		N_UI1 *cbuf, 
		N_UI4 cbuf_nbytes);

	N_SI4
nus_decompress_rlen_i1(const N_UI1 *compressed, 
		N_UI4 maxv, 
		N_UI4 cbytes, 
		N_UI1 *udata, 
		N_UI4 udata_nelems);

	N_SI4
nus_compress_rlen_i2(const N_SI2 *udata, 
		N_UI4 udata_nelems, 
		N_SI4 *maxv, 
		N_UI1 *cbuf, 
		N_UI4 cbuf_nbytes);

	N_SI4
nus_decompress_rlen_i2(const N_UI1 *compressed, 
		N_UI4 maxv, 
		N_UI4 cbytes, 
		N_SI2 *udata, 
		N_UI4 udata_nelems);

	N_SI4
nus_compress_rlen_i4(const N_SI4 *udata, 
		N_UI4 udata_nelems, 
		N_SI4 *maxv, 
		N_UI1 *cbuf,  
		N_UI4 cbuf_nbytes);

	N_SI4
nus_decompress_rlen_i4(const N_UI1 *compressed, 
		N_UI4 maxv, 
		N_UI4 cbytes, 
		N_SI4 *udata, 
		N_UI4 udata_nelems);

	N_SI4
nus_compress_rlen_r4(const float *udata, 
		N_UI4 udata_nelems, 
		N_SI4 *maxv, 
		N_UI1 *cbuf,  
		N_UI4 cbuf_nbytes);

	N_SI4
nus_decompress_rlen_r4(const N_UI1 *compressed, 
		N_UI4 maxv, 
		N_UI4 cbytes, 
		float *udata, 
		N_UI4 udata_nelems);

	N_SI4
nus_compress_rlen_r8(const double *udata, 
		N_UI4 udata_nelems, 
		N_SI4 *maxv, 
		N_UI1 *cbuf,  
		N_UI4 cbuf_nbytes);

	N_SI4
nus_decompress_rlen_r8(const N_UI1 *compressed, 
		N_UI4 maxv, 
		N_UI4 cbytes, 
		double *udata, 
		N_UI4 udata_nelems);

	long
nus_decode_cpsd(const unsigned char *src,
		unsigned char *dst, N_UI4 len);
	long
nus_encode_cpsd(const unsigned char *src,
		N_UI4 x, N_UI4 y, unsigned char *dst, N_UI4 dlen);

	long
nus_decode_jp2k(const unsigned char *src,
		unsigned char *dst, N_UI4 len);
	long
nus_encode_jp2k(const unsigned char *src,
		N_UI4 x, N_UI4 y, unsigned char *dst, N_UI4 dlen);
