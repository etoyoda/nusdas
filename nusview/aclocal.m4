AC_DEFUN(NUSAPP_CHECK_NUSDAS, [

  if test -f ../src/nusdas-config
  then
    echo using ../src/nusdas-config
    if test X"$CC" = X""
    then
      CC="`sh ../src/nusdas-config --cc`"
      echo CC=$CC
    fi
    if test X"$CFLAGS" = X""
    then
      CFLAGS="`sh ../src/nusdas-config --cflags`"
      echo CFLAGS=$CFLAGS
    fi
    if test X"$CPPFLAGS" = X""
    then
      CPPFLAGS="`sh ../src/nusdas-config --cppflags-at-srcdir`"
      echo CPPFLAGS=$CPPFLAGS
    fi
    if test X"$LDFLAGS" = X""
    then
      LDFLAGS="`sh ../src/nusdas-config --ldflags`"
      echo LDFLAGS=$LDFLAGS
    fi
    if test X"$LIBS" = X""
    then
      LIBS="`sh ../src/nusdas-config --libs-pure`"
      echo LIBS=$LIBS
    fi
  fi

  AC_ARG_WITH(nwpinc, [
  --with-nwpinc=dir                where <nwpl_capi.h> is [../nwplib8]dnl
  ], [CFLAGS="-I${withval} ${CFLAGS}"], [CFLAGS="-I../nwplib8 ${CFLAGS}"])
  AC_ARG_WITH(nwplib, [dnl
  --with-nwplib=/path/libnwp.a     NWPLIB library [../nwplib8]dnl
  ], [LDFLAGS="-L${withval} ${LDFLAGS}"], [LDFLAGS="-L../nwplib8 ${LDFLAGS}"])
  AC_ARG_WITH(nwp, [dnl
  --with-nwp=nwp                   switch library name [nwp]dnl
  ], [nwp=$withval], [nwp=nwp])

  AC_CHECK_HEADERS([nwpl_capi.h], [], [AC_MSG_ERROR('We need NWPLIB')])
  AC_CHECK_LIB($nwp, NWP_seq2ymdhm, [], [AC_MSG_ERROR('We need NWPLIB')])

  AC_ARG_WITH(nusinc, [dnl
  --with-nusinc=dir                where <nusdas.h> is [../src]dnl
  ], [CFLAGS="-I${withval} ${CFLAGS}"], [CFLAGS="-I../src ${CFLAGS}"])
  AC_ARG_WITH(nuslib, [dnl
  --with-nuslib=/path/libnusdas.a  NWPLIB library [../src]dnl
  ], [LDFLAGS="-L${withval} ${LDFLAGS}"], [LDFLAGS="-L../src ${LDFLAGS}"])
  AC_ARG_WITH(nusdas, [dnl
  --with-nusdas=nusdas             switch library name [nusdas]dnl
  ], [nusdas=$withval], [nusdas=nusdas])

  AC_CHECK_HEADERS([nusdas.h], [], [AC_MSG_ERROR('We need NuSDaS')])
  AC_CHECK_LIB($nusdas, NuSDaS_read, [], [dnl
  	AC_MSG_ERROR('We need NuSDaS: did you forget LIBS?')], [-lm])
])
