#include "CommandKeys.h"

sds sdsbuffer;

static void
ConnectCallBack(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
   // printf("Connected...\n");
}

static void
DisconnectCallBack(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK)
    {
        printf("Error: %s\n", c->errstr);
        return;
    }
   // printf("Disconnected...\n");
}


static void
GetCallBack(redisAsyncContext *c, void *r, void *privdata){
    int             len;
    int             i;
    redisReply      *reply = r;

    if (reply == NULL) return;

    for (i = 0; reply != NULL && i < ((redisReply *) reply)->elements; i++) {
        sdsbuffer = sdscat(sdsbuffer, ((redisReply *) reply)->element[i]->str);
        if (i != (((redisReply *) reply)->elements) - 1) sdsbuffer = sdscat(sdsbuffer, " ");
    }

    uv_stop(uv_default_loop());
    redisAsyncDisconnect(c);
 }

sds GetCommandKey(char *clientcommand){

    sds command;
    sds defaultbuffer;
    sds *tokens;
    int count;
    int j;
    int len = 0;

    sdsbuffer = sdsempty();
    command   = sdsempty();
    defaultbuffer = sdsempty();

    len = strlen(clientcommand);

    for(j=0;j<len;j++) {

        if(clientcommand[j] >= 'A' && clientcommand[j] <='Z') {
            clientcommand[j] = tolower(clientcommand[j]);
        }
    }

    command = sdscat(command,clientcommand);

    tokens = sdssplitlen(command,sdslen(command)," ",1,&count);

    if(!strcmp(tokens[0], "get"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0],"set"))
    {
              sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0],"rpushx"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0],"rpush"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0],"rpop"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "rpoplpush"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
        sdsbuffer = sdscat(sdsbuffer," ");
        sdsbuffer = sdscat(sdsbuffer,tokens[2]);
    }
    else if(!strcmp(tokens[0], "ltrim"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "lset"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "lrem"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "lrange"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "lpushx"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "lpush"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "lpop"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "llen"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "linsert"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "lindex"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "brpoplpush"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
        sdsbuffer = sdscat(sdsbuffer," ");
        sdsbuffer = sdscat(sdsbuffer,tokens[2]);
    }
    else if(!strcmp(tokens[0], "brpop"))
    {
        for(j=1;j<count-1;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "blpop"))
    {
        for(j=1;j<count-1;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "cluster"))
    {
        if(tokens[1] == "keyslot") {
            sdsbuffer = sdscat(sdsbuffer, tokens[2]);
        }
    }
    else if(!strcmp(tokens[0], "geoadd"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "geodist"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "geohash"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "geopos"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hdel"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hexists"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hget"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hgetall"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hincrby"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hincrbyfloat"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hkeys"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hlen"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hmget"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hmset"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hscan"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hset"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hsetnx"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hstrlen"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "hvals"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "pfadd"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "pfcount"))
    {

        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "pfmerge"))
    {

        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "del"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "dump"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "exists"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "expire"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "expireat"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "move"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "expire"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "object"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[2]);
    }
    else if(!strcmp(tokens[0], "persist"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "pexpire"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "pexpireat"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "pttl"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "rename"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
        sdsbuffer = sdscat(sdsbuffer," ");
        sdsbuffer = sdscat(sdsbuffer,tokens[2]);
    }
    else if(!strcmp(tokens[0], "renamenx"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
        sdsbuffer = sdscat(sdsbuffer," ");
        sdsbuffer = sdscat(sdsbuffer,tokens[2]);
    }
    else if(!strcmp(tokens[0], "restore"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "sort"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "touch"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "ttl"))
    {

        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "type"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "unlink"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "type"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "sadd"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "scard"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "sdiff"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "sdiffstore"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "sinter"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "sinterstore"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "sismember"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "smembers"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "smove"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
        sdsbuffer = sdscat(sdsbuffer," ");
        sdsbuffer = sdscat(sdsbuffer, tokens[2]);
    }
    else if(!strcmp(tokens[0], "spop"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "srandmember"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "srem"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "sscan"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "sunion"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "sscan"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "sunionstore"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "bzpopmax"))
    {
        for(j=1;j<count && !isdigit(tokens[j]) ;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "bzpopmin"))
    {
        for(j=1;j<count && !isdigit(tokens[j]) ;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "zadd"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zcard"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zcount"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zincrby"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zlexcount"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zpopmax"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zpopmin"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrange"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrangebylex"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrangebyscore"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrank"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrem"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zremrangebylex"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zremrangebyrank"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zremrangebyscore"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrevrange"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrevrangebylex"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrevrangebyscore"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrevrank"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zrevrank"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zscan"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "zscore"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "append"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "bitcount"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "bitfield"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "bitop"))
    {
        for(j=2;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "bitpos"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "decr"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "decrby"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "getbit"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "getrange"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "getset"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "incr"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "incrby"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "incrbyfloat"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "mget"))
    {
        for(j=1;j<count;j++)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "mset"))
    {
        for(j=1;j<count;j+=2)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "msetnx"))
    {
        for(j=1;j<count;j+=2)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
    }
    else if(!strcmp(tokens[0], "psetex"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "setbit"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "setex"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "setnx"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "setrange"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "strlen"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else
    {
        defaultbuffer = sdscat(defaultbuffer,"command getkeys ");
        defaultbuffer = sdscatsds(defaultbuffer,command);
        char *tempcommand  = malloc(sdslen(defaultbuffer)+1);
        strcpy(tempcommand, defaultbuffer);
        signal(SIGPIPE, SIG_IGN);
        uv_loop_t* loop = uv_default_loop();

        redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);

        if (c->err) {
            printf("Error: %s\n", c->errstr);
            return 1;
        }

        redisEventAttach(c,loop);
        redisAsyncSetConnectCallBack(c,ConnectCallBack);
        redisAsyncSetDisconnectCallBack(c,DisconnectCallBack);
        redisAsyncCommand(c, GetCallBack, (char*)"end-1", tempcommand);
        free(tempcommand);
        uv_run(loop, UV_RUN_DEFAULT);
    }


    sdsfree(defaultbuffer);
    sdsfree(command);
    sdsfreesplitres(tokens,count);
    return sdsbuffer;
}