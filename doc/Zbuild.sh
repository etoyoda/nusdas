#!/bin/sh
dir=`dirname $0`
display=$DISPLAY
case X"$1" in
X-at)
	at='at now'
	;;
X)
	at='sh -xe'
	;;
*)
	echo "usage: $0 [-at]"
	;;
esac
$at <<EOF
cd $dir
for source in ../src/a*.c
do
	if newer \$source nusdas.dvi
	then
		(cd ../src
		ruby cprodoc.rb -a nusdas.h \
		 -m s/nusdas_swab/endian_swab/i \
		 -m s/nusdas_encode/n_encode/i \
		 -m s/nusdas_decode/n_decode/i \
		 -m s/nusdas_bf/bf/i \
		 -m downcase \
		 -m s/_i1/_I1/ \
		 -o ../doc/capi_@.tex \$source
		ruby cprodoc.rb -L Fortran -a nusdas.h \
		 -x NuSDaS_bf \
		 -x NuSDaS_snprint \
		 -m s/nusdas_swab/endian_swab/i \
		 -m s/nusdas_encode/n_encode/i \
		 -m s/nusdas_decode/n_decode/i \
		 -m upcase \
		 -o ../doc/fapi_@.tex \$source
		)
	fi
done
for source in ../libsrf/s*.c
do
	if newer \$source nusdas.dvi
	then
		(cd ../src
		ruby cprodoc.rb \
		 -f srf_amd_aqc,srf_amd_rdic,srf_search_amdstn,srf_amd_slct \
		 -f srf_lv_set,srf_lv_trans,srf_rd_rdic,rdr_lv_trans \
		 -o ../doc/capi_@.tex \$source
		)
	fi
done
make
DISPLAY=$display xpdf nusdas.pdf
EOF
