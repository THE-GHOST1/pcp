#include <ctype.h>
#include "CommandKeys.h"

sds GetCommandKey(char *clientcommand){
    sds sdsbuffer;
    sds command;
    sds *tokens;
    int count;
    int j;
    int len = 0;

    sdsbuffer = sdsempty();
    command   = sdsempty();

    len = strlen(clientcommand);

    for(j=0;j<len;j++) {

        if(clientcommand[j] >= 'A' && clientcommand[j] <='Z') {
            clientcommand[j] = tolower(clientcommand[j]);
        }
    }

    command = sdscat(command,clientcommand);

    tokens = sdssplitlen(command,sdslen(command)," ",1,&count);

//    printf("client command : %s",clientcommand);

    for(j=0;j<count;j++)
    {
        printf("tokens at %d : %s\n",j,tokens[j]);
    }

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
    else if(!strcmp(tokens[0], "mset"))
    {
        for(j=1;j<count;j+=2)
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[j]);
            sdsbuffer = sdscat(sdsbuffer," ");
        }
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

    //sdsbuffer = sdscat(sdsbuffer,tokens[0]);
    sdsfree(command);
    sdsfreesplitres(tokens,count);
    return sdsbuffer;
}