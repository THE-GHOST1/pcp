#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include </home/prajwal/hiredis/hiredis.h>
#include </home/prajwal/hiredis/net.h>
#include </home/prajwal/hiredis/sds.h>
#include </home/prajwal/hiredis/async.h>
#include </home/prajwal/hiredis/adapters/libuv.h>

#define PORT 44323
struct sockaddr_in addr;
typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

static uint64_t data_cntr = 0;

struct KeyNode{
    sds key;
    struct KeyNode* next;
    struct KeyNode* prev;
};

struct ClientRequestData {
    sds ip;
    uv_tcp_t *client;
    struct KeyNode* keys;

};


sds ProcessRedisReply(redisReply *reply)
{
    sds end_line;
    end_line = sdsempty();
    end_line = sdscat(end_line, "\r\n");
    sds sdsbuffer;
    sdsbuffer = sdsempty();
    if (reply == NULL) sdsbuffer;
    switch((reply->type))
    {
        case 1 : {
            sds sometempbuf = sdsfromlonglong((strlen(reply->str)));
            sdsbuffer = sdscat(sdsbuffer, "$");
            sdsbuffer = sdscat(sdsbuffer, sometempbuf);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            sdsbuffer = sdscat(sdsbuffer, reply->str);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            sdsfree(sometempbuf);
            printf("string format: %s",sdsbuffer);
            return sdsbuffer;
        }
        case 2 :{
            int i;
            sds sometempbuf = sdsfromlonglong(((redisReply*)reply)->elements);
            sdsbuffer=sdscat(sdsbuffer,"*");
            sdsbuffer=sdscat(sdsbuffer,sometempbuf);
            sdsbuffer=sdscat(sdsbuffer,end_line);
            for(i=0;i<((redisReply*)reply)->elements;i++){
                sdsbuffer = sdscat(sdsbuffer,ProcessRedisReply((redisReply *) reply->element[i]));
                //tempbuffer = sdscat(tempbuffer, "");
            }
            sdsfree(sometempbuf);
            printf("string format: %s",sdsbuffer);
            return sdsbuffer;
        }
        case 3: {
            sds sometempbuf = sdsfromlonglong(reply->integer);
            sdsbuffer = sdscat(sdsbuffer,":");
            sdsbuffer = sdscat(sdsbuffer,sometempbuf);
            sdsbuffer = sdscat(sdsbuffer,end_line);
            sdsfree(sometempbuf);
            return sdsbuffer;
        }
        case 4 : {
            sdsbuffer = sdscat(sdsbuffer,"");
            return sdsbuffer;
        }
        case 5: {
            sdsbuffer = sdscat(sdsbuffer, "+");
            sdsbuffer = sdscat(sdsbuffer,reply->str);
            sdsbuffer = sdscat(sdsbuffer,end_line);
            return sdsbuffer;
        }
        default: {
            //ERROR_REPLY
            sdsbuffer = sdscat(sdsbuffer,"-");
            sdsbuffer = sdscat(sdsbuffer,reply->str);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            return sdsbuffer;
        }
    }
}



static void on_close_cb(uv_handle_t* handle) {
    free(handle);
}

static void after_write(uv_write_t* req, int status) {
    write_req_t* wr = (write_req_t*)req;

    if (wr->buf.base != NULL){}
        //free(wr->buf.base);
    free(wr);

    if (status == 0)
        return;

    fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));

    if (status == UV_ECANCELED)
        return;

    assert(status == UV_EPIPE);
    uv_close((uv_handle_t*)req->handle, on_close_cb);
}


static void after_shutdown(uv_shutdown_t* req, int status) {
    /*assert(status == 0);*/
    if (status < 0)
        fprintf(stderr, "err: %s\n", uv_strerror(status));
    fprintf(stderr, "data received: %lu\n", data_cntr / 1024 / 1024);
    data_cntr = 0;
    uv_close((uv_handle_t*)req->handle, on_close_cb);
    free(req);
}

static void after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    struct ClientRequestData *data;

    data = handle->data;
    redisReader *reader;
    void *reply;
    int ret;
    int i;
    int r;
    redisContext *c;
    write_req_t* wr;
    uv_shutdown_t* req;

    if (nread <= 0 && buf->base != NULL)
        free(buf->base);

    if (nread == 0)
        return;

    if (nread < 0) {
        /*assert(nread == UV_EOF);*/
        fprintf(stderr, "err: %s\n", uv_strerror(nread));

        req = (uv_shutdown_t*) malloc(sizeof(*req));
        assert(req != NULL);

        r = uv_shutdown(req, handle, after_shutdown);
        assert(r == 0);

        return;
    }
    data_cntr += nread;
    wr = (write_req_t*) malloc(sizeof(*wr));
    assert(wr != NULL);

    printf("Buffer : %s\n",buf->base);
    printf("Buffer len : %d\n",strlen(buf->base));

    reader = redisReaderCreate();
    redisReaderFeed(reader,(char *)(buf->base),strlen(buf->base));
    ret = redisReaderGetReply(reader,&reply);
    assert(ret == REDIS_OK);
    sds sdsbuffer;
    sdsbuffer = sdsempty();
    for(i=0;reply != NULL && i<((redisReply*)reply)->elements;i++)
    {
        sdsbuffer = sdscat(sdsbuffer, ((redisReply*)reply)->element[i]->str);
        if(i != (((redisReply*)reply)->elements)-1) sdsbuffer = sdscat(sdsbuffer, " ");
    }
    printf("sdbuffer for god's sake : %s\n",sdsbuffer);

    c = redisConnect("127.0.0.1",6379);
    char* command=malloc(sdslen(sdsbuffer));
    strcpy(command,sdsbuffer);
    reply = redisCommand(c,command);
    sds s=sdsempty();
    s=sdscat(s,"+OK\r\n");
    s = ProcessRedisReply(reply);
    printf("sdbuffer for god's sake reply : len %s %d\n",s,sdslen(s));
    wr->buf = uv_buf_init(s, sdslen(s));
    r = uv_write(&wr->req, handle, &wr->buf, 1, after_write);
    redisFree(c);
    freeReplyObject(reply);
    assert(r == 0);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    assert(buf->base != NULL);
    buf->len = suggested_size;
}

static void on_connection(uv_stream_t* server, int status) {
    uv_tcp_t* stream;
    int r;
    assert(status == 0);
    stream = malloc(sizeof(uv_tcp_t));
    assert(stream != NULL);
    r = uv_tcp_init(uv_default_loop(), stream);
    assert(r == 0);
    stream->data = server;
    struct ClientRequestData* data=malloc(sizeof(*data));
    stream->data=data;
    data->client=stream;
    r = uv_accept(server, (uv_stream_t*)stream);
    struct sockaddr_in name;
    int namelen = sizeof(name);
    uv_tcp_getpeername(&stream, (struct sockaddr*) &name, &namelen);
    char addr[16];
    char client_ip[32];
    uv_inet_ntop(AF_INET, &name.sin_addr, addr, sizeof(addr));
    snprintf(client_ip, sizeof(client_ip), "%s:%d", addr, ntohs(name.sin_port));
    data->ip=sdsnew(client_ip);
    assert(r == 0);
    r = uv_read_start((uv_stream_t*)stream, alloc_cb, after_read);
    assert(r == 0);
}

static int tcp_echo_server() {
    uv_tcp_t* tcp_server;

    int r;
    r = uv_ip4_addr("0.0.0.0", PORT, &addr);
    assert(r == 0);
    tcp_server = (uv_tcp_t*) malloc(sizeof(*tcp_server));
    assert(tcp_server != NULL);
    r = uv_tcp_init(uv_default_loop(), tcp_server);
    assert(r == 0);
    r = uv_tcp_bind(tcp_server, (const struct sockaddr*)&addr, 0);
    uv_tcp_keepalive(tcp_server,1,50);
    uv_tcp_simultaneous_accepts(tcp_server,1);
    assert(r == 0);
    r = uv_listen((uv_stream_t*)tcp_server, SOMAXCONN, on_connection);
    if(r)
    {
        fprintf(stderr,"LISTEN ERROR %s\n",uv_strerror(r));
        return 1;
    }
    return 0;
}

int main() {
    int r;
    r = tcp_echo_server();
    assert(r == 0);
    r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    assert(r == 0);
    return 0;
}
