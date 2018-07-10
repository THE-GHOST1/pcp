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
#include <redis.h>
#include <net.h>
#include <sdsalloc.h>
#include <libuv.h>
#include <slots.h>
#include <schema.h>
#include "CommandKeys.h"

#define PORT 44323

struct sockaddr_in addr;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

int64_t ClientCounter = 0;

struct ClientRequestData {
    redisAsyncContext *redisAsyncContext;
    sds ip;
    uv_tcp_t *client;
    int64_t ClientID; //Internal implementaion
};

FILE *fp;

static char* logfile = "/home/prajwal/pmproxy2.log";

static void
on_close_cb(uv_handle_t *handle) {
    free(handle);
}

static void
after_write(uv_write_t *req, int status) {
    write_req_t *wr = (write_req_t *) req;
    if (wr->buf.base != NULL) {}
    //  free(wr);
    if (status == 0) return;
    fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));
    if (status == UV_ECANCELED) return;
    assert(status == UV_EPIPE);

    uv_close((uv_handle_t *) req->handle, on_close_cb);
}

static void
after_shutdown(uv_shutdown_t *req, int status) {
    struct ClientRequestData        *data;

    data = req->handle->data;
    redisAsyncDisconnect(data->redisAsyncContext);
    if (status < 0) {
        fprintf(stderr, "%s\n", uv_strerror(status));
    }
    uv_close((uv_handle_t *) req->handle, on_close_cb);
    free(req);
}

static void
getCallback(redisAsyncContext *c, void *r, void *privdata){
    struct ClientRequestData    *data;
    int                         len;
    int                         i;

    sds s = sdsempty();
    redisReply *reply = r;
    write_req_t *wr = (write_req_t *) malloc(sizeof(*wr));

    if (reply == NULL) return;

    data = c->data;
    s = ProcessRedisReply(reply);
    printf("%s",s);
    wr->buf = uv_buf_init(s, sdslen(s));
    r = uv_write(&wr->req, data->client, &wr->buf, 1, after_write);
    //  sdsfree(s);
}


static void
after_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
    struct ClientRequestData            *data;
    redisReader                         *reader;
    void                                *reply;
    int                                 ret;
    int                                 i;
    int                                 r;
    redisAsyncContext                   *c;
    write_req_t                         *wr;
    uv_shutdown_t                       *req;
    sds                                 sdsbuffer;
    sds                                 keys;
    sds                                 tempbuf;
    sds                                 hostspec;
    redisSlots                          *expectedslots;
    redisAsyncContext                   *desiredcontex;
    if (nread <= 0 && buf->base != NULL) {
        free(buf->base);
    }
    if (nread == 0) return;

    if (nread < 0) {
        fprintf(stderr, "err: %s\n", uv_strerror(nread));

        req = (uv_shutdown_t *) malloc(sizeof(*req));
        assert(req != NULL);

        r = uv_shutdown(req, handle, after_shutdown);
        assert(r == 0);

        return;
    }
    wr = (write_req_t *) malloc(sizeof(*wr));
    assert(wr != NULL);

    data = handle->data;
    reader = redisReaderCreate();
    redisReaderFeed(reader, (char *) (buf->base), strlen(buf->base));
    ret = redisReaderGetReply(reader, &reply);

    assert(ret == REDIS_OK);

    sdsbuffer = sdsempty();
    keys = sdsempty();
    tempbuf   = sdsempty();
    hostspec = sdsnew("127.0.0.1:7001");

    tempbuf = ProcessRedisReply(reply);

    for (i = 0; reply != NULL && i < ((redisReply *) reply)->elements; i++) {
        sdsbuffer = sdscat(sdsbuffer, ((redisReply *) reply)->element[i]->str);
        if (i != (((redisReply *) reply)->elements) - 1) sdsbuffer = sdscat(sdsbuffer, " ");
    }

    printf("Input command : %s\n", sdsbuffer);
    char *command = malloc(sdslen(sdsbuffer));
    strcpy(command, sdsbuffer);
    keys = GetCommandKey(reply);
    printf("\nKeys for command %s is %s\n",command,keys);
    char *Clientcommand = malloc(sdslen(tempbuf));
    strcpy(Clientcommand, tempbuf);

    redis_init(hostspec,Clientcommand,(void *)handle);
//    desiredcontex = redisAsyncGet(expectedslots,command,keys);
//
//    if (desiredcontex->err) {
//        printf("Error: %s\n", desiredcontex->errstr);
//        return;
//    }
//
//    r = redisAsyncFormattedCommand(data->redisAsyncContext , getCallback, (char*)"end-1",Clientcommand,strlen(Clientcommand));
//    assert(r==0);
    fp=fopen(logfile,"a+");
    fputs(command,fp);
    fputs("\n",fp);
    fclose(fp);
    sdsfree(sdsbuffer);
    sdsfree(keys);
    freeReplyObject(reply);
}

static void
alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = malloc(suggested_size); //what size? break the msg (64 bit)
    assert(buf->base != NULL);
    buf->len = suggested_size;
}

static void
connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    //  printf("Connected...\n");
}

static void
disconnectCallback(const redisAsyncContext *c, int status) {
    struct ClientRequestData            *data;

    if (status != REDIS_OK)
    {
        printf("Error: %s\n", c->errstr);
        return;
    }

    data=c->data;
    printf("Client %d disconnecting",data->ClientID);
    printf("\nDisconnected...\n");
}

static void
on_connection(uv_stream_t *server, int status) {
    uv_tcp_t            *stream;
    int                 r;
    char                addr[16];
    char                client_ip[32];
    struct sockaddr_in name;
    struct ClientRequestData *data = malloc(sizeof(*data));

    int namelen = sizeof(name);
    ClientCounter++;
    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    redisEventAttach(c, uv_default_loop());
    redisAsyncSetConnectCallBack(c, connectCallback);
    if (c->err) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    redisEventAttach(c,uv_default_loop());
    redisAsyncSetConnectCallBack(c,connectCallback);
    redisAsyncSetDisconnectCallBack(c,disconnectCallback);
    assert(status == 0);

    stream = malloc(sizeof(uv_tcp_t));
    assert(stream != NULL);

    r = uv_tcp_init(uv_default_loop(), stream);
    assert(r == 0);

    stream->data = server;
    data->redisAsyncContext = c;
    c->data = data;
    stream->data = data;
    data->client = stream;

    r = uv_accept(server, (uv_stream_t *) stream);
    assert(r == 0);

    uv_tcp_getpeername(&stream, (struct sockaddr *) &name, &namelen);
    uv_inet_ntop(AF_INET, &name.sin_addr, addr, sizeof(addr));
    snprintf(client_ip, sizeof(client_ip), "%s:%d", addr, ntohs(name.sin_port));
    data->ip = sdsnew(client_ip);
    data->ClientID = ClientCounter;
    printf("\nClient %d connected\n",data->ClientID);
    r = uv_read_start((uv_stream_t *) stream, alloc_cb, after_read);
    assert(r == 0);
}

/*
 * Starting the server. Accepts multiple async requests
 * unpon succesful initialization returns 0
*/
static int
init_server() {
    uv_tcp_t        *tcp_server;
    int             r;

    uv_ip4_addr("0.0.0.0", PORT, &addr);
    tcp_server = (uv_tcp_t *) malloc(sizeof(*tcp_server));
    uv_tcp_init(uv_default_loop(), tcp_server);
    uv_tcp_bind(tcp_server, (const struct sockaddr *) &addr, 0);
    uv_tcp_keepalive(tcp_server, 1, 50);
    uv_tcp_simultaneous_accepts(tcp_server, 1);
    r = uv_listen((uv_stream_t *) tcp_server, SOMAXCONN, on_connection);
    if (r) {
        fprintf(stderr, "LISTEN ERROR %s\n", uv_strerror(r));
        return 1;
    }
    return 0;
}

int
main() {
    int         r;
    r = init_server();
    assert(r == 0);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return 0;
}
