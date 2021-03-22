/*-----------------------------------------------------------------------
Pandora client library by tabito 
     header file                      2003.5.10  HARA Tabito

  ------------------------------------------------------------------------*/

#ifndef PANDORA_LIB_H
#define PANDORA_LIB_H

#include "config.h"

#ifdef USE_NET

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>

#define PDR_STR_SIZE 256

#define PDR_ERR_ALLOC -10
#define PDR_ERR_NOT_ALLOCATED -11
#define PDR_ERR_NO_BUF -20
#define PDR_ERR_DISCONNECT -21
#define PDR_ERR_NOT_CONNECTED -22
#define PDR_ERR_STR_TOO_LONG -23
#define PDR_ERR_NOT_SET -24
#define PDR_ERR_GETHOSTBYNAME -25
#define PDR_ERR_TIMEOUT -26
#define PDR_ERR_SOCKET -30
#define PDR_ERR_CONNECT -31
#define PDR_ERR_NULL -90

typedef struct pdr_http_header
{
    char *name;
    char *value;
    struct pdr_http_header *next;
}pdr_http_header;

typedef struct {
    char *version;
    int code;
    char *phrase;
} pdr_http_status;


typedef struct{
    char *host;
    int port;
    char *root;
    char *path;
    char *resource_type;
    pdr_http_status http_status;
    pdr_http_header **hdr_table;
    unsigned char *data;
    int len;
    int alloc_byte;
    int socket;
    int timeout;
    int connect_timeout;
    int read_retry;
    int connect_retry;
    char *proxy_host;
    int proxy_port;
    char *req_path;
    pdr_http_header **req_hdr_table;
} pandora_data;


pandora_data* pdr_new();
int pdr_delete(pandora_data *pdr);
int pdr_data_free(pandora_data *pdr);
int pdr_set_server(pandora_data *pdr, char *server);
int pdr_set_path(pandora_data *pdr, char *path);
int pdr_set_root(pandora_data *pdr, char *root);
int pdr_set_resource_type(pandora_data *pdr, char *resource_type);
int pdr_set_timeout(pandora_data *pdr, int timeout);
int pdr_set_connect_timeout(pandora_data *pdr, int timeout);
int pdr_set_read_retry(pandora_data *pdr, int read_retry);
int pdr_set_connect_retry(pandora_data *pdr, int connect_retry);
int pdr_set_host_header(pandora_data *pdr, char *host_header);
int pdr_set_accept_header(pandora_data *pdr, char *accept_header);
int pdr_set_accept_encoding_header(pandora_data *pdr, char *header);
int pdr_set_proxy(pandora_data *pdr, char *proxy);
int pdr_process(pandora_data *pdr);
void* pdr_get_data(pandora_data *pdr);
int pdr_get_data_len(pandora_data *pdr);
int pdr_get_status_code(pandora_data *pdr);
int pdr_sock_close(pandora_data *pdr);
char* pdr_header_find(pandora_data *pdr, char *header);
int pdr_print_all_headers(pandora_data *pdr, FILE *fp);
char* pdr_get_request_path(pandora_data *pdr);
char *pdr_get_host(pandora_data *pdr);
int pdr_get_port(pandora_data *pdr); 
int pdr_req_hdr_init(pandora_data *pdr);

#endif
/* ifdef USE_NET */
#endif
/* ifdef PANDORA_LIB_H */
