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

struct sockaddr_in bind_addr;
redisAsyncContext *c;
static uv_tcp_t * client;

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
            return sdsbuffer;
        }
        case 2 :{
            int i;
            int len;
            char *cmd;
            sds tempbuffer;
            tempbuffer = sdsempty();
            for(i=0;i<((redisReply*)reply)->elements;i++){
                tempbuffer = sdscat(tempbuffer,ProcessRedisReply((redisReply *) reply->element[i]));
                tempbuffer = sdscat(tempbuffer, " ");
            }
            len = redisFormatCommand(&cmd,tempbuffer);
            sdsbuffer = sdscat(sdsbuffer , cmd);
            sdsfree(tempbuffer);
            free(cmd);
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
            sdsbuffer = sdscat(sdsbuffer,reply->str);
            return sdsbuffer;
        }
        default: {
            //ERROR_REPLY
            sdsbuffer = sdscat(sdsbuffer,"-");
            sdsbuffer = sdscat(sdsbuffer,reply->str);
            return sdsbuffer;
        }
    }
}

void on_write_end(uv_write_t *req, int status)
{
    if(status < 0)
    {
        fprintf(stderr,"uv_write Error : %s\n",uv_strerror(status));
        return;
    }
    printf("\nwrote !!\n");
    //uv_read_start(req->handle,alloc_buffer,on_read);

}
void getCallback(redisAsyncContext *c, void *r, void *privdata)
{
	redisReply *reply = r;
	int len;
	int i;
	sds sdsbuffer;
	sdsbuffer = sdsempty();
	if (reply == NULL) return;
    sdsbuffer = ProcessRedisReply(reply);
    printf("\nsds_buffer %s\n",sdsbuffer);

    uv_write_t write_req;
    int buf_count = 1;
    char* message="+OK\r\n";
    uv_buf_t buf = uv_buf_init(message,strlen(message)+1);
    uv_write(&write_req, (uv_stream_t*)client, &buf,1, on_write_end);
    sdsfree(sdsbuffer);
	/*uv_write_t *write_req=malloc(sizeof(uv_write_t));;
	int buf_count = 1;
	char* message=strdup("+OK\r\n");
	uv_buf_t buf_rep = uv_buf_init(message,strlen(message));
	uv_write(write_req, (uv_stream_t*)client, &buf_rep,1, on_write_end);*/
	//redisAsyncDisconnect(c);
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

void echo_read(uv_stream_t *client,ssize_t nread,uv_buf_t* buf)
{
	redisReader *reader;
	void *reply;
	int ret;
	int i;
	if(nread < 0)
	{
		if(nread != UV_EOF) 
		{
			fprintf(stderr,"Read Error ");
		}
		uv_close((uv_handle_t *)client,NULL);
	}
	
	uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
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
	//char* command="PING";
	//redisAsyncCommand(c,getCallback,(char*)"end-1",sdsbuffer);
	char* command="PING";
	redisContext *c = redisConnect("127.0.0.1", 6379);
	redisReply *reply_1 = (redisReply *)redisCommand(c, command);
	int len;
	sdsbuffer = sdsempty();
	if(reply_1!=NULL){
		sdsbuffer = ProcessRedisReply(reply_1);
		printf("\nsds_buffer %s\n",sdsbuffer);
	}
	freeReplyObject(reply_1);
	redisFree(c);
	uv_write_t write_req;
	int buf_count = 1;
	char* message="+OK\r\n";
	uv_buf_t buf_rep = uv_buf_init(message,strlen(message)+1);
	uv_write(&write_req, (uv_stream_t*)client, &buf_rep,1, on_write_end);
//	sdsfree(sdsbuffer);
//	sleep(5);
//	sdsfree(sdsbuffer);
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
	/*uv_loop_t* loop_redis = uv_loop_new();
	c = redisAsyncConnect("127.0.0.1", 6379);
	redisLibuvAttach(c,loop_redis);
	redisAsyncSetConnectCallback(c,connectCallback);
	redisAsyncSetDisconnectCallback(c,disconnectCallback);
	uv_run(loop_redis,UV_RUN_DEFAULT);*/
	client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), client);
	if (uv_accept(server, (uv_stream_t*) client) == 0)
	{
		uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
	}
	else
	{
	      uv_close((uv_handle_t*) client, NULL);
	}
}

int main()
{
	uv_loop_t* loop_tcp = uv_default_loop();

	signal(SIGPIPE, SIG_IGN);



	/*if (c->err)
	{
	        printf("Error: %s\n", c->errstr);
	        return 1;
 	}
	*/
	uv_tcp_t server;
	uv_tcp_init(loop_tcp,&server);
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

	uv_run(loop_tcp,UV_RUN_DEFAULT);
	return 0;
}
