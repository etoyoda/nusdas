/** @file
 * @brief ファイル名テンプレート dds_template の構築
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stdlib.h>
#include <string.h>
#include "sys_mem.h"
# define NEED_CHARS_DUP
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_time.h"
#include "sys_err.h"
#include "sys_kwd.h"
#include "dset.h"

	static int
expand_keywords(char * const dst, const char *src, nusdef_t *def,
	struct dds_template *tmpl, N_SI4 fill_basetime)
{
	int symbol_table[256] = {
		13, /* 0x00 */
		4, /* 0x01 */
		4, /* 0x02 */
		4, /* 0x03 */
		4, /* 0x04 */
		4, /* 0x05 */
		4, /* 0x06 */
		4, /* 0x07 */
		4, /* 0x08 */
		4, /* 0x09 */
		4, /* 0x0a */
		4, /* 0x0b */
		4, /* 0x0c */
		4, /* 0x0d */
		4, /* 0x0e */
		4, /* 0x0f */
		4, /* 0x10 */
		4, /* 0x11 */
		4, /* 0x12 */
		4, /* 0x13 */
		4, /* 0x14 */
		4, /* 0x15 */
		4, /* 0x16 */
		4, /* 0x17 */
		4, /* 0x18 */
		4, /* 0x19 */
		4, /* 0x1a */
		4, /* 0x1b */
		4, /* 0x1c */
		4, /* 0x1d */
		4, /* 0x1e */
		4, /* 0x1f */
		4, /* 0x20 */
		4, /* 0x21 */
		4, /* 0x22 */
		4, /* 0x23 */
		4, /* 0x24 */
		4, /* 0x25 */
		4, /* 0x26 */
		4, /* 0x27 */
		4, /* 0x28 */
		4, /* 0x29 */
		4, /* 0x2a */
		4, /* 0x2b */
		4, /* 0x2c */
		4, /* 0x2d */
		4, /* 0x2e */
		0, /* 0x2f */
		4, /* 0 */
		4, /* 1 */
		1, /* 2 */
		2, /* 3 */
		4, /* 4 */
		4, /* 5 */
		4, /* 6 */
		4, /* 7 */
		4, /* 8 */
		4, /* 9 */
		4, /* 0x3a */
		4, /* 0x3b */
		4, /* 0x3c */
		4, /* 0x3d */
		4, /* 0x3e */
		4, /* 0x3f */
		4, /* 0x40 */
		3, /* A */
		5, /* B */
		6, /* C */
		7, /* D */
		8, /* E */
		4, /* F */
		4, /* G */
		4, /* H */
		9, /* I */
		4, /* J */
		4, /* K */
		10, /* L */
		11, /* M */
		12, /* N */
		14, /* O */
		15, /* P */
		4, /* Q */
		16, /* R */
		17, /* S */
		18, /* T */
		19, /* U */
		20, /* V */
		4, /* W */
		4, /* X */
		4, /* Y */
		4, /* Z */
		4, /* 0x5b */
		4, /* 0x5c */
		4, /* 0x5d */
		4, /* 0x5e */
		21, /* _ */
		4, /* 0x60 */
		22, /* a */
		23, /* b */
		24, /* c */
		25, /* d */
		26, /* e */
		4, /* f */
		4, /* g */
		4, /* h */
		27, /* i */
		4, /* j */
		4, /* k */
		28, /* l */
		29, /* m */
		30, /* n */
		31, /* o */
		32, /* p */
		4, /* q */
		33, /* r */
		34, /* s */
		35, /* t */
		36, /* u */
		37, /* v */
		4, /* w */
		4, /* x */
		4, /* y */
		4, /* z */
		4, /* 0x7b */
		4, /* 0x7c */
		4, /* 0x7d */
		4, /* 0x7e */
		4, /* 0x7f */
		4, /* 0x80 */
		4, /* 0x81 */
		4, /* 0x82 */
		4, /* 0x83 */
		4, /* 0x84 */
		4, /* 0x85 */
		4, /* 0x86 */
		4, /* 0x87 */
		4, /* 0x88 */
		4, /* 0x89 */
		4, /* 0x8a */
		4, /* 0x8b */
		4, /* 0x8c */
		4, /* 0x8d */
		4, /* 0x8e */
		4, /* 0x8f */
		4, /* 0x90 */
		4, /* 0x91 */
		4, /* 0x92 */
		4, /* 0x93 */
		4, /* 0x94 */
		4, /* 0x95 */
		4, /* 0x96 */
		4, /* 0x97 */
		4, /* 0x98 */
		4, /* 0x99 */
		4, /* 0x9a */
		4, /* 0x9b */
		4, /* 0x9c */
		4, /* 0x9d */
		4, /* 0x9e */
		4, /* 0x9f */
		4, /* 0xa0 */
		4, /* 0xa1 */
		4, /* 0xa2 */
		4, /* 0xa3 */
		4, /* 0xa4 */
		4, /* 0xa5 */
		4, /* 0xa6 */
		4, /* 0xa7 */
		4, /* 0xa8 */
		4, /* 0xa9 */
		4, /* 0xaa */
		4, /* 0xab */
		4, /* 0xac */
		4, /* 0xad */
		4, /* 0xae */
		4, /* 0xaf */
		4, /* 0xb0 */
		4, /* 0xb1 */
		4, /* 0xb2 */
		4, /* 0xb3 */
		4, /* 0xb4 */
		4, /* 0xb5 */
		4, /* 0xb6 */
		4, /* 0xb7 */
		4, /* 0xb8 */
		4, /* 0xb9 */
		4, /* 0xba */
		4, /* 0xbb */
		4, /* 0xbc */
		4, /* 0xbd */
		4, /* 0xbe */
		4, /* 0xbf */
		4, /* 0xc0 */
		4, /* 0xc1 */
		4, /* 0xc2 */
		4, /* 0xc3 */
		4, /* 0xc4 */
		4, /* 0xc5 */
		4, /* 0xc6 */
		4, /* 0xc7 */
		4, /* 0xc8 */
		4, /* 0xc9 */
		4, /* 0xca */
		4, /* 0xcb */
		4, /* 0xcc */
		4, /* 0xcd */
		4, /* 0xce */
		4, /* 0xcf */
		4, /* 0xd0 */
		4, /* 0xd1 */
		4, /* 0xd2 */
		4, /* 0xd3 */
		4, /* 0xd4 */
		4, /* 0xd5 */
		4, /* 0xd6 */
		4, /* 0xd7 */
		4, /* 0xd8 */
		4, /* 0xd9 */
		4, /* 0xda */
		4, /* 0xdb */
		4, /* 0xdc */
		4, /* 0xdd */
		4, /* 0xde */
		4, /* 0xdf */
		4, /* 0xe0 */
		4, /* 0xe1 */
		4, /* 0xe2 */
		4, /* 0xe3 */
		4, /* 0xe4 */
		4, /* 0xe5 */
		4, /* 0xe6 */
		4, /* 0xe7 */
		4, /* 0xe8 */
		4, /* 0xe9 */
		4, /* 0xea */
		4, /* 0xeb */
		4, /* 0xec */
		4, /* 0xed */
		4, /* 0xee */
		4, /* 0xef */
		4, /* 0xf0 */
		4, /* 0xf1 */
		4, /* 0xf2 */
		4, /* 0xf3 */
		4, /* 0xf4 */
		4, /* 0xf5 */
		4, /* 0xf6 */
		4, /* 0xf7 */
		4, /* 0xf8 */
		4, /* 0xf9 */
		4, /* 0xfa */
		4, /* 0xfb */
		4, /* 0xfc */
		4, /* 0xfd */
		4, /* 0xfe */
		4  /* 0xff */
	};
	int action_table[107][38] = {
/* :Initial */
{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 151, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* "Default" */
{5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 151, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/* "_" */
{4, 7, 14, 21, 2, 32, 2, 2, 2, 2, 2, 42, 56, 3, 2, 2, 2, 62, 69, 2, 75, 2, 86, 97, 2, 2, 2, 2, 2, 107, 121, 2, 2, 2, 127, 134, 2, 140},
/* "_2" */
{4, 2, 2, 2, 2, 2, 2, 8, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 11, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_2D" */
{10, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_2d" */
{13, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 12, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_3" */
{4, 2, 2, 2, 2, 2, 2, 15, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_3D" */
{17, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 16, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_3d" */
{20, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 19, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_A" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 22, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_AT" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 23, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ATT" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 24, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ATTR" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 25, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ATTRI" */
{4, 2, 2, 2, 2, 26, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ATTRIB" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 27, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ATTRIBU" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 28, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ATTRIBUT" */
{4, 2, 2, 2, 2, 2, 2, 2, 29, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ATTRIBUTE" */
{31, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 30, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_B" */
{4, 2, 2, 33, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_BA" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 34, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_BAS" */
{4, 2, 2, 2, 2, 2, 2, 2, 35, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_BASE" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 36, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_BASET" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 37, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_BASETI" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_BASETIM" */
{4, 2, 2, 2, 2, 2, 2, 2, 39, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_BASETIME" */
{41, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 40, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_M" */
{4, 2, 2, 2, 2, 2, 2, 2, 43, 2, 2, 2, 2, 3, 50, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ME" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 44, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MEM" */
{4, 2, 2, 2, 2, 45, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MEMB" */
{4, 2, 2, 2, 2, 2, 2, 2, 46, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MEMBE" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 47, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MEMBER" */
{49, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 48, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MO" */
{4, 2, 2, 2, 2, 2, 2, 51, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MOD" */
{4, 2, 2, 2, 2, 2, 2, 2, 52, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MODE" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 53, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_MODEL" */
{55, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 54, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_N" */
{4, 2, 2, 57, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_NA" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 58, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_NAM" */
{4, 2, 2, 2, 2, 2, 2, 2, 59, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_NAME" */
{61, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 60, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_S" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 63, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_SP" */
{4, 2, 2, 64, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_SPA" */
{4, 2, 2, 2, 2, 2, 65, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_SPAC" */
{4, 2, 2, 2, 2, 2, 2, 2, 66, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_SPACE" */
{68, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 67, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_T" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 70, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_TI" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 71, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_TIM" */
{4, 2, 2, 2, 2, 2, 2, 2, 72, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_TIME" */
{74, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 73, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_V" */
{4, 2, 2, 76, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VA" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 77, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VAL" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 78, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VALI" */
{4, 2, 2, 2, 2, 2, 2, 79, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VALID" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 80, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VALIDT" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 81, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VALIDTI" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 82, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VALIDTIM" */
{4, 2, 2, 2, 2, 2, 2, 2, 83, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_VALIDTIME" */
{85, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 84, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_a" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 87, 2, 2},
/* "_at" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 88, 2, 2},
/* "_att" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 89, 2, 2, 2, 2},
/* "_attr" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 90, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_attri" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 91, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_attrib" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 92, 2},
/* "_attribu" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 93, 2, 2},
/* "_attribut" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 94, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_attribute" */
{96, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 95, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_b" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 98, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ba" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 99, 2, 2, 2},
/* "_bas" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 100, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_base" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 101, 2, 2},
/* "_baset" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 102, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_baseti" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 103, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_basetim" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 104, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_basetime" */
{106, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 105, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_m" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 108, 2, 2, 2, 2, 115, 2, 2, 2, 2, 2, 2},
/* "_me" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 109, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_mem" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 110, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_memb" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 111, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_membe" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 112, 2, 2, 2, 2},
/* "_member" */
{114, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 113, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_mo" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 116, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_mod" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 117, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_mode" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 118, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_model" */
{120, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 119, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_n" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 122, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_na" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 123, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_nam" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 124, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_name" */
{126, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 125, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_s" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 128, 2, 2, 2, 2, 2},
/* "_sp" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 129, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_spa" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 130, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_spac" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 131, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_space" */
{133, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 132, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_t" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 135, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_ti" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 136, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_tim" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 137, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_time" */
{139, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 138, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_v" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 141, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_va" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 142, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_val" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 143, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_vali" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 144, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_valid" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 145, 2, 2},
/* "_validt" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 146, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_validti" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 147, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_validtim" */
{4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 148, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
/* "_validtime" */
{150, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 149, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
	};
#line 335 "dds_tmpl.rb"
	int state = 0;
	int idst = 0;
	const char *mark = NULL;
	while (state >= 0) {
		int symbol, action;
		int save;
		symbol = symbol_table[*(unsigned char *)src];
		action = action_table[state][symbol];
		switch (action) {
case 0: /* Any */
		dst[idst] = *src;
		idst++;
		state = 1; /* "Default" */
		break;
case 1: /* Copy */
		dst[idst] = *src;
		idst++;
		/* state unchanged */
		break;
case 2: /* Revert */
		memcpy(dst + idst, mark, src - mark + 1);
		idst += (src - mark + 1);
		state = 1; /* "Default" */
		break;
case 3: /* Revert:NUL */
		memcpy(dst + idst, mark, src - mark + 1);
		idst += (src - mark + 1);
		state = -1; /* :End */
		break;
case 4: /* Revert:SL */
		memcpy(dst + idst, mark, src - mark + 1);
		idst += (src - mark + 1);
		state = 0; /* :Initial */
		break;
case 5: /* Slash */
		dst[idst] = *src;
		idst++;
		state = 0; /* :Initial */
		break;
case 6: /* _ */
		mark = src;
		state = 2; /* "_" */
		break;
case 7: /* _2 */
		state = 3; /* "_2" */
		break;
case 8: /* _2D */
		state = 4; /* "_2D" */
		break;
case 9: /* _2D:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 10: /* _2D:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 11: /* _2d */
		state = 5; /* "_2d" */
		break;
case 12: /* _2d:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 13: /* _2d:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 14: /* _3 */
		state = 6; /* "_3" */
		break;
case 15: /* _3D */
		state = 7; /* "_3D" */
		break;
case 16: /* _3D:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 6, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 17: /* _3D:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 6, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 18: /* _3d */
		state = 8; /* "_3d" */
		break;
case 19: /* _3d:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 6, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 20: /* _3d:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 6, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 21: /* _A */
		state = 9; /* "_A" */
		break;
case 22: /* _AT */
		state = 10; /* "_AT" */
		break;
case 23: /* _ATT */
		state = 11; /* "_ATT" */
		break;
case 24: /* _ATTR */
		state = 12; /* "_ATTR" */
		break;
case 25: /* _ATTRI */
		state = 13; /* "_ATTRI" */
		break;
case 26: /* _ATTRIB */
		state = 14; /* "_ATTRIB" */
		break;
case 27: /* _ATTRIBU */
		state = 15; /* "_ATTRIBU" */
		break;
case 28: /* _ATTRIBUT */
		state = 16; /* "_ATTRIBUT" */
		break;
case 29: /* _ATTRIBUTE */
		state = 17; /* "_ATTRIBUTE" */
		break;
case 30: /* _ATTRIBUTE:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type2, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 31: /* _ATTRIBUTE:SL */
		memcpy(dst + idst, (char *)&def->nustype.type2, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 32: /* _B */
		state = 18; /* "_B" */
		break;
case 33: /* _BA */
		state = 19; /* "_BA" */
		break;
case 34: /* _BAS */
		state = 20; /* "_BAS" */
		break;
case 35: /* _BASE */
		state = 21; /* "_BASE" */
		break;
case 36: /* _BASET */
		state = 22; /* "_BASET" */
		break;
case 37: /* _BASETI */
		state = 23; /* "_BASETI" */
		break;
case 38: /* _BASETIM */
		state = 24; /* "_BASETIM" */
		break;
case 39: /* _BASETIME */
		state = 25; /* "_BASETIME" */
		break;
case 40: /* _BASETIME:NUL */
		if (fill_basetime == -1) {
		    memcpy(dst + idst, "\xFF.23456789ab", 12);
		    save = tmpl->b;
		    tmpl->b = idst + tmpl->rel;
		    if (save >= 0 && tmpl->b - save < 0xFF) {
			    dst[idst] = tmpl->b - save;
		    }
		} else {
		    time_to_chars(dst + idst, fill_basetime);
		}
		idst += 12;
		state = -1; /* :End */
		break;
case 41: /* _BASETIME:SL */
		if (fill_basetime == -1) {
		    memcpy(dst + idst, "\xFF.23456789ab", 12);
		    save = tmpl->b;
		    tmpl->b = idst + tmpl->rel;
		    if (save >= 0 && tmpl->b - save < 0xFF) {
			    dst[idst] = tmpl->b - save;
		    }
		} else {
		    time_to_chars(dst + idst, fill_basetime);
		}
		idst += 12;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 42: /* _M */
		state = 26; /* "_M" */
		break;
case 43: /* _ME */
		state = 27; /* "_ME" */
		break;
case 44: /* _MEM */
		state = 28; /* "_MEM" */
		break;
case 45: /* _MEMB */
		state = 29; /* "_MEMB" */
		break;
case 46: /* _MEMBE */
		state = 30; /* "_MEMBE" */
		break;
case 47: /* _MEMBER */
		state = 31; /* "_MEMBER" */
		break;
case 48: /* _MEMBER:NUL */
		memcpy(dst + idst, "\xFF.23", 4);
		save = tmpl->m;
		tmpl->m = idst + tmpl->rel;
		if (save >= 0 && tmpl->m - save < 0xFF) {
			dst[idst] = tmpl->m - save;
		}
		idst += 4;
		state = -1; /* :End */
		break;
case 49: /* _MEMBER:SL */
		memcpy(dst + idst, "\xFF.23", 4);
		save = tmpl->m;
		tmpl->m = idst + tmpl->rel;
		if (save >= 0 && tmpl->m - save < 0xFF) {
			dst[idst] = tmpl->m - save;
		}
		idst += 4;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 50: /* _MO */
		state = 32; /* "_MO" */
		break;
case 51: /* _MOD */
		state = 33; /* "_MOD" */
		break;
case 52: /* _MODE */
		state = 34; /* "_MODE" */
		break;
case 53: /* _MODEL */
		state = 35; /* "_MODEL" */
		break;
case 54: /* _MODEL:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1, 4);
		idst += 4;
		state = -1; /* :End */
		break;
case 55: /* _MODEL:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1, 4);
		idst += 4;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 56: /* _N */
		state = 36; /* "_N" */
		break;
case 57: /* _NA */
		state = 37; /* "_NA" */
		break;
case 58: /* _NAM */
		state = 38; /* "_NAM" */
		break;
case 59: /* _NAME */
		state = 39; /* "_NAME" */
		break;
case 60: /* _NAME:NUL */
		/* 4字に満たない場合詰めるため memcpy にはしない */
		idst += nusdas_snprintf(dst + idst, 5, "%Ps", def->nustype.type3);
		state = -1; /* :End */
		break;
case 61: /* _NAME:SL */
		/* 4字に満たない場合詰めるため memcpy にはしない */
		idst += nusdas_snprintf(dst + idst, 5, "%Ps", def->nustype.type3);
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 62: /* _S */
		state = 40; /* "_S" */
		break;
case 63: /* _SP */
		state = 41; /* "_SP" */
		break;
case 64: /* _SPA */
		state = 42; /* "_SPA" */
		break;
case 65: /* _SPAC */
		state = 43; /* "_SPAC" */
		break;
case 66: /* _SPACE */
		state = 44; /* "_SPACE" */
		break;
case 67: /* _SPACE:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 4);
		idst += 4;
		state = -1; /* :End */
		break;
case 68: /* _SPACE:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 4);
		idst += 4;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 69: /* _T */
		state = 45; /* "_T" */
		break;
case 70: /* _TI */
		state = 46; /* "_TI" */
		break;
case 71: /* _TIM */
		state = 47; /* "_TIM" */
		break;
case 72: /* _TIME */
		state = 48; /* "_TIME" */
		break;
case 73: /* _TIME:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type2 + 2, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 74: /* _TIME:SL */
		memcpy(dst + idst, (char *)&def->nustype.type2 + 2, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 75: /* _V */
		state = 49; /* "_V" */
		break;
case 76: /* _VA */
		state = 50; /* "_VA" */
		break;
case 77: /* _VAL */
		state = 51; /* "_VAL" */
		break;
case 78: /* _VALI */
		state = 52; /* "_VALI" */
		break;
case 79: /* _VALID */
		state = 53; /* "_VALID" */
		break;
case 80: /* _VALIDT */
		state = 54; /* "_VALIDT" */
		break;
case 81: /* _VALIDTI */
		state = 55; /* "_VALIDTI" */
		break;
case 82: /* _VALIDTIM */
		state = 56; /* "_VALIDTIM" */
		break;
case 83: /* _VALIDTIME */
		state = 57; /* "_VALIDTIME" */
		break;
case 84: /* _VALIDTIME:NUL */
		memcpy(dst + idst, "\xFF.23456789ab", 12);
		save = tmpl->v;
		tmpl->v = idst + tmpl->rel;
		if (save >= 0 && tmpl->v - save < 0xFF) {
			dst[idst] = tmpl->v - save;
		}
		idst += 12;
		state = -1; /* :End */
		break;
case 85: /* _VALIDTIME:SL */
		memcpy(dst + idst, "\xFF.23456789ab", 12);
		save = tmpl->v;
		tmpl->v = idst + tmpl->rel;
		if (save >= 0 && tmpl->v - save < 0xFF) {
			dst[idst] = tmpl->v - save;
		}
		idst += 12;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 86: /* _a */
		state = 58; /* "_a" */
		break;
case 87: /* _at */
		state = 59; /* "_at" */
		break;
case 88: /* _att */
		state = 60; /* "_att" */
		break;
case 89: /* _attr */
		state = 61; /* "_attr" */
		break;
case 90: /* _attri */
		state = 62; /* "_attri" */
		break;
case 91: /* _attrib */
		state = 63; /* "_attrib" */
		break;
case 92: /* _attribu */
		state = 64; /* "_attribu" */
		break;
case 93: /* _attribut */
		state = 65; /* "_attribut" */
		break;
case 94: /* _attribute */
		state = 66; /* "_attribute" */
		break;
case 95: /* _attribute:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type2, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 96: /* _attribute:SL */
		memcpy(dst + idst, (char *)&def->nustype.type2, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 97: /* _b */
		state = 67; /* "_b" */
		break;
case 98: /* _ba */
		state = 68; /* "_ba" */
		break;
case 99: /* _bas */
		state = 69; /* "_bas" */
		break;
case 100: /* _base */
		state = 70; /* "_base" */
		break;
case 101: /* _baset */
		state = 71; /* "_baset" */
		break;
case 102: /* _baseti */
		state = 72; /* "_baseti" */
		break;
case 103: /* _basetim */
		state = 73; /* "_basetim" */
		break;
case 104: /* _basetime */
		state = 74; /* "_basetime" */
		break;
case 105: /* _basetime:NUL */
		if (fill_basetime == -1) {
		    memcpy(dst + idst, "\xFF.23456789ab", 12);
		    save = tmpl->b;
		    tmpl->b = idst + tmpl->rel;
		    if (save >= 0 && tmpl->b - save < 0xFF) {
			    dst[idst] = tmpl->b - save;
		    }
		} else {
		    time_to_chars(dst + idst, fill_basetime);
		}
		idst += 12;
		state = -1; /* :End */
		break;
case 106: /* _basetime:SL */
		if (fill_basetime == -1) {
		    memcpy(dst + idst, "\xFF.23456789ab", 12);
		    save = tmpl->b;
		    tmpl->b = idst + tmpl->rel;
		    if (save >= 0 && tmpl->b - save < 0xFF) {
			    dst[idst] = tmpl->b - save;
		    }
		} else {
		    time_to_chars(dst + idst, fill_basetime);
		}
		idst += 12;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 107: /* _m */
		state = 75; /* "_m" */
		break;
case 108: /* _me */
		state = 76; /* "_me" */
		break;
case 109: /* _mem */
		state = 77; /* "_mem" */
		break;
case 110: /* _memb */
		state = 78; /* "_memb" */
		break;
case 111: /* _membe */
		state = 79; /* "_membe" */
		break;
case 112: /* _member */
		state = 80; /* "_member" */
		break;
case 113: /* _member:NUL */
		memcpy(dst + idst, "\xFF.23", 4);
		save = tmpl->m;
		tmpl->m = idst + tmpl->rel;
		if (save >= 0 && tmpl->m - save < 0xFF) {
			dst[idst] = tmpl->m - save;
		}
		idst += 4;
		state = -1; /* :End */
		break;
case 114: /* _member:SL */
		memcpy(dst + idst, "\xFF.23", 4);
		save = tmpl->m;
		tmpl->m = idst + tmpl->rel;
		if (save >= 0 && tmpl->m - save < 0xFF) {
			dst[idst] = tmpl->m - save;
		}
		idst += 4;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 115: /* _mo */
		state = 81; /* "_mo" */
		break;
case 116: /* _mod */
		state = 82; /* "_mod" */
		break;
case 117: /* _mode */
		state = 83; /* "_mode" */
		break;
case 118: /* _model */
		state = 84; /* "_model" */
		break;
case 119: /* _model:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1, 4);
		idst += 4;
		state = -1; /* :End */
		break;
case 120: /* _model:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1, 4);
		idst += 4;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 121: /* _n */
		state = 85; /* "_n" */
		break;
case 122: /* _na */
		state = 86; /* "_na" */
		break;
case 123: /* _nam */
		state = 87; /* "_nam" */
		break;
case 124: /* _name */
		state = 88; /* "_name" */
		break;
case 125: /* _name:NUL */
		/* 4字に満たない場合詰めるため memcpy にはしない */
		idst += nusdas_snprintf(dst + idst, 5, "%Ps", def->nustype.type3);
		state = -1; /* :End */
		break;
case 126: /* _name:SL */
		/* 4字に満たない場合詰めるため memcpy にはしない */
		idst += nusdas_snprintf(dst + idst, 5, "%Ps", def->nustype.type3);
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 127: /* _s */
		state = 89; /* "_s" */
		break;
case 128: /* _sp */
		state = 90; /* "_sp" */
		break;
case 129: /* _spa */
		state = 91; /* "_spa" */
		break;
case 130: /* _spac */
		state = 92; /* "_spac" */
		break;
case 131: /* _space */
		state = 93; /* "_space" */
		break;
case 132: /* _space:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 4);
		idst += 4;
		state = -1; /* :End */
		break;
case 133: /* _space:SL */
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 4);
		idst += 4;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 134: /* _t */
		state = 94; /* "_t" */
		break;
case 135: /* _ti */
		state = 95; /* "_ti" */
		break;
case 136: /* _tim */
		state = 96; /* "_tim" */
		break;
case 137: /* _time */
		state = 97; /* "_time" */
		break;
case 138: /* _time:NUL */
		memcpy(dst + idst, (char *)&def->nustype.type2 + 2, 2);
		idst += 2;
		state = -1; /* :End */
		break;
case 139: /* _time:SL */
		memcpy(dst + idst, (char *)&def->nustype.type2 + 2, 2);
		idst += 2;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
case 140: /* _v */
		state = 98; /* "_v" */
		break;
case 141: /* _va */
		state = 99; /* "_va" */
		break;
case 142: /* _val */
		state = 100; /* "_val" */
		break;
case 143: /* _vali */
		state = 101; /* "_vali" */
		break;
case 144: /* _valid */
		state = 102; /* "_valid" */
		break;
case 145: /* _validt */
		state = 103; /* "_validt" */
		break;
case 146: /* _validti */
		state = 104; /* "_validti" */
		break;
case 147: /* _validtim */
		state = 105; /* "_validtim" */
		break;
case 148: /* _validtime */
		state = 106; /* "_validtime" */
		break;
case 149: /* _validtime:NUL */
		memcpy(dst + idst, "\xFF.23456789ab", 12);
		save = tmpl->v;
		tmpl->v = idst + tmpl->rel;
		if (save >= 0 && tmpl->v - save < 0xFF) {
			dst[idst] = tmpl->v - save;
		}
		idst += 12;
		state = -1; /* :End */
		break;
case 150: /* _validtime:SL */
		memcpy(dst + idst, "\xFF.23456789ab", 12);
		save = tmpl->v;
		tmpl->v = idst + tmpl->rel;
		if (save >= 0 && tmpl->v - save < 0xFF) {
			dst[idst] = tmpl->v - save;
		}
		idst += 12;
		dst[idst++] = '/';
		state = 0; /* :Initial */
		break;
default:
		state = -1; /* :End */
#line 345 "dds_tmpl.rb"
		}
		/* switch (action) の終わり */
		src++;
	}
	return idst + tmpl->rel;
}

	static int
expand_size_estimate(const char *str)
{
	int	n = 0;
	while (*str) {
		if (*str == '_') {
			switch (str[1]) {
				case 'b':
				case 'B':
					n += 4;
					break;
				case 'v':
				case 'V':
					n += 3;
					break;
				default:
					n++;
			}
		} else {
			n++;
		}
		str++;
	}
	return n;
}

/** @brief ファイル名テンプレート dds_template の構築
 *
 * 情報源は定義ファイル dds->def (うち path, filename) と dds->dirname
 * である。
 * @retval 0 正常終了
 * @retval 負 エラー (定義ファイルの解読エラーおよび nus_malloc)
 * @todo エラー通知法の確定
 */
	int
nusdds_build_template(struct dds_t *dds, /**< データセット */
	N_SI4 fill_basetime) /**< -1 でなければ基準時刻に置換 */
{
	nusdef_t *def = &dds->def;
	struct dds_template *tmpl = &dds->tmpl;
	char strbuf[sizeof(def->path) + sizeof(def->filename)];
	int status;

	/** @note 定義ファイルの解読が未完了ならば完成させる. */
	if ((status = dds_endparse(dds)) != 0)
		return status;

	/** @note 既に構築が完了していれば何もしないで正常終了. */
	if (tmpl->fullpath) {
		return 0;
	}
	tmpl->b = tmpl->m = tmpl->v = -1;

	/* --- strbuf に展開前のパスを構築する. --- */
	{
		char *p;
		size_t plen;
		strcpy(strbuf, def->path);
		p = strbuf;
		p += (plen = strlen(p));
		if ((plen > 0) && (p[-1] != '/') && def->filename[0]) {
			*p++ = '/';
			*p = '\0';
		}
		if ((def->filename[0] == '\0') && (p > strbuf)
				&& (p[-1] == '/')) {
			p[-1] = '\0';
		}
		strncat(strbuf, def->filename, sizeof(strbuf));
		if ((strbuf[0] == '\007') && (strbuf[1] == '4')) {
			nusdas_snprintf(strbuf, 32, "NUSD%ys", &(def->nustype));
		} else if (strbuf[0] == '\007') {
			int backup6;
			memcpy(strbuf, (char *)&def->nustype.type1 + 6, 2);
			/* 4 字に満たない場合右寄せするため
			   memcpy を用いない */
			backup6 = strbuf[6];
			nusdas_snprintf(strbuf + 2, 5, "%4Ps",
				def->nustype.type3);
			strbuf[6] = backup6;
		}
	}
	/* --- strbuf の文字列を tmpl->fullpath に展開する --- */
	{
		const char *dirname;
		size_t r;
		size_t allocsize;
		dirname = dds->dirname ? dds->dirname : "";
		if (*dirname == '\0') {
			dirname = ".";
		}
		r = strlen(dirname);
		allocsize = r + expand_size_estimate(strbuf) + 2;
		tmpl->fullpath = nus_malloc(allocsize);
		memcpy(tmpl->fullpath, dirname, r);
		if (tmpl->fullpath[r - 1] != '/') {
			tmpl->fullpath[r] = '/';
			r++;
		}
		tmpl->rel = r;
		tmpl->pathlen = expand_keywords(tmpl->fullpath + r,
			strbuf, def, tmpl, fill_basetime);
		tmpl->fullpath[tmpl->pathlen] = '\0';
	}
	
	return 0;
}

/* @brief パス名テンプレートに埋め込まれた置換の連鎖を数える/それぞれに
 *        ついてコールバック関数を呼ぶ
 * @param fullpath パス名テンプレート文字列
 * @param start_ofs 最初の置換対象のオフセット
 * @param callback コールバック関数
 * @param substr 置換対象の部分文字列を指すポインタ
 * @param arg コールバック関数に渡す引数
 * @return 置換の個数
 */
	unsigned
dds_tmpl_backscan(char *fullpath, int start_ofs,
		void (*callback)(char *substr, void *arg),
		void *arg)
{
	unsigned ofs, back, count;
	if (start_ofs < 0)
		return 0;
	ofs = start_ofs;
	count = 0;
	while (1) {
		count++;
		back = ((unsigned char *)fullpath)[ofs];
		if (callback) callback(fullpath + ofs, arg);
		if (back == 0xFF)
			break;
		ofs -= back;
	}
	return count;
}

	void
dds_tmpl_fill_time(char *substr, void *arg)
{
	time_to_chars(substr, *(N_SI4 *)arg);
}

	void
dds_tmpl_fill_member(char *substr, void *arg)
{
	memcpy4(substr, (char *)arg);
}

	char *
nusdds_tmpl_expand(struct dds_template *tmpl, const nusdims_t *dim)
{
	char *fullpath;
	unsigned i;
	/* 長さがわかっているので chars_dup */
	fullpath = chars_dup(tmpl->fullpath, tmpl->pathlen + 1);
	if (fullpath == NULL) {
		SETERR(NUSERR_MemShort);
		return NULL;
	}
	dds_tmpl_backscan(fullpath, tmpl->b, dds_tmpl_fill_time,
			(void *)&dim->basetime);
	dds_tmpl_backscan(fullpath, tmpl->v, dds_tmpl_fill_time,
			(void *)&dim->validtime1);
	dds_tmpl_backscan(fullpath, tmpl->m, dds_tmpl_fill_member,
			(void *)&dim->member);
	/* ファイル名に空白は許さない */
	for (i = 0; i < tmpl->pathlen; i++) {
		if (fullpath[i] == ' ') {
			fullpath[i] = '_';
		}
	}
	return fullpath;
}

struct ddstmpl_scanglob_info {
	char *fullpath;
	sym4_t dimchar;
	N_UI4 *entry;
};

	void
remember(char *substr, void *arg)
{
	struct ddstmpl_scanglob_info *info = arg;
	info->entry[0] = substr - info->fullpath;
	info->entry[1] = info->dimchar;
	info->entry += 2;
}

	static int
Ui4Compare(const void *va, const void *vb)
{
	const N_UI4 *a = va;
	const N_UI4 *b = vb;
	return (*a > *b) ? 1 : (*a < *b) ? -1 : 0;
}

/** @brief テンプレートから置換指定を抜き出す
 *
 * テンプレートが含む basetime, member, validtime の置換を抜き出し、
 * オフセット順にソートして返す。
 * @return 置換数の2倍 +1 の要素を含む int の配列で,
 * retval[0] = 最初の置換位置,
 * retval[1] = 最初の置換種別 ('B', 'M', 'V' のいずれか),
 * retval[2] = 次の置換位置,
 * retval[3] = 次の置換種別, ...,
 * retval[2n] = -1 (ターミネータ)
 * のごとくである。
 * @note 返却値は nus_malloc されたポインタであるので nus_free(3) すべきである。
 */
	N_UI4 *
nusdds_tmpl_scanglob(struct dds_template *tmpl)
{
	struct ddstmpl_scanglob_info info;
	unsigned count = 0;
	unsigned i;
	N_UI4 *globtable;
	count += dds_tmpl_backscan(tmpl->fullpath, tmpl->b, NULL, NULL);
	count += dds_tmpl_backscan(tmpl->fullpath, tmpl->m, NULL, NULL);
	count += dds_tmpl_backscan(tmpl->fullpath, tmpl->v, NULL, NULL);
	/* ファイル名に空白は許さない */
	for (i = 0; i < tmpl->pathlen; i++) {
		if (tmpl->fullpath[i] == ' ') {
			tmpl->fullpath[i] = '_';
		}
	}
	info.fullpath = tmpl->fullpath;
	info.entry = globtable = nus_malloc((count * 2 + 1) * sizeof(N_UI4));
	info.dimchar = SYM4_BTLS;
	dds_tmpl_backscan(tmpl->fullpath, tmpl->b, remember, &info);
	info.dimchar = SYM4_VTLS;
	dds_tmpl_backscan(tmpl->fullpath, tmpl->v, remember, &info);
	info.dimchar = SYM4_MBLS;
	dds_tmpl_backscan(tmpl->fullpath, tmpl->m, remember, &info);
	qsort(globtable, count, 2 * sizeof(int), Ui4Compare);
	globtable[count * 2] = ~(N_UI4)0;
	return globtable;
}
