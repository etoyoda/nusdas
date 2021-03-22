#include "nusdas.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "internal_types.h"
# define NEED_MAKE_UI8
# define NEED_MAKE_SI8
#include "sys_int.h"
# define NEED_MEM2SYM8
#include "sys_sym.h"
#define NEED_VSNPRINTF
#include "sys_err.h"

#define chk(expr)  do { if (!(expr)) { puts(#expr); abort(); } } while (0)

void test_g()
{
	char	buf[128];
	volatile N_UI4 i4;
	volatile float	*fp;
	int r;
	r = nusdas_snprintf(buf, sizeof buf, "%-8.5g", 3.141592653589793238e3);
	chk(r == 8);
	chk(!strcmp(buf, "3141.6  "));
	r = nusdas_snprintf(buf, sizeof buf, "%8.5g", 3.141592653589793238e3);
	chk(r == 8);
	chk(!strcmp(buf, "  3141.6"));
	r = nusdas_snprintf(buf, sizeof buf, "%.5g", 3.141592653589793238e5);
	chk(r == 10);
	chk(!strcmp(buf, "3.1416e+05"));
	nusdas_snprintf(buf, sizeof buf, "%.5g", 3.141592653589793238e4);
	chk(!strcmp(buf, "31416"));
	nusdas_snprintf(buf, sizeof buf, "%.5g", 3.141592653589793238e3);
	chk(!strcmp(buf, "3141.6"));
	nusdas_snprintf(buf, sizeof buf, "%g", 120.0);
	chk(!strcmp(buf, "120"));
	nusdas_snprintf(buf, sizeof buf, "%g", 0.0);
	chk(!strcmp(buf, "0"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.14e-9);
	chk(!strcmp(buf, "3.14e-09"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.14e8);
	chk(!strcmp(buf, "3.14e+08"));
	nusdas_snprintf(buf, sizeof buf, "%g", 1.0);
	chk(!strcmp(buf, "1"));
	nusdas_snprintf(buf, sizeof buf, "%g", 0.5);
	chk(!strcmp(buf, "0.5"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.141592653589793238);
	chk(!strcmp(buf, "3.14159"));
	nusdas_snprintf(buf, sizeof buf, "%g", 31.41592653589793238);
	chk(!strcmp(buf, "31.4159"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.141592653589793238e4);
	chk(!strcmp(buf, "31415.9"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.141592653589793238e5);
	chk(!strcmp(buf, "314159"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.141592653589793238e6);
	chk(!strcmp(buf, "3.14159e+06"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.141592653589793238e-4);
	chk(!strcmp(buf, "0.000314159"));
	nusdas_snprintf(buf, sizeof buf, "%g", 3.141592653589793238e-5);
	chk(!strcmp(buf, "3.14159e-05"));
	i4 = 0xFF800000uL;
	fp = (float *)&i4;
	nusdas_snprintf(buf, sizeof buf, "%g", *fp);
	chk(!strcmp(buf, "-Inf"));
	i4 = 0x7FC00000uL;
	nusdas_snprintf(buf, sizeof buf, "%g", *fp);
	chk(!strcmp(buf, "NaN"));
}

void test_a()
{
	char	buf[128];
	volatile N_UI4	i4;
	volatile float	*fp;
	N_UI8	i8;

	fp = (float *)&i4;

	*fp = 1.0;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.000000p+0"));

	*fp = 2.0;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.000000p+1"));

	*fp = 0.5;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.000000p-1"));

	*fp = 3.0;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.800000p+1"));

	*fp = 3.5;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.c00000p+1"));

	*fp = 3.75;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.e00000p+1"));

	i4 = 0x00000000uL;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x0.000000p+0"));

	i4 = 0x00000001uL;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x0.000002p-126"));

	i4 = 0x40000000uL;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.000000p+1"));

	i4 = 0x40654321uL;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "0x1.ca8642p+1"));

	i4 = 0x40654321uL;
	nusdas_snprintf(buf, sizeof buf, "%.3Pa", i4);
	chk(!strcmp(buf, "0x1.ca8p+1"));

	i4 = 0x40654321uL;
	nusdas_snprintf(buf, sizeof buf, "%12.3Pa", i4);
	chk(!strcmp(buf, "  0x1.ca8p+1"));

	i4 = 0x7F800000uL;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "Inf"));

	i4 = 0xFF800000uL;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "-Inf"));

	i4 = 0x7FC00000uL;
	nusdas_snprintf(buf, sizeof buf, "%Pa", i4);
	chk(!strcmp(buf, "NaN(0x400000)"));

	nusdas_snprintf(buf, sizeof buf, "%a", 0.0);
	chk(!strcmp(buf, "0x0.0000000000000p+0"));
	nusdas_snprintf(buf, sizeof buf, "%a", 0.5);
	chk(!strcmp(buf, "0x1.0000000000000p-1"));
	nusdas_snprintf(buf, sizeof buf, "%a", 1.0);
	chk(!strcmp(buf, "0x1.0000000000000p+0"));
	nusdas_snprintf(buf, sizeof buf, "%a", 2.0);
	chk(!strcmp(buf, "0x1.0000000000000p+1"));
	nusdas_snprintf(buf, sizeof buf, "%a", 3.0);
	chk(!strcmp(buf, "0x1.8000000000000p+1"));

	i8 = make_ui8(0, 0);
	nusdas_snprintf(buf, sizeof buf, "%Qa", i8);
	chk(!strcmp(buf, "0x0.0000000000000p+0"));
	i8 = make_ui8(0x7FF00000uL, 0x00000000uL);
	nusdas_snprintf(buf, sizeof buf, "%Qa", i8);
	chk(!strcmp(buf, "Inf"));
	i8 = make_ui8(0x7FFEDCBAuL, 0x87654321uL);
	nusdas_snprintf(buf, sizeof buf, "%Qa", i8);
	chk(!strcmp(buf, "NaN(0xedcba87654321)"));
	i8 = make_ui8(0x3FFEDCBAuL, 0x87654321uL);
	nusdas_snprintf(buf, sizeof buf, "%Qa", i8);
	chk(!strcmp(buf, "0x1.edcba87654321p+0"));
}

void test_d()
{
	char	buf[128];
	N_SI8	i8;
	int	r;

	r = nusdas_snprintf(buf, 8, "%d", 12345);
	chk(r == 5);
	chk(!strcmp(buf, "12345"));

	r = nusdas_snprintf(buf, 8, "%d", -12345);
	chk(r == 6);
	chk(!strcmp(buf, "-12345"));

	r = nusdas_snprintf(buf, 12, "%Pd", (N_SI4)0x80000001L);
	chk(r == 11);
	chk(!strcmp(buf, "-2147483647"));

	r = nusdas_snprintf(buf, 12, "%Pd", (N_SI4)0x80000000L);
	chk(r == 11);
	chk(!strcmp(buf, "-2147483648"));

	r = nusdas_snprintf(buf, 12, "%ld", 0x12345678L);
	chk(r == 9);
	chk(!strcmp(buf, "305419896"));

	r = nusdas_snprintf(buf, 12, "%011ld", 0x12345678L);
	chk(r == 11);
	chk(!strcmp(buf, "00305419896"));

	r = nusdas_snprintf(buf, 12, "%11ld", 0x12345678L);
	chk(r == 11);
	chk(!strcmp(buf, "  305419896"));

	r = nusdas_snprintf(buf, 2, "%d", 0);
	chk(r == 1);
	chk(!strcmp(buf, "0"));

	i8 = make_si8(0x12345678L, 0x22345678L);
	r = nusdas_snprintf(buf, 32, "%Qd", i8);
	chk(!strcmp(buf, "1311768465441576568"));

}

void test_u()
{
	char	buf[128];
	N_UI8	u8;
	int r;
	r = nusdas_snprintf(buf, 8, "simple");
	chk(r == 6);
	chk(strcmp(buf, "simple") == 0);
	r = nusdas_snprintf(buf, 7, "simple");
	chk(r == 6);
	r = nusdas_snprintf(buf, 6, "simple");
	chk(r == 5);
	chk(strcmp(buf, "simpl") == 0);
	r = nusdas_snprintf(buf, 10, "%2u/", 7);
	chk(strcmp(buf, " 7/") == 0);
	r = nusdas_snprintf(buf, 10, "%u", 32767);
	chk(strcmp(buf, "32767") == 0);
	r = nusdas_snprintf(buf, 10, "%2u", 3);
	chk(r == 2);
	chk(strcmp(buf, " 3") == 0);
	r = nusdas_snprintf(buf, 10, "%02u", 3);
	chk(r == 2);
	chk(strcmp(buf, "03") == 0);
	r = nusdas_snprintf(buf, 6, "%2u", 1234);
	chk(r == 4);
	r = nusdas_snprintf(buf, 5, "%2u", 1234);
	chk(r == 4);
	chk(strcmp(buf, "1234") == 0);
	r = nusdas_snprintf(buf, 4, "%2u", 1234);
	chk(r == -1);
	chk(strcmp(buf, "") == 0);

	r = nusdas_snprintf(buf, 18, "%08b %08b", 'A', 'T');
	chk(r == 17);
	chk(strcmp(buf, "01000001 01010100") == 0);

	r = nusdas_snprintf(buf, 18, "%3o %03o", '3', '3');
	chk(r == 7);
	chk(strcmp(buf, " 63 063") == 0);

	r = nusdas_snprintf(buf, 18, "%o", 012345);
	chk(r == 5);
	chk(strcmp(buf, "12345") == 0);

	r = nusdas_snprintf(buf, 18, "%lx", 0x12345678L);
	chk(r == 8);
	chk(strcmp(buf, "12345678") == 0);

	r = nusdas_snprintf(buf, 8, "%u %o %b %x", 0, 0, 0, 0);
	chk(r == 7);
	chk(!strcmp(buf, "0 0 0 0"));

	u8 = make_ui8(0x12345678L, 0x22345678L);
	r = nusdas_snprintf(buf, 17, "%Qx", u8);
	chk(r == 16);
	chk(!strcmp(buf, "1234567822345678"));

	r = nusdas_snprintf(buf, 32, "%Qo", u8);
	chk(!strcmp(buf, "110642547404215053170"));

	r = nusdas_snprintf(buf, 32, "%Qu", u8);
	chk(!strcmp(buf, "1311768465441576568"));

	r = nusdas_snprintf(buf, sizeof(buf), "%064Qb", u8);
	chk(!strcmp(buf, "00010010001101000101011001111000"
		            "00100010001101000101011001111000"));

	u8 = make_ui8(0x12345678, 0x9abcdef0);
	nusdas_snprintf(buf, sizeof buf, "%x %x %Qx", 1, 2, u8);
	chk(!strcmp(buf, "1 2 123456789abcdef0"));
	nusdas_snprintf(buf, sizeof buf, "%x %Qx %x", 1, u8, 3);
	chk(!strcmp(buf, "1 123456789abcdef0 3"));
	nusdas_snprintf(buf, sizeof buf, "%Qx %x %x", u8, 2, 3);
	chk(!strcmp(buf, "123456789abcdef0 2 3"));

}

void test_s()
{
	char	buf[128];
	N_SI4	i4;
	N_SI8	i8;
	nustype_t y;
	int	r;
	nusdas_snprintf(buf, 10, "too long string");
	chk(strcmp(buf, "too long ") == 0);
	buf[6] = 0x7F;
	r = nusdas_snprintf(buf, 10, "%s", "short");
	chk(r == 5);
	chk(strcmp(buf, "short") == 0);
	chk(buf[5] == 0);
	chk(buf[6] == 0x7F);
	nusdas_snprintf(buf, sizeof buf, "%s", "\a\b\f\t\r\n\v\x7F\033");
	chk(!strcmp(buf, "\\007\\010\\014\\t\\r\\n\\013\\177\\033"));
	nusdas_snprintf(buf, sizeof buf, "<%s>", "test");
	chk(strcmp(buf, "<test>") == 0);
	i4 = MEM2SYM4("TEST");
	nusdas_snprintf(buf, sizeof buf, "<%Ps>", i4);
	chk(strcmp(buf, "<TEST>") == 0);
	i8 = MEM2SYM8("testsym8");
	nusdas_snprintf(buf, sizeof buf, "<%Qs>", i8);
	chk(strcmp(buf, "<testsym8>") == 0);
	r = nusdas_snprintf(buf, 8, "%.4s", "longlonglong");
	chk(r == 4);
	chk(strcmp(buf, "long") == 0);
	y.type1 = MEM2SYM8("_GSMLLPP");
	y.type2 = MEM2SYM4("FCSV");
	y.type3 = MEM2SYM4("STD1");
	r = nusdas_snprintf(buf, 17, "%ys", &y);
	chk(r == 16);
	chk(strcmp(buf, "_GSMLLPPFCSVSTD1") == 0);
	r = nusdas_snprintf(buf, 19, "%#ys", &y);
	chk(r == 18);
	chk(strcmp(buf, "_GSMLLPP.FCSV.STD1") == 0);

	r = nusdas_snprintf(buf, 8, "%5s", "xy ");
	chk(r == 5);
	chk(strcmp(buf, "  xy ") == 0);
	r = nusdas_snprintf(buf, 8, "%-5s", "xy ");
	chk(r == 5);
	chk(strcmp(buf, "xy   ") == 0);

	r = nusdas_snprintf(buf, 10, "<%5.2s>", "xy");
	chk(r == 7);
	chk(strcmp(buf, "<   xy>") == 0);
	r = nusdas_snprintf(buf, 10, "<%-5.2s>", "xy");
	chk(r == 7);
	chk(strcmp(buf, "<xy   >") == 0);

	i4 = MEM2SYM4("HM  ");
	r = nusdas_snprintf(buf, sizeof buf, "<%Ps>", i4);
	chk(r == 4);
	chk(strcmp(buf, "<HM>") == 0);
	r = nusdas_snprintf(buf, sizeof buf, "<%4Ps>", i4);
	chk(r == 6);
	chk(strcmp(buf, "<  HM>") == 0);
	r = nusdas_snprintf(buf, sizeof buf, "<%-4Ps>", i4);
	chk(r == 6);
	chk(strcmp(buf, "<HM  >") == 0);

}

void test_t()
{
	N_SI4	t;
	N_SI8	vt;
	char	buf[128];
	int	r;

	t = 0;
	r = nusdas_snprintf(buf, sizeof(buf), "<%T>", t);
	chk(r == 14);
	chk(!strcmp(buf, "<180101010000>"));
	r = nusdas_snprintf(buf, sizeof(buf), "<%#T>", t);
	chk(r == 17);
	chk(!strcmp(buf, "<1801-01-01t0000>"));

	vt = make_si8(0, 0);
	r = nusdas_snprintf(buf, sizeof(buf), "<%QT>", vt);
	chk(r == 27);
	chk(!strcmp(buf, "<180101010000/180101010000>"));
	r = nusdas_snprintf(buf, sizeof(buf), "<%#QT>", vt);
	chk(r == 33);
	chk(!strcmp(buf, "<1801-01-01t0000/1801-01-01t0000>"));
}

int main()
{
	test_s();
	test_u();
	test_d();
	test_t();
	test_a();
	test_g();
	puts("ok");
	return 0;
}
