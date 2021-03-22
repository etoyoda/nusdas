#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_kwd.h"
# define NEED_PEEK_N_UI4
#include "sys_endian.h"
# define NEED_LONG_TO_SI8
# define NEED_SI8_ADDTO_UI4
# define NEED_UI8_RSHIFT
# define NEED_UI8_LSHIFT
# define NEED_SI8_CMP
# define NEED_SI8_ADD
# define NEED_SI8_SUB
# define NEED_MAKE_SI8
#include "sys_int.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h> /* for nus_malloc */
#include <string.h> /* for memcpy */
#include "sys_err.h"
#include "sys_mem.h"
#include "io_comm.h"
#include "dfile.h"
#include "ndf.h"
#include "glb.h"

	void
nusnsf_ini(struct nsf_t *param,
		union nusio_t *io, N_SI4 reclplus, N_UI4 pc_rbuffer)
{
	nus_debug(("seqscan_ini"));
	param->sf_io = io;
	param->sf_reclplus = reclplus;
	param->sf_pos = long_to_si8(0);
	param->sf_base_prev = param->sf_base = long_to_si8(-1);
	param->sf_rbuffer = pc_rbuffer;
	param->sf_size = long_to_si8(1 << (param->sf_rbuffer + 1));
	param->sf_rec = NULL;
	param->sf_recl = 0;
	param->sf_align = GlobalConfig(pc_alignment);
	param->sf_alibuf = NULL;
	param->sf_alibuflen = 0;
}

	int
nusnsf_close(struct nsf_t *sf, N_SI8 totalsize, const char* filename)
{
	int r;
	r = io_close(sf->sf_io, totalsize, filename);
	if (sf->sf_alibuf) {
		nus_free(sf->sf_alibuf);
	}
	return r;
}

	int
nusnsf_rewind(struct nsf_t *sf)
{
	sf->sf_pos = long_to_si8(0);
	sf->sf_recl = 0;
	sf->sf_base_prev = long_to_si8(-1);
	return 0;
}

/** @brief 指定位置から最低1ブロック読み取り
 * @note バッファリングは2ブロック単位で行われ、
 * バッファの前半に指定位置が入るので最低1ブロックがロードされる。
 */
	INLINE void
PointLoad(struct nsf_t *param, N_UI8 pos)
{
	if (param->sf_rbuffer == 0) {
		return;
	} else {
		/* レコード先頭位置をブロック長の倍数に切り捨てる */
		param->sf_base = ui8_rshift(pos, param->sf_rbuffer);
		param->sf_base = ui8_lshift(param->sf_base, param->sf_rbuffer);
		/* バッファ位置が違うときは2ブロック読む
		 * バッファ位置さえ合っていれば少なくとも最初の要求位置から
		 * 1ブロックは読まれている
		 * */
		if (! si8_eq(param->sf_base, param->sf_base_prev)) {
			io_load(param->sf_io, param->sf_base, param->sf_size);
			param->sf_base_prev = param->sf_base;
		}
	}
}

/** @brief 整端バッファ確保
 *
 * 整端バッファを @p recl バイトだけ確保する。
 * param->sf_rec から @p copysize バイトコピーし、
 * param->sf_rec を整端バッファで差し替える。
 *
 * @retval NUSERR_MemShort nus_malloc/nus_realloc 失敗
 * @retval 0 整端バッファは不要
 * @retval 1 整端バッファが必要、かつ確保された。
 */
	INLINE int
Align_Buffer(struct nsf_t *param, N_UI4 recl)
{
	/** 整端設定 param->sf_alloc が零または
	 * バイトオフセット param->sf_pos が整端ならばなにもしない。 */
	if (param->sf_align == 0 ||
			((param->sf_align & I8_LO(param->sf_pos)) == 0)) {
		return 0;
	}
	if (param->sf_alibuf == NULL) {
		param->sf_alibuf = nus_malloc(recl);
	} else if (recl > param->sf_alibuflen) {
		param->sf_alibuf = nus_realloc(param->sf_alibuf, recl);
	}
	if (param->sf_alibuf == NULL) {
		return nus_err((NUSERR_MemShort, "mem short"));
	}
	param->sf_alibuflen = recl;
	return 1;
}

/** @brief ファイル走査 (レコードヘッダだけを使う場合)
 * @note 直前には nusnsf_rewind() または nusnsf_read_head() が
 * 呼ばれていることを想定している
 * @note 戻った後 sf_rec はレコード先頭少なくとも 20 バイトを指している。
 */
	int
nusnsf_read_head(struct nsf_t *param)
{
	/* sf_pos は前のレコードの先頭を指しているので sf_recl 進める */
	si8_addto_ui4(param->sf_pos, param->sf_recl);
	PointLoad(param, param->sf_pos);
	param->sf_rec = io_peek(param->sf_io, param->sf_pos, long_to_si8(20));
	if (param->sf_rec == NULL) {
		nus_debug(("seqscan got EOF pos=%Qd", param->sf_pos));
		/* EOF 到達 */
		return -1;
	}
	param->sf_recl = PEEK_N_UI4(param->sf_rec) + param->sf_reclplus;
	switch (Align_Buffer(param, param->sf_recl)) {
		case 1:
			break;
		case 0:
			return 0;
		case NUSERR_MemShort:
		default:
			return -1;
	}
	memcpy(param->sf_alibuf, param->sf_rec, 20u);
	param->sf_rec = param->sf_alibuf;
	nus_debug(("seqscan %Ps", param->sf_rec[1]));
	return 0;
}

/** @brief 指定レコード長がロードされていることを確実にする
 *
 * 位置 sf->sf_pos から recl バイトだけ読み込ませる。
 * 直前に PointRoad(sf->sf_pos) されていることを仮定している
 */
	INLINE void
RangeLoad(struct nsf_t *sf, N_UI8 recl)
{
	if (sf->sf_rbuffer == 0) {
		return;
	} else {
		N_SI8 end_of_rec = si8_add(sf->sf_pos, recl);
		N_SI8 end_of_buffer = si8_add(sf->sf_base, sf->sf_size);
		if (si8_lessthan(end_of_buffer, end_of_rec)) {
			/** バッファ長が不足する場合, 通常より長いバッファを用いる */
			io_load(sf->sf_io, sf->sf_base,
				si8_sub(end_of_rec, sf->sf_base));
		}
	}
}

/** @brief 直前に nusnsf_read_head() で走査したレコードを全部読む
 * */
	int
nusnsf_read_full(struct nsf_t *param)
{
	void *rec;
	RangeLoad(param, make_si8(0, param->sf_recl));
	rec = io_peek(param->sf_io, param->sf_pos,
			long_to_si8(param->sf_recl));
	if (rec == NULL) return -1;
	if (param->sf_rec == param->sf_alibuf) {
		memcpy(param->sf_rec, rec, param->sf_recl);
	} else {
		param->sf_rec = rec;
	}
	return rec ? 0 : -1;
}

/** @brief ファイル走査 (レコード全体を使い、nus_malloc したバッファで返す)
 * @note この関数はファイル先頭部の読みだしで利用される
 */
	void *
nusnsf_read_rec(struct nsf_t *param)
{
	int r;
	void *buf;
	if ((r = nusnsf_read_head(param)) != 0) {
		return NULL;
	}
	if ((r = nusnsf_read_full(param)) != 0) {
		return NULL;
	}
	buf = nus_malloc(param->sf_recl);
	if (buf == NULL) {
		nus_err((NUSERR_MemShort, "memory short"));
		return NULL;
	}
	memcpy(buf, param->sf_rec, param->sf_recl);
	return buf;
}

/** @brief 指定位置・指定長さのレコード読みだし
 */
	void *
nusnsf_read_at(struct nsf_t *sf, N_SI8 pos, N_SI8 size)
{
	void *rec;
	N_UI4 recl;
	PointLoad(sf, sf->sf_pos = pos);
	RangeLoad(sf, size);
	if ((rec = io_peek(sf->sf_io, pos, long_to_si8(4))) == NULL) {
		nus_err((NUSERR_IO, "io_peek => NULL"));
		return NULL;
	}
	recl = PEEK_N_UI4(rec) + sf->sf_reclplus;
	if ((rec = io_peek(sf->sf_io, pos, make_si8(0, recl))) == NULL) {
		nus_err((NUSERR_IO, "io_peek => NULL"));
		return NULL;
	}
	switch (Align_Buffer(sf, recl)) {
		case 1:
			break;
		case 0:
			return rec;
		case NUSERR_MemShort:
		default:
			return NULL;
	}
	memcpy(sf->sf_alibuf, rec, recl);
	return sf->sf_alibuf;
}

	int
nusnsf_read_recl(struct nsf_t *sf, N_SI8 pos, N_UI4 *recl)
{
	void *rec;
	if ((rec = io_peek(sf->sf_io, pos, long_to_si8(4))) == NULL) {
		nus_err((NUSERR_IO, "io_peek => NULL"));
		return -1;
	}
	*recl = PEEK_N_UI4(rec) + sf->sf_reclplus;
	return 0;
}

	int
nusnsf_write_seq(struct nsf_t *sf, N_UI4 size, void *data)
{
	int r;
	si8_addto_ui4(sf->sf_pos, sf->sf_recl);
	r = io_write(sf->sf_io, sf->sf_pos, make_si8(0, size), data);
	sf->sf_recl = size;
	return r;
}

	int
nusnsf_write(struct nsf_t *sf, N_SI8 pos, N_UI4 size, void *data)
{
	N_UI4 vrecl;
	vrecl = PEEK_N_UI4((N_UI4 *)data + 2);
	if (vrecl < size) {
		memset((char *)data + vrecl + 4, '\0', size - vrecl - 8);
	}
	return io_write(sf->sf_io, pos, make_si8(0, size), data);
}

	void *
nusnsf_getwbuf(struct nsf_t *sf, N_SI8 pos, N_UI4 size)
{
	return io_getwbuf(sf->sf_io, pos, make_si8(0, size));
}

	int
nusnsf_issue(struct nsf_t *sf, N_SI8 pos, N_UI4 size)
{
	return io_issue(sf->sf_io, pos, make_si8(0, size));
}

	void *
nusnsf_read_before(struct nsf_t *sf, N_SI8 pos)
{
	/* NuSDaS データファイルは少なくとも
	 * (NUSD 30 + CNTL 45 + INDX 5 + END 7) = 87 語 = 348 バイトある。
	 */
	N_SI8 hacksize = long_to_si8(348);
	N_SI8 four = long_to_si8(4);
	N_SI8 recl8, eorhack, eormark;
	void *rec;
	N_UI4 recl;
	eorhack = si8_sub(pos, hacksize);
	PointLoad(sf, eorhack);
	RangeLoad(sf, hacksize);
	eormark = si8_sub(pos, four);
	if ((rec = io_peek(sf->sf_io, eormark, four)) == NULL) {
		nus_warn(("io_peek fails"));
		return NULL;
	}
	recl = PEEK_N_UI4(rec) + sf->sf_reclplus;
	recl8 = make_si8(0, recl);
	if (si8_morethan(recl8, pos)) {
		nus_warn(("broken record size %Qd > position %Qd", recl8, pos));
		return NULL;
	}
	sf->sf_pos = si8_sub(pos, recl8);
	PointLoad(sf, sf->sf_pos);
	RangeLoad(sf, recl8);
	sf->sf_recl = recl;
	sf->sf_rec = io_peek(sf->sf_io, sf->sf_pos, recl8);
	if (sf->sf_rec == NULL) {
		return NULL;
	}
	switch(Align_Buffer(sf, recl)) {
		case 1:
			break;
		case 0:
			return sf->sf_rec;
			break;
		case NUSERR_MemShort:
		return NULL;
	}
	memcpy(sf->sf_alibuf, sf->sf_rec, recl);
	sf->sf_rec = sf->sf_alibuf;
	return sf->sf_alibuf;
}
