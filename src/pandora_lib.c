/*-------------------------------------------------------------------------
Pandora client library by tabito
                  2003.5.10  HARA Tabito 
-------------------------------------------------------------------------*/
#include "config.h"
#include "pandora_lib.h"
#ifdef NUSDAS_CONFIG_H
# include "sys_mem.h"
#else
#endif

#if 0
#define CONNECTION_DEBUG
#endif
#if 0
#define CONNECTION_RETRY_DEBUG
#endif

#define DEFAULT_PORT 8080
#define BUF_SIZE 8192
#define READ_SIZE 8192
#define PDR_DEFAULT_CONNECT_TIMEOUT 5
#define PDR_DEFAULT_TIMEOUT 900
#define PDR_DEFAULT_READ_RETRY 1
#define PDR_DEFAULT_CONNECT_RETRY 5
#define PDR_HASH_NUM 11
#ifndef INADDR_NONE
# define INADDR_NONE -1
#endif

#define CONNECT connect_with_timeout

#ifndef errno
extern int errno;
#endif

struct connect_arg{
    int sock;
    const struct sockaddr *server;
    int salen;
    int rtcode;
};

struct st_connection{
    int socket;
    struct sockaddr_in server;
    struct st_connection *prev;
    struct st_connection *next;
};

static struct st_connection *con_pool = NULL;

int run_with_timeout (double timeout, void (*fun) (void *), void *arg);

static int pdr_init(pandora_data *pdr);
static int pdr_free(pandora_data *pdr);
static int readable_timeo(int fd, int sec);
static int hex_to_int(char *hex);
static int pdr_connect(pandora_data *pdr, struct sockaddr_in *server);
static int pdr_parse_status(pandora_data *pdr, char *str);
static pdr_http_header* pdr_parse_header(pandora_data *pdr, char *str);
static int pdr_get_response(pandora_data *pdr);
static int pdr_put_request(pandora_data *pdr);
static int pdr_hash(char *key);
static void pdr_header_table_init(pdr_http_header *hash_tbl[]);
static pdr_http_header* pdr_header_table_insert(pdr_http_header *hash_tbl[], 
                                   char* name, char* value);
static void pdr_header_table_free(pdr_http_header **hdr);
static char *pdr_make_path(pandora_data *pdr);

static void connect_with_timeout_callback(void *arg);
static int connect_with_timeout(int sockfd, const struct sockaddr *server,                       int salen, int nsec);

static struct st_connection* find_connection(struct sockaddr_in *server);
static int add_connection(struct sockaddr_in *server, int socket);
static int delete_connection(struct sockaddr_in *server);
static int delete_all_connection();
#ifdef CONNECTION_DEBUG
static void dump_connection();
#endif
/*--------------------------- timeout -------------------------------------*/
#undef USE_SIGNAL_TIMEOUT
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_SETJMP_H
# include <setjmp.h>
#endif
#ifndef HAVE_SIGSETJMP
/* If sigsetjmp is a macro, configure won't pick it up. */
# ifdef sigsetjmp
#  define HAVE_SIGSETJMP
# endif
#endif

#ifdef HAVE_SIGNAL
# ifdef HAVE_SIGSETJMP
#  define USE_SIGNAL_TIMEOUT
# endif
# ifdef HAVE_SIGBLOCK
#  define USE_SIGNAL_TIMEOUT
# endif
#endif

#ifdef USE_SIGNAL_TIMEOUT
# ifdef HAVE_SIGSETJMP
#  define SETJMP(env) sigsetjmp (env, 1)

static sigjmp_buf run_with_timeout_env;

static RETSIGTYPE
abort_run_with_timeout (int sig)
{
    assert (sig == SIGALRM);
    siglongjmp (run_with_timeout_env, -1);
}
# else /* not HAVE_SIGSETJMP */
#  define SETJMP(env) setjmp (env)

static jmp_buf run_with_timeout_env;

static RETSIGTYPE
abort_run_with_timeout (int sig)
{
    assert (sig == SIGALRM);

    int mask = siggetmask ();
    mask &= ~sigmask (SIGALRM);
    sigsetmask (mask);

    /* Now it's safe to longjump. */
    longjmp (run_with_timeout_env, -1);
}
# endif /* not HAVE_SIGSETJMP */

static void
alarm_set (double timeout)
{
#ifdef ITIMER_REAL
    /* Use the modern itimer interface. */
    struct itimerval itv;
    memset (&itv, 0, sizeof (itv));
    itv.it_value.tv_sec = (long) timeout;
    itv.it_value.tv_usec = 1000000L * (timeout - (long)timeout);
    if (itv.it_value.tv_sec == 0 && itv.it_value.tv_usec == 0)
        /* Ensure that we wait for at least the minimum interval.
           Specifying zero would mean "wait forever".  */
        itv.it_value.tv_usec = 1;
    setitimer (ITIMER_REAL, &itv, NULL);
#else  /* not ITIMER_REAL */
    /* Use the old alarm() interface. */
    int secs = (int) timeout;
    if (secs == 0)
        /* Round TIMEOUTs smaller than 1 to 1, not to zero.  This is
       because alarm(0) means "never deliver the alarm", i.e. "wait
       forever", which is not what someone who specifies a 0.5s
       timeout would expect.  */
        secs = 1;
    alarm (secs);
#endif /* not ITIMER_REAL */
}

/* Cancel the alarm set with alarm_set. */

static void
alarm_cancel (void)
{
#ifdef ITIMER_REAL
    struct itimerval disable;
    memset (&disable, 0, sizeof (disable));
    setitimer (ITIMER_REAL, &disable, NULL);
#else  /* not ITIMER_REAL */
    alarm (0);
#endif /* not ITIMER_REAL */
}

int
run_with_timeout (double timeout, void (*fun) (void *), void *arg)
{
    int saved_errno;

    if (timeout == 0)
    {
        fun (arg);
        return 0;
    }

    signal (SIGALRM, abort_run_with_timeout);
    if (SETJMP (run_with_timeout_env) != 0)
    {
        /* Longjumped out of FUN with a timeout. */
        signal (SIGALRM, SIG_DFL);
        return 1;
    }
    alarm_set (timeout);
    fun (arg);
    /* Preserve errno in case alarm() or signal() modifies it. */
    saved_errno = errno;
    alarm_cancel ();
    signal (SIGALRM, SIG_DFL);
    errno = saved_errno;

    return 0;
}

#else  /* not USE_SIGNAL_TIMEOUT */

#ifndef WINDOWS
int
run_with_timeout (double timeout UNUSED, void (*fun) (void *), void *arg)
{
    fun (arg);
    return 0;
}
#endif /* not WINDOWS */
#endif /* not USE_SIGNAL_TIMEOUT */

/*--------------------------  end  timeout --------------------------------*/

/*-------------------------------------------------------------------------*/
/*
=begin
---pandora_data* pdr_new()
   あたらしい pandora_data オブジェクトを作成して初期化する．

   成功すれば，そのポインタを，失敗すればNULLを返す．
=end
 */

pandora_data* pdr_new()
{
    pandora_data *pdr;

    pdr = (pandora_data*)nus_malloc(sizeof(pandora_data));
    if(pdr != NULL){
        pdr_init(pdr);
    }
    return pdr;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_delete(pandora_data *pdr)
   pandora_dataオブジェクトを削除する．
   引数:
   * pdr: 処理対象の pandora_data オブジェクトのポインタ

   返却値:
   * 0: 正常
   * -11 : pdr がallocateされていない
=end
 */

int pdr_delete(pandora_data *pdr)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    pdr_free(pdr);
    nus_free(pdr);
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_data_free(pandora_data *pdr)
   データ(http status, header, body)の領域を解放する．
   pandora_data オブジェクト自体は削除されず，set_***関数で設定した属性
   についてはそのまま残る．
   引数:
   * pdr: 処理対象の pandora_data オブジェクトのポインタ

   返却値
   * 0: 正常
   * -11: pdrがallocateされていない
=end
 */
int pdr_data_free(pandora_data *pdr)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    nus_free(pdr->data);
    pdr->data = NULL;
    pdr->alloc_byte = 0;
    pdr->len = 0;
    pdr_header_table_free(pdr->hdr_table);
    pdr_header_table_init(pdr->hdr_table);

    nus_free(pdr->http_status.version);
    pdr->http_status.version = NULL;
    pdr->http_status.code = 0;
    nus_free(pdr->http_status.phrase);
    pdr->http_status.phrase = NULL;
    return 0;

}
/*-------------------------------------------------------------------------*/
/*
=begin  
---int pdr_set_server(pandora_data *pdr, char *server)
   pdrオブジェクトに対して，リクエストを出すサーバー名(port番号も含む)を
   設定する．指定の仕方は

    server:port

   であり，serverで示される文字列は\0で終了していなければならない．

   ポート番号が指定されていない場合は，8080に設定される．

   この関数を呼び出した結果，サーバーが変更されるようであれば，接続中の
   コネクションは切断される．(この関数を呼び出しても変更がなければ切断
   されない)

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * server : 設定するサーバー名の文字列の先頭ポインタ(\0終端)

   返却値
   * 0: 正常
   * 1: 正常(設定が解除された)
   * -10: メモリーの確保に失敗
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_server(pandora_data *pdr, char *server)
{
    char *buf, *host;
    int i, len, port;
    int rt_val;

    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    if(server == NULL){
        nus_free(pdr->host);
        pdr->host = NULL;
        port = 0;
        return 1;
    }

    len = strlen(server);

    if((buf =(char*)nus_malloc(len+1))==NULL){
        fprintf(stderr, "nus_malloc error:%s,%d\n",__FILE__,__LINE__);
        rt_val = PDR_ERR_ALLOC;
        goto End;
    }

    strcpy(buf, server);
    port = -1;

    host = buf;
    for(i=0; i<len; i++){
        if(*(buf+i) == ':'){
            *(buf+i) = 0x00;
            port = atoi(buf+i+1);
            if(port == 0){
                port = DEFAULT_PORT;
            }
            break;
        }
    }
    if(port == -1){
        port = DEFAULT_PORT;
    }
    if(pdr->host == NULL ||(pdr->port != port || strcmp(pdr->host,host)!=0)){
        /*
        if(pdr->socket > 0){
            close(pdr->socket);
            pdr->socket = -1;
        }
        */
        nus_free(pdr->host);
	pdr->host = NULL;
        if((pdr->host =(char*)nus_malloc(strlen(host)+1))==NULL){
            fprintf(stderr,"nus_malloc error:%s, %d\n",__FILE__,__LINE__);
            rt_val = PDR_ERR_ALLOC;
        }
        strcpy(pdr->host, host);
        pdr->port = port;
    }
    rt_val = 0;
End:
    nus_free(buf);
    return rt_val;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_path(pandora_data *pdr. char *path)
   資源を指定するpathを指定する．pathで示される文字列は\0で
   終了していなければならない．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * path : 設定する資源のpathの文字列の先頭ポインタ(\0終端)

   返却値:
   * 0: 正常
   * 1: 正常(設定が解除された)
   * -10: メモリーの確保に失敗
=end
 */
int pdr_set_path(pandora_data *pdr, char *path)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    if(path == NULL){
        nus_free(pdr->path);
        pdr->path = NULL;
        return 1;
    }
    if(pdr->path == NULL || strcmp(pdr->path,path)!=0){
        nus_free(pdr->path);
        if((pdr->path = (char*)nus_malloc(strlen(path)+1))==NULL){
            fprintf(stderr,"nus_malloc error%s,%d\n",__FILE__,__LINE__);
            return PDR_ERR_ALLOC;
        }
        strcpy(pdr->path, path);
    }
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_root(pandora_data *pdr, char *root)
   振り分け先(資源を示すパスの前につく)を指定する．
   rootで示される文字列は\0で終了していなければならない．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * root : 設定する振り分け先の文字列の先頭ポインタ(\0終端)

   返却値
   * 0: 正常
   * 1: 正常(設定が解除された)
   * -10: メモリーの確保に失敗
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_root(pandora_data *pdr, char *root)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    if(root == NULL){
        nus_free(pdr->root);
        pdr->root = NULL;
        return 1;
    }
    if(pdr->root == NULL || strcmp(pdr->root,root)!=0){
        nus_free(pdr->root);
        if((pdr->root = (char*)nus_malloc(strlen(root)+1))==NULL){
            fprintf(stderr,"nus_malloc error%s,%d\n",__FILE__,__LINE__);
            return PDR_ERR_ALLOC;
        }
        strcpy(pdr->root, root);
    }
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_resource_type(pandora_data *pdr, char *resource_type)
   取得するデータの種別を指定する．たとえば，data.f32, meta.htmlなど．
   resource_typeで示される文字列は\0で終了していなければならない．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * resource_type : 設定するデータ種別の文字列の先頭ポインタ(\0終端)

   返却値
   * 0: 正常
   * 1: 正常(設定が解除された)
   * -10: メモリーの確保に失敗
   * -11: 対象のpdrがallocateされていない
=end
 */

int pdr_set_resource_type(pandora_data *pdr, char *resource_type)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    if(resource_type == NULL){
        nus_free(pdr->resource_type);
        pdr->resource_type = NULL;
        return 1;
    }
    if(pdr->resource_type == NULL 
       || strcmp(pdr->resource_type,resource_type)!=0){
        nus_free(pdr->resource_type);
        if((pdr->resource_type = (char*)nus_malloc(strlen(resource_type)+1))
           ==NULL){
            fprintf(stderr,"nus_malloc error%s,%d\n",__FILE__,__LINE__);
            return PDR_ERR_ALLOC;
        }
        strcpy(pdr->resource_type, resource_type);
    }
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_req_hdr_init(pandora_data *pdr)
   レスポンスのヘッダを格納するテーブルを初期化する

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ

   返却値
   * 0: 正常

=end
 */
int pdr_req_hdr_init(pandora_data *pdr)
{
    pdr_header_table_free(pdr->req_hdr_table);
    pdr_header_table_init(pdr->req_hdr_table);

    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_host_header(pandora_data *pdr, char *host_header)
   Host ヘッダーの文字列を指定する．
   host_headerで示される文字列は\0で終了していなければならない．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * host_header : 設定するHostヘッダーの文字列の先頭ポインタ(\0終端)

   返却値
   * 0: 正常
   * 1: 正常(設定が解除された)
   * -10: メモリーの確保に失敗
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_host_header(pandora_data *pdr, char *host_header)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    if(host_header != NULL){
        pdr_header_table_insert(pdr->req_hdr_table, 
                                "Host", host_header);
    }
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_accept_header(pandora_data *pdr, char *accept_header)
   Accept ヘッダーの文字列を指定する．
   accept_headerで示される文字列は\0で終了していなければならない．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * accept_header : 設定するAcceptヘッダーの文字列の先頭ポインタ(\0終端)

   返却値
   * 0: 正常
   * 1: 正常(設定が解除された)
   * -10: メモリーの確保に失敗
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_accept_header(pandora_data *pdr, char *accept_header)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    if(accept_header != NULL){
        pdr_header_table_insert(pdr->req_hdr_table, 
                                "Accept", accept_header);
    }
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_accept_encoding_header(pandora_data *pdr, char *header)
   Accept-Encoding ヘッダーの文字列を指定する．
   headerで示される文字列は\0で終了していなければならない．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * header : 設定する Accept-Encoding ヘッダーの文字列の先頭ポインタ(\0終端)

   返却値
   * 0: 正常
   * 1: 正常(設定が解除された)
   * -10: メモリーの確保に失敗
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_accept_encoding_header(pandora_data *pdr, char *header)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    if(header != NULL){
        pdr_header_table_insert(pdr->req_hdr_table, 
                                "Accept-Encoding", header);
    }
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_proxy(pandora_data *pdr, char *proxy)
   proxy サーバを使用する際に指定する．
   指定の方法は pdr_set_serverと同じである．

   proxy サーバーの指定を解除するためには，

    pdr_set_proxy(pdr, "");
    または
    pdr_set_proxt(pdr, NULL);

   とする．

   pdr_set_server と同様，この関数の呼び出しの結果，proxy サーバーが変更
   された場合は接続中のコネクションは切断される．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * proxy : 設定する proxy サーバーの文字列の先頭ポインタ(\0終端)

   返却値
   * 0: 正常(proxy_serverがセットされた)
   * 1: 正常(proxy_serverの設定が解除された)
   * -10: メモリーの確保に失敗
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_proxy(pandora_data *pdr, char *proxy)
{
    char *buf, *host;
    int i, len, port;
    int rt_val;

    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }

    if(proxy == NULL || (len = strlen(proxy))== 0){
        nus_free(pdr->proxy_host);
        pdr->proxy_host = NULL;
        pdr->proxy_port = 0;
        return 1;
    }

    if((buf =(char*)nus_malloc(len+1))==NULL){
        fprintf(stderr, "nus_malloc error:%s,%d\n",__FILE__,__LINE__);
        rt_val = PDR_ERR_ALLOC;
        goto End;
    }

    strcpy(buf, proxy);
    port = -1;

    host = buf;
    for(i=0; i<len; i++){
        if(*(buf+i) == ':'){
            *(buf+i) = 0x00;
            port = atoi(buf+i+1);
            if(port == 0){
                port = DEFAULT_PORT;
            }
            break;
        }
    }
    if(port == -1){
        port = DEFAULT_PORT;
    }
    if(pdr->proxy_host == NULL ||(pdr->proxy_port != port 
                                  || strcmp(pdr->proxy_host,host)!=0)){
        /*
        if(pdr->socket > 0){
            close(pdr->socket);
            pdr->socket = -1;
        }
        */
        nus_free(pdr->proxy_host);
        if((pdr->proxy_host =(char*)nus_malloc(strlen(host)+1))==NULL){
            fprintf(stderr,"nus_malloc error:%s, %d\n",__FILE__,__LINE__);
            rt_val = PDR_ERR_ALLOC;
            goto End;
        }
        strcpy(pdr->proxy_host, host);
        pdr->proxy_port = port;
    }
    rt_val = 0;
End:
    nus_free(buf);
    return rt_val;

}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_timeout(pandora_data *pdr, int timeout)
   タイムアウトを指定する．このタイムアウトはread
   に適用される．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * timeout : 設定する timeout の時間(単位は秒)

   返却値
   * 0: 正常
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_timeout(pandora_data *pdr, int timeout)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    pdr->timeout = timeout;
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_set_connect_timeout(pandora_data *pdr, int timeout)
   タイムアウトを指定する．このタイムアウトはconnection
   に適用される．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * timeout : 設定する timeout の時間(単位は秒)

   返却値
   * 0: 正常
   * -11: 対象のpdrがallocateされていない
=end
 */
int pdr_set_connect_timeout(pandora_data *pdr, int timeout)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    pdr->connect_timeout = timeout;
    return 0;
}
int pdr_set_read_retry(pandora_data *pdr, int read_retry)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    pdr->read_retry = (read_retry > 1) ? read_retry : 1;
    return 0;

}
int pdr_set_connect_retry(pandora_data *pdr, int connect_retry)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    pdr->connect_retry = (connect_retry > 2) ? connect_retry : 2;
    return 0;

}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_process(pandora_data *pdr)
   Pandora Serverにリクエストを出して，データの取得が行なわれる．

   連続してこの関数を呼び出すと，その前に取得したデータを解放して
   (pdr_data_freeが呼び出される)からデータ取得をすることに注意．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ

   返却値
    * ステータスコードが，200の場合には，取得したデータのサイズを返す．
    * それ以外の場合には，ステータスコードに-1をかけた値を返す．
    * その他のエラーは-99以上0未満の値を返す．
=end
 */
int pdr_process(pandora_data *pdr)
{
    int size;
    int code;
    int rt;
    int connect_retry = 0, read_retry = 0;
    struct sockaddr_in server; 

    if(pdr == NULL){
        return -99;
    }

    pdr_data_free(pdr);

    if(pdr->host[0]==0x00 || pdr->port == 0){
        fprintf(stderr, "pdr_process: Not set host , port\n");
        return PDR_ERR_NOT_SET;
    }

connect:
    rt = pdr_connect(pdr, &server);
    if(rt < 0){
        fprintf(stderr,"pdr_connect failed: host= %s:%d%s\n",
                pdr->host, pdr->port, pdr_get_request_path(pdr));
        return PDR_ERR_CONNECT;
    }

    rt = pdr_put_request(pdr);
    if(rt < 0){
#ifdef CONNECTION_DEBUG
        fprintf(stderr, "pdr_put_request:disconnected!\n");
#endif        
        delete_connection(&server);
        pdr->socket = -1;
        /*
        close(pdr->socket);
        pdr->socket = -1;
        */
        connect_retry ++;
        if(connect_retry >= pdr->connect_retry){
            perror("pdr_put_request");
            return PDR_ERR_DISCONNECT;
        }
        goto connect;
    }
    rt = pdr_get_response(pdr);
    if(rt == PDR_ERR_NOT_CONNECTED || rt == PDR_ERR_DISCONNECT){
        delete_connection(&server);
        pdr->socket = -1;
        connect_retry ++;
        if(connect_retry >= pdr->connect_retry){
            perror("pdr_get_response");
            return PDR_ERR_DISCONNECT;
        }
        goto connect;
    }
    else if(rt == PDR_ERR_TIMEOUT){
        delete_connection(&server);
        pdr->socket = -1;
        read_retry++;
        if(read_retry >= pdr->read_retry){
            char *req_path;
            req_path = pdr_make_path(pdr);
            fprintf(stderr,"pdr_process: pdr_get_response failed(read_retry exceeded.):rt = %d\n", rt);
            fprintf(stderr,"Requested URL http://%s:%d%s\n",pdr->host,pdr->port,
                req_path);
            return rt;
        }
        fprintf(stderr, "Read Retry: %d\n", read_retry);
        goto connect;
    }
    else if(rt < 0){
        char *req_path;
        /* fprintf(stderr,"Failed to get response\n"); */
        req_path = pdr_make_path(pdr);
        fprintf(stderr,"pdr_process: pdr_get_response failed:rt = %d\n", rt);
        fprintf(stderr,"Requested URL http://%s:%d%s\n",pdr->host,pdr->port,
                req_path);
        return rt;
    }
    else{
        size = rt;
    }
    
    code = pdr_get_status_code(pdr);
    if(code != 200){
        return -1*code;
    }
    return size;
}
/*------------------------------------------------------------------------*/
/*
=begin
---void* pdr_get_data(pandora_data *pdr)
   取得したデータの先頭のポインターを返す．
   データがない場合はNULLを返す．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
=end
*/
void* pdr_get_data(pandora_data *pdr)
{
    if(pdr == NULL){
        return NULL;
    }
    return (pdr->data);
}
/*------------------------------------------------------------------------*/
/*
=begin
---int pdr_get_data_len(pandora_data *pdr)
   取得したデータのサイズを返す．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
=end
 */
int pdr_get_data_len(pandora_data *pdr)
{
    if(pdr == NULL){
        return 0;
    }
    return (pdr->len);
}
/*------------------------------------------------------------------------*/
/*
=begin
---int pdr_get_status_code(pandora_data *pdr)
   HTTP ステータスコードを返す

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
=end
 */
int pdr_get_status_code(pandora_data *pdr)
{
    if(pdr == NULL){
        return PDR_ERR_NULL;
    }
    return (pdr->http_status.code);
}
/*------------------------------------------------------------------------*/
/*
=begin
---int pdr_sock_close(pandora_data *pdr)
   コネクションを強制的に切断する．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ

   返却値
   * 0 : 正常終了
   * <0 : エラー
=end
 */
int pdr_sock_close(pandora_data *pdr)
{
    int rt = 0;
    
    if(pdr == NULL){
        return PDR_ERR_NULL;
    }
    rt = close(pdr->socket);
    pdr->socket = -1;

    return rt;
}
/*-----------------------------------------------------------------------*/
/*
=begin
---char* pdr_header_find(pandora_data *pdr, char *header)
   headerで示されるヘッダーを検索して，その値の文字列(\0終端)の先頭ポインタを返す．
   検索の際には大文字/小文字の区別はしない．

   該当するものがない場合はNULLを返す．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * header : 探索する header の文字列の先頭ポインタ(\0終端)
=end
 */
char* pdr_header_find(pandora_data *pdr, char *header)
{
    int hash_val;
    pdr_http_header *p, *hdr;

    hash_val = pdr_hash(header);
    hdr = pdr->hdr_table[hash_val];
    for(p = hdr; p!=NULL; p = p->next){
        size_t len, i;
        if((len=strlen(p->name))==strlen(header)){
            for(i=0; i<len ;i++){
                if(tolower(p->name[i])!=tolower(header[i])){
                    break;
                }
            }
            if(i==len){
                return p->value;
            }

        }
        else{
            continue;
        }
    }

    return NULL;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_print_all_headers(pandora_data *pdr, FILE *fp)
   受けとったヘッダーをすべて，fpに出力する．

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
   * fp : 出力先 FILE ポインタ
   
   返却値
   * 0: 正常
   * -99: I/Oエラー
=end
*/

int pdr_print_all_headers(pandora_data *pdr, FILE *fp)
{
    int i;
    pdr_http_header *p;

    if(fp == NULL){
        return -99;
    }
    for(i=0;i<PDR_HASH_NUM;i++){
        for(p=pdr->hdr_table[i];p!=NULL;p=p->next){
            fprintf(fp,"%s: %s\r\n",p->name,p->value);
        }
    }
    fprintf(fp,"\r\n");
    return 0;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---char* pdr_get_request_path(pandora_data *pdr)
   リクエストとして出すパス文字列(\0終端)の先頭ポインタを返す

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
=end

 */
char* pdr_get_request_path(pandora_data *pdr)
{
    if(pdr == NULL){
        return NULL;
    }
    nus_free(pdr->req_path);
    pdr->req_path = pdr_make_path(pdr);
    return pdr->req_path;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---char* pdr_get_host(pandora_data *pdr)
   pdr にセットされたhost名文字列(\0終端)の先頭ポインタを返す

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
=end  
 */
char* pdr_get_host(pandora_data *pdr)
{
    if(pdr == NULL){
        return NULL;
    }
    return pdr->host;
}
/*-------------------------------------------------------------------------*/
/*
=begin
---int pdr_get_port(pandora_data *pdr)
   pdr にセットされたport番号を返す

   引数
   * pdr : 処理対象 pandora_data オブジェクトのポインタ
=end  
 */
int pdr_get_port(pandora_data *pdr)
{
    if(pdr == NULL){
        return -1;
    }
    return pdr->port;
}
/*-------------------------------------------------------------------------*/
static int pdr_init(pandora_data *pdr)
{
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    pdr->host = NULL;
    pdr->root = NULL;
    pdr->path = NULL;
    pdr->port = 0;
    pdr->data = NULL;
    pdr->resource_type = NULL;

    pdr->http_status.version = NULL;
    pdr->http_status.code = 0;
    pdr->http_status.phrase = NULL;

    pdr->len = 0;
    pdr->alloc_byte = 0;
    pdr->socket = -1;
    pdr->timeout = PDR_DEFAULT_TIMEOUT;
    pdr->connect_timeout = PDR_DEFAULT_CONNECT_TIMEOUT;
    pdr->read_retry = PDR_DEFAULT_READ_RETRY;
    pdr->connect_retry = PDR_DEFAULT_CONNECT_RETRY;

    if((pdr->hdr_table=
        (pdr_http_header**)nus_malloc(sizeof(pdr_http_header*)*PDR_HASH_NUM))
       ==NULL){
        fprintf(stderr,"nus_malloc error:%s,%d\n", __FILE__, __LINE__);
        return PDR_ERR_ALLOC;
    }
    pdr_header_table_init(pdr->hdr_table);

    if((pdr->req_hdr_table=
        (pdr_http_header**)nus_malloc(sizeof(pdr_http_header*)*PDR_HASH_NUM))
       ==NULL){
        fprintf(stderr,"nus_malloc error:%s,%d\n", __FILE__, __LINE__);
        return PDR_ERR_ALLOC;
    }
    pdr_header_table_init(pdr->req_hdr_table);

    pdr->proxy_host = NULL;
    pdr->proxy_port = 0;
    pdr->req_path = NULL;
    return 0;
}
/*-------------------------------------------------------------------------*/
static int pdr_free(pandora_data *pdr)
{
	return 0;
    if(pdr == NULL){
        return PDR_ERR_NOT_ALLOCATED;
    }
    /*
    if(pdr->socket > 0){
        close(pdr->socket);
        pdr->socket = -1;
    }
    */
    delete_all_connection();
    pdr->socket = -1;
    nus_free(pdr->host);
    pdr->host = NULL;
    nus_free(pdr->root);
    pdr->root = NULL;
    nus_free(pdr->path);
    pdr->path = NULL;
    nus_free(pdr->http_status.version);
    pdr->http_status.version = NULL;
    pdr->http_status.code = 0;
    nus_free(pdr->http_status.phrase);
    pdr->http_status.phrase = NULL;

    pdr->port = 0;
    nus_free(pdr->data);
    pdr->data = NULL;
    pdr->len = 0;
    pdr->alloc_byte = 0;
    pdr->socket = -1;
    pdr->timeout = PDR_DEFAULT_TIMEOUT;
    pdr->connect_timeout = PDR_DEFAULT_CONNECT_TIMEOUT;
    pdr->read_retry = PDR_DEFAULT_READ_RETRY;
    pdr->connect_retry = PDR_DEFAULT_CONNECT_RETRY;

    pdr_header_table_free(pdr->hdr_table);
    nus_free(pdr->hdr_table);
    pdr->hdr_table = NULL;

    pdr_header_table_free(pdr->req_hdr_table);
    nus_free(pdr->req_hdr_table);
    pdr->req_hdr_table = NULL;

    nus_free(pdr->proxy_host);
    pdr->proxy_host = NULL;
    pdr->proxy_port = 0;


    nus_free(pdr->req_path);
    pdr->req_path = NULL;
    return 0;
}
/*------------------------------------------------------------------------*/
static int pdr_connect(pandora_data *pdr, struct sockaddr_in *server)
{
    unsigned int dst_ip;
    int s;
    char *host;
    int port;
    int rt;
    struct st_connection *pcon;

    if(pdr == NULL){
        return PDR_ERR_NULL;
    }

    /* set server address */
    /* open socket, and connect */

    if(pdr->proxy_host != NULL){
        host = pdr->proxy_host;
        port = pdr->proxy_port;
    }
    else{
        host = pdr->host;
        port = pdr->port;
    }
    
    /* hostname -> IP address */
    if((dst_ip = inet_addr(host)) == INADDR_NONE){
        struct hostent *he;
        
        if(( he = gethostbyname(host)) == NULL){
            fprintf(stderr, "gethostbyname error\n");
            return PDR_ERR_GETHOSTBYNAME;
        }
        memcpy(&dst_ip, he->h_addr, he->h_length);
    }
    
    memset((char*)server, 0, sizeof(struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = dst_ip;
    server->sin_port = htons((unsigned short)port);
#ifdef CONNECTION_DEBUG
    fprintf(stderr, "find host:%s\n", host);
#endif
    
    if((pcon = find_connection(server)) == NULL){
        if( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("socket");
            return PDR_ERR_SOCKET;
        }
        
        if(CONNECT(s, (struct sockaddr*)server, sizeof(struct sockaddr_in),
                   pdr->connect_timeout)< 0){
            perror("connect");
            close(s);
            return PDR_ERR_CONNECT;
        }
        if((rt = add_connection(server, s)) < 0){
            return rt;
        }
        pdr->socket = s;
#ifdef CONNECTION_DEBUG
        fprintf(stderr, "new connection is created!\n");
#endif
    }
    else{
        pdr->socket = pcon->socket;
#ifdef CONNECTION_DEBUG
        fprintf(stderr, "socket is reused!\n");
#endif

    }

    return 0;
}
/*-------------------------------------------------------------------------*/
static struct st_connection* find_connection(struct sockaddr_in *server)
{
    struct st_connection *p;

    for(p = con_pool; p != NULL; p = p->next){
        if(memcmp(&(p->server), server, sizeof(struct sockaddr_in)) == 0){
            break;
        }
    }
#ifdef CONNECTION_DEBUG
    dump_connection();
    fprintf(stderr, "find_connection end\n");
#endif    
    return p;

} 
/*-------------------------------------------------------------------------*/
static int add_connection(struct sockaddr_in *server, int socket)
{
    struct st_connection *p, *pnew;

#ifdef CONNECTION_DEBUG
    fprintf(stderr, "before add_conection\n");
    dump_connection();
#endif
    if(con_pool == NULL){
        con_pool = (struct st_connection*)nus_malloc(sizeof(struct st_connection));
        if(con_pool == NULL){
            fprintf(stderr, "nus_malloc error:%s, %d\n", __FILE__, __LINE__);
            return -10;
        }
        pnew = con_pool;
        p = NULL;
    }
    else{
        for(p = con_pool; p->next != NULL; p = p->next){
            /* do nothing */
        }
        pnew = (struct st_connection*)nus_malloc(sizeof(struct st_connection));
        if(pnew == NULL){
            fprintf(stderr, "nus_malloc error:%s, %d\n", __FILE__, __LINE__);
            return -10;
        }
        p->next = pnew;
    }
    memcpy(&(pnew->server), server, sizeof(struct sockaddr_in));
    pnew->socket = socket;
    pnew->next = NULL;
    pnew->prev = p;
#ifdef CONNECTION_DEBUG
    fprintf(stderr, "after add_conection\n");
    dump_connection();
#endif

    return 0;
    
}
/*-------------------------------------------------------------------------*/
static int delete_connection(struct sockaddr_in *server)
{
    struct st_connection *p;

#ifdef CONNECTION_DEBUG
    fprintf(stderr, "delete connection!, %s\n", inet_ntoa(p->server.sin_addr));
    fprintf(stderr, "before delete_conection\n");
    dump_connection();
#endif
    for(p = con_pool; p != NULL; p = p->next){
        if(memcmp(&(p->server), server, sizeof(struct sockaddr_in)) == 0){
            if(p->prev != NULL){
                (p->prev)->next = p->next;
            }
            else{ /* first case */
                con_pool = p->next;
            }
            if(p->next != NULL){
                (p->next)->prev = p->prev;
            }
            close(p->socket);
            nus_free(p);
#ifdef CONNECTION_DEBUG
            fprintf(stderr, "after delete_conection\n");
            dump_connection();
#endif
            return 0;
        }
    }
    return 1;

}
/*-------------------------------------------------------------------------*/
static int delete_all_connection()
{
    struct st_connection *p, *next;

    next = NULL;
    for(p = con_pool; p != NULL; p = next){
        next = p->next;
        nus_free(p);
    }
    con_pool = NULL;
    return 0;
}
/*-------------------------------------------------------------------------*/
#ifdef CONNECTION_DEBUG
static void dump_connection()
{
    struct st_connection *p;
    int num;

    num = 1;
    for(p = con_pool; p != NULL; p = p->next){
       fprintf(stderr, "dump_connection:%d, %s\n", num++, inet_ntoa(p->server.sin_addr));
    }
}
#endif
/*-------------------------------------------------------------------------*/
static void connect_with_timeout_callback(void *arg)
{
    struct connect_arg *c_arg = (struct connect_arg *)arg;
    c_arg->rtcode = connect(c_arg->sock, c_arg->server, c_arg->salen);
}
/*-------------------------------------------------------------------------*/
static int connect_with_timeout(int sockfd, const struct sockaddr *server,
                                 int salen, int nsec)
{
    double timeout = (double)nsec;
    struct connect_arg c_arg;
    
    c_arg.sock = sockfd;
    c_arg.server = server;
    c_arg.salen = salen;

    if(run_with_timeout(timeout, connect_with_timeout_callback, &c_arg)){
        errno = ETIMEDOUT;
        fprintf(stderr,"connect time out\n");
        return -1;
    }
    if(c_arg.rtcode == -1 && errno == EINTR){
        errno = ETIMEDOUT;
    }
    return c_arg.rtcode;
}
/*-------------------------------------------------------------------------*/
static int readable_timeo(int fd, int sec)
{
    fd_set rset;
    struct timeval tv;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tv.tv_sec = sec;
    tv.tv_usec = 0;

    return (select(fd+1, &rset, NULL, NULL, &tv));

}
/*---------------------------------------------------------------------*/
static int hex_to_int(char *hex)
{
    int len;
    int rt = 0;
    int i,k;
    unsigned char c;
    int tmp;
    

    len = strlen(hex);
    i = 0;
    while(i < len){
        if(hex[i]==0x20){
            memmove(hex+i, hex+i+1,len-i);
            len--;
        }
        i++;
    }

    i = 0;
    rt = 0;
    while(i < len){
        c = hex[len-i-1];
        if(c>='0' && c<='9'){
            c = c-0x30;
        }
        else if(c>='a' && c<='f'){
            c = c-0x60+9;
        }
        tmp = c;
        for(k=0;k<i;k++){
            tmp *=16;
        }
        rt += tmp;
        i++;
    }

    return rt;
}
/*--------------------------------------------------------------------*/
static int pdr_get_response(pandora_data *pdr)
{
    int buf_len, buf_p;
    char buf[BUF_SIZE], status_buf[BUF_SIZE],
        header_buf[BUF_SIZE],c_size_buf[BUF_SIZE];
    int i,j,k,l;
    int c_len;
    int stage;
    int chunked_flag, close_flag;
    int total_read_size;
    int body_len = 0;
    pdr_http_header *old = NULL;
                

    if(pdr == NULL){
        return PDR_ERR_NULL;
    }

    i=0; j=0; k=0; l=0; c_len = -1;
    stage = 0;
    chunked_flag = 0;
    close_flag = 0;
    total_read_size = 0;

    while(1){
        if(readable_timeo(pdr->socket, pdr->timeout) <= 0){
            fprintf(stderr, "Time Out\n");
            return PDR_ERR_TIMEOUT;
        }
        buf_len = read(pdr->socket, buf, BUF_SIZE);
        if(buf_len <= 0){
            /*
            close(pdr->socket);
            pdr->socket = -1;
            */
            if(total_read_size == 0){
                /* fprintf(stderr,"Not connected!\n"); */
                return PDR_ERR_NOT_CONNECTED;
            }
            else if(close_flag == 0){
                return PDR_ERR_DISCONNECT;
            }
            else{
                goto End;
            }
        }
        total_read_size += buf_len;
        buf_p = 0;
        if(stage == 0){ /* http status */
            while(buf_p < buf_len){
                *(status_buf + i) = *(buf + buf_p++);
                if(*(status_buf+i)=='\n'){
                    *(status_buf+i)='\0';
                    pdr_parse_status(pdr,status_buf);
                    stage = 1;
                    j = 0;
                    break;
                }
                i++;
            }
        }
        if(stage == 1){ /* header */
            while(buf_p < buf_len){
                *(header_buf + j) = *(buf + buf_p++);
                if(*(header_buf + j) == '\n'){
                    if(j!=1){
                        *(header_buf + j - 1)='\0';
                        if(old !=NULL && 
                           (*header_buf == 0x20 || *header_buf =='\t')){
                            char *tmp_value;
                            tmp_value = (char*)nus_realloc(old->value,
                                                       strlen(old->value)+2
                                                       +strlen(header_buf)+1);
                            if(tmp_value == NULL){
                                fprintf(stderr,"nus_malloc error:%s, %d\n",
                                        __FILE__, __LINE__);
                                return(PDR_ERR_ALLOC);
                            }
                            old->value = tmp_value;
                            strcat(old->value, "\r\n");
                            strcat(old->value, header_buf);
                            j = 0;
                        }
                        else{
                            old = pdr_parse_header(pdr,header_buf);
                            if(old == NULL){
                                return PDR_ERR_ALLOC;
                            }
                            j = 0;
                        }
                    }
                    else{
                        char *t_enc, *cont_len, *conn;
                        
                        stage = 2;
                        body_len = 0;
                        l = 0;

                        t_enc = pdr_header_find(pdr, "Transfer-Encoding");
                        cont_len = pdr_header_find(pdr, "Content-Length");
                        conn = pdr_header_find(pdr, "Connection");
                        if(t_enc != NULL && strcmp(t_enc, "chunked")==0){
                            chunked_flag =1;
                            k = 0;
                            c_len = -1;
                        }
                        if(cont_len != NULL){
                            body_len = atoi(cont_len);
                        }
                        if(conn != NULL && strcmp(conn, "close")==0){
                            close_flag = 1;
                        }
                        if(chunked_flag == 0 && body_len > 0){
                            unsigned char *tmp_p;
                            if(pdr->alloc_byte < body_len){
                                tmp_p = (unsigned char*)nus_realloc(pdr->data, 
                                                                body_len);
                                if(tmp_p == NULL){
                                    fprintf(stderr,"nus_malloc error:%s,%d\n", 
                                            __FILE__,__LINE__);
                                    return(PDR_ERR_ALLOC);
                                }
                                pdr->data = tmp_p;
                                pdr->alloc_byte = body_len;
                            }
                        }
                        else if(chunked_flag == 1 || close_flag == 1){
                                /* do nothing */
                        }
                        else{
                            goto End;
                        }
                        break;
                    }
                }
                else{
                    j++;
                }
            }
        }
        if(stage == 2){
            if(chunked_flag == 1){
                while(buf_p < buf_len){
                    if(c_len == -1){
                        *(c_size_buf + k) = *(buf + buf_p++);
                        if(*(c_size_buf + k )=='\n'){
                            if(k == 1){
                                k = 0;
                                continue;
                            }
                            *(c_size_buf + k -1)='\0';

                            c_len = hex_to_int(c_size_buf);
                            if(c_len > 0){
                                body_len += c_len;

                                if(pdr->alloc_byte < body_len){
                                    unsigned char* tmp_p;
                                    tmp_p = (unsigned char*)nus_realloc(pdr->data, 
                                                                    body_len);
                                    if(tmp_p == NULL){
                                        fprintf(stderr,"nus_malloc error:%s,%d\n",
                                                __FILE__,__LINE__);
                                        return(PDR_ERR_ALLOC);
                                    }
                                    pdr->data = tmp_p;
                                    pdr->alloc_byte = body_len;
                                }
                            }
                            else if(c_len == 0){
                                stage = 3;
                                j = 0;
                                break;
                            }
                            k = 0;
                        }
                        else{
                            k++;
                            continue;
                        }
                    }
                    else if(l < body_len){
                        *(pdr->data + l++) = *(buf + buf_p++);
                        if(l == body_len){
                            c_len = -1;
                            continue;
                        }
                    }

                } /* while(buf_p < buf_len) */
            } /* end of if(chunked_flag == 1) */
            else if(body_len > 0){
                while(buf_p < buf_len){
                    *(pdr->data + l++) = *(buf + buf_p++);
                    if(l == body_len){
                        goto End;
                    }
                }
            }
            else if(close_flag == 1){
                while(buf_p < buf_len){
                    if(l == pdr->alloc_byte){
                        unsigned char *tmp_p;
                        int alloc_byte;
                        if(pdr->alloc_byte == 0){
                            alloc_byte = BUF_SIZE;
                        }
                        else{
                            alloc_byte = pdr->alloc_byte * 2;
                        }
                        tmp_p = (unsigned char*)nus_realloc(pdr->data, alloc_byte);
                        if(tmp_p == NULL){
                            fprintf(stderr,"nus_malloc error:%s,%d\n", 
                                    __FILE__,__LINE__);
                            return(PDR_ERR_ALLOC);
                        }
                        pdr->data = tmp_p;
                        pdr->alloc_byte = alloc_byte; 
                    }
                    *(pdr->data + l++) = *(buf + buf_p++);
                }
            }
            else{
                goto End;
            }
        } /* end of if(stage == 2) */
        if(stage == 3){
            while(buf_p < buf_len){
                *(header_buf+j) = *(buf + buf_p++);
                if(*(header_buf+j)=='\n'){
                    if(j == 1){
                        goto End;
                    }
                    else{
                        *(header_buf+k-1)='\0';
                        if(pdr_parse_header(pdr, header_buf)){
                            return PDR_ERR_ALLOC;
                        }
                        j=0;
                    }
                }
                else{
                    j++;
                }
            } /* end of while(buf_p < buf_len) */
        } /* end of if(stage == 3) */

    } /* end of while(1) */
End:
    pdr->len = l;
#ifdef CONNECTION_RETRY_DEBUG
    sleep(20);
#endif
    return pdr->len;
}
/*--------------------------------------------------------------------*/
static int pdr_parse_status(pandora_data *pdr, char *str)
{
    char *p;
    char *buf;
    char *elem[3];
    int elem_num;

    buf = (char*)nus_malloc(strlen(str)+1);
    if(buf == NULL){
        fprintf(stderr,"nus_malloc error:%s,%d\n",__FILE__,__LINE__);
        return(PDR_ERR_ALLOC);
    }
    strcpy(buf, str);

    elem_num = 0;
    elem[elem_num++] = buf;
    for(p = buf; *p!='\0'; p++){
        if(*p==0x20){
            *p = '\0';
            p++;
            while(*p==0x20){
                p++;
            }
            elem[elem_num++] = p;
            if(elem_num == 3){
                break;
            }
        }
    }
    if(elem_num != 3){
        return -1;
    }
    if((pdr->http_status.version = (char*)nus_malloc(strlen(elem[0])+1))==NULL){
        fprintf(stderr,"nus_malloc error:%s,%d\n", __FILE__, __LINE__);
        return(PDR_ERR_ALLOC);
    }
    pdr->http_status.code = atoi(elem[1]);

    if((pdr->http_status.phrase = (char*)nus_malloc(strlen(elem[2])+1))==NULL){
        fprintf(stderr,"nus_malloc error:%s,%d\n", __FILE__, __LINE__);
        return(PDR_ERR_ALLOC);
    }
    nus_free(buf);
    return 0;
}
/*-------------------------------------------------------------------------*/
static pdr_http_header* pdr_parse_header(pandora_data *pdr, char* str)
{
    char *value, *name;
    char *buf;
    char *p;
    pdr_http_header *hdr = NULL;

    buf = (char*)nus_malloc(strlen(str)+1);
    if(buf == NULL){
        fprintf(stderr,"nus_malloc error:%s,%d\n",__FILE__,__LINE__);
        return NULL;
    }
    strcpy(buf, str);
    
    for(p=buf; *p!='\0';p++){
        if(*p==':'){
            *p='\0';
            name = buf;
            p++;
            while(*p==0x20){
                p++;
            }
            value = p;
            hdr = pdr_header_table_insert(pdr->hdr_table, name, value);
            break;
        }
    }
    nus_free(buf);
    return hdr;
}
/*-----------------------------------------------------------------------*/
static int pdr_put_request(pandora_data *pdr)
{
    char send_buf[BUF_SIZE];
    int n, i, hash, find_flag;
    pdr_http_header *p;

    nus_free(pdr->req_path);
    pdr->req_path = pdr_make_path(pdr);

    if(pdr->proxy_host != NULL){
        sprintf(send_buf, "GET http://%s:%d%s HTTP/1.1\r\n", 
                pdr->host, pdr->port, pdr->req_path);
    }
    else{
        sprintf(send_buf, "GET %s HTTP/1.1\r\n", 
                pdr->req_path);
    }

    n = write(pdr->socket, send_buf, strlen(send_buf));
    if(n<=0){
        /*
        close(pdr->socket);
        pdr->socket = -1;
        */
        return PDR_ERR_DISCONNECT;
    }

    find_flag = 0;
    hash = pdr_hash("Connection");
    for(p = pdr->req_hdr_table[hash]; p!=NULL; p=p->next){
        if(strcmp(p->name, "Connection") == 0){
            find_flag = 1;
            break;
        }
    }
    if(find_flag == 0){
        pdr_header_table_insert(pdr->req_hdr_table, 
                                "Connection", "Keep-Alive");
    }
    find_flag = 0;
    hash = pdr_hash("Host");
    for(p = pdr->req_hdr_table[hash]; p!=NULL; p=p->next){
        if(strcmp(p->name, "Host") == 0){
            find_flag = 1;
            break;
        }
    }
    if(find_flag == 0){
        pdr_header_table_insert(pdr->req_hdr_table, 
                                "Host", pdr->host);
    }
        
    for(i=0;i<PDR_HASH_NUM;i++){
        for(p=pdr->req_hdr_table[i];p!=NULL;p=p->next){
            sprintf(send_buf,"%s: %s\r\n",p->name,p->value);
            n = write(pdr->socket, send_buf, strlen(send_buf));
            if(n<=0){
                /*
                close(pdr->socket);
                pdr->socket = -1;
                */
                return PDR_ERR_DISCONNECT;
            }
        }
    }

    sprintf(send_buf,"\r\n");
    n = write(pdr->socket, send_buf, strlen(send_buf));
    if(n<=0){
        /*
        close(pdr->socket);
        pdr->socket = -1;
        */
        return PDR_ERR_DISCONNECT;
    }
    return 0;
}
/*------------------------------------------------------------------------*/
static int pdr_hash(char *key)
{
    int hashval = 0;

    while(*key != '\0'){
        hashval += tolower(*key);
        key++;
    }
    return (hashval % PDR_HASH_NUM);
} 
/*------------------------------------------------------------------------*/
static void pdr_header_table_init(pdr_http_header *hash_tbl[])
{
    int i;

    for(i=0;i<PDR_HASH_NUM;i++){
        hash_tbl[i] = NULL;
    }
}
/*------------------------------------------------------------------------*/
static pdr_http_header* pdr_header_table_insert(pdr_http_header *hash_tbl[], 
                                 char* name, char* value)
{
    int hash;
    pdr_http_header *hdr, *hdra;

    if((hdra = (pdr_http_header*)nus_malloc(sizeof(pdr_http_header)))==NULL){
        fprintf(stderr,"nus_malloc failed:%s, %d\n", __FILE__,__LINE__);
        return NULL;
    }
    
    if((hdra->name = (char*)nus_malloc(strlen(name)+1))==NULL){
        fprintf(stderr,"nus_malloc failed:%s, %d\n", __FILE__,__LINE__);
        return NULL;
    }
    strcpy(hdra->name, name);

    if((hdra->value = (char*)nus_malloc(strlen(value)+1))==NULL){
        fprintf(stderr,"nus_malloc failed:%s, %d\n", __FILE__,__LINE__);
        return NULL;
    }
    strcpy(hdra->value, value);

    hash = pdr_hash(name);

    if(hash_tbl[hash] == NULL){
        if((hash_tbl[hash] = 
           (pdr_http_header*)nus_malloc(sizeof(pdr_http_header)))==NULL){
            fprintf(stderr,"nus_malloc failed:%s, %d\n", __FILE__,__LINE__);
            return NULL;
        }
        hash_tbl[hash] = hdra;
        hash_tbl[hash]->next = NULL;
        
    }
    else{
        hdr = hash_tbl[hash];
        while(hdr->next!=NULL){
            hdr = hdr->next;
        }
        hdr->next = hdra;
        hdra->next = NULL;
    }
    return hdra;
}
/*------------------------------------------------------------------------*/
static void pdr_header_table_free(pdr_http_header **hdr)
{
    int i;
    pdr_http_header *p, *next;

    if (hdr == NULL) return;
    for(i=0; i<PDR_HASH_NUM;i++){
        for(p = hdr[i]; p!=NULL; p = next){
            nus_free(p->name);
            nus_free(p->value);
            next = p->next;
            nus_free(p);
	    p = NULL;
        }
    }
    return;
}
/*-----------------------------------------------------------------------*/
static char* pdr_make_path(pandora_data *pdr)
{
    int len = 0;
    char *buf, *p, *q;
    int slash_flag = 0;    
    char *root = pdr->root;
    char *path;
    char *resource_type = pdr->resource_type;
    int naps8_mode = 0, cnt;

    if(root != NULL && strncmp(root + 1, "NAPS7", 5) == 0){
        naps8_mode = 0;
        path = pdr->path;
    }
    else {
        naps8_mode = 1;
        path = (char*)nus_malloc((strlen(pdr->path) + 1) * sizeof(char));
        memset(path, 0, sizeof(path));
        p = pdr->path;
        q = path;
        cnt = 0;
        while(*p){
            if(*p == '/'){
                *(q++) = *(p++);
                cnt++;
                continue;
            }
            if(cnt != 4 && cnt != 6){
                *(q++) = *(p++);
            }
            else{
                p++;
            }
        }
        *q = '\0';
    }


    if(root != NULL){
        len += strlen(root);
    }
    if(path != NULL){
        len += strlen(path);
    }
    if(resource_type != NULL){
        len += strlen(resource_type);
    }

    buf = (char*)nus_malloc(len * sizeof(char) + 3 + 1);
    memset(buf, 0, len+3+1);
    buf[0]='/';
    if(root != NULL){
        strcat(buf, root);
    }
    strcat(buf,"/");
    if(path != NULL){
        strcat(buf, path);
    }
    strcat(buf,"/");
    if(resource_type != NULL){
        strcat(buf, resource_type);
    }
    if(naps8_mode == 1){
        nus_free(path);
    }

    p = buf;


    while(*p){
        if(*p==0x20){
            q = p;
            while(*q){
                *(q) = *(q+1);
                q++;
            }
        }
        else if(*p=='/'){
            if(slash_flag == 0){
                slash_flag =1;
                p++;
            }
            else{
                q = p;
                while(*q){
                    *(q) = *(q+1);
                    q++;
                }
            }
        }
        else{
            if(slash_flag == 1){
                slash_flag = 0;
            }
            p++;
        }
    }

    return buf;
}
