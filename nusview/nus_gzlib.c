#include "nus_gzlib.h"

#define DEF_MEM_LEVEL 8
#  define OS_CODE  0x03  /* assume Unix */

static int const gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
static void putLong (unsigned char *ptr, uLong x);
static uLong getLong (unsigned char* in);

int nusgz_compress(unsigned char *indata, size_t insize, 
                   unsigned char *outptr, size_t outbufsize)
{
    z_stream z;
    int level = Z_DEFAULT_COMPRESSION; /* compression level */
    int strategy = Z_DEFAULT_STRATEGY; /* compression strategy */
    int err;
    int count, flush, status;
    uLong crc;
    char tmp[80];
    unsigned char *p;

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    
    err = deflateInit2(&z, level, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, 
                       strategy);
    if(err != Z_OK){
        fprintf(stderr, "deflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.avail_in = 0; 

    flush = Z_FINISH;
    crc = crc32(0L, Z_NULL, 0);
    p = (unsigned char*)outptr;

    sprintf(tmp, "%c%c%c%c%c%c%c%c%c%c", gz_magic[0], gz_magic[1],
            Z_DEFLATED, 0 /*flags*/, 
            0,0,0,0 /*time*/, 
            0 /*xflags*/, 
            OS_CODE);

    memcpy(p, tmp, 10);
    p += 10;

    z.next_out = p;
    z.avail_out = outbufsize - (p - outptr);

    z.next_in = indata;
    z.avail_in = insize;

    status = deflate(&z, flush);
    if (status != Z_STREAM_END){
        if(z.avail_out == 0){
            fprintf(stderr, "Not enough array buffer\n");
            return -4;
        }
        fprintf(stderr, "deflate: %s\n", (z.msg) ? z.msg : "???");
        return -1;
    }
    p = z.next_out;
    if (deflateEnd(&z) != Z_OK) {
        fprintf(stderr, "deflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    crc = crc32(crc, (const Bytef *)indata, (uInt)insize);

    putLong(p, crc);
    p += 4;
    putLong(p, (uLong)(insize & 0xffffffff));
    p += 4;

    return (p - outptr);

}

uLong nusgz_inq_decompressed_size(unsigned char *indata, size_t insize)
{

    unsigned char *p;

    p = indata + insize - 4;
    return getLong(p);    
}

int nusgz_decompress(unsigned char *indata, size_t insize, 
                     unsigned char *outdata, size_t outbufsize)
{
    z_stream z;
    int err;
    int count, flush, status;
    uLong crc, crcorg;
    int start;
    unsigned char *pp;
    unsigned char *pst;
    unsigned long outsize, tr_outsize;
    
    if(indata[0] != 0x1f|| indata[1] != 0x8b || indata[2] != 8){
        fprintf(stderr, "Input data is not gzip format!\n");
        return -99;
    }
    pst = indata + 10;
    if(indata[3] >> 2 & 0x01){ /* FEXTRA */
        unsigned short xlen;
        xlen = (unsigned char)indata[4] + (unsigned char)indata[5] * 256;
        pst += xlen;
    }
    else if(indata[3] >> 3 & 0x01 || indata[3] >> 4 & 0x01){ /* FNAME or FCOMMENT*/
        pp = pst;
        while(*pst){
            pst++;
        }
        pst++;
    }
    else if(indata[3] >> 1 & 0x01){ /* FHCRC */
	fprintf(stderr, "FHCRC!\n");
      pst += 2;
    }

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    
    err = inflateInit2(&z, -MAX_WBITS);
    if(err != Z_OK){
        fprintf(stderr, "inflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    tr_outsize = getLong(indata + insize - 4);
    if(tr_outsize > outbufsize){
        fprintf(stderr, "Inflate : Not enough memory array\n");
        return -4;
    }

    z.avail_in = 0;

    flush = Z_FINISH;
    crc = crc32(0L, Z_NULL, 0);

    z.next_out = outdata;
    z.avail_out = outbufsize;

    z.next_in = pst;
    z.avail_in = insize - (pst - indata) - 8;

    status = inflate(&z, flush);
    if (z.avail_in != 0){
        fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
        return -1;
    }
    outsize = z.next_out - outdata;

    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    crcorg = getLong(indata + insize - 8);
    crc = crc32(crc, (const Bytef *)outdata, (uInt)outsize);

    if(crc != crcorg){
        fprintf(stderr, "crc error\n");
        return -3;
    }
    if(outsize != tr_outsize){
        fprintf(stderr, "size error\n");
        return -4;
    }
    return outsize;

}

static void putLong (unsigned char *ptr, uLong x)
{
    int n;
    for (n = 0; n < 4; n++) {
        *(ptr++) = (int)(x & 0xff);
        x >>= 8;
    }
}

static uLong getLong (unsigned char* in)
{
    int c;
    uLong x;
    
    x = ((uLong)in[0]);
    x += ((uLong)in[1])<<8;
    x += ((uLong)in[2])<<16;
    x += ((uLong)in[3])<<24;
    return x;
}


