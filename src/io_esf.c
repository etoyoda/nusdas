/** @file
 * @brief ES ファイル API によるデータアクセス
 *
 */

#include "config.h"
#include "nusdas.h"

#ifdef USE_CSES

#ifdef HAVE_CSES_ESFILE_H
# include <cses/esfile.h>
#else
/* emulation by ordinary POSIX I/O with ramdisk */
# include <fcntl.h>
# include <unistd.h>
#endif

#include <sys/types.h>
#include <stdlib.h> /* for nus_malloc */
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
# define NEED_STRING_COPY
# define NEED_STRING_CAT
#include "sys_string.h"
#include "glb.h"

/*
 * emulation by ordinary POSIX I/O with ramdisk
 */
#ifndef HAVE_CSES_ESFILE_H
# define ES_SEEK_CUR SEEK_CUR
# define ES_SEEK_SET SEEK_SET
# define ES_RDWR O_RDWR
# define ES_RDONLY O_RDONLY
# define ES_WRONLY O_WRONLY
# define ES_CREAT O_CREAT
	int
es_open(const char *path, int flags, unsigned long long psize,
	unsigned long long ssize, int mode)
{
	return open(path, flags, mode);
}
	int
es_close(int esfd, int flags) {
	return close(esfd);
}
	long long
es_write(int esfd, void *buffer, unsigned long long nbytes) {
	return write(esfd, buffer, nbytes);
}
	long long
es_read(int esfd, void *buffer, unsigned long long nbytes) {
	return read(esfd, buffer, nbytes);
}
	long long
es_lseek(int esfd, offset_t offset, int whence) {
	return lseek(esfd, offset, whence);
}
#endif

/** @brief 必要な場合だけシーク動作 */
	INLINE int
Seek_If_Needed(nusio_t *io, N_SI8 ofs)
{
	offset_t oofs = si8_to_off(ofs);
#if !(HAVE_SI8_TYPE)
	if (I8_HI(ofs)) {
		nus_err((NUSERR_Over4GiB, "file size exceeds 4GiB."));
		return -1;
	}
#endif
	nus_debug(("es_lseek wanted fd=%d, ofs=%Qd eio.pos=%ld", io->eio.fd, ofs,
				(long)io->eio.pos));
	if (io->eio.pos == oofs) {
		nus_debug(("es_lseek skipped eio.fd.tell=%ld",
				(long)es_lseek(io->eio.fd, 0, ES_SEEK_CUR)));
		return 0;
	}
	io->eio.pos = es_lseek(io->eio.fd, oofs, ES_SEEK_SET);
	nus_debug(("es_lseek(%d, %Qd) => %Qd", io->eio.fd, ofs,
				off_to_si8(io->eio.pos)));
	if (io->eio.pos == (offset_t)-1) {
		return -1;
	}
	return 0;
}

/** @brief 割り付けられている場合だけバッファの破棄 */
	INLINE void
Free_Buffer(nusio_t *io)
{
	if (io->eio.iob) {
		nus_free(io->eio.iob);
		io->eio.iob = NULL;
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
	if (io->eio.iob == NULL) {
		return 0;
	}
	if (io->eio.iob->iob_stat != IOB_MODIFIED) {
		return 0;
	}
	if (!si8_eq(io->eio.iob->iob_tail, io->eio.iob->iob_head)) {
		return 0;
	}
	nus_err((NUSERR_IO, "io_getwbuf() buffer (head=%Qd fence=%Qd) disposed",
		io->eio.iob->iob_head, io->eio.iob->iob_fence));
	return -1;
}

/** @brief バッファを必要な場合だけ割り付ける */
	INLINE struct io_buffer_t *
Set_Buffer_Anyway(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	N_SI8 iobsize;
	size_t zsize = si8_to_size(size);
	if (io->eio.iob != NULL) {
		if (Check_Unissued_IOB(io) != 0) {
			return NULL;
		}
		iobsize = si8_sub(io->eio.iob->iob_fence,
				io->eio.iob->iob_head);
		if (si8_moreeq(iobsize, size)) {
			io->eio.iob->iob_head = ofs;
			io->eio.iob->iob_fence = si8_add(ofs, iobsize);
			return io->eio.iob;
		}
		Free_Buffer(io);
	}
	io->eio.iob = iobuf_new(zsize, ofs, size);
	return io->eio.iob;
}

/** @brief ES 版 io_close() */
	static int
eio_close(nusio_t *io, N_SI8 totalsize, const char* filename)
{
	int	r;
	if (io->eio.fd == 0) {
		/** 要検討 */
		return 0;
	}
	Free_Buffer(io);
	r = es_close(io->eio.fd, 0);
	nus_debug(("es_close(%d) => %d", io->eio.fd, (int)r));
	io->eio.fd = 0;
	nus_free(io);
	return 0;
}

/** @brief ES 版 io_load() */
	static int
eio_load(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	long long	r;
	offset_t	osize;
	struct io_buffer_t *buf;
	if (!(io->comm.flags & IO_READABLE)) {
		return -1;
	}
	if (iobuf_inside_or_eof(io->eio.iob, ofs, size) ) {
		nus_debug(("io_load(%d) hit %Qd...%Qd%s for %Qd:%Qd",
					io->eio.fd,
					io->eio.iob->iob_head,
					io->eio.iob->iob_tail,
					io->eio.iob->iob_eof ? "(EOF)" : "",
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
	r = es_read(io->eio.fd, buf->iob_ptr, osize);
	nus_debug(("es_read(%d, %Qd) => %ld", io->eio.fd, size, (long)r));
	if (r < 0) {
		nus_free(buf);
		return -1;
	}
	io->eio.pos += r;
	buf->iob_stat = IOB_FRESH;
	buf->iob_tail = si8_add(ofs, off_to_si8(r));
	buf->iob_eof = (r < osize);
	return 0;
}

/** @brief ES 版 io_peek() */
	static void *
eio_peek(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	struct io_buffer_t *buf;
	N_SI8 ofsdiff;
	if (!(io->comm.flags & IO_READABLE)) {
		return NULL;
	}
	switch (iobuf_outside(io->eio.iob, ofs, size)) {
		case 1:
			if (eio_load(io, ofs, size) != 0) {
				return NULL;
			} else if (0 != iobuf_outside(io->eio.iob, ofs, size)) {
				return NULL;
			}
			break;
		case -1:
			return NULL;
	}
	buf = io->eio.iob;
	ofsdiff = si8_sub(ofs, buf->iob_head);
	return (char *)buf->iob_ptr + si8_to_size(ofsdiff);
}

/** @brief ES 版 io_read() */
	static N_SI8
eio_read(nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf)
{
	offset_t	r;
	if (!(io->comm.flags & IO_READABLE)) {
		return long_to_si8(-1L);
	}
	if (iobuf_inside(io->eio.iob, ofs, size)) {
		size_t zdiff;
		zdiff = si8_to_size(si8_sub(ofs, io->eio.iob->iob_head));
		memcpy(buf, (char *)io->eio.iob->iob_ptr + zdiff,
				si8_to_size(size));
		return size;
	}
	if (Seek_If_Needed(io, ofs) != 0) {
		return long_to_si8(-1L);
	}
	r = es_read(io->eio.fd, buf, si8_to_off(size));
	nus_debug(("es_read(%d, %Qd) => %d", io->eio.fd, size, (int)r));
	if (r > 0) {
		io->eio.pos += r;
	}
	return off_to_si8(r);
}

/** @brief ES 版 io_write() */
	static int
eio_write(nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf)
{
	long long	r;
	offset_t	osize;
	if (!(io->comm.flags & IO_WRITABLE)) {
		return nus_err((NUSERR_ReadOnly, "io has no IO_WRITABLE"));
	}
	if (iobuf_overlap_any(io->eio.iob, ofs, size)) {
		switch (io->eio.iob->iob_stat) {
			case IOB_FRESH:
				io->eio.iob->iob_stat = IOB_STALE;
				break;
			case IOB_READING: /* ありえない */
				return nus_err((NUSERR_IO, "bug"));
			case IOB_MODIFIED:
				/* フラッシュや破棄では済まない論理破綻 */
				return nus_err((NUSERR_IO,
					"write while writing async"));
			case IOB_STALE: /* なにもしなくてよい */;
		}
	} else if (io->eio.iob) {
		io->eio.iob->iob_eof = 0;
	}
	if (Seek_If_Needed(io, ofs) != 0) {
		return -1;
	}
	r = es_write(io->eio.fd, buf, osize = si8_to_off(size));
	if (r < 0) {
		return nus_err((NUSERR_IO, "es_write(%d, %Qd) => %d (%s)",
				io->eio.fd, size, (int)r, strerror(errno)));
	}
	nus_debug(("es_write(%d, %Qd) => %d", io->eio.fd, size, (int)r));
	io->eio.pos += r;
	if (r == osize) {
		return 0;
	}
	buf = (char *)buf + r;
	osize -= r;
	r = es_write(io->eio.fd, buf, osize);
	if (r < 0) {
		return nus_err((NUSERR_IO, "es_write(%d, %Qd) => %d (%s)",
				io->eio.fd, size, (int)r, strerror(errno)));
	}
	io->eio.pos += r;
	if (r == osize) {
		return 0;
	}
	return nus_err((NUSERR_IO,
			"write(%Qd) wrote %d", size, (int)r));
}

/** @brief ES 版 io_getwbuf() */
	static void *
eio_getwbuf(nusio_t *io, N_SI8 ofs, N_SI8 size)
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

/** @brief ES 版 io_issue() */
	static int
eio_issue(nusio_t *io, N_SI8 ofs, N_SI8 size)
{
	struct io_buffer_t *iob;
	size_t headdiff, zsize;
	ssize_t r;
	NUSPROF_BUF_DECL;
	if (!(io->comm.flags & IO_WRITABLE)) {
		nus_err((NUSERR_IO, "read only file"));
		return -1;
	}
	if (iobuf_outoffence(iob = io->eio.iob, ofs, size)) {
		nus_err((NUSERR_IO,
			"eio_issue(%Qd, %Qd) out of io_getwbuf()",
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
	r = es_write(io->eio.fd, (char *)iob->iob_ptr + headdiff, zsize);
	NUSPROF_RESTORE;
	nus_debug(("es_write(%d, %Qd) => %d", io->eio.fd, size, (int)r));
	if (r >= 0) {
		io->eio.pos += r;
	}
	if (r < (ssize_t)zsize) {
		nus_err((NUSERR_IO, "write error"));
		return -1;
	}
	iob->iob_stat = IOB_MODIFIED;
	iob->iob_tail = si8_add(ofs, size);
	return 0;
}

static const struct io_functab eio_methods = {
	eio_close,
	eio_load,
	eio_peek,
	eio_read,
	eio_write,
	eio_getwbuf,
	eio_issue
};

#ifndef DEFAULT_CSES_DEVICE
# ifdef HAVE_CSES_ESFILE_H
#  define DEFAULT_CSES_DEVICE "es_dev"
# else
#  define DEFAULT_CSES_DEVICE "/dev/shm"
# endif
#endif

/** @brief ES デバイスのパス文字列を得る */
	static void
eio_devpath(char *buf, size_t bufsiz)
{
	const char *ev;
	string_copy(buf, "/", bufsiz);
	ev = getenv("CSES_DEVICE_NAME");
	if ((ev == NULL) || (ev[0] == '\0')) {
		ev = DEFAULT_CSES_DEVICE;
	}
	if (ev[0] == '/') { buf[0] = '\0'; }
	string_cat(buf, ev, bufsiz);
	if (buf[strlen(buf) - 1] != '/') { string_cat(buf, "/", bufsiz); }
}

/** @brief ES 版 io_open() */
	nusio_t *
eio_open(const char *filename, int flags)
{
	struct ioesf_t		*eio;
	int			oflags;
	unsigned long long	psize, ssize;
	const char		*leaf;
	if (check_tape(filename)) {
		return NULL;
	}
	eio = nus_malloc(sizeof(struct ioesf_t));
	if (eio == NULL) {
		return NULL;
	}
	eio->comm.methods = eio_methods;
	eio->comm.flags = flags & IO_READWRITE;
	switch (eio->comm.flags) {
		case IO_READWRITE:
			oflags = ES_RDWR | ES_CREAT;
			break;
		case IO_WRITABLE:
			oflags = ES_WRONLY | ES_CREAT;
			break;
		case IO_READABLE:
			oflags = ES_RDONLY;
			break;
		default:
			nus_free(eio);
			return NULL;
	}
	/* rewriting pathname */
	eio_devpath(eio->fnam, sizeof(eio->fnam));
	leaf = strrchr(filename, '/');
	if (leaf == NULL) {
	  leaf = filename;
	} else {
	  leaf++;
	}
	string_cat(eio->fnam, leaf, sizeof(eio->fnam));
	nus_warn(("CSES FILENAME %s", eio->fnam));
	psize = (unsigned long long)GlobalConfig(eio_psize) << 20;
	ssize = (unsigned long long)GlobalConfig(eio_ssize) << 20;
	eio->fd = es_open(eio->fnam, oflags, psize, ssize, 0644);
	/* perhaps do something and retry if ENOENT and IO_WRITABLE */
	nus_debug(("es_open(%s, psize=%luMB, ssize=%luMB, mode=0%o) => %d",
		eio->fnam,
		(unsigned long)(psize >> 20),
		(unsigned long)(ssize >> 20), oflags, eio->fd
		));
	if (eio->fd < 0) {
		nus_free(eio);
		return NULL;
	}
	eio->pos = 0;
	eio->iob = NULL;
	return (nusio_t *)eio;
}

#else
/* fallback if configured with --disable-cses */
# include <sys/types.h>
# include <stddef.h> /* for sys_err.h */
# include "io_comm.h"
# include "sys_err.h"

	nusio_t	*
eio_open(const char *filename, int flags)
{
	nus_warn(("eio_open: CSES disabled - pio_open used instead"));
	return pio_open(filename, flags);
}

#endif
/* USE_CSES */
