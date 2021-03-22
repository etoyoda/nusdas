/** @file
 * @brief 一番シンプルなファイルアクセス
 */

#include "config.h"
#include "nusdas.h"
# define NEED_LONG_TO_SI8
# define NEED_SI8_ADD
# define NEED_SI8_SUB
# define NEED_SI8_CMP
#include "sys_int.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_mem.h"
# define NEED_IOBUF_NEW
# define NEED_IOBUF_OUTSIDE
# define NEED_IOBUF_OUTOFFENCE
# define NEED_IOBUF_OVERLAP_ANY
#include "io_comm.h"
#include "sys_file.h"
#include "sys_err.h"
#include "internal_types.h"
#include "glb.h"
#include <sys/stat.h>
#include "iosub_flush.h"

/** @brief 未発行書き込みバッファの警告付き破棄
 *
 * io_getwbuf() して io_issue() を一度も発行していないまま
 * 他のバッファ利用メソッドに入った場合、どこまで書き出してよいか
 * わからないので破棄せざるを得ない。
 */
	INLINE int
Check_Unissued_IOB(nusio_t *io)
{
	if (io->sio.iob == NULL) {
		return 0;
	}
	if (io->sio.iob->iob_stat != IOB_MODIFIED) {
		return 0;
	}
	if (!si8_eq(io->sio.iob->iob_tail, io->sio.iob->iob_head)) {
		return 0;
	}
	nus_err((NUSERR_IO, "io_getwbuf() buffer (head=%Qd fence=%Qd) disposed",
		io->sio.iob->iob_head, io->sio.iob->iob_fence));
	return -1;
}

/** @brief バッファを破棄する
 */
	INLINE int
Free_Buffer(nusio_t *io)
{
	if (io->sio.iob == NULL) {
		return 0;
	}
	if (Check_Unissued_IOB(io) != 0) {
		return -1;
	}
	nus_free(io->sio.iob);
	io->sio.iob = NULL;
	return 0;
}

/** @brief 何が何でも欲しい大きさのバッファを用意する
 *
 * バッファがなければ割付。既存で未発行なら警告・内容破棄。
 * それが十分な大きさなら使う。小さいならバッファ破棄・再割付。
 */
	INLINE struct io_buffer_t *
Set_Buffer_Anyway(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	N_SI8 iobsize;
	size_t zsize = si8_to_size(size);
	if (io->sio.iob != NULL) {
		Check_Unissued_IOB(io);
		iobsize = si8_sub(io->sio.iob->iob_fence,
				io->sio.iob->iob_head);
		if (si8_moreeq(iobsize, size)) {
			io->sio.iob->iob_head = ofs;
			io->sio.iob->iob_fence = si8_add(ofs, iobsize);
			return io->sio.iob;
		}
		Free_Buffer(io);
	}
	io->sio.iob = iobuf_new(zsize, ofs, size);
	return io->sio.iob;
}

/** @brief 必要ならシークする */
	INLINE int
Seek_If_Needed(nusio_t *io, N_SI8 ofs, enum sio_status_t undesirable)
{
	int 	i;
	long	lofs;
	lofs = si8_to_long(ofs);
#if !(HAVE_SI8_TYPE)
	if (I8_HI(ofs)) {
		nus_err((NUSERR_Over4GiB, "file size exceeds 4GiB."));
		return -1;
	}
#endif
	if ((io->sio.pos != lofs) || (io->sio.sio_status == undesirable)) {
		i = fseek((FILE *)io->sio.fp, lofs, SEEK_SET);
#ifdef fileno
		nus_debug(("fseek(%d, %ld) => %d",
					fileno((FILE *)io->sio.fp), lofs, i));
#else
		nus_debug(("fseek(%p, %ld) => %d", io->sio.fp, lofs, i));
#endif
		if (i == -1) {
			return -1;
		}
		io->sio.pos = lofs;
	}
	return 0;
}

/** @brief stdio 版 io_load() */
	static int
sio_load(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	size_t	r, rsize;
	struct io_buffer_t *buf;
	if (!(io->comm.flags & IO_READABLE)) {
		return -1;
	}
	if (iobuf_inside_or_eof(io->sio.iob, ofs, size)) {
		nus_debug(("no need to load"));
		return 0;
	}
	if ((buf = Set_Buffer_Anyway(io, ofs, size)) == NULL) {
		return -1;
	}
	if (Seek_If_Needed(io, ofs, SIO_WRITING) != 0) {
		return -1;
	}
	rsize = si8_to_size(size);
	io->sio.sio_status = SIO_READING;
	r = fread(buf->iob_ptr, (size_t)1, rsize, (FILE *)io->sio.fp);
	nus_debug(("fread() => %zd", r));
	if (ferror((FILE *)io->sio.fp)) {
		nus_free(buf);
		io->sio.iob = NULL;
		return -1;
	}
	io->sio.pos += r;
	buf->iob_stat = IOB_FRESH;
	buf->iob_tail = si8_add(ofs, long_to_si8(r));
	buf->iob_eof = feof((FILE *)io->sio.fp);
	return 0;
}

/** @brief stdio 版 io_close()
 *
 * ファイルを閉じる.
 */
	static int
sio_close(nusio_t *io, N_SI8 totalsize, const char* filename)
{
	int	r, r1;
	int	rr = 0;
	off_t   last = -1;
	struct  stat stat_info;
	/* 以下二つは lustre flush 用だが、USE_LUSTRE_FLUSH がなくても矛盾しない */
	int	rc_flush = -1;
	struct	lustre_append la = {};
#if defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH)
	int	fd;
#endif /* defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH) */
	Free_Buffer(io);
	/* サイズチェック */
	if (totalsize) {
		r1 = fseeko((FILE *)io->sio.fp, 0, SEEK_END);
		if (r1) {
			nus_warn(("fseeko(%p) => %d : fail", io->sio.fp, r1));
		}else{
			nus_debug(("fseeko(%p) => %d : OK", io->sio.fp, r1));
			last = ftello((FILE *)io->sio.fp);
			if (-1 == last) {
				nus_warn(("ftello(%p) => %lld : fail", io->sio.fp, last));
			}else if (last == totalsize) {
				nus_debug(("ftello(%p) => %lld : OK expect size(%lld)", io->sio.fp, last, totalsize));
				/* Lustre Flush 指定の場合は、サイズに問題なければ末尾を取得する
				 * この関数は USE_LUSTRE_FLUSH がなければ何もせず0の構造体を返す */
				la = iosub_prepare_recover_stdio((FILE *)io->sio.fp, totalsize);
			}else{
				rr = nus_err((NUSERR_WR_Inconsistent, "ftello(%p) => %lld :  NG expect size(%lld)", io->sio.fp, last, totalsize));
			}
		}
		r1 = fflush((FILE *)io->sio.fp);
		if (0 != r1) {
			/* fflushに失敗した場合はwarn出力してfclose処理へ */
			nus_warn(("fflush(%p) => %d : fail", io->sio.fp, r1));
		} else {
			/* fflushに成功した場合は fileno取得へ */
			nus_debug(("fflush(%p) => %d : OK", io->sio.fp, r1));
#if defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH)
			fd = fileno((FILE *)io->sio.fp);
			if (-1 == fd) {
				/* filenoが取れない場合はwarn出力してflose処理へ */
				nus_warn(("fileno(%p) => %d : fail", io->sio.fp, fd));
			} else {
				nus_debug(("fileno(%p) => %d : OK", io->sio.fp, fd));
				iosub_fsync(fd);
				/* lustre flush 実行。成功なら0、失敗なら1、
				 * 非LustreやUSE_LUSTRE_FLUSHが無いなら2が返る */
				rc_flush = iosub_lustre_flush(fd);
			}
#endif /* defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH) */
		}
	}
	r = fclose((FILE *)io->sio.fp);
	if (r == 0) {
		nus_debug(("fclose(%p) => %d : OK", io->sio.fp, r));
	}else{
		nus_warn(("fclose(%p) => %d : fail", io->sio.fp, r));
	}
	if ( filename && totalsize && 0 == rr) {
		if (0 == rc_flush) {
			/* Lustre flush に成功しているなら尻切れ回復処置
			 * この関数はUSE_LUSTRE_FLUSHがなければ何もせず0を返す */
			rr = iosub_recover_lustre(filename, &la, 3);
		} else {
			rr = iosub_check_size(filename, totalsize);
		}
		iosub_free(&la);
	}
	nus_free(&io->sio);
	return rr;
}

/** @brief stdio 版 io_peek() */
	static void *
sio_peek(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	struct io_buffer_t *buf;
	N_SI8 ofsdiff;
	if (!(io->comm.flags & IO_READABLE)) {
		return NULL;
	}
	switch (iobuf_outside(io->sio.iob, ofs, size)) {
		case 1:
			if (sio_load(io, ofs, size) != 0) {
				return NULL;
			} else if (0 != iobuf_outside(io->sio.iob, ofs, size)) {
				return NULL;
			}
			break;
		case -1:
			return NULL;
	}
	if (io->sio.iob->iob_stat != IOB_FRESH) {
		return NULL;
	}
	buf = io->sio.iob;
	ofsdiff = si8_sub(ofs, buf->iob_head);
	return (char *)buf->iob_ptr + si8_to_size(ofsdiff);
}

/** @brief stdio 版 io_read() */
	static N_SI8
sio_read(nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf)
{
	size_t	r, rsize;
	if (!(io->comm.flags & IO_READABLE)) {
		return long_to_si8(-1);
	}
	if (iobuf_inside(io->sio.iob, ofs, size)) {
		N_SI8 headdiff = si8_sub(ofs, io->sio.iob->iob_head);
		r = si8_to_size(headdiff);
		memcpy(buf, (char *)io->sio.iob->iob_ptr + r,
				si8_to_size(size));
		return size;
	}
	if (io->sio.iob) {
		io->sio.iob->iob_stat = IOB_STALE;
	}
	if (Seek_If_Needed(io, ofs, SIO_WRITING) != 0) {
		return long_to_si8(-1L);
	}
	rsize = si8_to_size(size);
	io->sio.sio_status = SIO_READING;
	r = fread(buf, (size_t)1, rsize, (FILE *)io->sio.fp);
	nus_debug(("fread() => %zd", r));
	io->sio.pos += r;
	return long_to_si8(r);
}

/** @brief stdio 版 io_write() */
	static int
sio_write(nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf)
{
	size_t	r, rsize;
	if (!(io->comm.flags & IO_WRITABLE)) {
		return -1;
	}
	if (iobuf_overlap_any(io->sio.iob, ofs, size)) {
		switch (io->sio.iob->iob_stat) {
			case IOB_FRESH:
				io->sio.iob->iob_stat = IOB_STALE;
				break;
			case IOB_READING: /* ありえない */
			case IOB_MODIFIED:
				/* フラッシュや破棄では済まない論理破綻 */
				return -1;
			case IOB_STALE: /* なにもしなくてよい */;
		}
	} else if (io->sio.iob) {
		io->sio.iob->iob_eof = 0;
	}
	if (Seek_If_Needed(io, ofs, SIO_READING) != 0) {
		return -1;
	}
	io->sio.sio_status = SIO_WRITING;
	r = fwrite(buf, 1, rsize = si8_to_size(size), (FILE *)io->sio.fp);
	io->sio.pos += r;
	return (r == rsize) ? 0 : -1;
}

/** @brief stdio 版 io_getwbuf() */
	static void *
sio_getwbuf(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	struct io_buffer_t *iob;
	if (!(io->comm.flags & IO_WRITABLE)) {
		return NULL;
	}
	if ((iob = Set_Buffer_Anyway(io, ofs, size)) == NULL) {
		return NULL;
	}
	iob->iob_stat = IOB_MODIFIED;
	iob->iob_tail = iob->iob_head;
	iob->iob_eof = 0;
	return iob->iob_ptr;
}

/** @brief stdio 版 io_issue() */
	static int
sio_issue(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	struct io_buffer_t *iob;
	size_t headdiff, r;
	if (!(io->comm.flags & IO_WRITABLE)) {
		nus_err((NUSERR_IO, "read only file"));
		return -1;
	}
	if (iobuf_outoffence(iob = io->sio.iob, ofs, size)) {
		nus_err((NUSERR_IO, "out of buffer assigned by io_getwbuf()"));
		return -1;
	}
	if (Seek_If_Needed(io, ofs, SIO_READING) != 0) {
		return -1;
	}
	headdiff = si8_to_size(si8_sub(ofs, iob->iob_head));
	io->sio.sio_status = SIO_WRITING;
	r = fwrite((char *)iob->iob_ptr + headdiff, 1, si8_to_size(size),
			(FILE *)io->sio.fp);
#ifdef fileno
	nus_debug(("fwrite(%d, %Qd) => %lu", fileno((FILE *)io->sio.fp), size,
				(unsigned long)r));
#else
	nus_debug(("fwrite(%p, %Qd) => %lu", io->sio.fp, size,
				(unsigned long)r));
#endif
	io->sio.pos += r;
	if (r < si8_to_size(size)) {
		return -1;
	}
	io->sio.iob->iob_stat = IOB_MODIFIED;
	io->sio.iob->iob_tail = si8_add(ofs, size);
	return 0;
}

static const struct io_functab sio_methods = {
	sio_close,
	sio_load,
	sio_peek,
	sio_read,
	sio_write,
	sio_getwbuf,
	sio_issue,
};

/** @brief stdio 版 io_open() */
	nusio_t *
sio_open(const char *filename, int flags)
{
	struct iostd_t	*sio;
	N_UI4 vb;
	char *mode;
	int r;
	if (check_tape(filename)) {
		nus_warn(("fopen fail: file is on tape, %s", filename));
		return NULL;
	}
	sio = nus_malloc(sizeof(struct iostd_t));
	if (sio == NULL) {
		nus_warn(("fopen fail: lack of memory, %s", filename));
		return NULL;
	}
	sio->comm.methods = sio_methods;
	sio->comm.flags = flags & IO_READWRITE;
	switch (sio->comm.flags) {
		case IO_READWRITE:
			mode = "rb+";
			break;
		case IO_READABLE:
			mode = "rb";
			break;
		case IO_WRITABLE:
			mode = "wb";
			break;
		default:
			nus_debug(("fopen fail: invalid flag(%d) %s", sio->comm.flags, filename));
			/* 読み書きのどちらも指定されない場合 */
			nus_free(sio);
			return NULL;
	}
	sio->fp = fopen(filename, mode);
	if ((sio->fp == NULL) && (flags & IO_WRITABLE)) {
		make_dirs(filename);
		sio->fp = fopen(filename, "wb+");
	}
	if (sio->fp == NULL) {
		nus_debug(("fopen fail %s", filename));
		nus_free(sio);
		return NULL;
	}
#ifdef fileno
	nus_debug(("fopen succeed(%d) %s", fileno((FILE *)sio->fp), filename));
#else
	nus_debug(("fopen succeed(%p) %s", sio->fp, filename));
#endif
	/* 現在位置設定 */
	sio->pos = 0;
	sio->sio_status = SIO_INITIAL;
	if ((vb = GlobalConfig(io_setvbuf)) == 0) {
		nus_debug(("setvbuf skipped for %s", filename));
	} else {
		r = setvbuf(sio->fp, NULL, _IOFBF, vb * 1024);
		nus_debug(("setvbuf(%s, %PukB) => %u", filename, vb, r));
	}
	/* 自前バッファの初期化 */
	sio->iob = NULL;
	return (nusio_t *)sio;
}
