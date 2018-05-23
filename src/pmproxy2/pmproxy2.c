#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hiredis/hiredis.h>
#include <hiredis/net.h>
#include <hiredis/sds.h>
#include <async.h>
#include <hiredis/adapters/libuv.h>

struct sockaddr_in bind_addr;

void getCallback(redisAsyncContext *c, void *r, void *privdata) 
{
    redisReply *reply = r;
    char *cmd;
    int len;
    if (reply == NULL) return;
    len = redisFormatCommand(&cmd,"%s",(reply->str).c_str());
    printf("%s",cmd);
    free(cmd);    
    redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) 
{
    if (status != REDIS_OK) 
    {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) 
{
    if (status != REDIS_OK) 
    {    
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void on_input_event(uv_stream_t *client,ssize_t nread,uv_buf_t* buf)
{
    if(nread < 0)
    {
        if(nread != UV_EOF) 
        {
            fprintf(stderr,"Read Error ");
        }
        uv_close((uv_handle_t *)client,NULL);
    }

    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) 
    {
            printf("Error: %s\n", c->errstr);
            return 1; //
     }
    redisLibuvAttach(c,uv_default_loop());
    redisAsyncSetConnectCallback(c,connectCallback);    
    redisAsyncCommand(c, NULL, NULL,(buf->base).c_str());
    redisAsyncCommand(c, getCallback, (char*)"end-1","GET key");
    redisAsyncSetDisconnectCallback(c,disconnectCallback);    
    return;
}

void alloc_buffer(uv_handle_t * handle, size_t size,uv_buf_t *buf)
{
    buf->base = (char*) malloc(size);
    buf->len = size;
}

void on_new_connection(uv_stream_t *server, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
         return;
    }                    
    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);
    if (uv_accept(server, (uv_stream_t*) client) == 0)
    {
        uv_read_start((uv_stream_t*) client, alloc_buffer, on_input_event);
    }
    else
    {
          uv_close((uv_handle_t*) client, NULL);
    }
}

int main()
{
    signal(SIGPIPE, SIG_IGN);
    uv_tcp_t server;
    uv_tcp_init(uv_default_loop(),&server);
    uv_tcp_keepalive(&server,1,50);
    uv_tcp_simultaneous_accepts(&server,1);
    uv_ip4_addr("0.0.0.0",44323,&bind_addr);
    uv_tcp_bind(&server,(const struct sockaddr*)&bind_addr,0);
    int r = uv_listen((uv_stream_t*)&server,256, on_new_connection);
    if(r)
    {
        fprintf(stderr,"LISTEN ERROR %s\n",uv_strerror(r));
        return 1;
    }
    
    uv_run(uv_default_loop(),UV_RUN_DEFAULT);
    return 0;
}
