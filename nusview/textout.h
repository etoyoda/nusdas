/* textout.h - plain/html/xml テキスト出力ルーチン用ヘッダ
 * 2001-09-03 豊田英司
 */

#ifndef _TEXTOUT_H
#define _TEXTOUT_H

#ifndef NULL
# define NULL 0
#endif

typedef enum {
	HTTP_RESP_OK = 200,
	HTTP_RESP_Created = 201,
	HTTP_RESP_Partial = 206,
	HTTP_RESP_Multiple = 300,
	HTTP_RESP_Bad_Request = 400,
	HTTP_RESP_Forbidden = 403,
	HTTP_RESP_Not_Found = 404,
	HTTP_RESP_Not_Acceptable = 406,
	HTTP_RESP_Gone = 410,
	HTTP_RESP_Internal_Server_Error = 500
#define HTTP_ERR HTTP_RESP_Internal_Server_Error
} http_respcode;

extern int	text_init(const char *subtype);
extern int	tset_content_type(const char *ctype);
extern int	tputs(const char *string, const char *name);
extern int	twrite(const void *buffer, size_t size, size_t nelems);
extern int	twritemem(void *buffer, size_t size, size_t nelems);
extern int	tprintf(const char *name, const char *format, ...);
extern int	eprintf(http_respcode resp, const char *format, ...);
extern int	errprintf(const char *format, ...);
extern int	tagopen(const char *tagname, ...);
extern int	tagclose(const char *tagname);
extern int 	text_end(void);

#endif /* _TEXTOUT_H */
