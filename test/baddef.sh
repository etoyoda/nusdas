#!/bin/sh

rm -rf NUSDAS*
mkdir -p NUSDAS01/nusdas_def

cat > NUSDAS01/nusdas_def/bts168.def <<EOF
path        nwp_path_bs
type1       _MSM LL LY
type2       FC   SV
type3       STD1
validtime   34 min in
validtime1  ARITHMETIC   0  60
plane       1
plane1      SURF
element     4
elementmap  PSEA   0
elementmap  U      0
elementmap  V      0
elementmap  RAIN   2  1  0
                     30  1
size        121 127
basepoint   1   1   120E  47.6N
distance    0.2500 0.2000
packing     2UPC
EOF
./def_read.exe NUSDAS01/nusdas_def/bts168.def
