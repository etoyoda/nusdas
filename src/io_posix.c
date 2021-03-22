/** @file
 * @brief POSIX ファイル API (システムコール) によるファイルアクセス
 *
 */

#include "config.h"
#include "nusdas.h"
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h> /* for nus_malloc */
#include <unistd.h>
#include <errno.h>
#include <string.h> /* for memcpy */
#include "internal_types.h"
# define NEED_LONG_TO_SI8
# define NEED_SI8_ADD
# define NEED_SI8_SUB
# define NEED_SI8_SUBFROM
# define NEED_SI8_CMP
#include "sys_int.h"
#include "sys_mem.h"
# define NEED_IOBUF_NEW
# define NEED_IOBUF_OUTSIDE
# define NEED_IOBUF_OUTOFFENCE
# define NEED_IOBUF_OVERLAP_ANY
#include "io_comm.h"
#include "sys_err.h"
#include "sys_file.h"
#include "sys_time.h"
#include "glb.h"
#include <sys/stat.h>
#include "iosub_flush.h"

/** @brief 必要な場合だけシーク動作 */
	INLINE int
Seek_If_Needed(nusio_t *io, N_SI8 ofs)
{
	off_t oofs = si8_to_off(ofs);
#if !(HAVE_SI8_TYPE)
	if (I8_HI(ofs)) {
		nus_err((NUSERR_Over4GiB, "file size exceeds 4GiB."));
		return -1;
	}
#endif
	nus_debug(("lseek wanted fd=%d, ofs=%Qd pio.pos=%ld", io->pio.fd, ofs,
				(long)io->pio.pos));
	if (io->pio.pos == oofs) {
		nus_debug(("lseek skipped pio.fd.tell=%ld",
				(long)lseek(io->pio.fd, 0, SEEK_CUR)));
		return 0;
	}
	io->pio.pos = lseek(io->pio.fd, oofs, SEEK_SET);
	nus_debug(("lseek(%d, %Qd) => %Qd", io->pio.fd, ofs,
				off_to_si8(io->pio.pos)));
	if (io->pio.pos == (off_t)-1) {
		return -1;
	}
	return 0;
}

/** @brief 割り付けられている場合だけバッファの破棄 */
	INLINE void
Free_Buffer(nusio_t *io)
{
	if (io->pio.iob) {
		nus_free(io->pio.iob);
		io->pio.iob = NULL;
	}
}

/** @brief 未発行書き込みバッファの警告付き破棄
 *
 * io_getwbuf() して io_issue() を一度も発行していないまま
 * 他のバッファ利用メソッドに入った場合、どこまで書き出してよいか
 * わからないので破棄せざるを得ない。
 */
	INLINE int
Check_Unissued_IOB(nusio_t *io)
{
	if (io->pio.iob == NULL) {
		return 0;
	}
	if (io->pio.iob->iob_stat != IOB_MODIFIED) {
		return 0;
	}
	if (!si8_eq(io->pio.iob->iob_tail, io->pio.iob->iob_head)) {
		return 0;
	}
	nus_err((NUSERR_IO, "io_getwbuf() buffer (head=%Qd fence=%Qd) disposed",
		io->pio.iob->iob_head, io->pio.iob->iob_fence));
	return -1;
}

/** @brief バッファを必要な場合だけ割り付ける */
	INLINE struct io_buffer_t *
Set_Buffer_Anyway(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	N_SI8 iobsize;
	size_t zsize = si8_to_size(size);
	if (io->pio.iob != NULL) {
		if (Check_Unissued_IOB(io) != 0) {
			return NULL;
		}
		iobsize = si8_sub(io->pio.iob->iob_fence,
				io->pio.iob->iob_head);
		if (si8_moreeq(iobsize, size)) {
			io->pio.iob->iob_head = ofs;
			io->pio.iob->iob_fence = si8_add(ofs, iobsize);
			return io->pio.iob;
		}
		Free_Buffer(io);
	}
	io->pio.iob = iobuf_new(zsize, ofs, size);
	return io->pio.iob;
}

/** @brief POSIX 版 io_close() */
	static int
pio_close(nusio_t *io, N_SI8 totalsize, const char* filename)
{
	int	r;
	int	rr = 0;
	off_t	last = -1;
	struct  stat stat_info;
	int	rc_flush = -1;
	struct	lustre_append la = {};
	if (io->pio.fd == 0) {
		/** 要検討 */
		return 0;
	}
	Free_Buffer(io);
	/** サイズチェック */
	if (totalsize) {
		last = lseek(io->pio.fd, 0, SEEK_END);
		if (-1 == last){
			nus_warn(("lseek(%d) => %lld : fail", io->pio.fd, last));
		}else if (last == totalsize) {
			nus_debug(("lseek(%d) => %lld : OK expect size(%lld)", io->pio.fd, last, totalsize));
			/* Lustre Flush 指定の場合は、サイズに問題なければ末尾を取得する
			 * この関数は USE_LUSTRE_FLUSH がなければ何もせず0の構造体を返す */
			la = iosub_prepare_recover_posix(io->pio.fd, totalsize);
		}else{
			rr = nus_err((NUSERR_WR_Inconsistent, "lseek(%d) => %lld : NG expect size(%lld)", io->pio.fd, last, totalsize));
		}
#if defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH)
		iosub_fsync(io->pio.fd);
		/* lustre flush 実行。成功なら0、失敗なら1、
		 * 非LustreやUSE_LUSTRE_FLUSHが無いなら2が返る */
		rc_flush = iosub_lustre_flush(io->pio.fd);
#endif /* defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH) */
	}
	r = close(io->pio.fd);
	if (0 == r) {
		nus_debug(("close(%d) => %d : OK", io->pio.fd, r));
	}else{
		nus_warn(("close(%d) => %d : fail", io->pio.fd, r));
	}
	io->pio.fd = 0;
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
	nus_free(io);
	return rr;
}

/** @brief POSIX 版 io_load() */
	static int
pio_load(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	off_t	r, osize;
	struct io_buffer_t *buf;
	if (!(io->comm.flags & IO_READABLE)) {
		return -1;
	}
	if (iobuf_inside_or_eof(io->pio.iob, ofs, size) ) {
		nus_debug(("io_load(%d) hit %Qd...%Qd%s for %Qd:%Qd",
					io->pio.fd,
					io->pio.iob->iob_head,
					io->pio.iob->iob_tail,
					io->pio.iob->iob_eof ? "(EOF)" : "",
					ofs, size));
		return 0;
	}
	if ((buf = Set_Buffer_Anyway(io, ofs, size)) == NULL) {
		return -1;
	}
	if (Seek_If_Needed(io, ofs) != 0) {
		return -1;
	}
	osize =	si8_to_off(size);
	r = read(io->pio.fd, buf->iob_ptr, osize);
	nus_debug(("read(%d, %Qd) => %ld", io->pio.fd, size, (long)r));
	if (r < 0) {
		nus_free(buf);
		return -1;
	}
	io->pio.pos += r;
	buf->iob_stat = IOB_FRESH;
	buf->iob_tail = si8_add(ofs, off_to_si8(r));
	buf->iob_eof = (r < osize);
	return 0;
}

/** @brief POSIX 版 io_peek() */
	static void *
pio_peek(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	struct io_buffer_t *buf;
	N_SI8 ofsdiff;
	if (!(io->comm.flags & IO_READABLE)) {
		return NULL;
	}
	switch (iobuf_outside(io->pio.iob, ofs, size)) {
		case 1:
			if (pio_load(io, ofs, size) != 0) {
				return NULL;
			} else if (0 != iobuf_outside(io->pio.iob, ofs, size)) {
				return NULL;
			}
			break;
		case -1:
			return NULL;
	}
	buf = io->pio.iob;
	ofsdiff = si8_sub(ofs, buf->iob_head);
	return (char *)buf->iob_ptr + si8_to_size(ofsdiff);
}

/** @brief POSIX 版 io_read() */
	static N_SI8
pio_read(nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf)
{
	off_t	r;
	if (!(io->comm.flags & IO_READABLE)) {
		return long_to_si8(-1);
	}
	if (iobuf_inside(io->pio.iob, ofs, size)) {
		size_t zdiff;
		zdiff = si8_to_size(si8_sub(ofs, io->pio.iob->iob_head));
		memcpy(buf, (char *)io->pio.iob->iob_ptr + zdiff,
				si8_to_size(size));
		return size;
	}
	if (Seek_If_Needed(io, ofs) != 0) {
		return long_to_si8(-1);
	}
	r = read(io->pio.fd, buf, si8_to_off(size));
	nus_debug(("read(%d, %Qd) => %d", io->pio.fd, size, (int)r));
	if (r > 0) {
		io->pio.pos += r;
	}
	return off_to_si8(r);
}

/** @brief POSIX 版 io_write() */
	static int
pio_write(nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf)
{
	off_t	r, osize;
	if (!(io->comm.flags & IO_WRITABLE)) {
		return nus_err((NUSERR_ReadOnly, "io has no IO_WRITABLE"));
	}
	if (iobuf_overlap_any(io->pio.iob, ofs, size)) {
		switch (io->pio.iob->iob_stat) {
			case IOB_FRESH:
				io->pio.iob->iob_stat = IOB_STALE;
				break;
			case IOB_READING: /* ありえない */
				return nus_err((NUSERR_IO, "bug"));
			case IOB_MODIFIED:
				/* フラッシュや破棄では済まない論理破綻 */
				return nus_err((NUSERR_IO,
					"write while writing async"));
			case IOB_STALE: /* なにもしなくてよい */;
		}
	} else if (io->pio.iob) {
		io->pio.iob->iob_eof = 0;
	}
	if (Seek_If_Needed(io, ofs) != 0) {
		return -1;
	}
	r = write(io->pio.fd, buf, osize = si8_to_off(size));
	if (r < 0) {
		return nus_err((NUSERR_IO, "write(%d, %Qd) => %d (%s)",
				io->pio.fd, size, (int)r, strerror(errno)));
	}
	nus_debug(("write(%d, %Qd) => %d", io->pio.fd, size, (int)r));
	io->pio.pos += r;
	if (r == osize) {
		return 0;
	}
	buf = (char *)buf + r;
	osize -= r;
	r = write(io->pio.fd, buf, osize);
	if (r < 0) {
		return nus_err((NUSERR_IO, "write(%d, %Qd) => %d (%s)",
				io->pio.fd, size, (int)r, strerror(errno)));
	}
	io->pio.pos += r;
	if (r == osize) {
		return 0;
	}
	return nus_err((NUSERR_IO,
			"write(%Qd) wrote %d", size, (int)r));
}

/** @brief POSIX 版 io_getwbuf() */
	static void *
pio_getwbuf(nusio_t *io, N_SI8 ofs, N_SI8 size)
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

/** @brief POSIX 版 io_issue() */
	static int
pio_issue(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	struct io_buffer_t *iob;
	size_t headdiff, zsize;
	ssize_t r;
	NUSPROF_BUF_DECL;
	if (!(io->comm.flags & IO_WRITABLE)) {
		nus_err((NUSERR_IO, "read only file"));
		return -1;
	}
	if (iobuf_outoffence(iob = io->pio.iob, ofs, size)) {
		nus_err((NUSERR_IO,
			"pio_issue(%Qd, %Qd) out of io_getwbuf()",
			ofs, size));
		return -1;
	}
	if (Seek_If_Needed(io, ofs) != 0) {
		return -1;
	}
	headdiff = si8_to_size(si8_sub(ofs, iob->iob_head));
	zsize = si8_to_size(size);
	NUSPROF_BACKUP;
	NUSPROF_MARK(NP_SYSWRITE);
	r = write(io->pio.fd, (char *)iob->iob_ptr + headdiff, zsize);
	NUSPROF_RESTORE;
	nus_debug(("write(%d, %Qd) => %d", io->pio.fd, size, (int)r));
	if (r >= 0) {
		io->pio.pos += r;
	}
	if (r < (ssize_t)zsize) {
		nus_err((NUSERR_IO, "write error"));
		return -1;
	}
	iob->iob_stat = IOB_MODIFIED;
	iob->iob_tail = si8_add(ofs, size);
	return 0;
}

static const struct io_functab pio_methods = {
	pio_close,
	pio_load,
	pio_peek,
	pio_read,
	pio_write,
	pio_getwbuf,
	pio_issue
};

/** @brief POSIX 版 io_open() */
	nusio_t *
pio_open(const char *filename, int flags)
{
	struct ioposix_t *pio;
	int	oflags;
	if (check_tape(filename)) {
		return NULL;
	}
	pio = nus_malloc(sizeof(struct ioposix_t));
	if (pio == NULL) {
		return NULL;
	}
	pio->comm.methods = pio_methods;
	pio->comm.flags = flags & IO_READWRITE;
	switch (pio->comm.flags) {
		case IO_READWRITE:
			oflags = O_RDWR | O_CREAT;
			break;
		case IO_WRITABLE:
			oflags = O_WRONLY | O_CREAT;
			break;
		case IO_READABLE:
			oflags = O_RDONLY;
			break;
		default:
			nus_free(pio);
			return NULL;
	}
	pio->fd = open(filename, oflags, 0644);
	if ((pio->fd < 0) && (errno == ENOENT) && (flags & IO_WRITABLE)) {
		make_dirs(filename);
		pio->fd = open(filename, oflags, 0644);
	}
	nus_debug(("open(%s, 0%o, 0755) => %d", filename, oflags, pio->fd));
	if (pio->fd < 0) {
		nus_free(pio);
		return NULL;
	}
	pio->pos = 0;
	pio->iob = NULL;
	return (nusio_t *)pio;
}
