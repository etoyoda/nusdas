
AC_DEFUN(NUS_HOST_CHARSET, [
	AC_CHECK_PROG(SJISTOEUC, nkf, nkf -e, cat)
	AC_CHECK_PROG(EUCTOSJIS, nkf, nkf -s, cat)
	case $host in
	*-*-linux*|*-*-cygwin*)
		AC_CHECK_PROG(SJISTOEUC, iconv, iconv -f Shift_JIS -t euc-JP)
		AC_CHECK_PROG(EUCTOSJIS, iconv, iconv -t Shift_JIS -f euc-JP)
		;;
	*-*-aix*)
		AC_CHECK_PROG(SJISTOEUC, iconv, iconv -f IBM-943 -t IBM-eucJP)
		AC_CHECK_PROG(EUCTOSJIS, iconv, iconv -t IBM-943 -f IBM-eucJP)
		;;
	esac
	AC_ARG_ENABLE(charset,
		[  --enable-charset=charset   (shift_jis or euc-jp)], [
		nus_cv_host_charset=$enableval
	], [
		AC_CACHE_CHECK([charset for source/resource],
			nus_cv_host_charset, [
			case $host in
			*-*-*bsd*|*-*-linux*|*-*-sunos4.*|*-*-solaris2.*)
				nus_cv_host_charset=euc-jp
				;;
			*)
				nus_cv_host_charset=shift_jis
				;;
			esac
		])
	])
	CHARSET=$nus_cv_host_charset
	AC_SUBST(SJISTOEUC)
	AC_SUBST(EUCTOSJIS)
	AC_SUBST(CHARSET)
])


AC_DEFUN(NUS_C_INTMODEL, [
  AC_ARG_ENABLE(intmodel,
    [  --enable-intmodel=name  size of integer (ILP32LL64, LLP64, etc.)], [
      nus_cv_cc_intmodel=$enableval
    ], [
      AC_CACHE_CHECK([size of integer types], nus_cv_cc_intmodel)
  ])
  : ${INTMODEL:=$nus_cv_cc_intmodel}

  if test X"$cross_compiling" = X"yes"; then
    if test X"$INTMODEL" != X""; then
      nus_cv_cc_intmodel=$INTMODEL
    elif test X"$SI4_TYPE" != X""; then
      case $SI4_TYPE in
      long)
        nus_cv_cc_intmodel=LLP64
        echo "assuming 64bit environment (INTMODEL=LLP64)"
	echo "  if you are using 32bit environment, use INTMODEL=ILP32"
	;;
      int)
        nus_cv_cc_intmodel=LP64
	;;
      *int32*)
        nus_cv_cc_intmodel=ILP64/$SI4_TYPE
	;;
      *)
        echo "unsupported SI4_TYPE=$SI4_TYPE"
	exit 1
	;;
      esac
    else
      echo ""
      echo "Error! You must specify INTMODEL in cross compiling mode."
      echo "       INTMODEL=I16L32           good old MS-DOS"
      echo "       INTMODEL=ILP32            no support of 64 bit"
      echo "       INTMODEL=ILP32LL64        most 32bit systems"
      echo "       INTMODEL=LP64             64bit Unix systems"
      echo "       INTMODEL=LLP64            Windows64"
      echo "       INTMODEL=ILP64/_int32     NEC SX float2"
      exit 1
    fi
  fi
  AC_CHECK_TYPES([ssize_t, off_t])

  if test X"$INTMODEL" = X""; then
    AC_CHECK_SIZEOF(short)
    AC_CHECK_SIZEOF(int)
    AC_CHECK_SIZEOF(long)
    AC_CHECK_SIZEOF(long long)
    AC_CHECK_SIZEOF(void*)
    case $ac_cv_sizeof_short/$ac_cv_sizeof_int/$ac_cv_sizeof_long/$ac_cv_sizeof_long_long/$ac_cv_sizeof_voidp in
    2/2/4/0/?)
    	SI2_TYPE=int; SI4_TYPE=long; SI8_TYPE=; INTMODEL=I16L32
	;;
    2/4/4/0/4)
        SI2_TYPE=short; SI4_TYPE=int; SI8_TYPE=; INTMODEL=ILP32
        ;;
    2/4/4/8/4)
        SI2_TYPE=short; SI4_TYPE=int; SI8_TYPE="long long"; INTMODEL=ILP32LL64
        ;;
    2/4/4/8/8)
        SI2_TYPE=short; SI4_TYPE=int; SI8_TYPE="long long"; INTMODEL=IL32LLP64
        ;;
    2/4/8/?/8)
        SI2_TYPE=short; SI4_TYPE=int; SI8_TYPE=long; INTMODEL=LP64
        ;;
    2/8/8/?/8)
        SI2_TYPE=short; SI4_TYPE=; SI8_TYPE=int; INTMODEL=ILP64
    	;;
    *)
        echo unknown programming model.
	exit 1;
        ;;
    esac
    nus_cv_cc_intmodel=$INTMODEL
  else
    case "$INTMODEL" in
    *P64)
	AC_DEFINE([SIZEOF_VOIDP], [8])
        ;;
    *)
	AC_DEFINE([SIZEOF_VOIDP], [4])
	;;
    esac
  fi

  if test X"$SI2_TYPE" = X ; then
    case $INTMODEL in
    I16L32)	SI2_TYPE=int; SI4_TYPE=long; SI8_TYPE= ;;
    ILP32)	SI2_TYPE=short; SI4_TYPE=int; SI8_TYPE= ;;
    ILP32LL64|IL32LLP64)
    		SI2_TYPE=short; SI4_TYPE=int; SI8_TYPE="long long" ;;
    LP64)	SI2_TYPE=short; SI4_TYPE=int; SI8_TYPE=long ;;
    ILP64)	SI2_TYPE=short; SI4_TYPE=; SI8_TYPE=int ;;
    ILP64/_int32)
    		SI2_TYPE=short; SI4_TYPE=_int32; SI8_TYPE=int ;;
    *)
    		echo unknown INTMODEL=$INTMODEL
    		;;
    esac
  fi
  if test X"$SI4_TYPE" = X; then
     echo please specify SI4_TYPE
     exit 1
  fi
  echo N_SI2=$SI2_TYPE N_SI4=$SI4_TYPE N_SI8=$SI8_TYPE
  if test X"$SI8_TYPE" = X; then
    HAVE_SI8_TYPE=0 
  else
    HAVE_SI8_TYPE=1 
  fi

  AC_SUBST(HAVE_SI8_TYPE)
  AC_SUBST(SI8_TYPE)
  AC_SUBST(SI4_TYPE)
  AC_SUBST(SI2_TYPE)
  AC_SUBST(INTMODEL)
])

AC_DEFUN(NUS_C_SWITCH_LONG_LONG, [
  if test X"$HAVE_SI8_TYPE" = X"0"; then
    nus_cv_c_longlongswitch=no
  fi
  AC_CACHE_CHECK([switch(long long)], nus_cv_c_longlongswitch, [
    nus_cv_c_longlongswitch=undef
  ])
  if test X"$nus_cv_c_longlongswitch" = X"undef"
  then
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],
      [[long long i;
      switch (i) {
      case 0x3FFFFFFFFFFFFFFFLL: return 0; break;
      default: return 1; break; 
      }]]
    )], [
      nus_cv_c_longlongswitch=yes
    ], [
      nus_cv_c_longlongswitch=no
    ])
  fi
  if test X"$nus_cv_c_longlongswitch" = X"yes" ; then
    AC_DEFINE(ALLOW_LONG_LONG_SWITCH)
  fi
])

AC_DEFUN(NUS_C_INLINE, [
  AC_ARG_ENABLE(inline, [dnl
  --enable-inline=inline       enable C99-style inline function
  --enable-inline=gcc          enable "static inline" function
  --enable-inline=__inline__   enable extensional form of inline function
  --disable-inline             disable inline functions, make them static dnl
    ], [
      ac_cv_c_inline=$enableval
    ], [
      AC_CACHE_CHECK([for inline function], ac_cv_c_inline)
  ])
  AC_C_INLINE
  case $ac_cv_c_inline in
  no) ;;
  *) AC_DEFINE(HAVE_INLINE) ;;
  esac
])

AC_DEFUN(NUS_C_ALIGNMENT, [
  case $host in
  hppa2*|x86_64-*-linux*)
    AC_DEFINE([NEED_ALIGN], [12])
    ;;
  sparc64-*-linux*)
    AC_DEFINE([NEED_ALIGN], [14])
    ;;
  *)
    AC_DEFINE([NEED_ALIGN], [0])
    ;;
  esac
])

AC_DEFUN(NUS_ARCHIVER, [
  AC_CACHE_CHECK([archiver to create library], nus_cv_prog_archiver)
  : ${ARCHIVER:=$nus_cv_prog_archiver}
  AC_PATH_PROG(ARCHIVER, ar, [echo "specify ARCHIVER"; exit 1])
  AC_CACHE_CHECK([archiver flags], nus_cv_prog_archiver_flags, [
  nus_cv_prog_archiver_flags=rv
  ])
  : ${ARFLAGS:=${nus_cv_prog_archiver_flags}}
  AC_SUBST(ARFLAGS)
])

AC_DEFUN(NUS_USE_DEBUG, [
  AC_ARG_ENABLE(debug, [dnl
  --enable-debug               enable nus_debug macro
  --disable-debug              disable nus_debug macro, runs faster dnl
  ], [
     nus_cv_use_debug=$enableval
  ], [
     AC_CACHE_CHECK([for debug macro option], nus_cv_use_debug)
  ])
  case $nus_cv_use_debug in
  no) ;;
  *) AC_DEFINE(USE_NUS_DEBUG) ;;
  esac
])

AC_DEFUN(NUS_USE_PROFILE, [
  AC_ARG_ENABLE(profile, [dnl
  --enable-profile               enable profiling
  --disable-profile              disable profiling, runs faster dnl
  ], [
     nus_cv_use_profile=$enableval
  ], [
     AC_CACHE_CHECK([for profile macro option], nus_cv_use_profile)
  ])
  case $nus_cv_use_profile in
  yes) AC_DEFINE(USE_NUS_PROFILE) ;;
  *) ;;
  esac
])

AC_DEFUN(NUS_FUNC_CSES, [
  AC_ARG_ENABLE(cses, [dnl
  --enable-cses=device         enable HITACHI CSES for named device
  --disable-cses               disable HITACHI CSES dnl
  ], [
    nus_cv_use_cses=$enableval
  ], [
    AC_CACHE_CHECK([for HITACHI CSES], nus_cv_use_cses, [dnl
      nus_cv_use_cses=no dnl
    ])
  ])
  if test X"$nus_cv_use_cses" != X"no"
  then
    AC_CHECK_HEADERS(cses/esfile.h)
    AC_CHECK_LIB(cses, es_open)
    AC_CHECK_TYPES([offset_t])
    AC_DEFINE(USE_CSES)
    if test X"$nus_cv_use_cses" != X"yes" ; then
      AC_DEFINE_UNQUOTED(DEFAULT_CSES_DEVICE, "$nus_cv_use_cses")
    fi
  fi
])

AC_DEFUN(NUS_C_BIGENDIAN, [
  AC_ARG_ENABLE(le, [dnl
  --enable-le                  assume little endian
  --disable-le                 assume big endian dnl
  ], [
    if test X"$enableval" = X"no"; then
      AC_DEFINE([WORDS_BIGENDIAN])
      bo=big
    else
      bo=little
    fi
    AC_MSG_NOTICE([assumed byte order: $bo endian]) 
  ], [
    AC_C_BIGENDIAN
  ])
])

AC_DEFUN(NUS_PROG_F90, [
  AC_ARG_WITH(f90, [dnl
  --with-f90=command           Fortran 90 compiler
  --without-f90                do without Fortran dnl
  ], [
    if test X"$withval" = X"yes"; then
      command="$F90 ifort gfortran f95 f90"
      F90=
      AC_CHECK_PROGS(F90, $command, NONE)
    else
      command=${withval}
      F90=
      AC_CHECK_PROG(F90, $command, $command, NONE)
    fi
  ], [
    : ${F90:=${nus_cv_prog_f90:-NONE}}
    AC_CHECK_PROG(F90, ${F90}, ${F90}, NONE)
  ])
  AC_CACHE_CHECK([Fortran compiler dnl
], [nus_cv_prog_f90], [nus_cv_prog_f90=${F90}])
  if test X"$F90" = X"NONE"; then
    F90_AVAILABLE=no
  else
    F90_AVAILABLE=yes
  fi
  AC_SUBST(F90_AVAILABLE)
  AC_SUBST(FFLAGS)
])

AC_DEFUN(NUS_USE_SRF, [
  AC_ARG_WITH(srf, [dnl
  --with-srf                   builds SRF library (libsrf.a)
  --without-srf                no SRF library dnl
  ], [
    SRF_AVAILABLE=$withval
  ], [
    SRF_AVAILABLE=yes
  ])
  AC_SUBST(SRF_AVAILABLE)
])

AC_DEFUN(NUS_CFLAGS_HACK, [
  NUS_GCCOPT_BUG=0
  if test "$GCC" = yes; then
    AC_MSG_NOTICE([host: $host])
    NUS_GCC_VER=`sh -c "$CC -dumpversion" | cut -d. -f-2`
    AC_MSG_NOTICE([GCC version: $NUS_GCC_VER])
    AC_CHECK_SIZEOF(long)
    case "$host"/"$ac_cv_sizeof_long" in
    x86_64-*-*/4)
      case "$NUS_GCC_VER" in
      4.0|4.1|4.2|4.3)
        NUS_GCCOPT_BUG=1
        ;;
      esac
      ;;
    x86_64-*-*/*)
      case "$NUS_GCC_VER" in
      4.1)
        NUS_GCCOPT_BUG=1
        ;;
      esac
      ;;
    *)
      case "$NUS_GCC_VER" in
      4.0|4.1|4.2|4.3|4.4)
        NUS_GCCOPT_BUG=1
        ;;
      esac
      ;;
    esac
  fi
  if test $NUS_GCCOPT_BUG = 1 ; then
    AC_MSG_NOTICE([This gcc has problem with [-O[23]]. current CFLAGS: $CFLAGS])
    CFLAGS=[`echo " $CFLAGS " | sed '
              s/[ \t]/  /g
              s/ -O[23] / -O /g
              s/  / /g
            '`]
    AC_MSG_NOTICE([corrected CFLAGS: $CFLAGS])
  fi
  if test X"$GCC" = X"yes" && expr X"$CFLAGS" : '.*-Werror' > /dev/null
  then
    echo "Your CFLAGS looks too picky, so I'll drop '-Werror' for libsrf and libnwp."
    CFLAGS_SLOPPY=`echo $CFLAGS | sed 's/-Werror//'`
  else
    CFLAGS_SLOPPY="$CFLAGS"
  fi
  AC_SUBST(CFLAGS_SLOPPY)
])

AC_DEFUN(NUS_USE_NET, [
  AC_ARG_WITH(net, [dnl
  --with-net                   compile without network functions
  --without-net                disable network functions dnl
  ], [
    NET_AVAILABLE=$withval
    if test X"$withval" = X"yes" 
    then
      AC_DEFINE(USE_NET)
    fi
  ], [
    NET_AVAILABLE=yes
    AC_DEFINE(USE_NET)
  ])
  AC_SUBST(NET_AVAILABLE)
])

AC_DEFUN(NUS_FILEVER, [
  AC_ARG_ENABLE(dfver, [dnl
  --enable-dfver               writes ver 11 by default
  --enable-dfver=13            writes ver 13 by default
  --disable-dfver              writes ver 14 by default (default of configure) dnl
  ], [
     if test X"$enableval" = X"yes"
     then
       AC_DEFINE(NUS_DFVER, 11)
     else
       AC_DEFINE(NUS_DFVER, $enableval)
     fi
  ], [
       AC_DEFINE(NUS_DFVER, 14)
  ])
])

AC_DEFUN(NUS_SIO_DEFAULT, [
  AC_ARG_WITH(sio, [dnl
  --with-sio                   use stdio as default
  --without-sio                use posix io as default (default) dnl
  ], [
     if test X"$withval" = X"yes"
     then
       AC_DEFINE(SIO_DEFAULT, 1)
     fi
  ])
])

AC_DEFUN(NUS_CHECK_ZLIB, [
  AC_ARG_WITH(zlib, [dnl
  --with-zlib                  use zlib
  --without-zlib               don't use zlib dnl
  ], [
    use_zlib=$withval
  ], [
    use_zlib=yes
  ])
  if test X"$use_zlib" = X"yes"
  then
    AC_CHECK_LIB(z, deflate, :, [ dnl if zlib is missing
      AC_MSG_ERROR([give me LDFLAGS for -lz, or try --without-zlib])
    ])
    LIBS="-lz $LIBS"
    AC_CHECK_HEADER([zlib.h], :, [dnl if zlib.h is missing
      AC_MSG_ERROR([give me CFLAGS for zlib.h, or try --without-zlib])
    ]) 
    AC_DEFINE(USE_ZLIB)
    ZLIB_AVAILABLE=yes
  else
    AC_MSG_NOTICE(zlib compression is disabled.)
    ZLIB_AVAILABLE=no
  fi
  AC_SUBST(ZLIB_AVAILABLE)
])

AC_DEFUN(NUS_MALLOC, [
  AC_ARG_ENABLE(nusmalloc, [dnl
  --enable-nusmalloc           use nus_malloc() instead of malloc() (default)
  --disable-nusmalloc          use malloc() directly dnl
  ], [
    use_nusmalloc=$enableval
  ], [
    use_nusmalloc=yes
  ])
  if test X"$use_nusmalloc" != X"no"
  then
    AC_DEFINE(USE_NUS_MALLOC)
  fi
])

AC_DEFUN(NUS_CHECK_JASPER, [
  AC_ARG_ENABLE(jasper, [dnl
  --enable-jasper              use jasper
  --disable-jasper             don't use jasper dnl
  ], [
    use_jasper=${enableval:-yes}
  ], [
    use_jasper=yes
  ])
  if test X"$use_jasper" = X"yes"
  then
    JASPER_AVAILABLE=yes
    AC_CHECK_LIB(jasper, jas_image_create, :, [
      JASPER_AVAILABLE=no
    ])
    AC_CHECK_HEADERS([jasper.h jasper/jasper.h], [
      JASPER_AVAILABLE=yes
      break
    ], [
      JASPER_AVAILABLE=no
    ])
    if test X"$JASPER_AVAILABLE" = X"yes"
    then
      LIBS="-ljasper $LIBS"
      AC_DEFINE(USE_JASPER)
    else
      AC_MSG_NOTICE([disabling support for JPEG 2000 code format.])
    fi
  else
    AC_MSG_NOTICE([disabling support for JPEG 2000 code format.])
    JASPER_AVAILABLE=no
  fi
])

AC_DEFUN(NUS_CHECK_OPENJPEG, [
  AC_ARG_ENABLE(openjpeg, [dnl
  --enable-openjpeg            use OpenJPEG
  --disable-openjpeg           don't use  dnl
  ], [
    use_openjpeg=${enableval:-yes}
  ], [
    use_openjpeg=yes
  ])
  if test X"$use_openjpeg" = X"yes"
  then
    OPENJPEG_AVAILABLE=yes
    AC_CHECK_LIB(openjp2, opj_decode, :, [
      OPENJPEG_AVAILABLE=no
    ])
    AC_CHECK_HEADERS([openjpeg.h openjpeg/openjpeg.h], [
      OPENJPEG_AVAILABLE=yes
      break
    ], [
      OPENJPEG_AVAILABLE=no
    ])
    if test X"$OPENJPEG_AVAILABLE" = X"yes"
    then
      LIBS="-lopenjp2 $LIBS"
      AC_DEFINE(USE_OPENJPEG)
    else
      AC_MSG_NOTICE([disabling support for OpenJPEG library.])
    fi
  else
    AC_MSG_NOTICE([disabling support for OpenJPEG library.])
    OPENJPEG_AVAILABLE=no
  fi
])

AC_DEFUN(NUS_CHECK_GPFS_ARCHIVE, [
  AC_ARG_ENABLE(gpfs-archive-chk, [dnl
  --enable-gpfs-archive-chk    use GPFS archive check
  --disable-gpfs-archive-chk   don't use  dnl
  ], [
    use_gpfs_archive_chk=${enableval:-yes}
  ], [
    use_gpfs_archive_chk=yes
  ])
  if test X"$use_gpfs_archive_chk" = X"yes"
  then
    GPFS_ARCHIVE_CHK_AVAILABLE=yes
    AC_CHECK_LIB(gpfs, gpfs_stat_x, :, [
      GPFS_ARCHIVE_CHK_AVAILABLE=no
    ])
    AC_CHECK_HEADERS([gpfs_gpl.h], [
      GPFS_ARCHIVE_CHK_AVAILABLE=yes
      break
    ], [
      GPFS_ARCHIVE_CHK_AVAILABLE=no
    ])
    if test X"$GPFS_ARCHIVE_CHK_AVAILABLE" = X"yes"
    then
      LIBS="-lgpfs $LIBS"
      AC_DEFINE(USE_GPFS_ARCHIVE_CHK)
    else
      AC_MSG_NOTICE([disabling support for GPFS library.])
    fi
  else
    AC_MSG_NOTICE([disabling support for GPFS library.])
    GPFS_ARCHIVE_CHK_AVAILABLE=no
  fi
])

AC_DEFUN(NUS_CHECK_LUSTRE_FLUSH, [
  AC_ARG_ENABLE(lustre-flush, [dnl
  --enable-lustre-flush        use Lustre Flush
  --disable-lustre-flush       don't use  dnl
  ], [
    use_lustre_flush=${enableval:-no}
  ], [
    use_lustre_flush=no
  ])
  if test X"$use_lustre_flush" = X"yes"
  then
    LUSTRE_FLUSH_AVAILABLE=yes
    AC_CHECK_HEADERS([lustre/lustre_user.h], [
      LUSTRE_FLUSH_AVAILABLE=yes
      break
    ], [
      LUSTRE_FLUSH_AVAILABLE=no
    ])
    if test X"$LUSTRE_FLUSH_AVAILABLE" = X"yes"
    then
      AC_DEFINE(USE_LUSTRE_FLUSH)
    else
      AC_MSG_NOTICE([disabling support for Lustre library.])
    fi
  else
    AC_MSG_NOTICE([disabling support for Lustre library.])
    GPFS_ARCHIVE_CHK_AVAILABLE=no
  fi
])

AC_DEFUN(NUS_CHECK_FSYNC, [
  AC_ARG_ENABLE(fsync, [dnl
  --enable-fsync               use fsync before close
  --disable-fsync              don't use fsync before close dnl
  ], [
    use_fsync=${enableval:-no}
  ], [
    use_fsync=no
  ])
  if test X"$use_fsync" = X"yes"
  then
    FSYNC_AVAILABLE=yes
    AC_CHECK_HEADERS([unistd.h], [
      AC_DEFINE(USE_FSYNC)
      break
    ], [
      AC_MSG_NOTICE([disabling support for fsync.])
    ])
  else
    AC_MSG_NOTICE([disabling support for fsync.])
    FSUNC_AVAILABLE=no
  fi
])

AC_DEFUN(NUS_OMP, [
  AC_ARG_ENABLE(omp, [dnl
  --enable-omp                 use OpenMP (default)
  --enable-omp=31              use OpenMP 3.1
  --disable-omp                don't use OpenMP dnl
  ], [
    use_omp=$enableval
  ], [
    use_omp=yes
  ])
  if test X"$use_omp" = X"yes"
  then
    AC_DEFINE(USE_OMP)
  elif test X"$use_omp" != X"no"
  then
    AC_DEFINE_UNQUOTED(USE_OMP, [${use_omp}])
  fi
])
