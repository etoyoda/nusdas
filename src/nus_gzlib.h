#ifndef NUS_GZLIB_H
#define NUS_GZLIB_H

#ifndef NUSDAS_CONFIG_H
# error include config.h of NuSDaS
#endif

#ifdef USE_ZLIB

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

int nusgz_compress(unsigned char *indata, size_t insize, 
                   unsigned char *outptr, size_t outbufsize);

uLong nusgz_inq_decompressed_size(unsigned char *indata, size_t insize);


int nusgz_decompress(unsigned char *indata, size_t insize, 
                     unsigned char *outdata, size_t outbufsize);

#endif

#endif
