/** @file
 * @brief nus_vsnprintf() ¤Î¼ÂÁõ
 */

#include "config.h"
#include "nusdas.h"
#include <stddef.h>		/* for size_t */
#include <stdarg.h>		/* for va_start etc */
#include <string.h>		/* for memmove */
#include "internal_types.h"
# define NEED_MAKE_UI8
# define NEED_UI8_MOD_UI2
# define NEED_UI8_DIV_UI2
# define NEED_UI8_LSHIFT
# define NEED_UI8_RSHIFT
# define NEED_UI8_PLUSPLUS
# define NEED_SI8_UNARYMINUS
#include "sys_int.h"
#include "sys_kwd.h"
#include "sys_time.h"
#ifdef HAVE_UNISTD_H
# include <unistd.h>		/* for write(2) */
#endif
#include <math.h>

struct fmt_flags {
    int filler;
    int width;
    int prec;
    int ignzero;
    int ladjust;
    int plus;
    int alt;
};

static const struct fmt_flags flags_clear = { ' ', 0, 0, 0, 0, 0, 0 };

static int
iconv_backfill(char *buf, char *stop, int rwidth, const char *ibuf,
	       const char *ibuftail, struct fmt_flags *flags)
{
    int i;
    if (rwidth > (stop - buf)) {
	return -1;
    }
    if (rwidth < flags->width) {
	for (i = 0; i < (flags->width - rwidth); i++) {
	    *buf++ = flags->filler;
	}
	for (i = ibuftail - ibuf; i >= 0; i--) {
	    *buf++ = ibuf[i];
	}
	return flags->width;
    } else {
	for (i = ibuftail - ibuf; i >= 0; i--) {
	    *buf++ = ibuf[i];
	}
	return rwidth;
    }
}

static char hexup[16] = "0123456789ABCDEF";
static char hexdn[16] = "0123456789abcdef";

static int
conv_u(char *buf, char *stop, unsigned long val, struct fmt_flags *flags,
       int convchar)
{
    char ibuf[256];
    char *p;
    int rwidth;
    unsigned radix;
    char *charset;
    switch (convchar) {
    case 'b':
	radix = 2;
	charset = hexup;
	break;
    case 'o':
	radix = 8;
	charset = hexup;
	break;
    case 'x':
	radix = 16;
	charset = hexdn;
	break;
    case 'X':
	radix = 16;
	charset = hexup;
	break;
    default:
    case 'u':
	radix = 10;
	charset = hexup;
	break;
    }
    p = ibuf;
    while (val) {
	*p++ = charset[val % radix];
	val /= radix;
    }
    if (p == ibuf) {
	*p++ = '0';
    }
    rwidth = p - ibuf;
    p--;
    return iconv_backfill(buf, stop, rwidth, ibuf, p, flags);
}

static int
conv_u8(char *buf, char *stop, N_UI8 val, struct fmt_flags *flags,
	int convchar)
{
    char ibuf[256];
    char *p;
    int rwidth;
    N_UI2 radix;
    char *charset;
    switch (convchar) {
    case 'b':
	radix = 2;
	charset = hexup;
	break;
    case 'o':
	radix = 8;
	charset = hexup;
	break;
    case 'x':
	radix = 16;
	charset = hexdn;
	break;
    case 'X':
	radix = 16;
	charset = hexup;
	break;
    case 'u':
    default:
	radix = 10;
	charset = hexup;
	break;
    }
    p = ibuf;
    while (!ui8_iszero(val)) {
	*p++ = charset[ui8_mod_ui2(val, radix)];
	val = ui8_div_ui2(val, radix);
    }
    if (p == ibuf) {
	*p++ = '0';
    }
    rwidth = p - ibuf;
    p--;
    return iconv_backfill(buf, stop, rwidth, ibuf, p, flags);
}

static int conv_d(char *buf, char *stop, long val, struct fmt_flags *flags)
{
    char ibuf[64];
    char *p;
    int rwidth, negative, minhack = 0;
    p = ibuf;
    if ((negative = (val < 0)) != 0) {
	if ((minhack = ((val << 1) == 0)) != 0) {
	    val++;
	}
	val = -val;
    }
    while (val) {
	*p++ = (val % 10) + '0';
	val /= 10;
    }
    if (p == ibuf) {
	*p++ = '0';
    }
    if (negative) {
	*p++ = '-';
	if (minhack) {
	    ibuf[0]++;
	}
    } else if (flags->plus) {
	*p++ = '+';
    }
    rwidth = p - ibuf;
    p--;
    return iconv_backfill(buf, stop, rwidth, ibuf, p, flags);
}

static int
conv_d8(char *buf, char *stop, N_SI8 val, struct fmt_flags *flags)
{
    char ibuf[64];
    char *p;
    int rwidth, negative, minhack = 0;
    p = ibuf;
    if ((negative = si8_isnegative(val)) != 0) {
	N_SI8 val_double = ui8_lshift(val, 1);
	minhack = si8_iszero(val_double);
	if (minhack) {
	    ui8_plusplus(val);
	}
	val = si8_unaryminus(val);
    }
    while (!si8_iszero(val)) {
	*p++ = ui8_mod_ui2(val, 10) + '0';
	val = ui8_div_ui2(val, 10);
    }
    if (p == ibuf) {
	*p++ = '0';
    }
    if (negative) {
	*p++ = '-';
	if (minhack) {
	    ibuf[0]++;
	}
    } else if (flags->plus) {
	*p++ = '+';
    }
    rwidth = p - ibuf;
    p--;
    return iconv_backfill(buf, stop, rwidth, ibuf, p, flags);
}

static int
conv_ad(char *buf, char *stop, double val, struct fmt_flags *flags)
{
    char ibuf[64];
    char *p;
    N_UI4 expo, sign;
    N_UI8 ival, mant;
    int r;
    /* this ugly memory copy is needed to cope with optimization
     * at least for gcc 4.1.2 on x86_64 */
    memcpy((void *)&ival, (void *)&val, 8);
#if 0
    printf("ival=%08x %08x < %g\n", ival.loaddr, ival.hiaddr, val);
#endif
    mant = make_ui8(I8_HI(ival) & 0x000FFFFF, I8_LO(ival));
    expo = (I8_HI(ival) >> 20) & 0x7FF;
    sign = (I8_HI(ival) >> 31);
#if 0
    printf("mant=%08x %08x expo=%03x sign=%01x\n",
      mant.loaddr, mant.hiaddr,
      expo, sign);
#endif
    p = ibuf;
    if (flags->prec > 13)
	flags->prec = 13;
    if (flags->prec <= 0)
	flags->prec = 13;
    if (expo == 0x7FF) {
	if (ui8_iszero(mant)) {
	    memcpy(ibuf, "fnI", 3);
	    p += 3;
	} else {
	    *p++ = ')';
	    while (!ui8_iszero(mant) || flags->prec) {
		*p++ = hexdn[ui8_mod_ui2(mant, 0x10)];
		mant = ui8_rshift(mant, 4);
		flags->prec--;
	    }
	    memcpy(p, "x0(NaN", 6);
	    p += 6;
	}
    } else {
	int expb;
	int exp_nega;
	expb = (expo ? ((signed int) expo - 1023) :
			(si8_iszero(mant) ? 0 : -1022) );
	if ((exp_nega = (expb < 0)) != 0) {
	    expb = -expb;
	}
	while (expb) {
	    *p++ = (expb % 10) + '0';
	    expb /= 10;
	}
	if (p == ibuf) {
	    *p++ = '0';
	}
	*p++ = (exp_nega ? '-' : '+');
	*p++ = 'p';
	mant = ui8_rshift(mant, ((13 - flags->prec) * 4));
	while (flags->prec) {
	    *p++ = hexdn[ui8_mod_ui2(mant, 0x10)];
	    mant = ui8_rshift(mant, 4);
	    flags->prec--;
	}
	*p++ = '.';
	*p++ = (expo ? '1' : '0');
	*p++ = 'x';
	*p++ = '0';
    }
    if (sign || flags->plus) {
	*p++ = (sign ? '-' : '+');
    }
    p--;
    r = iconv_backfill(buf, stop, p - ibuf + 1, ibuf, p, flags);
#if 0
    printf("<%s>\n", buf);
#endif
    return r;
}

static int
conv_a8(char *buf, char *stop, N_UI8 ival, struct fmt_flags *flags)
{
    char ibuf[64];
    char *p = ibuf;
    N_UI4 expo, sign;
    N_UI8 mant;
    mant = make_ui8(I8_HI(ival) & 0x000FFFFF, I8_LO(ival));
    expo = (I8_HI(ival) >> 20) & 0x7FF;
    sign = (I8_HI(ival) >> 31);
    if (flags->prec > 13)
	flags->prec = 13;
    if (flags->prec <= 0)
	flags->prec = 13;
    if (expo == 0x7FF) {
	if (ui8_iszero(mant)) {
	    memcpy(ibuf, "fnI", 3);
	    p += 3;
	} else {
	    *p++ = ')';
	    while (!ui8_iszero(mant) || flags->prec) {
		*p++ = hexdn[ui8_mod_ui2(mant, 0x10)];
		mant = ui8_rshift(mant, 4);
		flags->prec--;
	    }
	    memcpy(p, "x0(NaN", 6);
	    p += 6;
	}
    } else {
	int expb;
	int exp_nega;
	expb = (expo ? ((signed int) expo - 1023) :
			(si8_iszero(mant) ? 0 : -1022) );
	if ((exp_nega = (expb < 0)) != 0) {
	    expb = -expb;
	}
	while (expb) {
	    *p++ = (expb % 10) + '0';
	    expb /= 10;
	}
	if (p == ibuf) {
	    *p++ = '0';
	}
	*p++ = (exp_nega ? '-' : '+');
	*p++ = 'p';
	mant = ui8_rshift(mant, ((13 - flags->prec) * 4));
	while (flags->prec) {
	    *p++ = hexdn[ui8_mod_ui2(mant, 0x10)];
	    mant = ui8_rshift(mant, 4);
	    flags->prec--;
	}
	*p++ = '.';
	*p++ = (expo ? '1' : '0');
	*p++ = 'x';
	*p++ = '0';
    }
    if (sign || flags->plus) {
	*p++ = (sign ? '-' : '+');
    }
    p--;
    return iconv_backfill(buf, stop, p - ibuf + 1, ibuf, p, flags);
}

static int
conv_a4(char *buf, char *stop, N_UI4 ival, struct fmt_flags *flags)
{
    char ibuf[64];
    char *p = ibuf;
    N_UI4 mant, expo, sign;
    mant = 0x007FFFFF & ival;
    expo = (ival >> 23) & 0xFF;
    sign = (ival >> 31);
    if (flags->prec > 6)
	flags->prec = 6;
    if (flags->prec <= 0)
	flags->prec = 6;
    if (expo == 0xFF) {
	if (mant == 0) {
	    memcpy(ibuf, "fnI", 3);
	    p += 3;
	} else {
	    *p++ = ')';
	    while (mant || flags->prec) {
		*p++ = hexdn[mant & 0xF];
		mant >>= 4;
		flags->prec--;
	    }
	    memcpy(p, "x0(NaN", 6);
	    p += 6;
	}
    } else {
	int expb;
	int exp_nega;
	expb = (expo ? ((signed int) expo - 127) : mant ? -126 : 0);
	if ((exp_nega = (expb < 0)) != 0) {
	    expb = -expb;
	}
	while (expb) {
	    *p++ = (expb % 10) + '0';
	    expb /= 10;
	}
	if (p == ibuf) {
	    *p++ = '0';
	}
	*p++ = (exp_nega ? '-' : '+');
	*p++ = 'p';
	mant <<= 1;
	mant >>= ((6 - flags->prec) * 4);
	while (flags->prec) {
	    *p++ = hexdn[mant & 0xF];
	    mant >>= 4;
	    flags->prec--;
	}
	*p++ = '.';
	*p++ = (expo ? '1' : '0');
	*p++ = 'x';
	*p++ = '0';
    }
    if (sign || flags->plus) {
	*p++ = (sign ? '-' : '+');
    }
    p--;
    return iconv_backfill(buf, stop, p - ibuf + 1, ibuf, p, flags);
}

static int
conv_s(char *buf, char *hardstop,
		const char *val, struct fmt_flags *flags)
{
    char *bufp = buf;
    const char *valp = val;
    char *stop;
    if (flags->prec && ((buf + flags->prec) < hardstop)) {
	stop = buf + flags->prec;
    } else {
	stop = hardstop;
    }
    while (*valp || flags->ignzero) {
	if (flags->ignzero && (valp >= (val + flags->prec)))
	    break;
	if (*valp == '\\') {
	    if (bufp + 2 > stop)
		break;
	    *bufp++ = '\\';
	    *bufp++ = '\\';
	} else if (*valp >= 0x20 && *valp <= 0x7E) {
	    if (bufp + 1 > stop)
		break;
	    *bufp++ = *valp;
	} else if (*valp == '\t') {
	    if (bufp + 2 > stop)
		break;
	    *bufp++ = '\\';
	    *bufp++ = 't';
	} else if (*valp == '\n') {
	    if (bufp + 2 > stop)
		break;
	    *bufp++ = '\\';
	    *bufp++ = 'n';
	} else if (*valp == '\r') {
	    if (bufp + 2 > stop)
		break;
	    *bufp++ = '\\';
	    *bufp++ = 'r';
	} else if (*valp == 0) {
	    if (bufp + 2 > stop)
		break;
	    *bufp++ = '\\';
	    *bufp++ = '0';
	} else {
	    int c = *(unsigned char *) valp;
	    if (bufp + 4 > stop)
		break;
	    *bufp++ = '\\';
	    *bufp++ = ((c >> 6) & 003) + '0';
	    *bufp++ = ((c >> 3) & 007) + '0';
	    *bufp++ = (c & 007) + '0';
	}
	valp++;
    }
    if (flags->width > hardstop - buf) {
	flags->width = hardstop - buf;
    }
    if ((bufp - buf) < flags->width) {
	if (flags->ladjust) {
	    while (bufp < (buf + flags->width))
		*bufp++ = ' ';
	} else {
	    int ofs, cnt;
	    ofs = (buf + flags->width) - bufp;
	    cnt = bufp - buf;
	    memmove(buf + ofs, buf, cnt);
	    ofs--;
	    while (ofs >= 0) {
		buf[ofs--] = ' ';
	    }
	    bufp = buf + flags->width;
	}
    }
    return bufp - buf;
}

static int
conv_ys(char *buf, char *stop, const nustype_t *val,
		const struct fmt_flags *xflags)
{
	struct fmt_flags flags;
	int r, rr;
	r = 0;
	flags.ignzero = 1;
	flags.width = flags.prec = 8;
	rr = conv_s(buf, stop, (const char *)&val->type1, &flags);
	buf += rr;
	r += rr;
	if (rr < 8) return r;
	if (xflags->alt) {
		flags.width = flags.prec = 1;
		rr = conv_s(buf, stop, ".", &flags);
		buf += rr;
		r += rr;
		if (rr < 1) return r;
	}
	flags.width = flags.prec = 4;
	rr = conv_s(buf, stop, (const char *)&val->type2, &flags);
	buf += rr;
	r += rr;
	if (rr < 4) return r;
	if (xflags->alt) {
		flags.width = flags.prec = 1;
		rr = conv_s(buf, stop, ".", &flags);
		buf += rr;
		r += rr;
		if (rr < 1) return r;
	}
	flags.width = flags.prec = 4;
	rr = conv_s(buf, stop, (const char *)&val->type3, &flags);
	r += rr;
	return r;
}

static int
conv_g(char *buf, char *stop, double val, struct fmt_flags *flags)
{
    double abs, absmin, absmax;
    char gbuf[64];
    char *p;
    unsigned long mant, absexp;
    N_UI8 i8val, mant8;
    N_UI4 expb;
    int expo, negexp, active, i;
    if (val == 0.0) {
	return conv_s(buf, stop, "0", flags);
    }
    i8val = *(N_UI8 *) & val;
    mant8 = make_ui8(I8_HI(i8val) & 0x000FFFFF, I8_LO(i8val));
    expb = (I8_HI(i8val) >> 20) & 0x7FF;
    if (expb == 0x7FF) {
	if (ui8_iszero(mant8)) {
	    if (I8_HI(i8val) >> 31) {
		return conv_s(buf, stop, "-Inf", flags);
	    } else {
		return conv_s(buf, stop, "+Inf", flags);
	    }
	} else {
	    return conv_s(buf, stop, "NaN", flags);
	}
    }
    abs = (val < 0) ? -val : val;
    if (flags->prec == 0) {
	    flags->prec = 6;
    }
    expo = flags->prec - 1;
    absmin = 1.0;
    absmax = 10.0;
    for (i = 0; i < flags->prec - 1; i++) {
	absmin *= 10.0;
	absmax *= 10.0;
    }
    absmin -= 0.5;
    absmax -= 0.5;
    while (abs < absmin) {
	abs *= 10.0;
	expo--;
    }
    while (abs >= absmax) {
	abs *= 0.1;
	expo++;
    }
    mant = floor(abs + 0.5); 
    p = gbuf;
    if (expo >= -4 && expo < flags->prec) {
	active = 0;
	for (i = expo - flags->prec + 1; i <= 0 || mant; i++) {
	    if (!active) {
		if (mant % 10 || i == 0) {
		    active = 1;
		} else {
		    mant /= 10;
		    continue;
		}
	    } else {
		if (i == 0) {
		    *p++ = '.';
		}
	    }
	    *p++ = "0123456789"[mant % 10];
	    mant /= 10;
	}
	goto sign;
    }
    if (expo < 0) {
	negexp = 1;
	absexp = -expo;
    } else {
	negexp = 0;
	absexp = expo;
    }
    while (absexp || p <= gbuf + 1) {
	*p++ = "0123456789"[absexp % 10];
	absexp /= 10;
    }
    if (negexp) {
	*p++ = '-';
    } else {
	*p++ = '+';
    }
    *p++ = 'e';
    active = 0;
    while (mant) {
	if (!active) {
	    if (mant % 10) {
		active = 1;
	    } else {
		mant /= 10;
		continue;
	    }
	} else {
	    if (mant < 10) {
		*p++ = '.';
	    }
	}
	*p++ = "0123456789"[mant % 10];
	mant /= 10;
    }
sign:
    if (val < 0) {
	*p++ = '-';
    } else if (flags->plus) {
	*p++ = '+';
    }
    if (flags->ladjust && (flags->width > (p - gbuf))) {
	int width = flags->width;
	flags->width = p - gbuf;
	memset(buf + flags->width, ' ', width - flags->width);
    	iconv_backfill(buf, stop, p - gbuf, gbuf, p - 1, flags);
	return width;
    }
    return iconv_backfill(buf, stop, p - gbuf, gbuf, p - 1, flags);
}

static int
conv_T(char *buf, char *stop, N_SI4 val, struct fmt_flags *flags)
{
    int width;
    width = flags->alt ? 15 : 12;
    if (buf + width > stop) {
	return -1;
    }
    time_to_chars(buf, val);
    if (flags->alt) {
	memmove(buf + 11, buf + 8, 4);
	memmove(buf + 8, buf + 6, 2);
	memmove(buf + 5, buf + 4, 2);
	buf[4] = buf[7] = '-';
	buf[10] = 't';
    }
    return width;
}

static void
set_Ps_flags(struct fmt_flags *flags, const char *str, int fixedwidth)
{
    int i;
    flags->prec = fixedwidth;
    for (i = fixedwidth - 1; i >= 0; i--) {
	if (str[i] == ' ') {
	    flags->prec--;
	}
    }
    flags->ignzero = 1;
}

static int
conv_ms(char *buf, char *stop, const nusdims_t *val,
		const struct fmt_flags *xflags)
{
	struct fmt_flags flags;
	int r, rr;
	int timewidth = xflags->alt ? 15 : 12;
	r = 0;
	flags.ignzero = 1;
	flags.alt = xflags->alt;
	flags.width = flags.prec = timewidth;
	rr = conv_T(buf, stop, val->basetime, &flags);
	buf += rr;
	r += rr;
	if (rr < timewidth) return r;
	flags.alt = 0;
#define INSERT_SLASH \
	if (xflags->alt) { \
		flags.width = flags.prec = 1; \
		rr = conv_s(buf, stop, "/", &flags); \
		buf += rr; \
		r += rr; \
		if (rr < 1) return r; \
	}
	INSERT_SLASH
	flags.width = flags.prec = 4;
	if (val->member == SYM4_ALLSPACE) {
		rr = conv_s(buf, stop, "none", &flags);
	} else {
		rr = conv_s(buf, stop, (const char *)&val->member, &flags);
	}
	buf += rr;
	r += rr;
	if (rr < 4) return r;
	INSERT_SLASH
	flags.width = flags.prec = timewidth;
	flags.alt = xflags->alt;
	rr = conv_T(buf, stop, val->validtime1, &flags);
	buf += rr;
	r += rr;
	if (rr < timewidth) return r;
	flags.alt = 0;
	INSERT_SLASH
	if (xflags->alt) {
		flags.width = 0;
		set_Ps_flags(&flags, (char *)&val->plane1, 8);
	} else {
		flags.width = flags.prec = 6;
	}
	rr = conv_s(buf, stop, (const char *)&val->plane1, &flags);
	buf += rr;
	r += rr;
	INSERT_SLASH
	if (xflags->alt) {
		flags.width = 0;
		set_Ps_flags(&flags, (char *)&val->element, 8);
	} else {
		flags.width = flags.prec = 6;
	}
	rr = conv_s(buf, stop, (const char *)&val->element, &flags);
	buf += rr;
	r += rr;
	return r;
#undef INSERT_SLASH
}

int
nus_vsnprintf(char *buf, size_t bufsize, const char *format, va_list ap)
{
    char *bufp, *stopper;
    const char *fmtp;
    int shift = 0, rwidth;
    struct fmt_flags flags;
    int *nump = &flags.width;
    enum argtype_t {
	TF_NONE, TF_INT, TF_SIZE_T, TF_LONG, TF_INT4, TF_INT8,
	TF_NUSTYPE, TF_NUSDIMS
    } typ = TF_NONE;
    N_UI8 argu8;
    N_UI4 argu4;
    double argd;
    long argi;
    unsigned long argu;
    char *args;
    bufp = buf;
    flags = flags_clear;
    stopper = buf + bufsize - 1;
    for (fmtp = format; *fmtp; fmtp++) {
	if (!shift) {
	    if (*fmtp == '%') {
		shift = 1;
	    } else {
		*bufp++ = *fmtp;
		if (bufp >= stopper) {
		    break;
		}
	    }
	    continue;
	}
	switch (*fmtp) {
	    /* --- percent itself --- */
	case '%':
	    shift = 0;
	    *bufp++ = '%';
	    break;
	    /* --- type indicators --- */
	case 'l':
	    typ = TF_LONG;
	    break;
	case 'P':
	    typ = TF_INT4;
	    break;
	case 'Q':
	    typ = TF_INT8;
	    break;
	case 'y':
	    typ = TF_NUSTYPE;
	    break;
	case 'm':
	    typ = TF_NUSDIMS;
	    break;
	case 'z':
	    typ = TF_SIZE_T;
	    break;
	    /* --- flags --- */
	case '.':
	    nump = &flags.prec;
	    break;
	case '-':
	    flags.ladjust = 1;
	    break;
	case '+':
	    flags.plus = 1;
	    break;
	case '#':
	    flags.alt = 1;
	    break;
	case '0':
	    flags.filler = '0';
	    break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    *nump *= 10;
	    *nump += (*fmtp - '0');
	    break;
	    /* --- conversion --- */
	case 'd':
	    if (typ == TF_INT8) {
		argu8 = va_arg(ap, N_UI8);
		rwidth = conv_d8(bufp, stopper, argu8, &flags);
		goto Conversion_Done;
	    } else if (typ == TF_INT4) {
		argi = va_arg(ap, N_SI4);
	    } else if (typ == TF_LONG) {
		argi = va_arg(ap, long);
	    } else if (typ == TF_SIZE_T) {
		argi = va_arg(ap, ssize_t);
	    } else {
		argi = va_arg(ap, int);
	    }
	    rwidth = conv_d(bufp, stopper, argi, &flags);
	    goto Conversion_Done;
	case 'b':
	case 'x':
	case 'X':
	case 'u':
	case 'o':
	    if (typ == TF_INT8) {
		argu8 = va_arg(ap, N_UI8);
		rwidth = conv_u8(bufp, stopper, argu8, &flags, *fmtp);
		goto Conversion_Done;
	    } else if (typ == TF_INT4) {
		argu = va_arg(ap, N_UI4);
	    } else if (typ == TF_LONG) {
		argu = va_arg(ap, unsigned long);
	    } else if (typ == TF_SIZE_T) {
		argu = va_arg(ap, size_t);
	    } else {
		argu = va_arg(ap, unsigned);
	    }
	    rwidth = conv_u(bufp, stopper, argu, &flags, *fmtp);
	    goto Conversion_Done;
	case 'p':
	    flags.filler = '0';
	    flags.width = SIZEOF_VOIDP * 2;
#if (SIZEOF_VOIDP == 8) && HAVE_SI8_TYPE
	    argu8 = (N_UI8)va_arg(ap, void *);
	    rwidth = conv_u8(bufp, stopper, argu8, &flags, 'x');
#else
	    argu = (N_UI4)(ptrdiff_t)va_arg(ap, void *);
	    rwidth = conv_u(bufp, stopper, argu, &flags, 'x');
#endif
	    goto Conversion_Done;
	case 'T':
	    if (typ == TF_INT8) {
		argu8 = va_arg(ap, N_UI8);
		rwidth = conv_T(bufp, stopper, I8_HI(argu8), &flags);
		if (rwidth < 0) {
		    goto Overflow;
		}
		bufp += rwidth;
		*bufp++ = '/';
		if (bufp >= stopper) {
		    goto Overflow;
		}
		rwidth = conv_T(bufp, stopper, I8_LO(argu8), &flags);
		goto Conversion_Done;
	    } else if (typ == TF_LONG) {
		argu4 = va_arg(ap, long);
	    } else if (typ == TF_SIZE_T) {
		argu4 = va_arg(ap, ssize_t);
	    } else {
		argu4 = va_arg(ap, N_UI4);
	    }
	    rwidth = conv_T(bufp, stopper, argu4, &flags);
	    goto Conversion_Done;
	case 'g':
	    argd = va_arg(ap, double);
	    rwidth = conv_g(bufp, stopper, argd, &flags);
	    goto Conversion_Done;
	case 'a':
	    if (typ == TF_INT8) {
		argu8 = va_arg(ap, N_UI8);
		rwidth = conv_a8(bufp, stopper, argu8, &flags);
	    } else if (typ == TF_INT4) {
		argu4 = va_arg(ap, N_UI4);
		rwidth = conv_a4(bufp, stopper, argu4, &flags);
	    } else {
		argd = va_arg(ap, double);
		rwidth = conv_ad(bufp, stopper, argd, &flags);
	    }
	    goto Conversion_Done;
	case 'c':
	    argu = va_arg(ap, unsigned);
	    *bufp = argu;
	    rwidth = 1;
	    goto Conversion_Done;
	case 's':
	    if (typ == TF_INT8) {
		argu8 = va_arg(ap, N_UI8);
		args = (char *) &argu8;
		set_Ps_flags(&flags, args, 8);
	    } else if (typ == TF_INT4) {
		argu4 = va_arg(ap, N_UI4);
		args = (char *) &argu4;
		set_Ps_flags(&flags, args, 4);
	    } else if (typ == TF_NUSTYPE) {
		nustype_t *argy;
		argy = va_arg(ap, nustype_t *);
		rwidth = conv_ys(bufp, stopper, argy, &flags);
		goto Conversion_Done;
	    } else if (typ == TF_NUSDIMS) {
		nusdims_t *argm;
		argm = va_arg(ap, nusdims_t *);
		rwidth = conv_ms(bufp, stopper, argm, &flags);
		goto Conversion_Done;
	    } else {
		args = va_arg(ap, char *);
	    }
	    rwidth = conv_s(bufp, stopper, args, &flags);
Conversion_Done:
	    if (rwidth < 0) {
		goto Overflow;
	    }
	    bufp += rwidth;
	    flags = flags_clear;
	    typ = TF_NONE;
	    shift = 0;
	    break;
	}
	if (bufp >= stopper) {
	    break;
	}
    }
    *bufp = '\0';
    return bufp - buf;
Overflow:
    *bufp = '\0';
    return -1;
}
