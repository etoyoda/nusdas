#include "config.h"
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "internal_types.h"
# define NEED_PEEK_FLOAT
#include "sys_endian.h"
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_err.h"
#include "sys_kwd.h"
#include "dfile.h"
#include "glb.h"
#include "ndf.h"

static float
FloatField(net4_t *cntl)
{
	float r;
	PEEK_float(&r, (unsigned char *)cntl);
	return r;
}

static int
check_ll(net4_t *cntl)
{
	float	lat0, lon0;
	float	di, dj;
	int r = 0;
	di = FloatField(cntl + 24);
	dj = FloatField(cntl + 25);
	if (di == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Di is zero"));
	}
	if (dj == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dj is zero"));
	}
	if (fabs(di) >= 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid interval Di=%g too large",
					di));
	}
	if (fabs(dj) >= 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid interval Dj=%g too large",
					dj));
	}
	lat0 = FloatField(cntl + 22);
	lon0 = FloatField(cntl + 23);
	if (fabs(lat0) > 90.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lat0=%g too large",
					lat0));
	}
	if (fabs(lon0) > 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lon0=%g too large",
					lon0));
	}
	return r;
}

static int
check_gs(net4_t *cntl)
{
	float	slon;
	float	di, dj;
	int r = 0;
	di = FloatField(cntl + 24);
	dj = FloatField(cntl + 25);
	if (di == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Di is zero"));
	}
	if (dj == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dj is zero"));
	}
	if (fabs(di) >= 180.0) {
		nus_warn(("grid interval Di=%g too large",
					di));
	}
	if (fabs(dj) >= 180.0) {
		nus_warn(("grid interval Dj=%g too large",
					dj));
	}
	slon = FloatField(cntl + 27);
	di = fabs(di);
	if (slon != 0.0) {
		nus_debug(("wavenumber=%g; good.", slon));
	}
	if (slon == 0.0) {
		if (di > 1.0e-5) {
			slon = 180.0 / di;
			nus_warn(("std lon 1 (wavenumber) assumed %g", slon));
			memcpy4((char *)(cntl + 27), (const char *)&slon);
			endian_swab4((char *)(cntl + 27), 1);
		} else {
			r |= nus_err((NUSERR_BadGrid,
		"neither wavenumber (std lon 1) nor x distance is specified"));
		}
	}
	return r;
}

static int
check_me(net4_t *cntl)
{
	float	lat0, lon0;
	float	dx, dy;
	float	slat;
	int r = 0;
	lat0 = FloatField(cntl + 22);
	lon0 = FloatField(cntl + 23);
	if (fabs(lat0) > 90.0) {
		r |= nus_err((NUSERR_BadGrid,
			"grid parameter lat0=%g too large", lat0));
	}
	if (fabs(lon0) > 180.0) {
		r |= nus_err((NUSERR_BadGrid,
			"grid parameter lat0=%g too large", lon0));
	}
	dx = FloatField(cntl + 24);
	dy = FloatField(cntl + 25);
	if (dx == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Di is zero"));
	}
	if (dy == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dj is zero"));
	}
	if (fabs(dx) < 180.0) {
		nus_warn(("grid interval Dx=%g too small", dx));
	}
	if (fabs(dy) < 180.0) {
		nus_warn(("grid interval Dy=%g too small", dy));
	}
	slat = FloatField(cntl + 26);
	if (fabs(slat) >= 90.0) {
		r |= nus_err((NUSERR_BadGrid,
			"grid parameter slat=%g too large", slat));
	}
	return r;
}

static int
check_ps(net4_t *cntl)
{
	int r;
	float slon;
	r = check_me(cntl);
	slon = FloatField(cntl + 27);
	if (fabs(slon) > 180.0) {
		r |= nus_err((NUSERR_BadGrid,
			"grid parameter slon=%g too large", slon));
	}
	return r;
}

static int
check_lm(net4_t *cntl)
{
	int r;
	float slat1, slat2;
	r = check_ps(cntl);
	slat1 = FloatField(cntl + 26);
	slat2 = FloatField(cntl + 28);
	if (fabs(slat2) >= 90.0) {
		r |= nus_err((NUSERR_BadGrid,
			"grid parameter slat2=%g too large", slat2));
	}
	if (slat1 == 0.0 || slat2 == 0.0) {
		r |= nus_err((NUSERR_BadGrid,
			"one of std lat (%g %g) is zero", slat1, slat2));
	}
	if (fabs(slat1) > fabs(slat2)) {
		r |= nus_err((NUSERR_BadGrid,
			"grid parameter slat1=%g > slat2=%g",
			slat1, slat2));
	}
	if (slat1 * slat2 < 0.0) {
		r |= nus_err((NUSERR_BadGrid,
			"grid slat1=%g slat2=%g on different hemisphere",
			slat1, slat2));
	}
	return r;
}

static int
check_ol(net4_t *cntl)
{
	int r;
	r = check_lm(cntl);
	return r;
}

static int
check_rd(net4_t *cntl)
{
	float	dx, dy;
	float	lat0, lon0;
	int r = 0;
	dx = FloatField(cntl + 24);
	dy = FloatField(cntl + 25);
	if (dx == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dx is zero"));
	}
	if (dy == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dy is zero"));
	}
	if (fabs(dx) < 180.0) {
		nus_warn(("grid interval Dx=%g too small", dx));
	}
	if (fabs(dy) < 180.0) {
		nus_warn(("grid interval Dy=%g too small", dy));
	}
	lat0 = FloatField(cntl + 22);
	lon0 = FloatField(cntl + 23);
	if (fabs(lat0) > 90.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lat0=%g too large",
					lat0));
	}
	if (fabs(lon0) > 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lon0=%g too large",
					lon0));
	}
	return r;
}

static int
check_rt(net4_t *cntl)
{
	float	dx, dy;
	int r = 0;
	dx = FloatField(cntl + 24);
	dy = FloatField(cntl + 25);
	if (dx == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dx is zero"));
	}
	if (dy == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dy is zero"));
	}
	if (fabs(dx) < 180.0) {
		nus_warn(("grid interval Dx=%g too small", dx));
	}
	if (fabs(dy) >= 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid interval Dy=%g too large", dy));
	}
	return r;
}

static int
check_yp(net4_t *cntl)
{
	float	dj;
	float	lat0, lon0;
	int r = 0;
	dj = FloatField(cntl + 25);
	if (dj == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Dj is zero"));
	}
	if (fabs(dj) >= 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid interval Dj=%g too large",
					dj));
	}
	lat0 = FloatField(cntl + 22);
	lon0 = FloatField(cntl + 23);
	if (fabs(lat0) > 90.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lat0=%g too large",
					lat0));
	}
	if (fabs(lon0) > 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lon0=%g too large",
					lon0));
	}
	return r;
}

static int
check_xp(net4_t *cntl)
{
	float	di;
	float	lat0, lon0;
	int r = 0;
	di = FloatField(cntl + 24);
	if (di == 0.0) {
		r |= nus_err((NUSERR_BadGrid, "grid parameter Di is zero"));
	}
	if (fabs(di) >= 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid interval Di=%g too large",
					di));
	}
	lat0 = FloatField(cntl + 22);
	lon0 = FloatField(cntl + 23);
	if (fabs(lat0) > 90.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lat0=%g too large",
					lat0));
	}
	if (fabs(lon0) > 180.0) {
		r |= nus_err((NUSERR_BadGrid,
					"grid parameter lon0=%g too large",
					lon0));
	}
	return r;
}

int 
ndf_grid_check(net4_t *cntl)
{
	sym4_t	proj = cntl[17];
	int r;
	switch (proj) {
		case SYM4_LL:
			r = check_ll(cntl);
			break;
		case SYM4_GS:
			r = check_gs(cntl);
			break;
		case SYM4_YP:
			r = check_yp(cntl);
			break;
		case SYM4_XP:
			r = check_xp(cntl);
			break;
		case SYM4_FG:
		case SYM4_SB:
		case SYM4_ST:
		case SYM4_RG:
			r = 0;
			break;
		case SYM4_MER:
			r = check_me(cntl);
			break;
		case SYM4_PSN:
		case SYM4_PSS:
			r = check_ps(cntl);
			break;
		case SYM4_LMN:
		case SYM4_LMS:
			r = check_lm(cntl);
			break;
		case SYM4_OL:
			r = check_ol(cntl);
			break;
		case SYM4_RD:
			r = check_rd(cntl);
			break;
		case SYM4_RT:
			r = check_rt(cntl);
			break;
		case SYM4_XX:
			r = nus_err((NUSERR_BadGrid,
						"bad projection %Ps", proj));
			break;
		default:
			nus_warn(("unknown projection %Ps", proj));
			r = 0;
			break;
	}
	if (r && GlobalConfig(io_badgrid)) {
		nus_warn(("above error for grid parameter is bypassed"));
		return 0;
	}
	return r;
}
