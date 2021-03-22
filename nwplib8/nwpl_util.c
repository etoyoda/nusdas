/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  ◇◇◇ 数値予報標準ライブラリ ◇◇◇

  ・ファイルの有無を検査する

  更新日    | 更新者 |更新内容
  ----------+--------+-------------------------------
  2000.08   | Oikawa | 新規作成
  2000.09.27| Oikawa | Fortranインターフェースのバグ修正。strをNULL初期化。
  2002.07.04|Yasutani| Fortranインターフェースのバグ修正。文字列長を引数から.

   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef USE_LUSTRE_FLUSH
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <lustre/lustre_user.h>
#endif /* USE_LUSTRE_FLUSH */

/* --------------------------------------------------------------------------
     ファイルの有無を検査する

       返り値  意味
         0     ファイルは存在しない。
         1     通常ファイルである。
         2     シンボリックリンクであり、実体が存在する。
         3     シンボリックリンクであり、実体は存在しない。
   -------------------------------------------------------------------------- */
int nwp_filestat(char *fname) {

/* 引数：
   (I) char *fname  : ファイル名   */

  struct stat buf;
  char str[100];
  int i,rc,st,lst;

  st = stat(fname,&buf); 
  lst = lstat(fname,&buf);
  
  if(st && lst)       { rc = 0; }         /* stat=-1,lstat=-1 */
  else if(st && !lst) { rc = 3; }         /* stat=-1,lstat=0  */
  else {                                  /* stat=0 ,lstat=0  */
    i = readlink(fname,str,100);
    if(i>0) { rc = 2; }
    else    { rc = 1; }
  }

  return rc;
}

/* ==========================================================================
     Fortran90 interface 
   ========================================================================== */


/* --- ファイルの有無を検査する --------------------------------------------- */
/*        (  尻尾の空白文字を落とす )       fname_length は隠れ引数     */
void NWP_filestat(char *fname, int *rc, int fname_length) {
  char str[256];
  int  i = 0;
  /*poption noparallel */
  while (
    fname[i] != ' '  &&
    i < fname_length &&
    i < (int)sizeof(str) -1 ) {
      str[i] = fname[i];
      i++;
  }
  str[i] = 0;                
  *rc = nwp_filestat(str);
  return;
}

static int luster_test_mode() {
  static int test_mode = -1;
  char* test_str;
  if (0 == test_mode || 1 == test_mode) return test_mode;
  test_str = getenv("NWP_LUSTRE_LACK_TEST");
  if (NULL == test_str) return test_mode = 0;
  return test_mode = 1 == atoi(test_str) ? 1: 0;
}

static off_t get_size_for_recover(const char* fname) {
  struct stat stat_info;
  if (0 != stat(fname, &stat_info)) return -1;
  if (S_IFREG != (stat_info.st_mode & S_IFMT)) return -1;
  return stat_info.st_size;
}

/* --------------------------------------------------------------------------
     Lustre 用のファイル回復措置を行う

       返り値  意味
         0     ファイル回復不要であった。
         1     ファイルがLustreによる異常だったが回復できた。
         3     検査/回復中にIOエラー又はallocateエラーが発生した。
         4     ファイルがLustreによる異常で回復できなかった。
   -------------------------------------------------------------------------- */
int nwp_lustre_recover(const char *fname) {
  static const int show_msg = 1;
#ifdef USE_LUSTRE_FLUSH
  static const int show_dbg = 1;
  static const int try_count = 3;
  struct statfs sfs;
  off_t original_size;
  int append;
  int fd, i;
  char* buffer = NULL;
  struct ioc_data_version idv;
  int lack_offset;
  off_t flush_size, lack_size, pos_rc;
  
  /* check file system */
  if (0 != statfs(fname, &sfs)) {
    if(show_dbg) fprintf(stderr, "E!(lustre) cannot get file system information %s\n", fname);
    return 3;
  }
  if (LL_SUPER_MAGIC != sfs.f_type && 0 == luster_test_mode()) return 0;
  
  /* check file size */
  original_size = get_size_for_recover(fname);
  if (original_size < 0) {
    if(show_dbg) fprintf(stderr, "E!(lustre) cannot get file size %s\n", fname);
    return 3;
  }
  append = original_size % (3 * (1 << 22));
  if (append == original_size) append = 0;
  
  fd = open(fname, O_RDWR); /* to check writablility, use not O_RDONLY but O_RDWR */
  if (-1 == fd) {
    if(show_dbg) fprintf(stderr, "E!(lustre) cannot open file %s\n", fname);
    return 3;
  }
  /* read buffer */
  if (append) {
    pos_rc = lseek(fd, -append, SEEK_END);
    if (pos_rc + append != original_size) {
      if(show_dbg) fprintf(stderr, "E!(lustre) cannot seek file %s\n", fname);
      return 3;
    }
    if (NULL == (buffer = (char*)malloc(append))) {
      if(show_dbg) fprintf(stderr, "E!(lustre) cannot allocate memory(%d) %s\n", append, fname);
      return 3;
    }
    if (append != read(fd, buffer, append)){
      free(buffer);
      if(show_dbg) fprintf(stderr, "E!(lustre) cannot read file %s\n", fname);
      return 3;
    }
  }
  /* if 0 == append, buffer will be NULL
   * But "if (lack_size > append) return 2" will execute,
   * so do not use buffer. Dont worry */

  for (i = 0; i <= try_count; ++i){
    /* flush */
    if (-1 != fd) {
      fsync(fd);
      idv.idv_flags = LL_DV_RD_FLUSH | LL_DV_WR_FLUSH;
      if (0 == luster_test_mode() && 0 != ioctl(fd, LL_IOC_DATA_VERSION, &idv)) {
        if (0 == i) {
          if(show_dbg) fprintf(stderr, "E!(lustre) cannot lustre flush %s\n", fname);
          return 3;
        } else {
          if(show_msg) fprintf(stderr, "E!(lustre) cannot lustre flush %s\n", fname);
          return 4;
        }
      }
      close(fd);
    }
    
    /* check file size */
    if(luster_test_mode()) truncate(fname, original_size - append);
    flush_size = get_size_for_recover(fname);
    if(flush_size < 0) {
      if (0 == i) {
        if(show_dbg) fprintf(stderr, "E!(lustre) cannot check file size %s\n", fname);
        return 3;
      } else {
        if(show_msg) fprintf(stderr, "E!(lustre) fail lustre recover (size unknown) %s\n", fname);
        return 4;
      }
    } else if(original_size == flush_size) {
      if (0 == i) return 0;
      if(show_msg) fprintf(stderr, "W!(lustre) success lustre file recover %s\n", fname);
      return 1;
    } else if(flush_size > original_size) {
      if (0 == i) return 0;
      if(show_msg) fprintf(stderr, "E!(lustre) fail lustre file recover %s\n", fname);
      return 4;
    }
    
    /* recover fail */
    if (try_count == i) {
      if(show_msg) fprintf(stderr, "E!(lustre) fail lustre file recover with %d times trial %s\n", try_count, fname);
      return 4;
    }
    if(show_msg) fprintf(stderr, "W!(lustre) expect_size(%d) != file_size(%d), try lustre recover %s\n", original_size, flush_size, fname);
    
    /* lack_size must be >0 because of previous "if" sentenses */
    lack_size = original_size - flush_size;
    if (lack_size > append) {
      if(show_msg) fprintf(stderr, "E!(lustre) lack_size(%d) > recover_buffer(%d), cannot recover %s\n", lack_size, append, fname);
      return 4;
    }
    /* lack_offset must be between 0 and (append - 1)  because of previous "if" sentenses */
    lack_offset = append - lack_size;
    
    /* recover */
    fd = open(fname, O_RDWR | O_APPEND);
    if (-1 == fd) continue;
    write(fd, buffer + lack_offset, lack_size);
  }
  return 2;
#endif /* USE_LUSTRE_FLUSH */
  return 0;
}

/* ==========================================================================
     Fortran90 interface 
   ========================================================================== */


/* --- ファイルの有無を検査する --------------------------------------------- */
/*        (  尻尾の空白文字を落とす )       fname_length は隠れ引数     */
void NWP_lustre_recover(const char *fname, int *rc, int fname_length) {
  char* str;
  int i;
  str = (char*) malloc(fname_length + 1);
  str[fname_length] = 0;
  for(i = 0; i < fname_length; ++i){
    if (' ' == fname[i]){
      str[i] = 0;
      break;
    }
    str[i] = fname[i];
  }
  *rc = nwp_lustre_recover(str);
  free(str);
  return;
}
