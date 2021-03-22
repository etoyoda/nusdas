/* textout.c - plain/html/xml 統合テキスト出力
 * 2001-09-03 豊田英司
 * 2001-09-04 長谷川昌樹
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "stringplus.h"
#include <nusdas.h>
#include "nusdim.h"
#include "textout.h"

typedef enum {
	TEXT_SUBTYPE_PLAIN,
	TEXT_SUBTYPE_HTML,
	TEXT_SUBTYPE_XML
} text_subtype_t;

/* 両方向リストにしないと挿入効率が悪い
 */
typedef struct namval_t {
	struct namval_t	*next;
	struct namval_t	*prev;
	const char 	*value;
	size_t		size;
} namval_t;

/* こちらは毎回検査するので単方向
 */
typedef struct msgbuf_t {
	struct msgbuf_t	*next;
	struct namval_t	*namval;
	const char	*name;
	int		multilines;
} msgbuf_t;

static http_respcode	response_code = HTTP_RESP_OK;

static const char	*content_type = NULL;
static text_subtype_t	text_subtype = TEXT_SUBTYPE_PLAIN;

static msgbuf_t		bdy_hdr_buf = {NULL, NULL, NULL, 0};

	int
tset_content_type(const char *ctype)
{
	int	r = 0;
	if (bdy_hdr_buf.namval != NULL)
		return -1;

	content_type = ctype;

	if ((ctype == NULL) || (streqi(ctype, "text/plain"))) {
		text_subtype = TEXT_SUBTYPE_PLAIN;
	} else if (streqi(ctype, "text/html")) {
		text_subtype = TEXT_SUBTYPE_HTML;
	} else if (streqi(ctype, "text/xml")) {
		text_subtype = TEXT_SUBTYPE_XML;
	} else {
		/* do nothing */
		r = 1;
	}
	return r;
}

/* text_init - 初期化
 * ctype に内容タイプを指定する。
 */
	int
text_init(const char *ctype)
{
	bdy_hdr_buf.next = NULL;
	bdy_hdr_buf.namval = NULL;
	bdy_hdr_buf.name = NULL;
	bdy_hdr_buf.multilines = 0;
	return tset_content_type(ctype);
}

/*
 * namval_t で作られるリストに長さ size の string を追加する。
 */
	int
add_entry_name(namval_t **head, const char *string, size_t size)
{
	namval_t *this;

	this = malloc(sizeof(namval_t));
	if (this == NULL)
		return -1;
	if (*head)
		(*head)->next = this;
	this->prev = *head;
	this->next = NULL;
	this->value = string;
	this->size = size;
	*head = this;
	return size;
}

/* bdy_hdr_buf に名前 name 長さ size のデータ string を追加する。
 * name が NULL の場合は本文となる。
 * string は malloc() されたものでなければならない。name はなんでもよい。
 */
	int
add_entry(const char *string, size_t size, const char *name)
{
	char *n;
	msgbuf_t *this;
	msgbuf_t *save;

	if (name == NULL)
		return add_entry_name(&(bdy_hdr_buf.namval), string, size);

	for (save = this = &bdy_hdr_buf; this; save = this, this = this->next) {
		if (this->name && strcmp(this->name, name) == 0) {
			this->multilines++;
			return add_entry_name(&(this->namval), string, size);
		}
	}

	n = malloc(strlen(name) + 1);
	if (n) strcpy(n, name);

	this = malloc(sizeof(msgbuf_t));
	if (this == NULL)
		return -1;

	save->next = this;
	this->next = NULL;
	this->namval = NULL;
	this->name = n;
	this->multilines = 0;
	return add_entry_name(&(this->namval), string, size);
}

	int
tputs(const char *string, const char *name)
{
	char	*s;
	size_t	siz;

	if (content_type == NULL) {
		if (string == NULL)
			return 0;
		if (name == NULL) {
			fputs(string, stdout);
		} else {
			fputs(name, stdout);
			putchar(':');
			putchar(' ');
			fputs(string, stdout);
		}
		return 0;
	}

	if (string) {
		siz = strlen(string);
		s = malloc(siz + 1);
		if (s == NULL)
			siz = 0;
		if (s) strcpy(s, string);
	} else {
		s = NULL;
		siz = 0;
	}
	if (name == NULL) {
		return add_entry(s, siz, NULL);
	} else {
		char	*p;
		for (p = s; *p; p++) {
			if ((*p == '\n') || (*p == '\r'))
				*p = ' ';
		}
		return add_entry(s, siz, name);
	}
}

	void
network_byteord(char *buffer, size_t size, size_t nelems)
{
	char	tmp[16];
	size_t	i, j;

	if (size <= 1 || size > 16)
		return;

	*((int *)tmp) = 0x12345678;
	if (tmp[0] == 0x12)
		return;

	for (i = 0; i < nelems; i++) {
		for (j = 0; j < size; j++) {
			tmp[j] = buffer[i * size + j];
		}
		for (j = 0; j < size; j++) {
			buffer[(i + 1) * size - (j + 1)] = tmp[j];
		}
	}
}

	int
twrite(const void *buffer, size_t size, size_t nelems)
{
	void *newbuffer;
	newbuffer = malloc(size * nelems);
	if (newbuffer == NULL)
		return -1;
	memcpy(newbuffer, buffer, size * nelems);
	network_byteord(newbuffer, size, nelems);
	return add_entry(newbuffer, size * nelems, NULL);
}

	int
twritemem(void *buffer, size_t size, size_t nelems)
{
	network_byteord(buffer, size, nelems);
	return add_entry(buffer, size * nelems, NULL);
}

#define TPRINTF_BUFLEN 2048

	int
tprintf(const char *name, const char *format, ...)
{
	va_list		ap;
	int		r;
	char	buffer[TPRINTF_BUFLEN + 1];
	va_start(ap, format);
	r = vsprintf(buffer, format, ap);
	va_end(ap);
	if (r < 0)
		return r;
	buffer[TPRINTF_BUFLEN] = '\0';
	if (tputs(buffer, name) < 0)
		return -1;
	return r;
}

	int
tagopen(const char *tagname, ...)
{
	va_list		ap;
	char		*attrname, *attrvalue;
	int		r;

	if ((text_subtype != TEXT_SUBTYPE_HTML)
		&& (text_subtype != TEXT_SUBTYPE_XML)) {
		return 0;
	}
	r = tprintf(NULL, "<%s", tagname);
	if (r < 0)
		return r;
	
	va_start(ap, tagname);
	for (;;) {
		attrname = va_arg(ap, char *);
		if (!attrname)
			break;
		r = tprintf(NULL, " %s", attrname);
		attrvalue = va_arg(ap, char *);
		if (!attrvalue)
			break;
		r = tprintf(NULL, "='%s'", attrvalue);
		if (r < 0)
			break;
	}
	va_end(ap);
	if (r < 0)
		return r;
	r = tprintf(NULL, ">");
	return r;
}

	int
tagclose(const char *tagname)
{
	if ((text_subtype != TEXT_SUBTYPE_HTML)
		&& (text_subtype != TEXT_SUBTYPE_XML)) {
		return 0;
	}
	return tprintf(NULL, "</%s>", tagname);
}

	int
eprintf(http_respcode resp, const char *format, ...)
{
	va_list		ap;
	int		r;
	char	buffer[TPRINTF_BUFLEN + 1];

	response_code = resp;

	va_start(ap, format);
	r = vsprintf(buffer, format, ap);
	va_end(ap);
	if (r < 0)
		return r;

	buffer[TPRINTF_BUFLEN] = '\0';
	if (tputs(buffer, NULL) < 0)
		return -1;
	return r;
}

#ifndef ACTIVATE_NUSDAS_STDERR

	int
errprintf(const char *format, ...)
{
	va_list	ap;
	int	r;
	char	buffer[TPRINTF_BUFLEN + 1];

	va_start(ap, format);
	r = vsprintf(buffer, format, ap);
	va_end(ap);
	if (r < 0)
		return r;

	buffer[TPRINTF_BUFLEN] = '\0';
	if (tputs(buffer, "X-Nusdas-Warning") < 0)
		return -1;
	return r;
}

	int
errflush(void)
{
	return 0;
}

	void
errsys(const char *cause)
{
	tprintf("X-Nusdas-Syserr", "%s: %s\n", cause, strerror(errno));
}
#endif

static char *reason_phrase[1000];

void
init_reason_phrase()
{
  int i;
  for (i = 0; i < 1000; i++)
    reason_phrase[i] = "";
  reason_phrase[100] = "Continue";
  reason_phrase[101] = "Switching Protocols";
  reason_phrase[200] = "OK";
  reason_phrase[201] = "Created";
  reason_phrase[202] = "Accepted";
  reason_phrase[203] = "Non-Authoritative Information";
  reason_phrase[204] = "No Content";
  reason_phrase[205] = "Reset Content";
  reason_phrase[206] = "Partial Content";
  reason_phrase[300] = "Multiple Choices";
  reason_phrase[301] = "Moved Permanently";
  reason_phrase[302] = "Found";
  reason_phrase[303] = "See Other";
  reason_phrase[304] = "Not Modified";
  reason_phrase[305] = "Use Proxy";
  reason_phrase[307] = "Temporary Redirect";
  reason_phrase[400] = "Bad Request";
  reason_phrase[401] = "Unauthorized";
  reason_phrase[402] = "Payment Required";
  reason_phrase[403] = "Forbidden";
  reason_phrase[404] = "Not Found";
  reason_phrase[405] = "Method Not Allowed";
  reason_phrase[406] = "Not Acceptable";
  reason_phrase[407] = "Proxy Authentication Required";
  reason_phrase[408] = "Request Time-out";
  reason_phrase[409] = "Conflict";
  reason_phrase[410] = "Gone";
  reason_phrase[411] = "Length Required";
  reason_phrase[412] = "Precondition Failed";
  reason_phrase[413] = "Request Entity Too Large";
  reason_phrase[414] = "Request-URI Too Large";
  reason_phrase[415] = "Unsupported Media Type";
  reason_phrase[416] = "Requested range not satisfiable";
  reason_phrase[417] = "Expectation Failed";
  reason_phrase[500] = "Internal Server Error";
  reason_phrase[501] = "Not Implemented";
  reason_phrase[502] = "Bad Gateway";
  reason_phrase[503] = "Service Unavailable";
  reason_phrase[504] = "Gateway Time-out";
  reason_phrase[505] = "HTTP Version not supported";
}

	int
text_end(void)
{
	msgbuf_t	*msg;
	namval_t	*cur;
	size_t		siz;

	siz = 0;
	for (cur = bdy_hdr_buf.namval; cur; cur = cur->prev) {
		siz += cur->size;
	}

	if (reason_phrase[0] == NULL)
	  init_reason_phrase();
	if (content_type != NULL) {

		fprintf(stdout, "HTTP/1.1 %03d %s\r\n",
			response_code, reason_phrase[response_code]);
		fprintf(stdout, "Content-Length: %d\r\n", (int)siz);
		fprintf(stdout, "Content-Type: %s\r\n", content_type);

		for (msg = bdy_hdr_buf.next; msg; msg = msg->next) {
			if (msg->name == NULL)
				continue;
			fputs(msg->name, stdout);
			if (*msg->name)
				fputs(": ", stdout);
			for (cur = msg->namval; cur && cur->prev; cur = cur->prev)
				;
			if (msg->multilines)
				fputs("\r\n", stdout);
			for (; cur; cur = cur->next) {
				if (cur->value == NULL)
					continue;
				if (msg->multilines)
					fputs("\t", stdout);
				fwrite(cur->value, 1, cur->size, stdout);
				fputs("\r\n", stdout);
			}
		}

		fputs("\r\n", stdout);
	}

	for (cur = bdy_hdr_buf.namval; cur && cur->prev; cur = cur->prev)
		;
	for (;cur ;cur = cur->next) {
		if (cur->value == NULL)
			continue;
		fwrite(cur->value, 1, cur->size, stdout);
	}
	return 0;
}
