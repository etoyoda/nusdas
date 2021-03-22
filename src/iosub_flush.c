/** @file
 * @brief fsync and lustre flush
 */

#include "config.h"
#include <stddef.h>
#include "sys_err.h"
#include "iosub_flush.h"
#include <sys/stat.h>
#include <stdlib.h>
#ifdef USE_LUSTRE_FLUSH
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <lustre/lustre_user.h>
#endif /* USE_LUSTRE_FLUSH */

	static int
iosub_test_mode()
{
	static int test_mode = -1;
	char* test_str;
	if (0 == test_mode || 1 == test_mode) return test_mode;
	test_str = getenv("NWP_LUSTRE_LACK_TEST");
	if (NULL == test_str) return test_mode = 0;
	return test_mode = 1 == atoi(test_str) ? 1: 0;
}

	static int
iosub_is_lustre(const int fd)
{
#ifdef USE_LUSTRE_FLUSH
	int	rc;
	struct	statfs sfs;
	rc = fstatfs(fd, &sfs);
	if (0 != rc) {
		nus_warn(("fstatfs(%d) => %d : fail", fd, rc));
		return 0;
	} else if (LL_SUPER_MAGIC == sfs.f_type) {
		nus_debug(("fstatfs(%d) => %d : OK, LUSTRE(%llx)", fd, rc, sfs.f_type));
		return 1;
	}
	nus_debug(("fstatfs(%d) => %d : OK, not LUSTRE(%llx)", fd, rc, sfs.f_type));
#endif /* USE_LUSTRE_FLUSH */
	return 0;
}

	struct lustre_append
iosub_prepare_recover_stdio(FILE* file, const size_t expect_size)
{
	/* this sentense means every element of la is 0 or NULL */
	struct lustre_append la = {0};
#ifdef USE_LUSTRE_FLUSH
	int rc, append;
	void* buffer;
	/* calculate need size for tail lack */
	append = expect_size % (3 * (1 << 22));
	/* do nothing if size is less than 3 * 2^22 or multiple of 3 * 2^22 */
	if (append == expect_size || 0 == append) return la;
	/* seek */
	rc = fseeko(file, -append, SEEK_END);
	if (rc) {
		nus_warn(("fseeko(%p) => %d : fail", file, rc));
		nus_warn(("cannot fseeko, so skip Lustre Append"));
		return la;
	} else {
		nus_debug(("fseeko(%p) => %d : OK", file, rc));
	}
	/* allocate buffer */
	buffer = malloc(append);
	if (!buffer) {
		nus_warn(("cannot allocate, so skip Lustre Append"));
		return la;
	}
	/* read buffer */
	rc = fread(buffer, append, 1, file);
	if (1 != rc) {
		nus_warn(("fread(%p,%d) => %d : fail", file, append, rc));
		nus_warn(("cannot read buffer, so skip Lustre Append"));
		free(buffer);
		return la;
	} else {
		nus_debug(("fread(%p,%d) => %d : OK", file, append, rc));
	}
	/* create struct */
	la.size = expect_size;
	la.append = append;
	la.tail = buffer;
#endif /* USE_LUSTRE_FLUSH */
	return la;
}

	struct lustre_append
iosub_prepare_recover_posix(const int fd, const size_t expect_size)
{
	struct lustre_append la = {};
#ifdef USE_LUSTRE_FLUSH
	off_t rc1;
	size_t rc2;
	int append, rc;
	void* buffer;
	/* calculate need size for tail lack */
	append = expect_size % (3 * (1 << 22));
	/* do nothing if size is less than 3 * 2^22 or multiple of 3 * 2^22 */
	if (append == expect_size || 0 == append) return la;
	/* seek */
	rc1 = lseek(fd, -append, SEEK_END);
	if (rc1 + append != expect_size) {
		nus_warn(("lseek(%d) => %lld : fail expect(%lld)", fd, rc1, expect_size - append));
		nus_warn(("cannot lseek, so skip Lustre Append"));
		return la;
	} else {
		nus_debug(("lseek(%d) => %lld : OK expect(%lld)", fd, rc1, expect_size - append));
	}
	/* allocate buffer */
	buffer = malloc(append);
	if (!buffer) {
		nus_warn(("cannot allocate, so skip Lustre Append"));
		return la;
	}
	/* read buffer */
	rc2 = read(fd, buffer, append);
	if (append != rc2) {
		nus_warn(("read(%d,%lld) => %lld : fail expect(%lld)", fd, append, rc2));
		nus_warn(("cannot read buffer, so skip Lustre Append"));
		free(buffer);
		return la;
	} else {
		nus_debug(("fread(%d,%lld) => %lld : OK", fd, append, rc2));
	}
	la.size = expect_size;
	la.append = append;
	la.tail = buffer;
#endif /* USE_LUSTRE_FLUSH */
	return la;
}

	int
iosub_fsync(const int fd)
{
#if defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH)
	int rc;
	rc = fsync(fd);
	if (0 == rc) {
		nus_debug(("fsync(%d) => %d : OK", fd, rc));
		return 0;
	}
	nus_warn(("fsync(%d) => %d : fail", fd, rc));
#endif /* defined(USE_FSYNC) || defined(USE_LUSTRE_FLUSH) */
	return 1;
}

	int
iosub_lustre_flush(const int fd)
{
#ifdef USE_LUSTRE_FLUSH
	int	rc;
	struct	ioc_data_version idv;
	if (iosub_test_mode()) return 0;
	rc = iosub_is_lustre(fd);
	if (0 == rc) return 2;
	/* llapi_get_data_version */
	idv.idv_flags = LL_DV_RD_FLUSH | LL_DV_WR_FLUSH;
	rc = ioctl(fd, LL_IOC_DATA_VERSION, &idv);
	if (0 == rc) {
		nus_debug(("llapi_get_data_version(%d) => %d : OK (data_version=%llu)", fd, rc, idv.idv_version));
		return 0;
	}
	nus_warn(("llapi_get_data_version(%d) => %d : fail", fd, rc));
	return 1;
#else /* USE_LUSTRE_FLUSH */
	if (iosub_test_mode()) return 0;
	return 2;
#endif /* USE_LUSTRE_FLUSH */
}
	int
iosub_check_size(const char* filename, const size_t expect_size)
{
	int	rc;
	struct	stat stat_info;
	rc = stat(filename, &stat_info);
	if (0 != rc) {
		nus_warn(("stat(%s) => %d : fail", filename, rc));
	} else if (stat_info.st_size == expect_size) {
		nus_debug(("stat(%s).st_size => %lld : OK expect size(%lld)", filename, stat_info.st_size, expect_size));
	} else {
		return nus_err((NUSERR_WR_Inconsistent, "stat(%s).st_size => %lld : NG expect size(%lld)", filename, stat_info.st_size, expect_size));
	}
	return 0;
}

	int
iosub_recover_lustre(const char* filename, const struct lustre_append* la, const unsigned int try_count)
{
#ifdef USE_LUSTRE_FLUSH
	int	rc, fd, i, lack_offset;
	off_t	lack_size;
	ssize_t	wrote;
	struct	stat stat_info;
	if (0 == la->append) return 0;
	for (i = try_count; i >= 0; --i){
		if (iosub_test_mode()) truncate(filename, la->size - la->append);
		rc = stat(filename, &stat_info);
		if (0 != rc) {
			nus_warn(("stat(%s) => %d : fail", filename, rc));
			if (i == try_count) return 0;
			return nus_err((NUSERR_WR_Inconsistent, "cannot recover Lustre missing tail problem!"));
		} else if (stat_info.st_size == la->size) {
			nus_debug(("stat(%s).st_size => %lld : OK expect size(%lld)", filename, stat_info.st_size, la->size));
			if (i != try_count) nus_warn(("success to recover Lustre missing tail problem!"));
			return 0;
		} else if (stat_info.st_size > la->size) {
			nus_warn(("stat(%s).st_size => %lld : NG expect size(%lld) is smaller than filesize", filename, stat_info.st_size, la->size));
			if (i == try_count) return 0;
			return nus_err((NUSERR_WR_Inconsistent, "cannot recover Lustre missing tail problem"));
		}
		
		if (0 == i) {
			nus_warn(("stat(%s).st_size => %lld : NG expect size(%lld)", filename, stat_info.st_size, la->size));
			break;
		} else {
			nus_warn(("stat(%s).st_size => %lld : NG expect size(%lld), try recover", filename, stat_info.st_size, la->size));
		}
		/* lack_size must be >0 because of previous "else if" sentenses */
		lack_size = la->size - stat_info.st_size;
		if (lack_size > la->append) {
			nus_warn(("lack size(%lld) is larger than buffer size(%lld)", lack_size, la->append));
			return nus_err((NUSERR_WR_Inconsistent, "cannot recover Lustre missing tail problem."));
		}
		/* lack_offset must be between 0 and (la->append - 1)  because of previous "if" and "else if" sentenses */
		lack_offset = la->append - lack_size;
		
		fd = open(filename, O_RDWR | O_APPEND);
		if (-1 == fd) {
			nus_warn(("open(%d) => %d : fail", filename, rc));
			continue;
		} else {
			nus_debug(("open(%d) => %d : OK", filename, rc));
		}
		wrote = write(fd, la->tail + lack_offset, lack_size);
		if (lack_size == wrote) {
			nus_debug(("write(%d, %d) => %lld : OK", filename, lack_size, wrote));
			iosub_fsync(fd);
			iosub_lustre_flush(fd);
		} else {
			nus_warn(("write(%d, %d) => %lld : fail", filename, lack_size, wrote));
		}
		rc = close(fd);
		if (0 == rc) {
			nus_debug(("close(%d) => %d : OK", fd, rc));
		}else{
			nus_warn(("close(%d) => %d : fail", fd, rc));
		}
	}
	return nus_err((NUSERR_WR_Inconsistent, "cannot recover Lustre missing tail problem with %d times recover", try_count));
#else
	return 0;
#endif
}

	void
iosub_free(struct lustre_append* la)
{
	if(!la->tail) return;
	free(la->tail);
	la->tail = NULL;
}
