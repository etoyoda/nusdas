/** @file
 * @brief implementation of NDF binary encoder/decoder (codec).
 *  [generated from ndf_codec.rb]
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_kwd.h"
# define NEED_PEEK_N_SI2
# define NEED_PEEK_N_UI2
# define NEED_PEEK_N_SI4
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

#ifdef HAVE_LRINT
# define ROUND(x)	lrint(x)
#else
# define ROUND(x)	floor((x) + 0.5)
#endif

INLINE void
maxmin_i1_mask(struct obuffer_t *buf, N_UI1 *maxp, N_UI1 *minp)
{
	N_UI4	i;
	const N_UI1 *source = buf->ob_ptr;
	const unsigned char *mask_ptr = buf->ob_mask;
	N_UI1 min0 = (N_UI1)0xFF;
	N_UI1 max0 = (N_UI1)0;
	for (i = 0; i < buf->nelems; i++) {
		if (mask_ptr[i / 8] & (128u >> (i % 8))) {
			if (source[i] > max0) max0 = source[i];
			if (source[i] < min0) min0 = source[i];
		}
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i1_none(struct obuffer_t *buf, N_UI1 *maxp, N_UI1 *minp)
{
	N_UI4	i;
	const N_UI1 *source = buf->ob_ptr;
#ifndef AVOID_PIPELINE_HACK
	N_UI1 min0 = (N_UI1)0xFF;
	N_UI1 min1 = (N_UI1)0xFF;
	N_UI1 min2 = (N_UI1)0xFF;
	N_UI1 min3 = (N_UI1)0xFF;
	N_UI1 max0 = (N_UI1)0;
	N_UI1 max1 = (N_UI1)0;
	N_UI1 max2 = (N_UI1)0;
	N_UI1 max3 = (N_UI1)0;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0,min1,min2,min3) reduction(max:max0,max1,max2,max3)
#  endif
# else
	/*poption parallel tlocal(i)
	 min(min0, min1, min2, min3) max(max0, max1, max2, max3) */
# endif
	for (i = 0; i < (buf->nelems & ~3u); i += 4) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
		if (source[i+1] > max1) max1 = source[i+1];
		if (source[i+1] < min1) min1 = source[i+1];
		if (source[i+2] > max2) max2 = source[i+2];
		if (source[i+2] < min2) min2 = source[i+2];
		if (source[i+3] > max3) max3 = source[i+3];
		if (source[i+3] < min3) min3 = source[i+3];
	}
	/*poption noparallel */
	for (i = (buf->nelems & ~3u) ; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
	if (max1 > max0) max0 = max1; if (min1 < min0) min0 = min1;
	if (max2 > max0) max0 = max2; if (min2 < min0) min0 = min2;
	if (max3 > max0) max0 = max3; if (min3 < min0) min0 = min3;
#else
	N_UI1 min0 = (N_UI1)0xFF;
	N_UI1 max0 = (N_UI1)0;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
#endif
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i1_udfv(struct obuffer_t *buf, N_UI1 *maxp, N_UI1 *minp)
{
	N_UI4	i;
	const N_UI1 *source = buf->ob_ptr;
	N_UI1 min0 = (N_UI1)0xFF;
	N_UI1 max0 = (N_UI1)0;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0
			&& source[i] != GlobalConfig(pc_missing_ui1)
		) max0 = source[i];
		if (source[i] < min0
			&& source[i] != GlobalConfig(pc_missing_ui1)
		) min0 = source[i];
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i2_mask(struct obuffer_t *buf, N_SI2 *maxp, N_SI2 *minp)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	const unsigned char *mask_ptr = buf->ob_mask;
	N_SI2 min0 = (N_SI2)0x7FFF;
	N_SI2 max0 = (N_SI2)-0x8000;
	for (i = 0; i < buf->nelems; i++) {
		if (mask_ptr[i / 8] & (128u >> (i % 8))) {
			if (source[i] > max0) max0 = source[i];
			if (source[i] < min0) min0 = source[i];
		}
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i2_none(struct obuffer_t *buf, N_SI2 *maxp, N_SI2 *minp)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
#ifndef AVOID_PIPELINE_HACK
	N_SI2 min0 = (N_SI2)0x7FFF;
	N_SI2 min1 = (N_SI2)0x7FFF;
	N_SI2 min2 = (N_SI2)0x7FFF;
	N_SI2 min3 = (N_SI2)0x7FFF;
	N_SI2 max0 = (N_SI2)-0x8000;
	N_SI2 max1 = (N_SI2)-0x8000;
	N_SI2 max2 = (N_SI2)-0x8000;
	N_SI2 max3 = (N_SI2)-0x8000;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0,min1,min2,min3) reduction(max:max0,max1,max2,max3)
#  endif
# else
	/*poption parallel tlocal(i)
	 min(min0, min1, min2, min3) max(max0, max1, max2, max3) */
# endif
	for (i = 0; i < (buf->nelems & ~3u); i += 4) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
		if (source[i+1] > max1) max1 = source[i+1];
		if (source[i+1] < min1) min1 = source[i+1];
		if (source[i+2] > max2) max2 = source[i+2];
		if (source[i+2] < min2) min2 = source[i+2];
		if (source[i+3] > max3) max3 = source[i+3];
		if (source[i+3] < min3) min3 = source[i+3];
	}
	/*poption noparallel */
	for (i = (buf->nelems & ~3u) ; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
	if (max1 > max0) max0 = max1; if (min1 < min0) min0 = min1;
	if (max2 > max0) max0 = max2; if (min2 < min0) min0 = min2;
	if (max3 > max0) max0 = max3; if (min3 < min0) min0 = min3;
#else
	N_SI2 min0 = (N_SI2)0x7FFF;
	N_SI2 max0 = (N_SI2)-0x8000;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
#endif
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i2_udfv(struct obuffer_t *buf, N_SI2 *maxp, N_SI2 *minp)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_SI2 min0 = (N_SI2)0x7FFF;
	N_SI2 max0 = (N_SI2)-0x8000;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0
			&& source[i] != GlobalConfig(pc_missing_si2)
		) max0 = source[i];
		if (source[i] < min0
			&& source[i] != GlobalConfig(pc_missing_si2)
		) min0 = source[i];
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i4_mask(struct obuffer_t *buf, N_SI4 *maxp, N_SI4 *minp)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	const unsigned char *mask_ptr = buf->ob_mask;
	N_SI4 min0 = 0x7FFFFFFFL;
	N_SI4 max0 = (N_SI4)0x80000000L;
	for (i = 0; i < buf->nelems; i++) {
		if (mask_ptr[i / 8] & (128u >> (i % 8))) {
			if (source[i] > max0) max0 = source[i];
			if (source[i] < min0) min0 = source[i];
		}
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i4_none(struct obuffer_t *buf, N_SI4 *maxp, N_SI4 *minp)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
#ifndef AVOID_PIPELINE_HACK
	N_SI4 min0 = 0x7FFFFFFFL;
	N_SI4 min1 = 0x7FFFFFFFL;
	N_SI4 min2 = 0x7FFFFFFFL;
	N_SI4 min3 = 0x7FFFFFFFL;
	N_SI4 max0 = (N_SI4)0x80000000L;
	N_SI4 max1 = (N_SI4)0x80000000L;
	N_SI4 max2 = (N_SI4)0x80000000L;
	N_SI4 max3 = (N_SI4)0x80000000L;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0,min1,min2,min3) reduction(max:max0,max1,max2,max3)
#  endif
# else
	/*poption parallel tlocal(i)
	 min(min0, min1, min2, min3) max(max0, max1, max2, max3) */
# endif
	for (i = 0; i < (buf->nelems & ~3u); i += 4) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
		if (source[i+1] > max1) max1 = source[i+1];
		if (source[i+1] < min1) min1 = source[i+1];
		if (source[i+2] > max2) max2 = source[i+2];
		if (source[i+2] < min2) min2 = source[i+2];
		if (source[i+3] > max3) max3 = source[i+3];
		if (source[i+3] < min3) min3 = source[i+3];
	}
	/*poption noparallel */
	for (i = (buf->nelems & ~3u) ; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
	if (max1 > max0) max0 = max1; if (min1 < min0) min0 = min1;
	if (max2 > max0) max0 = max2; if (min2 < min0) min0 = min2;
	if (max3 > max0) max0 = max3; if (min3 < min0) min0 = min3;
#else
	N_SI4 min0 = 0x7FFFFFFFL;
	N_SI4 max0 = (N_SI4)0x80000000L;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
#endif
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_i4_udfv(struct obuffer_t *buf, N_SI4 *maxp, N_SI4 *minp)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI4 min0 = 0x7FFFFFFFL;
	N_SI4 max0 = (N_SI4)0x80000000L;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0
			&& source[i] != GlobalConfig(pc_missing_si4)
		) max0 = source[i];
		if (source[i] < min0
			&& source[i] != GlobalConfig(pc_missing_si4)
		) min0 = source[i];
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_r4_mask(struct obuffer_t *buf, float *maxp, float *minp)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	const unsigned char *mask_ptr = buf->ob_mask;
	float min0 = FLT_MAX;
	float max0 = -FLT_MAX;
	for (i = 0; i < buf->nelems; i++) {
		if (mask_ptr[i / 8] & (128u >> (i % 8))) {
			if (source[i] > max0) max0 = source[i];
			if (source[i] < min0) min0 = source[i];
		}
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_r4_none(struct obuffer_t *buf, float *maxp, float *minp)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
#ifndef AVOID_PIPELINE_HACK
	float min0 = FLT_MAX;
	float min1 = FLT_MAX;
	float min2 = FLT_MAX;
	float min3 = FLT_MAX;
	float max0 = -FLT_MAX;
	float max1 = -FLT_MAX;
	float max2 = -FLT_MAX;
	float max3 = -FLT_MAX;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0,min1,min2,min3) reduction(max:max0,max1,max2,max3)
#  endif
# else
	/*poption parallel tlocal(i)
	 min(min0, min1, min2, min3) max(max0, max1, max2, max3) */
# endif
	for (i = 0; i < (buf->nelems & ~3u); i += 4) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
		if (source[i+1] > max1) max1 = source[i+1];
		if (source[i+1] < min1) min1 = source[i+1];
		if (source[i+2] > max2) max2 = source[i+2];
		if (source[i+2] < min2) min2 = source[i+2];
		if (source[i+3] > max3) max3 = source[i+3];
		if (source[i+3] < min3) min3 = source[i+3];
	}
	/*poption noparallel */
	for (i = (buf->nelems & ~3u) ; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
	if (max1 > max0) max0 = max1; if (min1 < min0) min0 = min1;
	if (max2 > max0) max0 = max2; if (min2 < min0) min0 = min2;
	if (max3 > max0) max0 = max3; if (min3 < min0) min0 = min3;
#else
	float min0 = FLT_MAX;
	float max0 = -FLT_MAX;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
#endif
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_r4_udfv(struct obuffer_t *buf, float *maxp, float *minp)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	float min0 = FLT_MAX;
	float max0 = -FLT_MAX;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0
			&& source[i] != GlobalConfig(pc_missing_r4)
		) max0 = source[i];
		if (source[i] < min0
			&& source[i] != GlobalConfig(pc_missing_r4)
		) min0 = source[i];
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_r8_mask(struct obuffer_t *buf, double *maxp, double *minp)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	const unsigned char *mask_ptr = buf->ob_mask;
	double min0 = DBL_MAX;
	double max0 = -DBL_MAX;
	for (i = 0; i < buf->nelems; i++) {
		if (mask_ptr[i / 8] & (128u >> (i % 8))) {
			if (source[i] > max0) max0 = source[i];
			if (source[i] < min0) min0 = source[i];
		}
	}
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_r8_none(struct obuffer_t *buf, double *maxp, double *minp)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
#ifndef AVOID_PIPELINE_HACK
	double min0 = DBL_MAX;
	double min1 = DBL_MAX;
	double min2 = DBL_MAX;
	double min3 = DBL_MAX;
	double max0 = -DBL_MAX;
	double max1 = -DBL_MAX;
	double max2 = -DBL_MAX;
	double max3 = -DBL_MAX;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0,min1,min2,min3) reduction(max:max0,max1,max2,max3)
#  endif
# else
	/*poption parallel tlocal(i)
	 min(min0, min1, min2, min3) max(max0, max1, max2, max3) */
# endif
	for (i = 0; i < (buf->nelems & ~3u); i += 4) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
		if (source[i+1] > max1) max1 = source[i+1];
		if (source[i+1] < min1) min1 = source[i+1];
		if (source[i+2] > max2) max2 = source[i+2];
		if (source[i+2] < min2) min2 = source[i+2];
		if (source[i+3] > max3) max3 = source[i+3];
		if (source[i+3] < min3) min3 = source[i+3];
	}
	/*poption noparallel */
	for (i = (buf->nelems & ~3u) ; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
	if (max1 > max0) max0 = max1; if (min1 < min0) min0 = min1;
	if (max2 > max0) max0 = max2; if (min2 < min0) min0 = min2;
	if (max3 > max0) max0 = max3; if (min3 < min0) min0 = min3;
#else
	double min0 = DBL_MAX;
	double max0 = -DBL_MAX;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0) max0 = source[i];
		if (source[i] < min0) min0 = source[i];
	}
#endif
	*maxp = max0;
	*minp = min0;
}

INLINE void
maxmin_r8_udfv(struct obuffer_t *buf, double *maxp, double *minp)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	double min0 = DBL_MAX;
	double max0 = -DBL_MAX;
# ifdef USE_OMP
#  if USE_OMP >= 31
#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
#  endif
# else
	/*poption parallel tlocal(i) min(min0) max(max0) */
# endif
	for (i = 0; i < buf->nelems; i++) {
		if (source[i] > max0
			&& source[i] != GlobalConfig(pc_missing_r8)
		) max0 = source[i];
		if (source[i] < min0
			&& source[i] != GlobalConfig(pc_missing_r8)
		) min0 = source[i];
	}
	*maxp = max0;
	*minp = min0;
}

static long
encode_1pac_mask_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	N_SI2 max0;
	N_SI2 min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i2_mask(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI1 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
			packed[i] = (pval);
			i++;
		}
	}
	return 12 + mask_nbytes + i * 1;
}

static long
decode_1pac_mask_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + mask_nbytes);
	result = (N_SI2 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = ROUND((N_UI1)(packed[i]) * amp + base);
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si2);
		}
	}
	return nelems;
}

static long
encode_1pac_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI1 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
			packed[i] = (pval);
			i++;
		}
	}
	return 12 + mask_nbytes + i * 1;
}

static long
decode_1pac_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + mask_nbytes);
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = ROUND((N_UI1)(packed[i]) * amp + base);
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	return nelems;
}

static long
encode_1pac_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "1PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:1PAC", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 8 + 1 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_1pac_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI1 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
			packed[i] = (pval);
			i++;
		}
	}
	return 12 + mask_nbytes + i * 1;
}

static long
decode_1pac_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = (N_UI1)(packed[i]) * amp + base;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_1pac_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=1PAC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI1 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
			packed[i] = (pval);
			i++;
		}
	}
	return 12 + mask_nbytes + i * 1;
}

static long
decode_1pac_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = (N_UI1)(packed[i]) * amp + base;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_1pac_none_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	N_SI2 max0;
	N_SI2 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i2_none(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI1 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = (pval);
	}
	return 12 + buf->nelems * 1;
}

static long
decode_1pac_none_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ROUND((N_UI1)(packed[i]) * amp + base);
		result[i + 1] = ROUND((N_UI1)(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ROUND((N_UI1)(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ROUND((N_UI1)(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_1pac_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI1 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = (pval);
	}
	return 12 + buf->nelems * 1;
}

static long
decode_1pac_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ROUND((N_UI1)(packed[i]) * amp + base);
		result[i + 1] = ROUND((N_UI1)(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ROUND((N_UI1)(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ROUND((N_UI1)(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_1pac_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "1PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:1PAC", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 8 + 1 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_1pac_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI1 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = (pval);
	}
	return 12 + buf->nelems * 1;
}

static long
decode_1pac_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI1)(packed[i]) * amp + base;
		result[i + 1] = (N_UI1)(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI1)(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI1)(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_1pac_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=1PAC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI1 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = (pval);
	}
	return 12 + buf->nelems * 1;
}

static long
decode_1pac_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI1)(packed[i]) * amp + base;
		result[i + 1] = (N_UI1)(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI1)(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI1)(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_1pac_udfv_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	N_SI2 max0;
	N_SI2 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i2_udfv(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI1(drec, 0xFF);
	POKE_float(drec + 1, base);
	POKE_float(drec + 4 + 1, amp);
	packed = (N_UI1 *)(drec + 8 + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_si2) ) {
			pval = (N_UI1)0xFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI1)0xFE ) dval = (N_UI1)0xFE;
			else if ( dval < (N_UI1)0 ) dval = (N_UI1)0;
			pval = (N_UI1)(dval);
		}
		packed[i] = (pval);
	}
	return 12 + 1 + buf->nelems * 1;
}

static long
decode_1pac_udfv_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
        PEEK_float(&base, src + 1);
        PEEK_float(&amp, src + 4 + 1);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + 1);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(ROUND((N_UI1)(packed[i]) * amp + base));
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(ROUND((N_UI1)(packed[i + 1]) * amp + base));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(ROUND((N_UI1)(packed[nelems - 1]) * amp + base));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(ROUND((N_UI1)(packed[i]) * amp + base));
	}
#endif
	return nelems;
}

static long
encode_1pac_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI1(drec, 0xFF);
	POKE_float(drec + 1, base);
	POKE_float(drec + 4 + 1, amp);
	packed = (N_UI1 *)(drec + 8 + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_UI1)0xFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI1)0xFE ) dval = (N_UI1)0xFE;
			else if ( dval < (N_UI1)0 ) dval = (N_UI1)0;
			pval = (N_UI1)(dval);
		}
		packed[i] = (pval);
	}
	return 12 + 1 + buf->nelems * 1;
}

static long
decode_1pac_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
        PEEK_float(&base, src + 1);
        PEEK_float(&amp, src + 4 + 1);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + 1);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI1)(packed[i]) * amp + base));
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI1)(packed[i + 1]) * amp + base));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI1)(packed[nelems - 1]) * amp + base));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI1)(packed[i]) * amp + base));
	}
#endif
	return nelems;
}

static long
encode_1pac_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "1PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:1PAC", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 1 + 8 + 1 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_1pac_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI1(drec, 0xFF);
	POKE_float(drec + 1, base);
	POKE_float(drec + 4 + 1, amp);
	packed = (N_UI1 *)(drec + 8 + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_UI1)0xFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI1)0xFE ) dval = (N_UI1)0xFE;
			else if ( dval < (N_UI1)0 ) dval = (N_UI1)0;
			pval = (N_UI1)(dval);
		}
		packed[i] = (pval);
	}
	return 12 + 1 + buf->nelems * 1;
}

static long
decode_1pac_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
        PEEK_float(&base, src + 1);
        PEEK_float(&amp, src + 4 + 1);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + 1);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI1)(packed[i]) * amp + base);
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI1)(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI1)(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI1)(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_1pac_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI1 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=1PAC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / 0xFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI1(drec, 0xFF);
	POKE_float(drec + 1, base);
	POKE_float(drec + 4 + 1, amp);
	packed = (N_UI1 *)(drec + 8 + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_UI1)0xFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI1)0xFE ) dval = (N_UI1)0xFE;
			else if ( dval < (N_UI1)0 ) dval = (N_UI1)0;
			pval = (N_UI1)(dval);
		}
		packed[i] = (pval);
	}
	return 12 + 1 + buf->nelems * 1;
}

static long
decode_1pac_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
        PEEK_float(&base, src + 1);
        PEEK_float(&amp, src + 4 + 1);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 8 + 1);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI1)(packed[i]) * amp + base);
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI1)(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI1)(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI1)(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_2pac_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_SI2 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (ROUND((source[j] - base_d) * scale));
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 12 + mask_nbytes + i * 2;
}

static long
decode_2pac_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8 + mask_nbytes);
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = ROUND((N_SI2)pval * amp + base);
#else
			result[j] = ROUND((N_SI2)NTOH2(packed[i]) * amp + base);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	return nelems;
}

static long
encode_2pac_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2PAC", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 8 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2pac_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_SI2 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (ROUND((source[j] - base_d) * scale));
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 12 + mask_nbytes + i * 2;
}

static long
decode_2pac_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8 + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = (N_SI2)pval * amp + base;
#else
			result[j] = (N_SI2)NTOH2(packed[i]) * amp + base;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_2pac_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2PAC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_SI2 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (ROUND((source[j] - base_d) * scale));
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 12 + mask_nbytes + i * 2;
}

static long
decode_2pac_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8 + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = (N_SI2)pval * amp + base;
#else
			result[j] = (N_SI2)NTOH2(packed[i]) * amp + base;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_2pac_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_SI2 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (ROUND((source[i] - base_d) * scale));
		packed[i] = HTON2(pval);
	}
	return 12 + buf->nelems * 2;
}

static long
decode_2pac_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ROUND((N_SI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ROUND((N_SI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ROUND((N_SI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ROUND((N_SI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_2pac_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2PAC", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 8 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2pac_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_SI2 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (ROUND((source[i] - base_d) * scale));
		packed[i] = HTON2(pval);
	}
	return 12 + buf->nelems * 2;
}

static long
decode_2pac_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_2pac_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2PAC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_SI2 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (ROUND((source[i] - base_d) * scale));
		packed[i] = HTON2(pval);
	}
	return 12 + buf->nelems * 2;
}

static long
decode_2pac_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_2pac_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_SI2(drec, 0x8000);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_SI2 *)(drec + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_SI2)0x8000;
		} else {
			double dval = ROUND((source[i] - base_d) * scale);
			if ( dval > (N_SI2)0x7FFF ) dval = (N_SI2)0x7FFF;
			else if ( dval < (N_SI2)0x8001 ) dval = (N_SI2)0x8001;
			pval = (N_SI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	return 12 + 2 + buf->nelems * 2;
}

static long
decode_2pac_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8 + 2);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_SI2)NTOH2(packed[i]) * amp + base));
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_SI2)NTOH2(packed[i + 1]) * amp + base));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_SI2)NTOH2(packed[nelems - 1]) * amp + base));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_SI2)NTOH2(packed[i]) * amp + base));
	}
#endif
	return nelems;
}

static long
encode_2pac_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2PAC", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 2 + 8 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2pac_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_SI2(drec, 0x8000);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_SI2 *)(drec + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_SI2)0x8000;
		} else {
			double dval = ROUND((source[i] - base_d) * scale);
			if ( dval > (N_SI2)0x7FFF ) dval = (N_SI2)0x7FFF;
			else if ( dval < (N_SI2)0x8001 ) dval = (N_SI2)0x8001;
			pval = (N_SI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	return 12 + 2 + buf->nelems * 2;
}

static long
decode_2pac_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8 + 2);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_2pac_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2PAC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		float width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / (N_SI2)0x7FFD;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_SI2(drec, 0x8000);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_SI2 *)(drec + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_SI2)0x8000;
		} else {
			double dval = ROUND((source[i] - base_d) * scale);
			if ( dval > (N_SI2)0x7FFF ) dval = (N_SI2)0x7FFF;
			else if ( dval < (N_SI2)0x8001 ) dval = (N_SI2)0x8001;
			pval = (N_SI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	return 12 + 2 + buf->nelems * 2;
}

static long
decode_2pac_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 8 + 2);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_2upc_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 12 + mask_nbytes + i * 2;
}

static long
decode_2upc_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8 + mask_nbytes);
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_UI2 pval;
			pval = PEEK_N_UI2((unsigned char *)(packed + i));
			result[j] = ROUND(pval * amp + base);
#else
			result[j] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	return nelems;
}

static long
encode_2upc_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPC", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 8 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upc_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 12 + mask_nbytes + i * 2;
}

static long
decode_2upc_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8 + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_UI2 pval;
			pval = PEEK_N_UI2((unsigned char *)(packed + i));
			result[j] = pval * amp + base;
#else
			result[j] = (N_UI2)NTOH2(packed[i]) * amp + base;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_2upc_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)(drec + 8 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 12 + mask_nbytes + i * 2;
}

static long
decode_2upc_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8 + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_UI2 pval;
			pval = PEEK_N_UI2((unsigned char *)(packed + i));
			result[j] = pval * amp + base;
#else
			result[j] = (N_UI2)NTOH2(packed[i]) * amp + base;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_2upc_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	return 12 + buf->nelems * 2;
}

static long
decode_2upc_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ROUND((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ROUND((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_2upc_none_nc(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI2 *source = buf->ob_ptr;
	const float *baseamp;
	N_UI2 *packed;
	/* code */
	baseamp = (float *)((N_UI1 *)buf->ob_ptr);
	POKE_float(drec, baseamp[0]);
	POKE_float(drec + 4, baseamp[1]);
	source = (const N_UI2 *)(baseamp + 2);
	packed = (N_UI2 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		packed[i] = NTOH2(source[i]);
	}
	return 12 + buf->nelems * 2;
}

static long
decode_2upc_none_nc(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI2	*result_packed;
	N_UI4	*result_baseamp;
	const N_UI2	*packed;
	const N_UI4 *baseamp;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	baseamp = (const N_UI4 *)(src);
	result_baseamp = (N_UI4 *)((N_UI1 *)buf->ib_ptr);
#if NEED_ALIGN & 4
	memcpy(result_baseamp, baseamp, 8);
	endian_swab4(result_baseamp, 2);
#else
	result_baseamp[0] = NTOH4(baseamp[0]);
	result_baseamp[1] = NTOH4(baseamp[1]);
#endif
	packed = (N_UI2 *)(src + 8);
	result_packed = (N_UI2 *)((N_UI1 *)buf->ib_ptr + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result_packed[i] = NTOH2(packed[i]);
	}
	return nelems;
}

static long
encode_2upc_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPC", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 8 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upc_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	return 12 + buf->nelems * 2;
}

static long
decode_2upc_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_UI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_2upc_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	return 12 + buf->nelems * 2;
}

static long
decode_2upc_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_UI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_2upc_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)(drec + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	return 12 + 2 + buf->nelems * 2;
}

static long
decode_2upc_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8 + 2);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i]) * amp + base));
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i + 1]) * amp + base));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[nelems - 1]) * amp + base));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i]) * amp + base));
	}
#endif
	return nelems;
}

static long
encode_2upc_udfv_nc(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI2 *source = buf->ob_ptr;
	const float *baseamp;
	N_UI2 *packed;
	POKE_N_UI2(drec, 0xFFFF);
	/* code */
	baseamp = (float *)((N_UI1 *)buf->ob_ptr + 2);
	POKE_float(drec + 2, baseamp[0]);
	POKE_float(drec + 4 + 2, baseamp[1]);
	source = (const N_UI2 *)(baseamp + 2);
	packed = (N_UI2 *)(drec + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		packed[i] = NTOH2(source[i]);
	}
	return 12 + 2 + buf->nelems * 2;
}

static long
decode_2upc_udfv_nc(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI2	*result_packed;
	N_UI4	*result_baseamp;
	const N_UI2	*packed;
	const N_UI4 *baseamp;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	baseamp = (const N_UI4 *)(src + 2);
	result_baseamp = (N_UI4 *)((N_UI1 *)buf->ib_ptr + 2);
#if NEED_ALIGN & 4
	memcpy(result_baseamp, baseamp, 8);
	endian_swab4(result_baseamp, 2);
#else
	result_baseamp[0] = NTOH4(baseamp[0]);
	result_baseamp[1] = NTOH4(baseamp[1]);
#endif
	packed = (N_UI2 *)(src + 8 + 2);
	result_packed = (N_UI2 *)((N_UI1 *)buf->ib_ptr + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result_packed[i] = NTOH2(packed[i]);
	}
	return nelems;
}

static long
encode_2upc_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPC", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 2 + 8 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upc_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)(drec + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	return 12 + 2 + buf->nelems * 2;
}

static long
decode_2upc_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8 + 2);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_2upc_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPC",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)(drec + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	return 12 + 2 + buf->nelems * 2;
}

static long
decode_2upc_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI2 *)(src + 8 + 2);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_2upj_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	dptr = drec + 8 + mask_nbytes;
	rlen = nus_encode_jp2k((const unsigned char *)packed, i, 1, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + mask_nbytes + rlen;
}

static long
decode_2upj_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + mask_nbytes;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - mask_nbytes);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPJ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPJ", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = buf->nelems;
	N_UI4 offset = 16 + mask_nbytes + 8;
	if ( offset + 24 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size for 2UPJ too small %d", buf->nelems));
	if (memcmp("\xff\x4f\xff\x51", src + offset, 4)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 header"));
	if (memcmp("\xff\xd9", src + buf->nelems - 2, 2)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 footer"));
	N_UI4 jas_x = PEEK_N_UI4(src + offset + 8);
	N_UI4 jas_y = PEEK_N_UI4(src + offset + 12);
	N_UI4 jas_x0 = PEEK_N_UI4(src + offset + 16);
	N_UI4 jas_y0 = PEEK_N_UI4(src + offset + 20);
	N_SI8 expect_grid = (jas_x - jas_x0) * (jas_y - jas_y0);
	if (jas_x0 >= jas_x) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.x0:%d >= 2upj.x:%d", jas_x0, jas_x));
	if (jas_y0 >= jas_y) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.y0:%d >= 2upj.y:%d", jas_y0, jas_y));
	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: data.size:%d != 2upj.size:%d", dat_size, expect_grid));
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upj_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	dptr = drec + 8 + mask_nbytes;
	rlen = nus_encode_jp2k((const unsigned char *)packed, i, 1, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + mask_nbytes + rlen;
}

static long
decode_2upj_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + mask_nbytes;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - mask_nbytes);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = (N_UI2)NTOH2(packed[i]) * amp + base;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPJ",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	dptr = drec + 8 + mask_nbytes;
	rlen = nus_encode_jp2k((const unsigned char *)packed, i, 1, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + mask_nbytes + rlen;
}

static long
decode_2upj_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + mask_nbytes;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - mask_nbytes);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = (N_UI2)NTOH2(packed[i]) * amp + base;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upj_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - 0);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ROUND((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ROUND((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_none_nc(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI2 *source = buf->ob_ptr;
	const float *baseamp;
	N_UI2 *packed;
	unsigned char *dptr;
	long rlen;
	/* code */
	baseamp = (float *)((N_UI1 *)buf->ob_ptr);
	POKE_float(drec, baseamp[0]);
	POKE_float(drec + 4, baseamp[1]);
	source = (const N_UI2 *)(baseamp + 2);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		packed[i] = NTOH2(source[i]);
	}
	dptr = drec + 8;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upj_none_nc(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI2	*result_packed;
	N_UI4	*result_baseamp;
	const N_UI2	*packed;
	const N_UI4 *baseamp;
	N_UI4		i, nelems;
	unsigned char *dptr;
	const unsigned char *sptr;
	long ret;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	baseamp = (const N_UI4 *)(src);
	result_baseamp = (N_UI4 *)((N_UI1 *)buf->ib_ptr);
#if NEED_ALIGN & 4
	memcpy(result_baseamp, baseamp, 8);
	endian_swab4(result_baseamp, 2);
#else
	result_baseamp[0] = NTOH4(baseamp[0]);
	result_baseamp[1] = NTOH4(baseamp[1]);
#endif
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -68);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result_packed = (N_UI2 *)((N_UI1 *)buf->ib_ptr + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result_packed[i] = NTOH2(packed[i]);
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPJ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPJ", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = buf->nelems;
	N_UI4 offset = 16 + 8;
	if ( offset + 24 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size for 2UPJ too small %d", buf->nelems));
	if (memcmp("\xff\x4f\xff\x51", src + offset, 4)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 header"));
	if (memcmp("\xff\xd9", src + buf->nelems - 2, 2)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 footer"));
	N_UI4 jas_x = PEEK_N_UI4(src + offset + 8);
	N_UI4 jas_y = PEEK_N_UI4(src + offset + 12);
	N_UI4 jas_x0 = PEEK_N_UI4(src + offset + 16);
	N_UI4 jas_y0 = PEEK_N_UI4(src + offset + 20);
	N_SI8 expect_grid = (jas_x - jas_x0) * (jas_y - jas_y0);
	if (jas_x0 >= jas_x) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.x0:%d >= 2upj.x:%d", jas_x0, jas_x));
	if (jas_y0 >= jas_y) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.y0:%d >= 2upj.y:%d", jas_y0, jas_y));
	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: data.size:%d != 2upj.size:%d", dat_size, expect_grid));
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upj_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upj_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - 0);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_UI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPJ",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upj_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - 0);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_UI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upj_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - 2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i]) * amp + base));
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i + 1]) * amp + base));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[nelems - 1]) * amp + base));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i]) * amp + base));
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_udfv_nc(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI2 *source = buf->ob_ptr;
	const float *baseamp;
	N_UI2 *packed;
	unsigned char *dptr;
	long rlen;
	POKE_N_UI2(drec, 0xFFFF);
	/* code */
	baseamp = (float *)((N_UI1 *)buf->ob_ptr + 2);
	POKE_float(drec + 2, baseamp[0]);
	POKE_float(drec + 4 + 2, baseamp[1]);
	source = (const N_UI2 *)(baseamp + 2);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		packed[i] = NTOH2(source[i]);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upj_udfv_nc(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI2	*result_packed;
	N_UI4	*result_baseamp;
	const N_UI2	*packed;
	const N_UI4 *baseamp;
	N_UI4		i, nelems;
	unsigned char *dptr;
	const unsigned char *sptr;
	long ret;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	baseamp = (const N_UI4 *)(src + 2);
	result_baseamp = (N_UI4 *)((N_UI1 *)buf->ib_ptr + 2);
#if NEED_ALIGN & 4
	memcpy(result_baseamp, baseamp, 8);
	endian_swab4(result_baseamp, 2);
#else
	result_baseamp[0] = NTOH4(baseamp[0]);
	result_baseamp[1] = NTOH4(baseamp[1]);
#endif
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -68);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result_packed = (N_UI2 *)((N_UI1 *)buf->ib_ptr + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result_packed[i] = NTOH2(packed[i]);
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPJ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPJ", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = buf->nelems;
	N_UI4 offset = 16 + 2 + 8;
	if ( offset + 24 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size for 2UPJ too small %d", buf->nelems));
	if (memcmp("\xff\x4f\xff\x51", src + offset, 4)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 header"));
	if (memcmp("\xff\xd9", src + buf->nelems - 2, 2)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 footer"));
	N_UI4 jas_x = PEEK_N_UI4(src + offset + 8);
	N_UI4 jas_y = PEEK_N_UI4(src + offset + 12);
	N_UI4 jas_x0 = PEEK_N_UI4(src + offset + 16);
	N_UI4 jas_y0 = PEEK_N_UI4(src + offset + 20);
	N_SI8 expect_grid = (jas_x - jas_x0) * (jas_y - jas_y0);
	if (jas_x0 >= jas_x) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.x0:%d >= 2upj.x:%d", jas_x0, jas_x));
	if (jas_y0 >= jas_y) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.y0:%d >= 2upj.y:%d", jas_y0, jas_y));
	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: data.size:%d != 2upj.size:%d", dat_size, expect_grid));
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upj_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upj_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - 2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upj_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPJ",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_jp2k((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upj_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_jp2k(sptr, dptr, PEEK_N_UI4(src - 64) -60 -8 - 2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	dptr = drec + 8 + mask_nbytes;
	rlen = nus_encode_cpsd((const unsigned char *)packed, i, 1, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + mask_nbytes + rlen;
}

static long
decode_2upp_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + mask_nbytes;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPP") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPP", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	int j, width;
	expect_size = 16 + mask_nbytes + 8;
	N_UI4 expect_grid = PEEK_N_UI4(src + expect_size);
	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPP: data.size:%d != 2upp.size:%d", dat_size, expect_grid));
	N_UI4 n_g = 1 + (dat_size - 1) / 32;
	expect_size += 4 + 2 * n_g;
	N_UI4 packed_size = 0;
	for (j = 0; j < n_g; ++j) {
		width = 1 + ((src[expect_size + j / 2] >> (0 == j % 2 ? 4 : 0)) & 15);
		if (j == n_g - 1 && 0 != dat_size % 32) {
			packed_size += (((dat_size % 32) * width + 7) / 8 + 3) / 4 * 4;
		} else {
			packed_size += 4 * width;
		}
	}
	expect_size += (n_g - 1) / 2 + 1 + packed_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upp_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	dptr = drec + 8 + mask_nbytes;
	rlen = nus_encode_cpsd((const unsigned char *)packed, i, 1, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + mask_nbytes + rlen;
}

static long
decode_2upp_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + mask_nbytes;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = (N_UI2)NTOH2(packed[i]) * amp + base;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPP",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_float(drec + mask_nbytes, base);
	POKE_float(drec + 4 + mask_nbytes, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI2 pval;
			pval = ((source[j] - base_d) * scale + 0.5);
#if NEED_ALIGN & 2
			POKE_N_UI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	dptr = drec + 8 + mask_nbytes;
	rlen = nus_encode_cpsd((const unsigned char *)packed, i, 1, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + mask_nbytes + rlen;
}

static long
decode_2upp_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_float(&base, src + mask_nbytes);
        PEEK_float(&amp, src + 4 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + mask_nbytes;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = (N_UI2)NTOH2(packed[i]) * amp + base;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upp_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ROUND((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ROUND((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ROUND((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_none_nc(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI2 *source = buf->ob_ptr;
	const float *baseamp;
	N_UI2 *packed;
	unsigned char *dptr;
	long rlen;
	/* code */
	baseamp = (float *)((N_UI1 *)buf->ob_ptr);
	POKE_float(drec, baseamp[0]);
	POKE_float(drec + 4, baseamp[1]);
	source = (const N_UI2 *)(baseamp + 2);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		packed[i] = NTOH2(source[i]);
	}
	dptr = drec + 8;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upp_none_nc(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI2	*result_packed;
	N_UI4	*result_baseamp;
	const N_UI2	*packed;
	const N_UI4 *baseamp;
	N_UI4		i, nelems;
	unsigned char *dptr;
	const unsigned char *sptr;
	long ret;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	baseamp = (const N_UI4 *)(src);
	result_baseamp = (N_UI4 *)((N_UI1 *)buf->ib_ptr);
#if NEED_ALIGN & 4
	memcpy(result_baseamp, baseamp, 8);
	endian_swab4(result_baseamp, 2);
#else
	result_baseamp[0] = NTOH4(baseamp[0]);
	result_baseamp[1] = NTOH4(baseamp[1]);
#endif
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result_packed = (N_UI2 *)((N_UI1 *)buf->ib_ptr + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result_packed[i] = NTOH2(packed[i]);
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPP") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPP", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	int j, width;
	expect_size = 16 + 8;
	N_UI4 expect_grid = PEEK_N_UI4(src + expect_size);
	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPP: data.size:%d != 2upp.size:%d", dat_size, expect_grid));
	N_UI4 n_g = 1 + (dat_size - 1) / 32;
	expect_size += 4 + 2 * n_g;
	N_UI4 packed_size = 0;
	for (j = 0; j < n_g; ++j) {
		width = 1 + ((src[expect_size + j / 2] >> (0 == j % 2 ? 4 : 0)) & 15);
		if (j == n_g - 1 && 0 != dat_size % 32) {
			packed_size += (((dat_size % 32) * width + 7) / 8 + 3) / 4 * 4;
		} else {
			packed_size += 4 * width;
		}
	}
	expect_size += (n_g - 1) / 2 + 1 + packed_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upp_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upp_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_UI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPP",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	/* missing = NONE */
	POKE_float(drec, base);
	POKE_float(drec + 4, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		pval = ((source[i] - base_d) * scale + 0.5);
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + rlen;
}

static long
decode_2upp_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	/* code */
        PEEK_float(&base, src);
        PEEK_float(&amp, src + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
		result[i + 1] = (N_UI2)NTOH2(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_UI2)NTOH2(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_UI2)NTOH2(packed[i]) * amp + base;
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	N_SI4 max0;
	N_SI4 min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upp_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i]) * amp + base));
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i + 1]) * amp + base));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[nelems - 1]) * amp + base));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(ROUND((N_UI2)NTOH2(packed[i]) * amp + base));
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_udfv_nc(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI2 *source = buf->ob_ptr;
	const float *baseamp;
	N_UI2 *packed;
	unsigned char *dptr;
	long rlen;
	POKE_N_UI2(drec, 0xFFFF);
	/* code */
	baseamp = (float *)((N_UI1 *)buf->ob_ptr + 2);
	POKE_float(drec + 2, baseamp[0]);
	POKE_float(drec + 4 + 2, baseamp[1]);
	source = (const N_UI2 *)(baseamp + 2);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		packed[i] = NTOH2(source[i]);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upp_udfv_nc(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI2	*result_packed;
	N_UI4	*result_baseamp;
	const N_UI2	*packed;
	const N_UI4 *baseamp;
	N_UI4		i, nelems;
	unsigned char *dptr;
	const unsigned char *sptr;
	long ret;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	baseamp = (const N_UI4 *)(src + 2);
	result_baseamp = (N_UI4 *)((N_UI1 *)buf->ib_ptr + 2);
#if NEED_ALIGN & 4
	memcpy(result_baseamp, baseamp, 8);
	endian_swab4(result_baseamp, 2);
#else
	result_baseamp[0] = NTOH4(baseamp[0]);
	result_baseamp[1] = NTOH4(baseamp[1]);
#endif
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result_packed = (N_UI2 *)((N_UI1 *)buf->ib_ptr + 8 + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result_packed[i] = NTOH2(packed[i]);
	}
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "2UPP") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:2UPP", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	int j, width;
	expect_size = 16 + 2 + 8;
	N_UI4 expect_grid = PEEK_N_UI4(src + expect_size);
	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPP: data.size:%d != 2upp.size:%d", dat_size, expect_grid));
	N_UI4 n_g = 1 + (dat_size - 1) / 32;
	expect_size += 4 + 2 * n_g;
	N_UI4 packed_size = 0;
	for (j = 0; j < n_g; ++j) {
		width = 1 + ((src[expect_size + j / 2] >> (0 == j % 2 ? 4 : 0)) & 15);
		if (j == n_g - 1 && 0 != dat_size % 32) {
			packed_size += (((dat_size % 32) * width + 7) / 8 + 3) / 4 * 4;
		} else {
			packed_size += 4 * width;
		}
	}
	expect_size += (n_g - 1) / 2 + 1 + packed_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_2upp_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upp_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_2upp_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI2 *packed;
	float base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	unsigned char *dptr;
	long rlen;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > FLT_MAX) || (min0 < -FLT_MAX)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=2UPP",
			(double)min0, (double)max0));
	}
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = min0;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		amp_d = (max0 - base_d) / (N_UI2)0xFFFC ;
		scale = 1.0 / amp_d;
	}
	amp  = (float)amp_d;
	base = (float)base_d;
	POKE_N_UI2(drec, 0xFFFF);
	POKE_float(drec + 2, base);
	POKE_float(drec + 4 + 2, amp);
	packed = (N_UI2 *)nus_malloc(sizeof(N_UI2)*buf->nelems);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_UI2)0xFFFF;
		} else {
			double dval = (source[i] - base_d) * scale + 0.5;
			if ( dval > (N_UI2)0xFFFE ) dval = (N_UI2)0xFFFE;
			else if ( dval < (N_UI2)0 ) dval = (N_UI2)0;
			pval = (N_UI2)(dval);
		}
		packed[i] = HTON2(pval);
	}
	dptr = drec + 8 + 2;
	rlen = nus_encode_cpsd((const unsigned char *)packed, nxd, nyd, dptr, sizeof(N_UI2)*buf->nelems*2);
	nus_free(packed);
	if (rlen < 0) { return rlen; }
	return 12 + 2 + rlen;
}

static long
decode_2upp_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	const unsigned char *sptr;
	long ret;
	unsigned char *dptr;
	float	base, amp;
	N_UI2 missval;
	/* code */
	missval = NTOH2(*(N_UI2 *)(src));
        PEEK_float(&base, src + 2);
        PEEK_float(&amp, src + 4 + 2);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	dptr = (unsigned char *)nus_malloc(sizeof(N_UI2)*nelems);
	sptr = src + 8 + 2;
	ret = nus_decode_cpsd(sptr, dptr, sizeof(N_UI2)*nelems*2);
	if (ret < 0) {
		nus_free(dptr);
		return ret;
	}
	packed = (N_UI2 *)dptr;
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i]) * amp + base);
		result[i + 1] = ((N_UI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_UI2)NTOH2(packed[i]) * amp + base);
	}
#endif
	nus_free(dptr);
	return nelems;
}

static long
encode_4pac_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "4PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:4PAC", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 16 + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_4pac_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI4 *packed;
	double base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		double width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / 0x7FFFFFFDL;
		scale = 1.0 / amp_d;
	}
	amp  = (double)amp_d;
	base = (double)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_double(drec + mask_nbytes, base);
	POKE_double(drec + 8 + mask_nbytes, amp);
	packed = (N_SI4 *)(drec + 16 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI4 pval;
			pval = (ROUND((source[j] - base_d) * scale));
#if NEED_ALIGN & 4
			POKE_N_SI4(packed + i, pval);
#else
			packed[i] = HTON4(pval);
#endif
			i++;
		}
	}
	return 20 + mask_nbytes + i * 4;
}

static long
decode_4pac_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	double	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_double(&base, src + mask_nbytes);
        PEEK_double(&amp, src + 8 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 16 + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 4
			N_SI4 pval;
			pval = PEEK_N_SI4((unsigned char *)(packed + i));
			result[j] = (N_SI4)pval * amp + base;
#else
			result[j] = (N_SI4)NTOH4(packed[i]) * amp + base;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_4pac_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI4 *packed;
	double base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		double width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / 0x7FFFFFFDL;
		scale = 1.0 / amp_d;
	}
	amp  = (double)amp_d;
	base = (double)base_d;
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	POKE_double(drec + mask_nbytes, base);
	POKE_double(drec + 8 + mask_nbytes, amp);
	packed = (N_SI4 *)(drec + 16 + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI4 pval;
			pval = (ROUND((source[j] - base_d) * scale));
#if NEED_ALIGN & 4
			POKE_N_SI4(packed + i, pval);
#else
			packed[i] = HTON4(pval);
#endif
			i++;
		}
	}
	return 20 + mask_nbytes + i * 4;
}

static long
decode_4pac_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	double	base, amp;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
        PEEK_double(&base, src + mask_nbytes);
        PEEK_double(&amp, src + 8 + mask_nbytes);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 16 + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 4
			N_SI4 pval;
			pval = PEEK_N_SI4((unsigned char *)(packed + i));
			result[j] = (N_SI4)pval * amp + base;
#else
			result[j] = (N_SI4)NTOH4(packed[i]) * amp + base;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_4pac_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "4PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:4PAC", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 16 + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_4pac_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI4 *packed;
	double base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		double width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / 0x7FFFFFFDL;
		scale = 1.0 / amp_d;
	}
	amp  = (double)amp_d;
	base = (double)base_d;
	/* missing = NONE */
	POKE_double(drec, base);
	POKE_double(drec + 8, amp);
	packed = (N_SI4 *)(drec + 16);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		pval = (ROUND((source[i] - base_d) * scale));
		packed[i] = HTON4(pval);
	}
	return 20 + buf->nelems * 4;
}

static long
decode_4pac_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	double	base, amp;
	/* code */
        PEEK_double(&base, src);
        PEEK_double(&amp, src + 8);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 16);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI4)NTOH4(packed[i]) * amp + base;
		result[i + 1] = (N_SI4)NTOH4(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI4)NTOH4(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI4)NTOH4(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_4pac_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI4 *packed;
	double base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		double width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / 0x7FFFFFFDL;
		scale = 1.0 / amp_d;
	}
	amp  = (double)amp_d;
	base = (double)base_d;
	/* missing = NONE */
	POKE_double(drec, base);
	POKE_double(drec + 8, amp);
	packed = (N_SI4 *)(drec + 16);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		pval = (ROUND((source[i] - base_d) * scale));
		packed[i] = HTON4(pval);
	}
	return 20 + buf->nelems * 4;
}

static long
decode_4pac_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	double	base, amp;
	/* code */
        PEEK_double(&base, src);
        PEEK_double(&amp, src + 8);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 16);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI4)NTOH4(packed[i]) * amp + base;
		result[i + 1] = (N_SI4)NTOH4(packed[i + 1]) * amp + base;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI4)NTOH4(packed[nelems - 1]) * amp + base;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI4)NTOH4(packed[i]) * amp + base;
	}
#endif
	return nelems;
}

static long
encode_4pac_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "4PAC") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:4PAC", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 4 + 16 + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_4pac_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI4 *packed;
	double base, amp;
	float max0;
	float min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		double width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / 0x7FFFFFFDL;
		scale = 1.0 / amp_d;
	}
	amp  = (double)amp_d;
	base = (double)base_d;
	POKE_N_SI4(drec, 0x80000000L);
	POKE_double(drec + 4, base);
	POKE_double(drec + 8 + 4, amp);
	packed = (N_SI4 *)(drec + 16 + 4);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_SI4)0x80000000L;
		} else {
			double dval = ROUND((source[i] - base_d) * scale);
			if ( dval > (N_SI4)0x7FFFFFFFL ) dval = (N_SI4)0x7FFFFFFFL;
			else if ( dval < (N_SI4)0x80000001L ) dval = (N_SI4)0x80000001L;
			pval = (N_SI4)(dval);
		}
		packed[i] = HTON4(pval);
	}
	return 20 + 4 + buf->nelems * 4;
}

static long
decode_4pac_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	double	base, amp;
	N_SI4 missval;
	/* code */
	missval = (N_SI4)NTOH4(*(N_SI4 *)(src));
        PEEK_double(&base, src + 4);
        PEEK_double(&amp, src + 8 + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 16 + 4);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[i]) * amp + base);
		result[i + 1] = ((N_SI4)NTOH4(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI4)NTOH4(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_4pac_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI4 *packed;
	double base, amp;
	double max0;
	double min0;
	double scale, base_d, amp_d;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if (0 == min0) min0 = 0;
	if (0 == max0) max0 = 0;
	base_d = (min0 + max0) * 0.5;
	if (min0 == max0) {
		amp_d = scale = 1.0;
	} else {
		double width;
		if ((max0 - base_d) > (base_d - min0)) {
			width = max0 - base_d;
		} else {
			width = base_d - min0;
		}
		amp_d = width / 0x7FFFFFFDL;
		scale = 1.0 / amp_d;
	}
	amp  = (double)amp_d;
	base = (double)base_d;
	POKE_N_SI4(drec, 0x80000000L);
	POKE_double(drec + 4, base);
	POKE_double(drec + 8 + 4, amp);
	packed = (N_SI4 *)(drec + 16 + 4);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_SI4)0x80000000L;
		} else {
			double dval = ROUND((source[i] - base_d) * scale);
			if ( dval > (N_SI4)0x7FFFFFFFL ) dval = (N_SI4)0x7FFFFFFFL;
			else if ( dval < (N_SI4)0x80000001L ) dval = (N_SI4)0x80000001L;
			pval = (N_SI4)(dval);
		}
		packed[i] = HTON4(pval);
	}
	return 20 + 4 + buf->nelems * 4;
}

static long
decode_4pac_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	double	base, amp;
	N_SI4 missval;
	/* code */
	missval = (N_SI4)NTOH4(*(N_SI4 *)(src));
        PEEK_double(&base, src + 4);
        PEEK_double(&amp, src + 8 + 4);
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 16 + 4);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[i]) * amp + base);
		result[i + 1] = ((N_SI4)NTOH4(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[i + 1]) * amp + base);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI4)NTOH4(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[nelems - 1]) * amp + base);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[i]) * amp + base);
	}
#endif
	return nelems;
}

static long
encode_i1_mask_i1(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI1 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_UI1 max0;
	N_UI1 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i1_mask(buf, &max0, &min0);
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_UI1 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = (source[j]);
			packed[i] = (pval);
			i++;
		}
	}
	return 4 + mask_nbytes + i * 1;
}

static long
decode_i1_mask_i1(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_UI1	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + mask_nbytes);
	result = (N_UI1 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = packed[i];
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_ui1);
		}
	}
	return nelems;
}

static long
encode_i1_mask_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_SI2 max0;
	N_SI2 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i2_mask(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_UI1 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = (source[j]);
			packed[i] = (pval);
			i++;
		}
	}
	return 4 + mask_nbytes + i * 1;
}

static long
decode_i1_mask_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + mask_nbytes);
	result = (N_SI2 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = packed[i];
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si2);
		}
	}
	return nelems;
}

static long
encode_i1_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_SI4 max0;
	N_SI4 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_UI1 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = (source[j]);
			packed[i] = (pval);
			i++;
		}
	}
	return 4 + mask_nbytes + i * 1;
}

static long
decode_i1_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + mask_nbytes);
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = packed[i];
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	return nelems;
}

static long
encode_i1_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I1  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I1  ", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 1 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i1_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI1 *packed;
	float max0;
	float min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_UI1 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = (source[j]);
			packed[i] = (pval);
			i++;
		}
	}
	return 4 + mask_nbytes + i * 1;
}

static long
decode_i1_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = packed[i];
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_i1_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI1 *packed;
	double max0;
	double min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_UI1 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_UI1 pval;
			pval = (source[j]);
			packed[i] = (pval);
			i++;
		}
	}
	return 4 + mask_nbytes + i * 1;
}

static long
decode_i1_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			result[j] = packed[i];
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_i1_none_i1(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI1 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_UI1 max0;
	N_UI1 min0;
	/* code */
	maxmin_i1_none(buf, &max0, &min0);
	/* missing = NONE */
	packed = (N_UI1 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = (source[i]);
		packed[i] = (pval);
	}
	return 4 + buf->nelems * 1;
}

static long
decode_i1_none_i1(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_UI1	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src);
	result = (N_UI1 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = packed[i];
		result[i + 1] = packed[i + 1];
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = packed[nelems - 1];
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = packed[i];
	}
#endif
	return nelems;
}

static long
encode_i1_none_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_SI2 max0;
	N_SI2 min0;
	/* code */
	maxmin_i2_none(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_UI1 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = (source[i]);
		packed[i] = (pval);
	}
	return 4 + buf->nelems * 1;
}

static long
decode_i1_none_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = packed[i];
		result[i + 1] = packed[i + 1];
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = packed[nelems - 1];
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = packed[i];
	}
#endif
	return nelems;
}

static long
encode_i1_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_UI1 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = (source[i]);
		packed[i] = (pval);
	}
	return 4 + buf->nelems * 1;
}

static long
decode_i1_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = packed[i];
		result[i + 1] = packed[i + 1];
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = packed[nelems - 1];
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = packed[i];
	}
#endif
	return nelems;
}

static long
encode_i1_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I1  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I1  ", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 1 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i1_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI1 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_UI1 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = (source[i]);
		packed[i] = (pval);
	}
	return 4 + buf->nelems * 1;
}

static long
decode_i1_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	float	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = packed[i];
		result[i + 1] = packed[i + 1];
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = packed[nelems - 1];
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = packed[i];
	}
#endif
	return nelems;
}

static long
encode_i1_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI1 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_UI1 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		pval = (source[i]);
		packed[i] = (pval);
	}
	return 4 + buf->nelems * 1;
}

static long
decode_i1_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	double	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = packed[i];
		result[i + 1] = packed[i + 1];
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = packed[nelems - 1];
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = packed[i];
	}
#endif
	return nelems;
}

static long
encode_i1_udfv_i1(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_UI1 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_UI1 max0;
	N_UI1 min0;
	/* code */
	maxmin_i1_udfv(buf, &max0, &min0);
	POKE_N_UI1(drec, 0xFF);
	packed = (N_UI1 *)(drec + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_ui1) ) {
			pval = (N_UI1)0xFF;
		} else {
			pval = (N_UI1)(source[i]);
			if ( (N_UI1)0xFF == pval ) pval = (N_UI1)0xFE;
		}
		packed[i] = (pval);
	}
	return 4 + 1 + buf->nelems * 1;
}

static long
decode_i1_udfv_i1(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_UI1	*result;
	N_UI4		i, nelems;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 1);
	result = (N_UI1 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_ui1)
			: (N_UI1)(packed[i]);
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_ui1)
			: (N_UI1)(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_ui1)
			: (N_UI1)(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_ui1)
			: (N_UI1)(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i1_udfv_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_SI2 max0;
	N_SI2 min0;
	/* code */
	maxmin_i2_udfv(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	POKE_N_UI1(drec, 0xFF);
	packed = (N_UI1 *)(drec + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_si2) ) {
			pval = (N_UI1)0xFF;
		} else {
			pval = (N_UI1)(source[i]);
			if ( (N_UI1)0xFF == pval ) pval = (N_UI1)0xFE;
		}
		packed[i] = (pval);
	}
	return 4 + 1 + buf->nelems * 1;
}

static long
decode_i1_udfv_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 1);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(packed[i]);
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i1_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_UI1 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	POKE_N_UI1(drec, 0xFF);
	packed = (N_UI1 *)(drec + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_UI1)0xFF;
		} else {
			pval = (N_UI1)(source[i]);
			if ( (N_UI1)0xFF == pval ) pval = (N_UI1)0xFE;
		}
		packed[i] = (pval);
	}
	return 4 + 1 + buf->nelems * 1;
}

static long
decode_i1_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 1);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(packed[i]);
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i1_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I1  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I1  ", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 1 + 1 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i1_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_UI1 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	POKE_N_UI1(drec, 0xFF);
	packed = (N_UI1 *)(drec + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_UI1)0xFF;
		} else {
			pval = (N_UI1)(source[i]);
			if ( (N_UI1)0xFF == pval ) pval = (N_UI1)0xFE;
		}
		packed[i] = (pval);
	}
	return 4 + 1 + buf->nelems * 1;
}

static long
decode_i1_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 1);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)(packed[i]);
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i1_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_UI1 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > (N_UI1)0xFF) || (min0 < (N_UI1)0)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I1  ",
			(double)min0, (double)max0));
	}
	POKE_N_UI1(drec, 0xFF);
	packed = (N_UI1 *)(drec + 1);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_UI1 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_UI1)0xFF;
		} else {
			pval = (N_UI1)(source[i]);
			if ( (N_UI1)0xFF == pval ) pval = (N_UI1)0xFE;
		}
		packed[i] = (pval);
	}
	return 4 + 1 + buf->nelems * 1;
}

static long
decode_i1_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI1	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_UI1 missval;
	/* code */
	missval = (*(N_UI1 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI1 *)(src + 1);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)(packed[i]);
		result[i + 1] = ((N_UI1)(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_UI1)(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_UI1)(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i2_mask_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI2 max0;
	N_SI2 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i2_mask(buf, &max0, &min0);
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (source[j]);
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_i2_mask_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (N_SI2 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = pval;
#else
			result[j] = (N_SI2)NTOH2(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si2);
		}
	}
	return nelems;
}

static long
encode_i2_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI4 max0;
	N_SI4 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (source[j]);
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_i2_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = (N_SI2)pval;
#else
			result[j] = (N_SI2)NTOH2(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	return nelems;
}

static long
encode_i2_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I2  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I2  ", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i2_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float max0;
	float min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (source[j]);
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_i2_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = pval;
#else
			result[j] = (N_SI2)NTOH2(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_i2_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	double max0;
	double min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (source[j]);
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_i2_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = pval;
#else
			result[j] = (N_SI2)NTOH2(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_i2_none_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI2 max0;
	N_SI2 min0;
	/* code */
	maxmin_i2_none(buf, &max0, &min0);
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (source[i]);
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_i2_none_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]);
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i2_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (source[i]);
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_i2_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]);
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i2_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I2  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I2  ", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i2_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (source[i]);
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_i2_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]);
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i2_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (source[i]);
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_i2_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]);
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i2_udfv_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI2 max0;
	N_SI2 min0;
	/* code */
	maxmin_i2_udfv(buf, &max0, &min0);
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si2) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)(source[i]);
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_i2_udfv_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[i]));
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_i2_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)(source[i]);
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_i2_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[i]));
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_i2_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I2  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I2  ", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 2 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i2_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)(source[i]);
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_i2_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i]));
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_i2_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I2  ",
			(double)min0, (double)max0));
	}
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)(source[i]);
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_i2_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i]));
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_i4_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI4 *packed;
	N_SI4 max0;
	N_SI4 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI4 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI4 pval;
			pval = (source[j]);
#if NEED_ALIGN & 4
			POKE_N_SI4(packed + i, pval);
#else
			packed[i] = HTON4(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 4;
}

static long
decode_i4_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + mask_nbytes);
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 4
			N_SI4 pval;
			pval = PEEK_N_SI4((unsigned char *)(packed + i));
			result[j] = pval;
#else
			result[j] = (N_SI4)NTOH4(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	return nelems;
}

static long
encode_i4_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I4  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I4  ", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i4_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI4 *packed;
	float max0;
	float min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if ((max0 > 0x7FFFFFFFL) || (min0 < (N_SI4)0x80000000L)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I4  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI4 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI4 pval;
			pval = (source[j]);
#if NEED_ALIGN & 4
			POKE_N_SI4(packed + i, pval);
#else
			packed[i] = HTON4(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 4;
}

static long
decode_i4_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 4
			N_SI4 pval;
			pval = PEEK_N_SI4((unsigned char *)(packed + i));
			result[j] = pval;
#else
			result[j] = (N_SI4)NTOH4(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_i4_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI4 *packed;
	double max0;
	double min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > 0x7FFFFFFFL) || (min0 < (N_SI4)0x80000000L)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I4  ",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI4 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI4 pval;
			pval = (source[j]);
#if NEED_ALIGN & 4
			POKE_N_SI4(packed + i, pval);
#else
			packed[i] = HTON4(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 4;
}

static long
decode_i4_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 4
			N_SI4 pval;
			pval = PEEK_N_SI4((unsigned char *)(packed + i));
			result[j] = pval;
#else
			result[j] = (N_SI4)NTOH4(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_i4_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI4 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	/* missing = NONE */
	packed = (N_SI4 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		pval = (source[i]);
		packed[i] = HTON4(pval);
	}
	return 4 + buf->nelems * 4;
}

static long
decode_i4_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI4)NTOH4(packed[i]);
		result[i + 1] = (N_SI4)NTOH4(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI4)NTOH4(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI4)NTOH4(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i4_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I4  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I4  ", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i4_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI4 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if ((max0 > 0x7FFFFFFFL) || (min0 < (N_SI4)0x80000000L)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I4  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI4 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		pval = (source[i]);
		packed[i] = HTON4(pval);
	}
	return 4 + buf->nelems * 4;
}

static long
decode_i4_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI4)NTOH4(packed[i]);
		result[i + 1] = (N_SI4)NTOH4(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI4)NTOH4(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI4)NTOH4(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i4_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI4 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > 0x7FFFFFFFL) || (min0 < (N_SI4)0x80000000L)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I4  ",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI4 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		pval = (source[i]);
		packed[i] = HTON4(pval);
	}
	return 4 + buf->nelems * 4;
}

static long
decode_i4_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI4)NTOH4(packed[i]);
		result[i + 1] = (N_SI4)NTOH4(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI4)NTOH4(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI4)NTOH4(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_i4_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI4 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	POKE_N_SI4(drec, 0x80000000L);
	packed = (N_SI4 *)(drec + 4);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_SI4)0x80000000L;
		} else {
			pval = (N_SI4)(source[i]);
			if ( (N_SI4)0x80000000L == pval ) pval = (N_SI4)0x80000001L;
		}
		packed[i] = HTON4(pval);
	}
	return 4 + 4 + buf->nelems * 4;
}

static long
decode_i4_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_SI4 missval;
	/* code */
	missval = (N_SI4)NTOH4(*(N_SI4 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 4);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI4)NTOH4(packed[i]));
		result[i + 1] = ((N_SI4)NTOH4(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI4)NTOH4(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI4)NTOH4(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI4)NTOH4(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI4)NTOH4(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_i4_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "I4  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:I4  ", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 4 + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_i4_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI4 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if ((max0 > 0x7FFFFFFFL) || (min0 < (N_SI4)0x80000000L)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I4  ",
			(double)min0, (double)max0));
	}
	POKE_N_SI4(drec, 0x80000000L);
	packed = (N_SI4 *)(drec + 4);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_SI4)0x80000000L;
		} else {
			pval = (N_SI4)(source[i]);
			if ( (N_SI4)0x80000000L == pval ) pval = (N_SI4)0x80000001L;
		}
		packed[i] = HTON4(pval);
	}
	return 4 + 4 + buf->nelems * 4;
}

static long
decode_i4_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_SI4 missval;
	/* code */
	missval = (N_SI4)NTOH4(*(N_SI4 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 4);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[i]));
		result[i + 1] = ((N_SI4)NTOH4(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI4)NTOH4(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI4)NTOH4(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_i4_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI4 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > 0x7FFFFFFFL) || (min0 < (N_SI4)0x80000000L)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=I4  ",
			(double)min0, (double)max0));
	}
	POKE_N_SI4(drec, 0x80000000L);
	packed = (N_SI4 *)(drec + 4);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI4 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_SI4)0x80000000L;
		} else {
			pval = (N_SI4)(source[i]);
			if ( (N_SI4)0x80000000L == pval ) pval = (N_SI4)0x80000001L;
		}
		packed[i] = HTON4(pval);
	}
	return 4 + 4 + buf->nelems * 4;
}

static long
decode_i4_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_SI4 missval;
	/* code */
	missval = (N_SI4)NTOH4(*(N_SI4 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI4 *)(src + 4);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[i]));
		result[i + 1] = ((N_SI4)NTOH4(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI4)NTOH4(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI4)NTOH4(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI4)NTOH4(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_n1i2_mask_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI2 max0;
	N_SI2 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i2_mask(buf, &max0, &min0);
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (source[j]);
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_n1i2_mask_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (N_SI2 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = pval;
#else
			result[j] = (N_SI2)NTOH2(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si2);
		}
	}
	return nelems;
}

static long
encode_n1i2_mask_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI4 max0;
	N_SI4 min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_i4_mask(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = (source[j]);
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_n1i2_mask_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (N_SI4 *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = (N_SI2)pval;
#else
			result[j] = (N_SI2)NTOH2(packed[i]);
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_si4);
		}
	}
	return nelems;
}

static long
encode_n1i2_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "N1I2") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:N1I2", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_n1i2_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float max0;
	float min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r4_mask(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = ((source[j] * 10));
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_n1i2_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = pval * 0.1;
#else
			result[j] = (N_SI2)NTOH2(packed[i]) * 0.1;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_n1i2_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	double max0;
	double min0;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	maxmin_r8_mask(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (N_SI2 *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			N_SI2 pval;
			pval = ((source[j] * 10));
#if NEED_ALIGN & 2
			POKE_N_SI2(packed + i, pval);
#else
			packed[i] = HTON2(pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 2;
}

static long
decode_n1i2_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
#if NEED_ALIGN & 2
			N_SI2 pval;
			pval = PEEK_N_SI2((unsigned char *)(packed + i));
			result[j] = pval * 0.1;
#else
			result[j] = (N_SI2)NTOH2(packed[i]) * 0.1;
#endif
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_n1i2_none_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI2 max0;
	N_SI2 min0;
	/* code */
	maxmin_i2_none(buf, &max0, &min0);
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (source[i]);
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_n1i2_none_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]);
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_n1i2_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_none(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = (source[i]);
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_n1i2_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]);
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]);
	}
#endif
	return nelems;
}

static long
encode_n1i2_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "N1I2") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:N1I2", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_n1i2_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_none(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = ((source[i] * 10));
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_n1i2_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]) * 0.1;
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]) * 0.1;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]) * 0.1;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]) * 0.1;
	}
#endif
	return nelems;
}

static long
encode_n1i2_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_none(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	/* missing = NONE */
	packed = (N_SI2 *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		pval = ((source[i] * 10));
		packed[i] = HTON2(pval);
	}
	return 4 + buf->nelems * 2;
}

static long
decode_n1i2_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = (N_SI2)NTOH2(packed[i]) * 0.1;
		result[i + 1] = (N_SI2)NTOH2(packed[i + 1]) * 0.1;
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = (N_SI2)NTOH2(packed[nelems - 1]) * 0.1;
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = (N_SI2)NTOH2(packed[i]) * 0.1;
	}
#endif
	return nelems;
}

static long
encode_n1i2_udfv_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI2 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI2 max0;
	N_SI2 min0;
	/* code */
	maxmin_i2_udfv(buf, &max0, &min0);
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si2) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)(source[i]);
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_n1i2_udfv_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI2	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (N_SI2 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[i]));
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si2)
			: (N_SI2)((N_SI2)NTOH2(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_n1i2_udfv_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const N_SI4 *source = buf->ob_ptr;
	N_SI2 *packed;
	N_SI4 max0;
	N_SI4 min0;
	/* code */
	maxmin_i4_udfv(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_si4) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)(source[i]);
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_n1i2_udfv_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	N_SI4	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (N_SI4 *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[i]));
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[i + 1]));
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[nelems - 1]));
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_si4)
			: (N_SI4)((N_SI2)NTOH2(packed[i]));
	}
#endif
	return nelems;
}

static long
encode_n1i2_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "N1I2") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:N1I2", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 2 + 2 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_n1i2_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	N_SI2 *packed;
	float max0;
	float min0;
	/* code */
	maxmin_r4_udfv(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r4) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)((source[i] * 10));
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_n1i2_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (float *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i]) * 0.1);
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i + 1]) * 0.1);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[nelems - 1]) * 0.1);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)((N_SI2)NTOH2(packed[i]) * 0.1);
	}
#endif
	return nelems;
}

static long
encode_n1i2_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	N_SI2 *packed;
	double max0;
	double min0;
	/* code */
	maxmin_r8_udfv(buf, &max0, &min0);
	if ((max0 > (N_SI2)0x7FFF) || (min0 < (N_SI2)-0x8000)) {
		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
			"range of packing=N1I2",
			(double)min0, (double)max0));
	}
	POKE_N_SI2(drec, 0x8000);
	packed = (N_SI2 *)(drec + 2);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
		N_SI2 pval;
		if ( source[i] == GlobalConfig(pc_missing_r8) ) {
			pval = (N_SI2)0x8000;
		} else {
			pval = (N_SI2)((source[i] * 10));
			if ( (N_SI2)0x8000 == pval ) pval = (N_SI2)0x8001;
		}
		packed[i] = HTON2(pval);
	}
	return 4 + 2 + buf->nelems * 2;
}

static long
decode_n1i2_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_SI2	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_SI2 missval;
	/* code */
	missval = (N_SI2)NTOH2(*(N_SI2 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_SI2 *)(src + 2);
	result = (double *)(buf->ib_ptr);
	
#ifndef AVOID_PIPELINE_HACK
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < (nelems & ~1u); i += 2) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i]) * 0.1);
		result[i + 1] = ((N_SI2)NTOH2(packed[i + 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i + 1]) * 0.1);
	}
	if ((nelems & 1u) != 0) {
		result[nelems - 1] = ((N_SI2)NTOH2(packed[nelems - 1]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[nelems - 1]) * 0.1);
	}
#else
#ifdef USE_OMP
#pragma omp parallel for
#else
/*poption parallel */
#endif
	for (i = 0; i < nelems; i++) {
		result[i] = ((N_SI2)NTOH2(packed[i]) == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)((N_SI2)NTOH2(packed[i]) * 0.1);
	}
#endif
	return nelems;
}

static long
encode_r4_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "R4  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:R4  ", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_r4_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	float *packed;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (float *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
#if WORDS_BIGENDIAN
			float pval;
			pval = source[j];
			memcpy4((char *)(packed + i), (const char *)&pval);
#else
			float pval;
			pval = (source[j]);
			POKE_float(&packed[i], pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 4;
}

static long
decode_r4_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI4 *)(src + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			float pval;
			PEEK_float(&pval, (unsigned char *)(&packed[i]));
			result[j] = pval;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_r4_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	float *packed;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (float *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
#if WORDS_BIGENDIAN
			float pval;
			pval = source[j];
			memcpy4((char *)(packed + i), (const char *)&pval);
#else
			float pval;
			pval = (source[j]);
			POKE_float(&packed[i], pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 4;
}

static long
decode_r4_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI4 *)(src + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			float pval;
			PEEK_float(&pval, (unsigned char *)(&packed[i]));
			result[j] = pval;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_r4_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "R4  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:R4  ", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_r4_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	float *packed;
	/* code */
	/* missing = NONE */
	packed = (float *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (source[i]);
#else
		float pval;
		pval = (source[i]);
		POKE_float(&packed[i], pval);
#endif
	}
	return 4 + buf->nelems * 4;
}

static long
decode_r4_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI4 *)(src);
	result = (float *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		float pval;
		PEEK_float(&pval, (unsigned char *)(&packed[i]));
		result[i] = pval;
	}
	return nelems;
}

static long
encode_r4_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	float *packed;
	/* code */
	/* missing = NONE */
	packed = (float *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (source[i]);
#else
		float pval;
		pval = (source[i]);
		POKE_float(&packed[i], pval);
#endif
	}
	return 4 + buf->nelems * 4;
}

static long
decode_r4_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI4 *)(src);
	result = (double *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		float pval;
		PEEK_float(&pval, (unsigned char *)(&packed[i]));
		result[i] = pval;
	}
	return nelems;
}

static long
encode_r4_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "R4  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:R4  ", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 4 + 4 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_r4_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	float *packed;
	/* code */
	POKE_float(drec, FLT_MAX);
	packed = (float *)(drec + 4);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (
			(source[i] == GlobalConfig(pc_missing_r4))
			? FLT_MAX
			: (float)(source[i]));
#else
		float pval;
		pval = (
			(source[i] == GlobalConfig(pc_missing_r4))
			? FLT_MAX
			: (float)(source[i]));
		POKE_float(&packed[i], pval);
#endif
	}
	return 4 + 4 + buf->nelems * 4;
}

static long
decode_r4_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI4	*packed;
	float	*result;
	N_UI4		i, nelems;
	float missval;
	/* code */
	*(N_UI4 *)&missval = NTOH4(*(N_UI4 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI4 *)(src + 4);
	result = (float *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		float pval;
		PEEK_float(&pval, (unsigned char *)(&packed[i]));
		result[i] = (pval == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)(pval);
	}
	return nelems;
}

static long
encode_r4_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	float *packed;
	/* code */
	POKE_float(drec, FLT_MAX);
	packed = (float *)(drec + 4);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (
			(source[i] == GlobalConfig(pc_missing_r8))
			? FLT_MAX
			: (float)(source[i]));
#else
		float pval;
		pval = (
			(source[i] == GlobalConfig(pc_missing_r8))
			? FLT_MAX
			: (float)(source[i]));
		POKE_float(&packed[i], pval);
#endif
	}
	return 4 + 4 + buf->nelems * 4;
}

static long
decode_r4_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI4	*packed;
	double	*result;
	N_UI4		i, nelems;
	float missval;
	/* code */
	*(N_UI4 *)&missval = NTOH4(*(N_UI4 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI4 *)(src + 4);
	result = (double *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		float pval;
		PEEK_float(&pval, (unsigned char *)(&packed[i]));
		result[i] = (pval == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)(pval);
	}
	return nelems;
}

static long
encode_r8_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "R8  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:R8  ", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	expect_size = 16 + mask_nbytes + 8 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_r8_mask_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	double *packed;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (double *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
#if WORDS_BIGENDIAN
			double pval;
			pval = source[j];
			memcpy8((char *)(packed + i), (const char *)&pval);
#else
			double pval;
			pval = (source[j]);
			POKE_double(&packed[i], pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 8;
}

static long
decode_r8_mask_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI8	*packed;
	float	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI8 *)(src + mask_nbytes);
	result = (float *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			double pval;
			PEEK_double(&pval, (unsigned char *)(&packed[i]));
			result[j] = pval;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r4);
		}
	}
	return nelems;
}

static long
encode_r8_mask_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	double *packed;
	const unsigned char *mask_ptr = buf->ob_mask;
	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
	N_UI4 j;
	/* code */
	if (buf->ob_mask == NULL) {
		return NUSERR_WR_MaskMissing;
	}
	memcpy(drec, mask_ptr, mask_nbytes);
	i = 0;
	packed = (double *)(drec + mask_nbytes);
	/*poption noparallel */
	for (j = 0; j < buf->nelems; j++) {
		if (mask_ptr[j / 8] & (128 >> (j % 8))) {
#if WORDS_BIGENDIAN
			double pval;
			pval = source[j];
			memcpy8((char *)(packed + i), (const char *)&pval);
#else
			double pval;
			pval = (source[j]);
			POKE_double(&packed[i], pval);
#endif
			i++;
		}
	}
	return 4 + mask_nbytes + i * 8;
}

static long
decode_r8_mask_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI8	*packed;
	double	*result;
	N_UI4		i, nelems;
	N_UI4		j;
	const unsigned char *mask_ptr = src;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	/* code */
	i = 0;
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI8 *)(src + mask_nbytes);
	result = (double *)(buf->ib_ptr);
		/*poption noparallel */
	for (j = 0; j < nelems; j++) {
		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			double pval;
			PEEK_double(&pval, (unsigned char *)(&packed[i]));
			result[j] = pval;
			i++;
		} else {
			result[j] = GlobalConfig(pc_missing_r8);
		}
	}
	return nelems;
}

static long
encode_r8_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "R8  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:R8  ", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 8 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_r8_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	double *packed;
	/* code */
	/* missing = NONE */
	packed = (double *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (source[i]);
#else
		double pval;
		pval = (source[i]);
		POKE_double(&packed[i], pval);
#endif
	}
	return 4 + buf->nelems * 8;
}

static long
decode_r8_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI8	*packed;
	float	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI8 *)(src);
	result = (float *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		double pval;
		PEEK_double(&pval, (unsigned char *)(&packed[i]));
		result[i] = pval;
	}
	return nelems;
}

static long
encode_r8_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	double *packed;
	/* code */
	/* missing = NONE */
	packed = (double *)(drec);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (source[i]);
#else
		double pval;
		pval = (source[i]);
		POKE_double(&packed[i], pval);
#endif
	}
	return 4 + buf->nelems * 8;
}

static long
decode_r8_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI8	*packed;
	double	*result;
	N_UI4		i, nelems;
	/* code */
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI8 *)(src);
	result = (double *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		double pval;
		PEEK_double(&pval, (unsigned char *)(&packed[i]));
		result[i] = pval;
	}
	return nelems;
}

static long
encode_r8_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "R8  ") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:R8  ", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	expect_size = 16 + 8 + 8 * dat_size;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_r8_udfv_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const float *source = buf->ob_ptr;
	double *packed;
	/* code */
	POKE_double(drec, DBL_MAX);
	packed = (double *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (
			(source[i] == GlobalConfig(pc_missing_r4))
			? DBL_MAX
			: (double)(source[i]));
#else
		double pval;
		pval = (
			(source[i] == GlobalConfig(pc_missing_r4))
			? DBL_MAX
			: (double)(source[i]));
		POKE_double(&packed[i], pval);
#endif
	}
	return 4 + 8 + buf->nelems * 8;
}

static long
decode_r8_udfv_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI8	*packed;
	float	*result;
	N_UI4		i, nelems;
	double missval;
	/* code */
	*(N_UI8 *)&missval = NTOH8(*(N_UI8 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI8 *)(src + 8);
	result = (float *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		double pval;
		PEEK_double(&pval, (unsigned char *)(&packed[i]));
		result[i] = (pval == missval)
			? GlobalConfig(pc_missing_r4)
			: (float)(pval);
	}
	return nelems;
}

static long
encode_r8_udfv_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	i;
	const double *source = buf->ob_ptr;
	double *packed;
	/* code */
	POKE_double(drec, DBL_MAX);
	packed = (double *)(drec + 8);
#ifdef USE_OMP
#pragma omp parallel for
#else
	/*poption parallel */
#endif
	for (i = 0; i < buf->nelems; i++) {
#if WORDS_BIGENDIAN
		packed[i] = (
			(source[i] == GlobalConfig(pc_missing_r8))
			? DBL_MAX
			: (double)(source[i]));
#else
		double pval;
		pval = (
			(source[i] == GlobalConfig(pc_missing_r8))
			? DBL_MAX
			: (double)(source[i]));
		POKE_double(&packed[i], pval);
#endif
	}
	return 4 + 8 + buf->nelems * 8;
}

static long
decode_r8_udfv_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	const N_UI8	*packed;
	double	*result;
	N_UI4		i, nelems;
	double missval;
	/* code */
	*(N_UI8 *)&missval = NTOH8(*(N_UI8 *)(src));
	nelems = nxd * nyd;
	if (buf->nelems < nelems) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nelems));
	}
	packed = (N_UI8 *)(src + 8);
	result = (double *)(buf->ib_ptr);
	for (i = 0; i < nelems; i++) {
		double pval;
		PEEK_double(&pval, (unsigned char *)(&packed[i]));
		result[i] = (pval == missval)
			? GlobalConfig(pc_missing_r8)
			: (double)(pval);
	}
	return nelems;
}

static long
encode_rlen_mask_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "RLEN") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:RLEN", pack));
	if ( strcmp(miss, "MASK") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:MASK", miss));
	int i;
	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
	N_UI4 rlen_nbit = PEEK_N_UI4(src + 16 + mask_nbytes);
	N_UI4 rlen_num = PEEK_N_UI4(src + 16 + mask_nbytes + 8);
	expect_size = 16 + mask_nbytes + 12 + (rlen_nbit * rlen_num - 1) / 8 + 1;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_rlen_none_i1(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_SI4	maxv;
	N_SI4 cmpr_nbytes;
	cmpr_nbytes = nus_compress_rlen_i1(buf->ob_ptr, buf->nelems, &maxv, 
		drec + 12, buf->nelems);
	if (cmpr_nbytes < 0) {
		return cmpr_nbytes;
	}
	POKE_N_UI4(drec, 8u);
	POKE_N_UI4(drec + 4, maxv);
	POKE_N_UI4(drec + 8, cmpr_nbytes);
	return 16 + cmpr_nbytes;
}

static long
decode_rlen_none_i1(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	maxv, cmpr_nbytes;
	N_SI4   nelems;
	if (buf->nelems < nxd * nyd) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nxd * nyd));
	}
	if (PEEK_N_UI4(src) != 8u) {
		return nus_err((NUSERR_RD_NoCodec,
			"cannot uncompress %Pu-bit RLEN data",
			PEEK_N_UI4(src)));
	}
	maxv = PEEK_N_UI4(src + 4);
	cmpr_nbytes = PEEK_N_UI4(src + 8);
	nelems = nus_decompress_rlen_i1(src + 12, maxv, cmpr_nbytes,
		buf->ib_ptr, buf->nelems);
	return nelems;
}

static long
encode_rlen_none_i2(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_SI4	maxv;
	N_SI4 cmpr_nbytes;
	cmpr_nbytes = nus_compress_rlen_i2(buf->ob_ptr, buf->nelems, &maxv, 
		drec + 12, buf->nelems);
	if (cmpr_nbytes < 0) {
		return cmpr_nbytes;
	}
	POKE_N_UI4(drec, 8u);
	POKE_N_UI4(drec + 4, maxv);
	POKE_N_UI4(drec + 8, cmpr_nbytes);
	return 16 + cmpr_nbytes;
}

static long
decode_rlen_none_i2(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	maxv, cmpr_nbytes;
	N_SI4   nelems;
	if (buf->nelems < nxd * nyd) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nxd * nyd));
	}
	if (PEEK_N_UI4(src) != 8u) {
		return nus_err((NUSERR_RD_NoCodec,
			"cannot uncompress %Pu-bit RLEN data",
			PEEK_N_UI4(src)));
	}
	maxv = PEEK_N_UI4(src + 4);
	cmpr_nbytes = PEEK_N_UI4(src + 8);
	nelems = nus_decompress_rlen_i2(src + 12, maxv, cmpr_nbytes,
		buf->ib_ptr, buf->nelems);
	return nelems;
}

static long
encode_rlen_none_i4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_SI4	maxv;
	N_SI4 cmpr_nbytes;
	cmpr_nbytes = nus_compress_rlen_i4(buf->ob_ptr, buf->nelems, &maxv, 
		drec + 12, buf->nelems);
	if (cmpr_nbytes < 0) {
		return cmpr_nbytes;
	}
	POKE_N_UI4(drec, 8u);
	POKE_N_UI4(drec + 4, maxv);
	POKE_N_UI4(drec + 8, cmpr_nbytes);
	return 16 + cmpr_nbytes;
}

static long
decode_rlen_none_i4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	maxv, cmpr_nbytes;
	N_SI4   nelems;
	if (buf->nelems < nxd * nyd) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nxd * nyd));
	}
	if (PEEK_N_UI4(src) != 8u) {
		return nus_err((NUSERR_RD_NoCodec,
			"cannot uncompress %Pu-bit RLEN data",
			PEEK_N_UI4(src)));
	}
	maxv = PEEK_N_UI4(src + 4);
	cmpr_nbytes = PEEK_N_UI4(src + 8);
	nelems = nus_decompress_rlen_i4(src + 12, maxv, cmpr_nbytes,
		buf->ib_ptr, buf->nelems);
	return nelems;
}

static long
encode_rlen_none_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "RLEN") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:RLEN", pack));
	if ( strcmp(miss, "NONE") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:NONE", miss));
	dat_size = nxd * nyd;
	N_UI4 rlen_nbit = PEEK_N_UI4(src + 16);
	N_UI4 rlen_num = PEEK_N_UI4(src + 16 + 8);
	expect_size = 16 + 12 + (rlen_nbit * rlen_num - 1) / 8 + 1;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static long
encode_rlen_none_r4(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_SI4	maxv;
	N_SI4 cmpr_nbytes;
	cmpr_nbytes = nus_compress_rlen_r4(buf->ob_ptr, buf->nelems, &maxv, 
		drec + 12, buf->nelems);
	if (cmpr_nbytes < 0) {
		return cmpr_nbytes;
	}
	POKE_N_UI4(drec, 8u);
	POKE_N_UI4(drec + 4, maxv);
	POKE_N_UI4(drec + 8, cmpr_nbytes);
	return 16 + cmpr_nbytes;
}

static long
decode_rlen_none_r4(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	maxv, cmpr_nbytes;
	N_SI4   nelems;
	if (buf->nelems < nxd * nyd) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nxd * nyd));
	}
	if (PEEK_N_UI4(src) != 8u) {
		return nus_err((NUSERR_RD_NoCodec,
			"cannot uncompress %Pu-bit RLEN data",
			PEEK_N_UI4(src)));
	}
	maxv = PEEK_N_UI4(src + 4);
	cmpr_nbytes = PEEK_N_UI4(src + 8);
	nelems = nus_decompress_rlen_r4(src + 12, maxv, cmpr_nbytes,
		buf->ib_ptr, buf->nelems);
	return nelems;
}

static long
encode_rlen_none_r8(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_SI4	maxv;
	N_SI4 cmpr_nbytes;
	cmpr_nbytes = nus_compress_rlen_r8(buf->ob_ptr, buf->nelems, &maxv, 
		drec + 12, buf->nelems);
	if (cmpr_nbytes < 0) {
		return cmpr_nbytes;
	}
	POKE_N_UI4(drec, 8u);
	POKE_N_UI4(drec + 4, maxv);
	POKE_N_UI4(drec + 8, cmpr_nbytes);
	return 16 + cmpr_nbytes;
}

static long
decode_rlen_none_r8(struct ibuffer_t *buf,
	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4	maxv, cmpr_nbytes;
	N_SI4   nelems;
	if (buf->nelems < nxd * nyd) {
		return nus_err((NUSERR_RD_SmallBuf,
			"buffer %Pu elements < record %Pu elements",
			buf->nelems, nxd * nyd));
	}
	if (PEEK_N_UI4(src) != 8u) {
		return nus_err((NUSERR_RD_NoCodec,
			"cannot uncompress %Pu-bit RLEN data",
			PEEK_N_UI4(src)));
	}
	maxv = PEEK_N_UI4(src + 4);
	cmpr_nbytes = PEEK_N_UI4(src + 8);
	nelems = nus_decompress_rlen_r8(src + 12, maxv, cmpr_nbytes,
		buf->ib_ptr, buf->nelems);
	return nelems;
}

static long
encode_rlen_udfv_nd(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
{
	N_UI4 dat_x, dat_y, dat_size, expect_size;
	char pack[5], miss[5];
	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
	dat_x = PEEK_N_UI4(src);
	dat_y = PEEK_N_UI4(src + 4);
	memcpy(pack, src + 8, 4);
	memcpy(miss, src + 12, 4);
	pack[4] = miss[4] = 0;
	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
	if ( strcmp(pack, "RLEN") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:RLEN", pack));
	if ( strcmp(miss, "UDFV") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:UDFV", miss));
	dat_size = nxd * nyd;
	N_UI4 rlen_nbit = PEEK_N_UI4(src + 16 + 1);
	N_UI4 rlen_num = PEEK_N_UI4(src + 16 + 1 + 8);
	expect_size = 16 + 1 + 12 + (rlen_nbit * rlen_num - 1) / 8 + 1;
	if (expect_size > buf->nelems) {
		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
	} else if (expect_size < buf->nelems) {
		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		buf->nelems = expect_size;
	}
	memcpy(drec, src + 16, expect_size - 16);
	return 4 + expect_size - 16;
}

static struct ndf_codec_t codec_table[] = {
 { SYM4_1PAC, SYM4_MASK, SYM4_I2, 77, 9, encode_1pac_mask_i2, decode_1pac_mask_i2 },
 { SYM4_1PAC, SYM4_MASK, SYM4_I4, 77, 9, encode_1pac_mask_i4, decode_1pac_mask_i4 },
 { SYM4_1PAC, SYM4_MASK, SYM4_ND, 77, 9, encode_1pac_mask_nd, NULL },
 { SYM4_1PAC, SYM4_MASK, SYM4_R4, 77, 9, encode_1pac_mask_r4, decode_1pac_mask_r4 },
 { SYM4_1PAC, SYM4_MASK, SYM4_R8, 77, 9, encode_1pac_mask_r8, decode_1pac_mask_r8 },
 { SYM4_1PAC, SYM4_NONE, SYM4_I2, 76, 8, encode_1pac_none_i2, decode_1pac_none_i2 },
 { SYM4_1PAC, SYM4_NONE, SYM4_I4, 76, 8, encode_1pac_none_i4, decode_1pac_none_i4 },
 { SYM4_1PAC, SYM4_NONE, SYM4_ND, 76, 8, encode_1pac_none_nd, NULL },
 { SYM4_1PAC, SYM4_NONE, SYM4_R4, 76, 8, encode_1pac_none_r4, decode_1pac_none_r4 },
 { SYM4_1PAC, SYM4_NONE, SYM4_R8, 76, 8, encode_1pac_none_r8, decode_1pac_none_r8 },
 { SYM4_1PAC, SYM4_UDFV, SYM4_I2, 77, 8, encode_1pac_udfv_i2, decode_1pac_udfv_i2 },
 { SYM4_1PAC, SYM4_UDFV, SYM4_I4, 77, 8, encode_1pac_udfv_i4, decode_1pac_udfv_i4 },
 { SYM4_1PAC, SYM4_UDFV, SYM4_ND, 77, 8, encode_1pac_udfv_nd, NULL },
 { SYM4_1PAC, SYM4_UDFV, SYM4_R4, 77, 8, encode_1pac_udfv_r4, decode_1pac_udfv_r4 },
 { SYM4_1PAC, SYM4_UDFV, SYM4_R8, 77, 8, encode_1pac_udfv_r8, decode_1pac_udfv_r8 },
 { SYM4_2PAC, SYM4_MASK, SYM4_I4, 77, 17, encode_2pac_mask_i4, decode_2pac_mask_i4 },
 { SYM4_2PAC, SYM4_MASK, SYM4_ND, 77, 17, encode_2pac_mask_nd, NULL },
 { SYM4_2PAC, SYM4_MASK, SYM4_R4, 77, 17, encode_2pac_mask_r4, decode_2pac_mask_r4 },
 { SYM4_2PAC, SYM4_MASK, SYM4_R8, 77, 17, encode_2pac_mask_r8, decode_2pac_mask_r8 },
 { SYM4_2PAC, SYM4_NONE, SYM4_I4, 76, 16, encode_2pac_none_i4, decode_2pac_none_i4 },
 { SYM4_2PAC, SYM4_NONE, SYM4_NC, 76, 16, encode_2upc_none_nc, decode_2upc_none_nc },
 { SYM4_2PAC, SYM4_NONE, SYM4_ND, 76, 16, encode_2pac_none_nd, NULL },
 { SYM4_2PAC, SYM4_NONE, SYM4_R4, 76, 16, encode_2pac_none_r4, decode_2pac_none_r4 },
 { SYM4_2PAC, SYM4_NONE, SYM4_R8, 76, 16, encode_2pac_none_r8, decode_2pac_none_r8 },
 { SYM4_2PAC, SYM4_UDFV, SYM4_I4, 78, 16, encode_2pac_udfv_i4, decode_2pac_udfv_i4 },
 { SYM4_2PAC, SYM4_UDFV, SYM4_NC, 78, 16, encode_2upc_udfv_nc, decode_2upc_udfv_nc },
 { SYM4_2PAC, SYM4_UDFV, SYM4_ND, 78, 16, encode_2pac_udfv_nd, NULL },
 { SYM4_2PAC, SYM4_UDFV, SYM4_R4, 78, 16, encode_2pac_udfv_r4, decode_2pac_udfv_r4 },
 { SYM4_2PAC, SYM4_UDFV, SYM4_R8, 78, 16, encode_2pac_udfv_r8, decode_2pac_udfv_r8 },
 { SYM4_2UPC, SYM4_MASK, SYM4_I4, 77, 17, encode_2upc_mask_i4, decode_2upc_mask_i4 },
 { SYM4_2UPC, SYM4_MASK, SYM4_ND, 77, 17, encode_2upc_mask_nd, NULL },
 { SYM4_2UPC, SYM4_MASK, SYM4_R4, 77, 17, encode_2upc_mask_r4, decode_2upc_mask_r4 },
 { SYM4_2UPC, SYM4_MASK, SYM4_R8, 77, 17, encode_2upc_mask_r8, decode_2upc_mask_r8 },
 { SYM4_2UPC, SYM4_NONE, SYM4_I4, 76, 16, encode_2upc_none_i4, decode_2upc_none_i4 },
 { SYM4_2UPC, SYM4_NONE, SYM4_NC, 76, 16, encode_2upc_none_nc, decode_2upc_none_nc },
 { SYM4_2UPC, SYM4_NONE, SYM4_ND, 76, 16, encode_2upc_none_nd, NULL },
 { SYM4_2UPC, SYM4_NONE, SYM4_R4, 76, 16, encode_2upc_none_r4, decode_2upc_none_r4 },
 { SYM4_2UPC, SYM4_NONE, SYM4_R8, 76, 16, encode_2upc_none_r8, decode_2upc_none_r8 },
 { SYM4_2UPC, SYM4_UDFV, SYM4_I4, 78, 16, encode_2upc_udfv_i4, decode_2upc_udfv_i4 },
 { SYM4_2UPC, SYM4_UDFV, SYM4_NC, 78, 16, encode_2upc_udfv_nc, decode_2upc_udfv_nc },
 { SYM4_2UPC, SYM4_UDFV, SYM4_ND, 78, 16, encode_2upc_udfv_nd, NULL },
 { SYM4_2UPC, SYM4_UDFV, SYM4_R4, 78, 16, encode_2upc_udfv_r4, decode_2upc_udfv_r4 },
 { SYM4_2UPC, SYM4_UDFV, SYM4_R8, 78, 16, encode_2upc_udfv_r8, decode_2upc_udfv_r8 },
 { SYM4_2UPJ, SYM4_MASK, SYM4_I4, 77, 33, encode_2upj_mask_i4, decode_2upj_mask_i4 },
 { SYM4_2UPJ, SYM4_MASK, SYM4_ND, 77, 33, encode_2upj_mask_nd, NULL },
 { SYM4_2UPJ, SYM4_MASK, SYM4_R4, 77, 33, encode_2upj_mask_r4, decode_2upj_mask_r4 },
 { SYM4_2UPJ, SYM4_MASK, SYM4_R8, 77, 33, encode_2upj_mask_r8, decode_2upj_mask_r8 },
 { SYM4_2UPJ, SYM4_NONE, SYM4_I4, 76, 32, encode_2upj_none_i4, decode_2upj_none_i4 },
 { SYM4_2UPJ, SYM4_NONE, SYM4_NC, 76, 32, encode_2upj_none_nc, decode_2upj_none_nc },
 { SYM4_2UPJ, SYM4_NONE, SYM4_ND, 76, 32, encode_2upj_none_nd, NULL },
 { SYM4_2UPJ, SYM4_NONE, SYM4_R4, 76, 32, encode_2upj_none_r4, decode_2upj_none_r4 },
 { SYM4_2UPJ, SYM4_NONE, SYM4_R8, 76, 32, encode_2upj_none_r8, decode_2upj_none_r8 },
 { SYM4_2UPJ, SYM4_UDFV, SYM4_I4, 78, 32, encode_2upj_udfv_i4, decode_2upj_udfv_i4 },
 { SYM4_2UPJ, SYM4_UDFV, SYM4_NC, 78, 32, encode_2upj_udfv_nc, decode_2upj_udfv_nc },
 { SYM4_2UPJ, SYM4_UDFV, SYM4_ND, 78, 32, encode_2upj_udfv_nd, NULL },
 { SYM4_2UPJ, SYM4_UDFV, SYM4_R4, 78, 32, encode_2upj_udfv_r4, decode_2upj_udfv_r4 },
 { SYM4_2UPJ, SYM4_UDFV, SYM4_R8, 78, 32, encode_2upj_udfv_r8, decode_2upj_udfv_r8 },
 { SYM4_2UPP, SYM4_MASK, SYM4_I4, 77, 33, encode_2upp_mask_i4, decode_2upp_mask_i4 },
 { SYM4_2UPP, SYM4_MASK, SYM4_ND, 77, 33, encode_2upp_mask_nd, NULL },
 { SYM4_2UPP, SYM4_MASK, SYM4_R4, 77, 33, encode_2upp_mask_r4, decode_2upp_mask_r4 },
 { SYM4_2UPP, SYM4_MASK, SYM4_R8, 77, 33, encode_2upp_mask_r8, decode_2upp_mask_r8 },
 { SYM4_2UPP, SYM4_NONE, SYM4_I4, 76, 32, encode_2upp_none_i4, decode_2upp_none_i4 },
 { SYM4_2UPP, SYM4_NONE, SYM4_NC, 76, 32, encode_2upp_none_nc, decode_2upp_none_nc },
 { SYM4_2UPP, SYM4_NONE, SYM4_ND, 76, 32, encode_2upp_none_nd, NULL },
 { SYM4_2UPP, SYM4_NONE, SYM4_R4, 76, 32, encode_2upp_none_r4, decode_2upp_none_r4 },
 { SYM4_2UPP, SYM4_NONE, SYM4_R8, 76, 32, encode_2upp_none_r8, decode_2upp_none_r8 },
 { SYM4_2UPP, SYM4_UDFV, SYM4_I4, 78, 32, encode_2upp_udfv_i4, decode_2upp_udfv_i4 },
 { SYM4_2UPP, SYM4_UDFV, SYM4_NC, 78, 32, encode_2upp_udfv_nc, decode_2upp_udfv_nc },
 { SYM4_2UPP, SYM4_UDFV, SYM4_ND, 78, 32, encode_2upp_udfv_nd, NULL },
 { SYM4_2UPP, SYM4_UDFV, SYM4_R4, 78, 32, encode_2upp_udfv_r4, decode_2upp_udfv_r4 },
 { SYM4_2UPP, SYM4_UDFV, SYM4_R8, 78, 32, encode_2upp_udfv_r8, decode_2upp_udfv_r8 },
 { SYM4_4PAC, SYM4_MASK, SYM4_ND, 85, 33, encode_4pac_mask_nd, NULL },
 { SYM4_4PAC, SYM4_MASK, SYM4_R4, 85, 33, encode_4pac_mask_r4, decode_4pac_mask_r4 },
 { SYM4_4PAC, SYM4_MASK, SYM4_R8, 85, 33, encode_4pac_mask_r8, decode_4pac_mask_r8 },
 { SYM4_4PAC, SYM4_NONE, SYM4_ND, 84, 32, encode_4pac_none_nd, NULL },
 { SYM4_4PAC, SYM4_NONE, SYM4_R4, 84, 32, encode_4pac_none_r4, decode_4pac_none_r4 },
 { SYM4_4PAC, SYM4_NONE, SYM4_R8, 84, 32, encode_4pac_none_r8, decode_4pac_none_r8 },
 { SYM4_4PAC, SYM4_UDFV, SYM4_ND, 88, 32, encode_4pac_udfv_nd, NULL },
 { SYM4_4PAC, SYM4_UDFV, SYM4_R4, 88, 32, encode_4pac_udfv_r4, decode_4pac_udfv_r4 },
 { SYM4_4PAC, SYM4_UDFV, SYM4_R8, 88, 32, encode_4pac_udfv_r8, decode_4pac_udfv_r8 },
 { SYM4_I1  , SYM4_MASK, SYM4_I1, 69, 9, encode_i1_mask_i1, decode_i1_mask_i1 },
 { SYM4_I1  , SYM4_MASK, SYM4_I2, 69, 9, encode_i1_mask_i2, decode_i1_mask_i2 },
 { SYM4_I1  , SYM4_MASK, SYM4_I4, 69, 9, encode_i1_mask_i4, decode_i1_mask_i4 },
 { SYM4_I1  , SYM4_MASK, SYM4_ND, 69, 9, encode_i1_mask_nd, NULL },
 { SYM4_I1  , SYM4_MASK, SYM4_R4, 69, 9, encode_i1_mask_r4, decode_i1_mask_r4 },
 { SYM4_I1  , SYM4_MASK, SYM4_R8, 69, 9, encode_i1_mask_r8, decode_i1_mask_r8 },
 { SYM4_I1  , SYM4_NONE, SYM4_I1, 68, 8, encode_i1_none_i1, decode_i1_none_i1 },
 { SYM4_I1  , SYM4_NONE, SYM4_I2, 68, 8, encode_i1_none_i2, decode_i1_none_i2 },
 { SYM4_I1  , SYM4_NONE, SYM4_I4, 68, 8, encode_i1_none_i4, decode_i1_none_i4 },
 { SYM4_I1  , SYM4_NONE, SYM4_ND, 68, 8, encode_i1_none_nd, NULL },
 { SYM4_I1  , SYM4_NONE, SYM4_R4, 68, 8, encode_i1_none_r4, decode_i1_none_r4 },
 { SYM4_I1  , SYM4_NONE, SYM4_R8, 68, 8, encode_i1_none_r8, decode_i1_none_r8 },
 { SYM4_I1  , SYM4_UDFV, SYM4_I1, 69, 8, encode_i1_udfv_i1, decode_i1_udfv_i1 },
 { SYM4_I1  , SYM4_UDFV, SYM4_I2, 69, 8, encode_i1_udfv_i2, decode_i1_udfv_i2 },
 { SYM4_I1  , SYM4_UDFV, SYM4_I4, 69, 8, encode_i1_udfv_i4, decode_i1_udfv_i4 },
 { SYM4_I1  , SYM4_UDFV, SYM4_ND, 69, 8, encode_i1_udfv_nd, NULL },
 { SYM4_I1  , SYM4_UDFV, SYM4_R4, 69, 8, encode_i1_udfv_r4, decode_i1_udfv_r4 },
 { SYM4_I1  , SYM4_UDFV, SYM4_R8, 69, 8, encode_i1_udfv_r8, decode_i1_udfv_r8 },
 { SYM4_I2  , SYM4_MASK, SYM4_I2, 69, 17, encode_i2_mask_i2, decode_i2_mask_i2 },
 { SYM4_I2  , SYM4_MASK, SYM4_I4, 69, 17, encode_i2_mask_i4, decode_i2_mask_i4 },
 { SYM4_I2  , SYM4_MASK, SYM4_ND, 69, 17, encode_i2_mask_nd, NULL },
 { SYM4_I2  , SYM4_MASK, SYM4_R4, 69, 17, encode_i2_mask_r4, decode_i2_mask_r4 },
 { SYM4_I2  , SYM4_MASK, SYM4_R8, 69, 17, encode_i2_mask_r8, decode_i2_mask_r8 },
 { SYM4_I2  , SYM4_NONE, SYM4_I2, 68, 16, encode_i2_none_i2, decode_i2_none_i2 },
 { SYM4_I2  , SYM4_NONE, SYM4_I4, 68, 16, encode_i2_none_i4, decode_i2_none_i4 },
 { SYM4_I2  , SYM4_NONE, SYM4_ND, 68, 16, encode_i2_none_nd, NULL },
 { SYM4_I2  , SYM4_NONE, SYM4_R4, 68, 16, encode_i2_none_r4, decode_i2_none_r4 },
 { SYM4_I2  , SYM4_NONE, SYM4_R8, 68, 16, encode_i2_none_r8, decode_i2_none_r8 },
 { SYM4_I2  , SYM4_UDFV, SYM4_I2, 70, 16, encode_i2_udfv_i2, decode_i2_udfv_i2 },
 { SYM4_I2  , SYM4_UDFV, SYM4_I4, 70, 16, encode_i2_udfv_i4, decode_i2_udfv_i4 },
 { SYM4_I2  , SYM4_UDFV, SYM4_ND, 70, 16, encode_i2_udfv_nd, NULL },
 { SYM4_I2  , SYM4_UDFV, SYM4_R4, 70, 16, encode_i2_udfv_r4, decode_i2_udfv_r4 },
 { SYM4_I2  , SYM4_UDFV, SYM4_R8, 70, 16, encode_i2_udfv_r8, decode_i2_udfv_r8 },
 { SYM4_I4  , SYM4_MASK, SYM4_I4, 69, 33, encode_i4_mask_i4, decode_i4_mask_i4 },
 { SYM4_I4  , SYM4_MASK, SYM4_ND, 69, 33, encode_i4_mask_nd, NULL },
 { SYM4_I4  , SYM4_MASK, SYM4_R4, 69, 33, encode_i4_mask_r4, decode_i4_mask_r4 },
 { SYM4_I4  , SYM4_MASK, SYM4_R8, 69, 33, encode_i4_mask_r8, decode_i4_mask_r8 },
 { SYM4_I4  , SYM4_NONE, SYM4_I4, 68, 32, encode_i4_none_i4, decode_i4_none_i4 },
 { SYM4_I4  , SYM4_NONE, SYM4_ND, 68, 32, encode_i4_none_nd, NULL },
 { SYM4_I4  , SYM4_NONE, SYM4_R4, 68, 32, encode_i4_none_r4, decode_i4_none_r4 },
 { SYM4_I4  , SYM4_NONE, SYM4_R8, 68, 32, encode_i4_none_r8, decode_i4_none_r8 },
 { SYM4_I4  , SYM4_UDFV, SYM4_I4, 72, 32, encode_i4_udfv_i4, decode_i4_udfv_i4 },
 { SYM4_I4  , SYM4_UDFV, SYM4_ND, 72, 32, encode_i4_udfv_nd, NULL },
 { SYM4_I4  , SYM4_UDFV, SYM4_R4, 72, 32, encode_i4_udfv_r4, decode_i4_udfv_r4 },
 { SYM4_I4  , SYM4_UDFV, SYM4_R8, 72, 32, encode_i4_udfv_r8, decode_i4_udfv_r8 },
 { SYM4_N1I2, SYM4_MASK, SYM4_I2, 69, 17, encode_n1i2_mask_i2, decode_n1i2_mask_i2 },
 { SYM4_N1I2, SYM4_MASK, SYM4_I4, 69, 17, encode_n1i2_mask_i4, decode_n1i2_mask_i4 },
 { SYM4_N1I2, SYM4_MASK, SYM4_ND, 69, 17, encode_n1i2_mask_nd, NULL },
 { SYM4_N1I2, SYM4_MASK, SYM4_R4, 69, 17, encode_n1i2_mask_r4, decode_n1i2_mask_r4 },
 { SYM4_N1I2, SYM4_MASK, SYM4_R8, 69, 17, encode_n1i2_mask_r8, decode_n1i2_mask_r8 },
 { SYM4_N1I2, SYM4_NONE, SYM4_I2, 68, 16, encode_n1i2_none_i2, decode_n1i2_none_i2 },
 { SYM4_N1I2, SYM4_NONE, SYM4_I4, 68, 16, encode_n1i2_none_i4, decode_n1i2_none_i4 },
 { SYM4_N1I2, SYM4_NONE, SYM4_ND, 68, 16, encode_n1i2_none_nd, NULL },
 { SYM4_N1I2, SYM4_NONE, SYM4_R4, 68, 16, encode_n1i2_none_r4, decode_n1i2_none_r4 },
 { SYM4_N1I2, SYM4_NONE, SYM4_R8, 68, 16, encode_n1i2_none_r8, decode_n1i2_none_r8 },
 { SYM4_N1I2, SYM4_UDFV, SYM4_I2, 70, 16, encode_n1i2_udfv_i2, decode_n1i2_udfv_i2 },
 { SYM4_N1I2, SYM4_UDFV, SYM4_I4, 70, 16, encode_n1i2_udfv_i4, decode_n1i2_udfv_i4 },
 { SYM4_N1I2, SYM4_UDFV, SYM4_ND, 70, 16, encode_n1i2_udfv_nd, NULL },
 { SYM4_N1I2, SYM4_UDFV, SYM4_R4, 70, 16, encode_n1i2_udfv_r4, decode_n1i2_udfv_r4 },
 { SYM4_N1I2, SYM4_UDFV, SYM4_R8, 70, 16, encode_n1i2_udfv_r8, decode_n1i2_udfv_r8 },
 { SYM4_R4  , SYM4_MASK, SYM4_ND, 69, 33, encode_r4_mask_nd, NULL },
 { SYM4_R4  , SYM4_MASK, SYM4_R4, 69, 33, encode_r4_mask_r4, decode_r4_mask_r4 },
 { SYM4_R4  , SYM4_MASK, SYM4_R8, 69, 33, encode_r4_mask_r8, decode_r4_mask_r8 },
 { SYM4_R4  , SYM4_NONE, SYM4_ND, 68, 32, encode_r4_none_nd, NULL },
 { SYM4_R4  , SYM4_NONE, SYM4_R4, 68, 32, encode_r4_none_r4, decode_r4_none_r4 },
 { SYM4_R4  , SYM4_NONE, SYM4_R8, 68, 32, encode_r4_none_r8, decode_r4_none_r8 },
 { SYM4_R4  , SYM4_UDFV, SYM4_ND, 72, 32, encode_r4_udfv_nd, NULL },
 { SYM4_R4  , SYM4_UDFV, SYM4_R4, 72, 32, encode_r4_udfv_r4, decode_r4_udfv_r4 },
 { SYM4_R4  , SYM4_UDFV, SYM4_R8, 72, 32, encode_r4_udfv_r8, decode_r4_udfv_r8 },
 { SYM4_R8  , SYM4_MASK, SYM4_ND, 69, 65, encode_r8_mask_nd, NULL },
 { SYM4_R8  , SYM4_MASK, SYM4_R4, 69, 65, encode_r8_mask_r4, decode_r8_mask_r4 },
 { SYM4_R8  , SYM4_MASK, SYM4_R8, 69, 65, encode_r8_mask_r8, decode_r8_mask_r8 },
 { SYM4_R8  , SYM4_NONE, SYM4_ND, 68, 64, encode_r8_none_nd, NULL },
 { SYM4_R8  , SYM4_NONE, SYM4_R4, 68, 64, encode_r8_none_r4, decode_r8_none_r4 },
 { SYM4_R8  , SYM4_NONE, SYM4_R8, 68, 64, encode_r8_none_r8, decode_r8_none_r8 },
 { SYM4_R8  , SYM4_UDFV, SYM4_ND, 76, 64, encode_r8_udfv_nd, NULL },
 { SYM4_R8  , SYM4_UDFV, SYM4_R4, 76, 64, encode_r8_udfv_r4, decode_r8_udfv_r4 },
 { SYM4_R8  , SYM4_UDFV, SYM4_R8, 76, 64, encode_r8_udfv_r8, decode_r8_udfv_r8 },
 { SYM4_RLEN, SYM4_MASK, SYM4_ND, 81, 9, encode_rlen_mask_nd, NULL },
 { SYM4_RLEN, SYM4_NONE, SYM4_I1, 80, 8, encode_rlen_none_i1, decode_rlen_none_i1 },
 { SYM4_RLEN, SYM4_NONE, SYM4_I2, 80, 8, encode_rlen_none_i2, decode_rlen_none_i2 },
 { SYM4_RLEN, SYM4_NONE, SYM4_I4, 80, 8, encode_rlen_none_i4, decode_rlen_none_i4 },
 { SYM4_RLEN, SYM4_NONE, SYM4_ND, 80, 8, encode_rlen_none_nd, NULL },
 { SYM4_RLEN, SYM4_NONE, SYM4_R4, 80, 8, encode_rlen_none_r4, decode_rlen_none_r4 },
 { SYM4_RLEN, SYM4_NONE, SYM4_R8, 80, 8, encode_rlen_none_r8, decode_rlen_none_r8 },
 { SYM4_RLEN, SYM4_UDFV, SYM4_ND, 81, 8, encode_rlen_udfv_nd, NULL },
 { 0, 0, 0, -1, -1, NULL, NULL }
};

struct ndf_codec_t *ndf_get_codec(sym4_t packing, sym4_t missing, sym4_t bffm)
{
	struct ndf_codec_t *codec;
	for (codec = codec_table; codec->packing; codec++) {
		if (packing == codec->packing
			&& missing == codec->missing
			&& bffm == codec->bffm)
			return codec;
	}
	return NULL;
}
