/** @file
 * 64ビット整数型をエミュレートするライブラリ sys_int.h のテスト
 * @note 出力を sys_int.rb にかけて検証する
 */
#include <stdio.h>
#include <stdlib.h>
#include "nusdas.h"
#include "config.h"
# define NEED_MAKE_UI8
# define NEED_SI8_ADD
# define NEED_SI8_SUB
# define NEED_SI8_SUBFROM
# define NEED_UI8_MOD_UI2
# define NEED_UI8_DIV_UI2
# define NEED_SI8_CMP
#include "sys_int.h"

#if defined(__WIN32__) \
	|| defined(__STRICT_ANSI__)
# define HAVE_NO_DRAND48
#endif
#ifdef HAVE_NO_DRAND48

long mrand48(void)
{
	unsigned long ret;
	ret = (unsigned)rand();
	ret |= (ret << 20);
	ret <<= 1;
	ret &= 0x80008000;
	ret |= (unsigned)rand();
	ret <<= 16;
	ret |= (unsigned)rand();
	return (long)ret;
}

unsigned long lrand48(void)
{
	unsigned long ret;
	ret = (unsigned)rand();
	ret <<= 16;
	ret |= (unsigned)rand();
	return ret;
}

#endif

N_UI4 randf4(void) {
	N_SI4	s;
	s = mrand48();
	return *(N_UI4 *)&s;
}

N_UI2 randu2(void) {
	N_UI4	base;
	int	shift;
	base = randf4();
	shift = ((base & 0xFFFF) % 15) + 16;
	return (base >> shift);
}

N_UI4 randu4(void) {
	N_UI4	base;
	int	shift;
	base = randf4();
	shift = ((lrand48() >> 20) % 29);
	return (base >> shift);
}

N_UI8 randu8(void) {
	int	place;
	N_UI8	result;
	place = ((lrand48() >> 18) % 62);
	if (place < 30) {
		result = make_ui8(0, randf4() >> (29 - place));
	} else {
		result = make_ui8(randf4() >> (61 - place), randf4());
	}
	return result;
}

N_UI8 rands8(void) {
	int	place, first;
	N_UI8	result;
	N_UI4	hi;
	place = (((first = lrand48()) >> 18) % 62);
	hi = (first & (1 << 12)) ? (1uL << 31) : 0;
	if (place < 31) {
		result = make_ui8(hi, randf4() >> (30 - place));
	} else {
		hi |= (randf4() >> (62 - place));
		result = make_ui8(hi, randf4());
	}
	return result;
}

void test1_add(void)
{
	int i;
	for (i = 0; i < 10000; i++) {
		N_SI8 a, b, c, c2;
		a = rands8();
		b = rands8();
		c = si8_add(a, b);
		printf("si8_add %08lx%08lx %08lx%08lx %08lx%08lx\n",
				(unsigned long)I8_HI(a),
				(unsigned long)I8_LO(a),
				(unsigned long)I8_HI(b),
				(unsigned long)I8_LO(b),
				(unsigned long)I8_HI(c),
				(unsigned long)I8_LO(c));
		c2 = a;
		si8_addto(c2, b);
		if (!si8_eq(c2, c)) {
			puts("+/+= mismatch");
			exit(1);
		}
	}
}

void test2_sub(void)
{
	int i;
	for (i = 0; i < 10000; i++) {
		N_SI8 a, b, c, c2;
		a = rands8();
		b = rands8();
		c = si8_sub(a, b);
		printf("si8_sub %08lx%08lx %08lx%08lx %08lx%08lx\n",
				(unsigned long)I8_HI(a),
				(unsigned long)I8_LO(a),
				(unsigned long)I8_HI(b),
				(unsigned long)I8_LO(b),
				(unsigned long)I8_HI(c),
				(unsigned long)I8_LO(c));
		c2 = a;
		si8_subfrom(c2, b);
		if (!si8_eq(c2, c)) {
			puts("-/-= mismatch");
			exit(1);
		}
	}
}

void test3_mod(void)
{
	int i;
	for (i = 0; i < 10000; i++) {
		N_UI8 a, d;
		N_UI2 b, c;
		a = randu8();
		b = randu2();
		if (b == 0)
			continue;
		c = ui8_mod_ui2(a, b);
		printf("ui8_mod_ui2 %08lx%08lx %04x %04x\n",
				(unsigned long)I8_HI(a),
				(unsigned long)I8_LO(a),
				(unsigned)b,
				(unsigned)c);
		d = ui8_div_ui2(a, b);
		printf("ui8_div_ui2 %08lx%08lx %04x %08lx%08lx\n",
				(unsigned long)I8_HI(a),
				(unsigned long)I8_LO(a),
				(unsigned)b,
				(unsigned long)I8_HI(d),
				(unsigned long)I8_LO(d));
	}
}

int main()
{
	test1_add();
	test2_sub();
	test3_mod();
	return 0;
}
