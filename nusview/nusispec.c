/* nusispec.c
 *             --> driver/web_nusdas/meta_ispec/
 * native    : binary int32 stream
 * option  -m: text/html  table style
 *         -t: text/plain 
 *         -r: text/x-rd 
 *         -x: text/plain hex dump 
 * 2003-02-06  Kenji Hatano 
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "nusdas.h"
#include "nusdim.h"
#include "textout.h"
#include "nwpl_capi.h"

static char rcsid[] = "$Id: nusispec.c,v 1.2 2007-02-27 11:08:33 suuchi04 Exp $";

static char *saito[32] = {
            "                 "
           ,"                 "
           ,"                 "
           ,"                 "
           ,"                 "
           ,"                 "
           ,"                 "
           ,"                 "
           ,"                 "
           ,"AMeDAS (¥¢¥á¥À¥¹)"
           ,"                 "
           ,"                 "
           ,"ISHI (ÀÐ³ÀÅç)    "
           ,"OKSP (²­ÆìSP)    "
           ,"NASP (Ì¾À¥SP)    "
           ,"TANE (¼ï»ÒÅç)    "
           ,"SEFU (Ê¡²¬)      "
           ,"MURO (¼¼¸ÍÌ¨)    "
           ,"HAIG (¹­Åç)      "
           ,"MISA (¾¾¹¾)      "
           ,"TAKA (Âçºå)      "
           ,"NAGO (Ì¾¸Å²°)    "
           ,"TOJI (Ê¡°æ)      "
           ,"MAKI (ÀÅ²¬)      "
           ,"KURU (Ä¹Ìî)      "
           ,"KASH (Åìµþ)      "
           ,"YAHI (¿·³ã)      "
           ,"AKIT (½©ÅÄ)      "
           ,"SEND (ÀçÂæ)      "
           ,"HAKO (È¡´Û)      "
           ,"KUSH (¶üÏ©)      "
           ,"SAPP (»¥ËÚ)      "
};

    int
grids_ispec_native(N_SI4 *ispec, char *path)
{
    size_t       elemsiz;

    tset_content_type("application/x-int32-stream");
    elemsiz = sizeof(N_SI4);

    twrite(ispec, elemsiz, 128);

    return 0;
}


static int
(*handler)(N_SI4 *, char *) = grids_ispec_native;


    int
ispec_nump_htxt(N_SI4 *ispec, char *path)
{
    unsigned char   *p = (unsigned char *)ispec;
    char    str[256];
    int     i;

    tset_content_type("text/plain");

    tprintf(0, "\n");

    tprintf(0, "ISPEC: %s \n", path);

#define CCC(p) (isprint(*(p))?*(p):' ')
    for (i=0; i<512; i+=16) {
        sprintf (str, "%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x : %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
                    , *(p+0), *(p+1), *(p+2), *(p+3)
                    , *(p+4), *(p+5), *(p+6), *(p+7)
                    , *(p+8), *(p+9), *(p+10), *(p+11)
                    , *(p+12), *(p+13), *(p+14), *(p+15)
                    , CCC(p+0), CCC(p+1), CCC(p+2), CCC(p+3)
                    , CCC(p+4), CCC(p+5), CCC(p+6), CCC(p+7)
                    , CCC(p+8), CCC(p+9), CCC(p+10), CCC(p+11)
                    , CCC(p+12), CCC(p+13), CCC(p+14), CCC(p+15));
        tprintf(0, "%8d: %s \n", i, str);
        p += 16;
    }
#undef CCC

    tprintf(0, "\n");

    return 0;
}


int
ispec_dump_text (N_SI4 *ispec, char *path, char *contenttype)
{
    const char   *p = (char *)ispec;
    char   *q, str[256];
    int     i, y, m, d, h, g;

    tset_content_type(contenttype);

    tprintf(0, "\n=begin\n== ispec info\n=end\n");

    tprintf(0, "=begin RT\n");

    if (strcmp(contenttype, "text/plain") == 0)
      tprintf(0, "ISPEC, %s \n", path);

    strncpy (str, p, 4); str[4] = '\0';
    tprintf(0, "¥Ç¡¼¥¿¼ïÊÌ, %s.\n", str);

    nwp_seq2ymdhm(&y, &m, &d, &h, &g, ispec[1]);
    tprintf(0, "ÂÐ¾Ý»þ¹ï* , %4d/%2d/%2d %02d:%02d \n", y, m, d, h, g);

    for (i=0; i<16; i++) {
        tprintf (0, "%10s, %d \n", saito[31-i], (ispec[3] >> (i*2)) & 0x03);
    }
    for (i=0; i< 4; i++) {
        tprintf (0, "%10s, %d \n", saito[15-i], (ispec[2] >> (i*2)) & 0x03);
    }

    nwp_seq2ymdhm(&y, &m, &d, &h, &g, ispec[4]);
    tprintf(0, "½é´ü»þ¹ï* , %4d/%2d/%2d %02d:%02d \n", y, m, d, h, g);

    nwp_seq2ymdhm(&y, &m, &d, &h, &g, ispec[5]);
    tprintf(0, "½èÍý»þ¹ï* , %4d/%2d/%2d %02d:%02d \n", y, m, d, h, g);

    q = str;
    for (i=24; i<127; i++) {
        int     c = *(p+i);
        *q++ = isprint(c) ? c : ' ';
    }
    *q = '\0';
    tprintf(0, "Comment, %s\n", str);

    tprintf(0, "=end\n");

    return 0;
}

int
ispec_dump_text_plain (N_SI4 *ispec, char *path)
{
  ispec_dump_text (ispec, path, "text/plain");
  return 0;
}

int
ispec_dump_text_rd (N_SI4 *ispec, char *path)
{
  ispec_dump_text (ispec, path, "text/x-rd");
  return 0;
}

    int
ispec_dump_html(N_SI4 *ispec, char *path)
{
    const char   *p = (char *)ispec;
    char   *q, str[256];
    int     i, y, m, d, h, g;
    
    tset_content_type("text/html");

    tagopen("html", NULL);
    tagopen("head", NULL);
    tagopen("title", NULL);
/*  tprintf(NULL, "meta for %s", nusdims_to_path(dp));  */
    tprintf(NULL, "ISPEC: %s", path);
    tagclose("title");    
    tagclose("head");
    tputs("\r\n", NULL);

    tagopen("body", NULL);
    tagopen("table", "border", NULL);

#define TABLE_ROW(cname, desc, format, value) \
    tagopen("tr", NULL); \
    tagopen("td", "name", cname, NULL); \
    tputs(desc, NULL); \
    tagclose("td"); \
    tagopen("td", NULL); \
    tprintf(NULL, format, value); \
    tagclose("td"); \
    tagclose("tr"); tputs("\r\n", NULL);

    strncpy (str, p, 4); str[4] = '\0';
    TABLE_ROW("data_kind", "¥Ç¡¼¥¿¼ïÊÌ \t", "%s", str);

    nwp_seq2ymdhm(&y, &m, &d, &h, &g, ispec[1]);
    sprintf(str, "%4d/%2d/%2d %02d:%02d", y, m, d, h, g);
    TABLE_ROW("valid", "ÂÐ¾Ý»þ¹ï* \t", "%s", str);


    for (i=0; i<16; i++) {
        sprintf (str, "\t %s ", saito[31-i]);
        TABLE_ROW("flag", str, "%d", (ispec[3] >> (i*2)) & 0x03);
    }
    for (i=0; i< 4; i++) {
        sprintf (str, "\t %s ", saito[15-i]);
        TABLE_ROW("flag", str, "%d", (ispec[2] >> (i*2)) & 0x03);
    }

    nwp_seq2ymdhm(&y, &m, &d, &h, &g, ispec[4]);
    sprintf(str, "%4d/%2d/%2d %02d:%02d", y, m, d, h, g);
    TABLE_ROW("init", "½é´ü»þ¹ï* \t", "%s", str);

    nwp_seq2ymdhm(&y, &m, &d, &h, &g, ispec[5]);
    sprintf(str, "%4d/%2d/%2d %02d:%02d", y, m, d, h, g);
    TABLE_ROW("proc", "½èÍý»þ¹ï* \t", "%s", str);

    q = str;
    for (i=24; i<127; i++) {
        int     c = *(p + i);
        *q++ = isprint(c) ? c : ' ';
    }
    *q = '\0';
    TABLE_ROW("comment", "Comment \t", "%s", str);

    tagclose("table");
    tagclose("body");
    tagclose("html");
    tputs("\r\n", NULL);
    return 0;
}


    void
option(const char *str)
{
    if (str == NULL) return;

    if (*str == '-') str++;

    switch (*str) {

      case 't': handler = ispec_dump_text_plain; break;
      case 'r': handler = ispec_dump_text_rd; break;

      case 'm': handler = ispec_dump_html; break;

      case 'x': handler = ispec_nump_htxt; break;

      default : break;
    }
}



    int
main(int argc, char **argv)
{
    struct nus_dims_t     dims;
    int     argind;
    int     r;
    N_SI4   ispec[128];
    char   *path = NULL;


    text_init("text/plain");

    if (argv[1] == NULL) {
        eprintf(HTTP_ERR, "usage: %s <path>\n\n", argv[0]);
        text_end();
        return 1;
    }

    for (argind = 1; argv[argind] != NULL; argind++) {
        if (argv[argind][0] == '-') {
            option(argv[argind]);

        } else {
            if (path == NULL) path = argv[argind];
        }
    }

    r = path_to_nusdims(path, &dims);
    if (r & NUSDIM_ELEMENT) {
        r = nusdas_subc_srf (dims.type.type1, dims.type.type2, dims.type.type3
                            , &dims.base, dims.member, &dims.valid, dims.plane
                            , dims.element, "ISPC", ispec, N_IO_GET);

        if (r == 0) {
            r = handler(ispec, path);
            if (r < 0) {
                eprintf(HTTP_ERR, "nusdims_ispec %d\n", r);
                r = 4;
            }

        } else {
            eprintf(HTTP_ERR, "nusdas_subc_srf %d\n", r);
            r = 3;
        }

    } else {
        eprintf(HTTP_ERR, "element not specified, or other parameter.\n");
        tprintf("X-path", "%s\n", path);
        tprintf("X-resolved-path", "0x%x %s\n", r, nusdims_to_path(&dims));
        r = 2;
    }

    text_end();

    return r;
}
