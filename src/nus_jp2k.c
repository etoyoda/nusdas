/** @file
 * @brief JPEG2000 °µ½Ì¤Î¼ÂÁõ
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_kwd.h"
# define NEED_PEEK_N_UI4
# define NEED_PEEK_FLOAT
# define NEED_PEEK_DOUBLE
# define NEED_POKE_FLOAT
# define NEED_POKE_DOUBLE
# define NEED_MAKE_UI8
#include "sys_endian.h"
#include "sys_int.h"
#include "glb.h"

#include <string.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "dset.h"
#include "dfile.h"
#include "sys_file.h"
#include "ndf_codec.h"
#include "sys_err.h"
# define NEED_MEMCPY4
# define NEED_MEMCPY8
#include "sys_string.h"
#include "sys_mem.h"

#ifdef USE_OPENJPEG
# ifdef HAVE_OPENJPEG_OPENJPEG_H
#  include "openjpeg/openjpeg.h"
# elif defined(HAVE_OPENJPEG_H)
#  include "openjpeg.h"
# else
#  error OPENJPEG.H NOT FOUND
# endif
# undef  USE_JPEG2000_JASPER
# define USE_JPEG2000_OPENJPEG
#elif defined(USE_JASPER)
# ifdef HAVE_JASPER_JASPER_H
#  include "jasper/jasper.h"
# elif defined(HAVE_JASPER_H)
#  include "jasper.h"
# else 
#  error JASPER.H NOT FOUND
# endif
# define USE_JPEG2000_JASPER
# undef  USE_JPEG2000_OPENJPEG
#else
# undef USE_JPEG2000_JASPER
# undef  USE_JPEG2000_OPENJPEG
#endif

#include <stdio.h>
#ifdef HAVE_LRINT
# define ROUND(x)	lrint(x)
#else
# define ROUND(x)	floor((x) + 0.5)
#endif

#ifdef USE_JPEG2000_JASPER

	long
nus_encode_jp2k(const unsigned char *src,
	N_UI4 x, N_UI4 y, unsigned char *dst, N_UI4 dlen)
{
	jas_image_t *image;
	jas_image_cmptparm_t compparms;
	jas_stream_t *out;
	jas_matrix_t *data;
	char opts[256];
	N_UI2 value;
	N_UI4 i, j;
	int tilewidth = 1024, tileheight = 1024;
	long rt;

	if (!(out = jas_stream_memopen((char *)dst, dlen)) ) {
		return nus_err((NUSERR_WR_EncodeFail,
			"cannot create image data(jas_stream_memopen)"));
	}
	compparms.tlx = 0;
	compparms.tly = 0;
	compparms.hstep = 1;
	compparms.vstep = 1;
	compparms.width = x;
	compparms.height = y;
	compparms.prec = 16;
	compparms.sgnd = false;
	if (!(image = jas_image_create(1, &compparms, JAS_CLRSPC_GENGRAY)) ) {
		jas_stream_close(out);
		return nus_err((NUSERR_WR_EncodeFail,
			"cannot create image data(jas_mage_create)"));
	}
	if (!(data = jas_matrix_create(y, x)) ) {
		jas_stream_close(out);
		jas_image_destroy(image);
		return nus_err((NUSERR_WR_EncodeFail,
			"cannot create image data(jas_matrix_create)"));
	}
	for (j = 0; j < y; j++) {
		for (i = 0; i < x; i++) {
			int n = (j*x+i)*2;
			value = ((N_UI2)*(src+n) << 8) | (N_UI2)*(src+n+1);
			jas_matrix_set(data, j, i, value);
		}
	}
	if (jas_image_writecmpt(image, 0, 0, 0, x, y, data)) {
		jas_stream_close(out);
		jas_image_destroy(image);
		jas_matrix_destroy(data);
		return nus_err((NUSERR_WR_EncodeFail,
			"cannot create image data(jas_image_writecmpt)"));
	}
	opts[0] = '\0';
	if (x > tilewidth || y > tileheight) {
		sprintf(opts, "tilewidth=%d, tileheight=%d", tilewidth, tileheight);
	}
	if (jpc_encode(image, out, opts)) {
		jas_stream_close(out);
		jas_image_destroy(image);
		jas_matrix_destroy(data);
		return nus_err((NUSERR_WR_EncodeFail,
			"cannot create image data(jpc_encode)"));
	}
	rt = (long)out->rwcnt_;
	jas_stream_close(out);
	jas_image_destroy(image);
	jas_matrix_destroy(data);
	return rt;
}

	long
nus_decode_jp2k(const unsigned char *src,
	unsigned char *dst, N_UI4 len)
{
	jas_stream_t *in;
	jas_image_t *image;
	unsigned char *dp = (unsigned char *)dst;
	N_UI2 value;
	N_UI4 x, y, width, height;

	if(!(in = jas_stream_memopen((char *)src, len))) {
		return nus_err((NUSERR_WR_EncodeFail,
			"cannot load image data(jas_stream_memopen)"));
	}
	if (!(image = jpc_decode(in, ""))) {
		jas_stream_close(in);
		return nus_err((NUSERR_WR_EncodeFail,
			"cannot load image data(jpc_decode)"));
	}
	width = jas_image_width(image);
	height = jas_image_height(image);
	for(y = 0; y < height; y++){
		for(x = 0; x < width; x++){
			int n = (y*width+x)*2;
			value = jas_image_readcmptsample(image, 0, x, y);
			*(dp+n)   = (value >> 8) & 0xFF;
			*(dp+n+1) = (value     ) & 0xFF;
		}
	}
	jas_stream_close(in);
	jas_image_destroy(image);
	return width*height*2;
}

#elif defined(USE_OPENJPEG)

typedef struct{
	unsigned char* first;
	unsigned char* current;
	OPJ_OFF_T remain;
}pos_data_t;

	static void
message(const char* msg, void* user_data)
{
	switch(*(int *)user_data){
		case 0: nus_err((NUSERR_WR_EncodeFail, "openjpeg:%s", msg)); break;
		case 1: nus_warn(("openjpeg:%s", msg)); break;
		case 2: nus_debug(("openjpeg:%s", msg)); break;
	}
}
	static OPJ_SIZE_T
openjpeg_read(void* jpg, OPJ_SIZE_T size, void* user_data)
{
	pos_data_t* pd = (pos_data_t*)user_data;
	if (0 >= pd->remain) return (OPJ_SIZE_T)-1;
	if (pd->remain < size) size = pd->remain;
	memcpy(jpg, pd->current, size);
	pd->current += size;
	pd->remain -= size;
	return size;
}
	static OPJ_SIZE_T
openjpeg_write(void* jpg, OPJ_SIZE_T size, void* user_data)
{
	pos_data_t* pd = (pos_data_t*)user_data;
	if (0 >= pd->remain) return (OPJ_SIZE_T) - 1;
	if (pd->remain < size) size = pd->remain;
	memcpy(pd->current, jpg, size);
	pd->current += size;
	pd->remain -= size;
	return size;
}
	static OPJ_OFF_T
openjpeg_skip(OPJ_OFF_T size, void* user_data)
{
	pos_data_t* pd = (pos_data_t*)user_data;
	if(0 >= pd->remain && size > 0) return (OPJ_SIZE_T) - 1;
	if(pd->remain < size) size = pd->remain;
	pd->current += size;
	pd->remain -= size;
	return size;
}
	static OPJ_BOOL
openjpeg_seek(OPJ_OFF_T size, void* user_data)
{
	OPJ_OFF_T total;
	pos_data_t* pd = (pos_data_t*)user_data;
	total = pd->current - pd->first + pd->remain;
	if(total < size) return OPJ_FALSE;
	pd->current = pd->first + size;
	pd->remain = total - size;
	return OPJ_TRUE;
}
	static void
openjpeg_initialize(opj_codec_t* codec, opj_stream_t* stream)
{
	static int err[3] = {0, 1, 2};
	opj_set_error_handler  (codec, &message, &err[0]);
	opj_set_warning_handler(codec, &message, &err[1]);
	opj_set_info_handler   (codec, &message, &err[2]);
	opj_stream_set_read_function (stream, &openjpeg_read);
	opj_stream_set_write_function(stream, &openjpeg_write);
	opj_stream_set_skip_function (stream, &openjpeg_skip);
	opj_stream_set_seek_function (stream, &openjpeg_seek);
}
	static void
openjpeg_finalize(opj_codec_t* codec, opj_stream_t* stream, opj_image_t* image)
{
	if (codec) opj_destroy_codec(codec);
	if (stream) opj_stream_destroy(stream);
	if (image) opj_image_destroy(image);
}

	long
nus_encode_jp2k(const unsigned char *src,
	N_UI4 x, N_UI4 y, unsigned char *dst, N_UI4 dlen)
{
	opj_codec_t* codec = NULL;
	opj_stream_t* stream = NULL;
	opj_image_t* image = NULL;
	opj_image_cmptparm_t compparms;
	opj_cparameters_t params;
	pos_data_t pd = {dst, dst, dlen};
	OPJ_INT32 *data, *last;
	const int tilewidth = 1024, tileheight = 1024;
	const int input_size = x * y;
	int pos;

	compparms.dx   = 1;
	compparms.dy   = 1;
	compparms.w    = x;
	compparms.h    = y;
	compparms.x0   = 0;
	compparms.y0   = 0;
	compparms.prec = 16;
	compparms.bpp  = 16;
	compparms.sgnd = 0;
	if (!(image = opj_image_create(1, &compparms, OPJ_CLRSPC_GRAY)) ) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot create image"));
	}
	data = image->comps->data;
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for(pos = 0; pos < input_size; ++pos){
		data[pos] = src[pos * 2] << 8 | src[pos * 2 + 1];
	}
	image->x0 = 0;
	image->y0 = 0;
	image->x1 = x;
	image->y1 = y;
	
	if (!(codec = opj_create_compress(OPJ_CODEC_J2K))) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot create encoder"));
	}
	if (!(stream = opj_stream_default_create(OPJ_FALSE))) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot create stream"));
	}
	openjpeg_initialize(codec, stream);
	opj_stream_set_user_data(stream, &pd, NULL);
	opj_stream_set_user_data_length(stream, dlen);
	
	opj_set_default_encoder_parameters(&params);
	params.tcp_numlayers = 1;
	params.cp_disto_alloc = 1;
	params.numresolution = 1;
	params.tile_size_on = OPJ_TRUE;
	params.cp_tx0 = 0;
	params.cp_ty0 = 0;
	params.cp_tdx = x > tilewidth  ? tilewidth  : x;
	params.cp_tdy = y > tileheight ? tileheight : y;
	params.cod_format = 0;
	params.mode = 0;
	
	if (!opj_setup_encoder(codec, &params, image)) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot setup encoder"));
	}
	if (!opj_start_compress(codec, image, stream)) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot compress image"));
	}
	if (!opj_encode(codec, stream)) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot ecncode j2k"));
	}
	if (!opj_end_compress(codec, stream)) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot end compress"));
	}
	openjpeg_finalize(codec, stream, image);
	return pd.current - pd.first;
}

	long
nus_decode_jp2k(const unsigned char *src,
	unsigned char *dst, N_UI4 len)
{
	opj_codec_t* codec = NULL;
	opj_stream_t* stream = NULL;
	opj_image_t* image = NULL;
	opj_dparameters_t params;
	pos_data_t pd = {(unsigned char*)src, (unsigned char*)src, len};
	const OPJ_INT32 *decoded, *last;
	int decoded_size, pos;
	
	if (!(codec = opj_create_decompress(OPJ_CODEC_J2K))) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot create decoder"));
	}
	if (!(stream = opj_stream_default_create(OPJ_TRUE))) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot create stream"));
	}
	openjpeg_initialize(codec, stream);
	opj_stream_set_user_data(stream, &pd, NULL);
	opj_stream_set_user_data_length(stream, len);

	opj_set_default_decoder_parameters(&params);
	if (!opj_setup_decoder(codec, &params)) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			 "openjpeg cannot setup decoder"));
	}
	if (!opj_read_header(stream, codec, &image)) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot read j2k header"));
	}
	if (!image) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg: invalid image"));
	}
	if (!opj_decode(codec, stream, image)) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg cannot decode j2k"));
	}
	if (!image->numcomps) {
		openjpeg_finalize(codec, stream, image);
		return nus_err((NUSERR_WR_EncodeFail,
			"openjpeg: empty image"));
	}
	decoded = image->comps->data;
	decoded_size = image->comps->w * image->comps->h;
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (pos = 0; pos < decoded_size; ++pos){
		dst[pos * 2] = (decoded[pos] >> 8) & 0xFF;
		dst[pos * 2 + 1] = decoded[pos] & 0xFF;
	}
	openjpeg_finalize(codec, stream, image);
	return 2 * decoded_size;
}

#else

	long
nus_encode_jp2k(const unsigned char *src,
	N_UI4 x, N_UI4 y, unsigned char *dst, N_UI4 dlen)
{
	return nus_err((NUSERR_WR_NoCodec,
		"cannot compress JPEG 2000 Code Stream format"));
}

	long
nus_decode_jp2k(const unsigned char *src,
	unsigned char *dst, N_UI4 len)
{
	return nus_err((NUSERR_RD_NoCodec,
		"cannot uncompress JPEG 2000 Code Stream format"));
}
#endif
