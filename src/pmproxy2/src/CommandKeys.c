#include "CommandKeys.h"

sds sdsbuffer;

keystruct *keys;

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
checkError(redisReply *reply, const char *format, va_list argp)
{
    /* TODO: needs to be passed to info callbacks not stderr */
    fputs("Error: ", stderr);
    vfprintf(stderr, format, argp);
    if (reply && reply->type == REDIS_REPLY_ERROR)
        fprintf(stderr, "\nRedis: %s\n", reply->str);
    else
        fputc('\n', stderr);
}

static int
checkString(redisReply *reply, const char *format, ...)
{
    if (reply && reply->type == REDIS_REPLY_STRING) {
        return reply->str;
    } else if (!reply || reply->type != REDIS_REPLY_ERROR) {
        va_list	argp;
        va_start(argp, format);
        checkError(reply, format, argp);
        va_end(argp);
    }
    return -1;
}

static int
checkArray(redisReply *reply, const char *format, ...)
{
    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        return 0;
    } else if (!reply || reply->type != REDIS_REPLY_ERROR) {
        va_list	argp;
        va_start(argp, format);
        checkError(reply, format, argp);
        va_end(argp);
    }
    return -1;
}

static long long
checkInteger(redisReply *reply, const char *format, ...)
{
    if (reply && reply->type == REDIS_REPLY_INTEGER) {
        return reply->integer;
    } else {
        va_list	argp;
        va_start(argp, format);
        checkError(reply, format, argp);
        va_end(argp);
    }
    return -1;
}

static int
decodeRedisKey(keystruct *keys, redisReply *reply)
{
    redisReply		*node;
    sds             currentcommand;
    int			i, n;

    currentcommand = sdsempty();
    /* expecting command name, pos of key , array of characteristics ,start and end of key , step count */

    node = reply->element[0];
    if ((currentcommand = checkString(node, "%s name", "Key")) < 0)
        return -EINVAL;

    node = reply->element[3];
    if ((n = (__uint32_t)checkInteger(node, "%s first", "Key")) < 0)
        return -EINVAL;

    if(n == 1) {
        keys->keysinposone = sdscat(keys->keysinposone, currentcommand);
        keys->keysinposone = sdscat(keys->keysinposone, " ");
    }
    else if(n > 0 && n != 1) {
        keys->keysinposx = sdscat(keys->keysinposx, currentcommand);
        keys->keysinposx = sdscat(keys->keysinposx, " ");
        keys->keysinposx = sdscat(keys->keysinposx,sdsfromlonglong(n));
        keys->keysinposx = sdscat(keys->keysinposx," ");
    }
    return 0;
}

static void
decodeRedisKeys(keystruct *keys, redisReply *reply)
{
    redisReply *currentkeydetails;
    int			i;

    for (i = 0; i < reply->elements; i++) {
        currentkeydetails = reply->element[i];
        if (checkArray(currentkeydetails, "%s entry %d", "COMMAND", i) == 0)
            decodeRedisKey(keys, currentkeydetails);
    }
}

sds
ProcessRedisReply(redisReply *reply) {

    /*
     * Returns a sds string of Redis reply in RESP format.
    */

    sds             end_line;
    sds             sdsbuffer;
    end_line = sdsempty();
    end_line = sdscat(end_line, "\r\n");
    sdsbuffer = sdsempty();
    if (reply == NULL) sdsbuffer;
    switch ((reply->type)) {
        case 1 : {
            //REDIS_REPLY_STRING
            sds sometempbuf = sdsfromlonglong((strlen(reply->str)));
            sdsbuffer = sdscat(sdsbuffer, "$");
            sdsbuffer = sdscat(sdsbuffer, sometempbuf);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            sdsbuffer = sdscat(sdsbuffer, reply->str);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            sdsfree(sometempbuf);
            return sdsbuffer;
        }
        case 2 : {
            //REDIS_REPLY_ARRAY
            int64_t i;
            sds sometempbuf = sdsfromlonglong(((redisReply *) reply)->elements);
            sdsbuffer = sdscat(sdsbuffer, "*");
            sdsbuffer = sdscat(sdsbuffer, sometempbuf);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            for (i = 0; i < ((redisReply *) reply)->elements; i++) {
                sdsbuffer = sdscat(sdsbuffer, ProcessRedisReply((redisReply *) reply->element[i]));
            }
            sdsfree(sometempbuf);
            return sdsbuffer;
        }
        case 3: {
            //REDIS_REPLY_INTEGER
            sds sometempbuf = sdsfromlonglong(reply->integer);
            sdsbuffer = sdscat(sdsbuffer, ":");
            sdsbuffer = sdscat(sdsbuffer, sometempbuf);
            sdsbuffer = sdscat(sdsbuffer, end_line );
            sdsfree(sometempbuf);
            return sdsbuffer;
        }
        case 4 : {
            //REDIS_REPLY_NULL
            sdsbuffer = sdscat(sdsbuffer, "");
            return sdsbuffer;
        }
        case 5: {
            //REDIS_REPLY_STATUS
            sdsbuffer = sdscat(sdsbuffer, "+");
            sdsbuffer = sdscat(sdsbuffer, reply->str);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            return sdsbuffer;
        }
        default: {
            //REDIS_REPLY_ERROR
            sdsbuffer = sdscat(sdsbuffer, "-");
            sdsbuffer = sdscat(sdsbuffer, reply->str);
            sdsbuffer = sdscat(sdsbuffer, end_line);
            return sdsbuffer;
        }
    }
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

void loadkeystruct(redisReply *reply)
{


    if ((keys = (keystruct *)calloc(1, sizeof(keystruct))) == NULL){
        return;
    }

    keys->keysinposone = sdsempty();
    keys->keysinposx = sdsempty();

    if (checkArray(reply, "%s", "COMMAND") == 0) {
        decodeRedisKeys(keys, reply);
    }

    printf("\nkeys in pos one : %s\n",keys->keysinposone);
    printf("\n keys in pos x : %s\n",keys->keysinposx);

}

sds GetCommandKey(redisReply *reply){

    sds command;
    sds defaultbuffer;
    sds *tokens;
    sds tempbuf;
    int count;
    int j;
    int i;
    int len = 0;
    int keyonecount;
    sds *keyonetokens;
    int keyxcount;
    int xpos;
    sds *keyxtokens;
    int quotecnt = 0;
    sdsbuffer = sdsempty();
    command   = sdsempty();
    defaultbuffer = sdsempty();
    tempbuf  = sdsempty();
    sds evalstr = sdsempty();


    for (i = 0; reply != NULL && i < ((redisReply *) reply)->elements; i++) {
        tempbuf = sdscat(tempbuf, ((redisReply *) reply)->element[i]->str);
        if (i != (((redisReply *) reply)->elements) - 1) tempbuf = sdscat(tempbuf, " ");
    }

    char *clientcommand = malloc(sdslen(tempbuf));
    strcpy(clientcommand, tempbuf);

    len = strlen(clientcommand);

    printf("client command : %s\n",clientcommand);

    for(j=0;j<len;j++) {

        if(clientcommand[j] >= 'A' && clientcommand[j] <='Z') {
            clientcommand[j] = tolower(clientcommand[j]);
        }
    }

    command = sdscat(command,clientcommand);

    tokens = sdssplitlen(command,sdslen(command)," ",1,&count);

    keyonetokens = sdssplitlen(keys->keysinposone,sdslen(keys->keysinposone)," ",1,&keyonecount);

    keyxtokens = sdssplitlen(keys->keysinposx,sdslen(keys->keysinposx)," ",1,&keyxcount);

    for(i=0;i<keyonecount;i++)
    {
        if(!strcmp(tokens[0], keyonetokens[i]))
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[1]);
            return sdsbuffer;
        }
    }

    for(i=0;i<keyxcount;i++)
    {
        if(!strcmp(tokens[0], keyxtokens[i]))
        {
            xpos = atoi(keyxtokens[i+1]);
            sdsbuffer = sdscat(sdsbuffer,tokens[xpos]);
            return sdsbuffer;
        }
    }

    if(!strcmp(tokens[0], "eval"))
    {
        for(i=0;i<count;i++) printf("\n %s\t \n",tokens[i]);

        for(i=0;i<count;i++)
        {
            char *st = tokens[i];
            if(st[0] == '\0')
            {
                quotecnt++;
            }
            if(quotecnt == 2)
            {
                if(atoi(tokens[i+1]) != 0)
                {
                    sdsbuffer = sdscat(sdsbuffer,tokens[i+2]);
                    return sdsbuffer;
                }
            }
        }
    }
    else if(!strcmp(tokens[0], "EVALSHA"))
    {
        if(atoi(tokens[2]) != 0 )
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[3]);
        }
    }
    else if(!strcmp(tokens[0], "sort"))
    {
        sdsbuffer = sdscat(sdsbuffer,tokens[1]);
    }
    else if(!strcmp(tokens[0], "ZUNIONSTORE"))
    {
        if(atoi(tokens[2]) != 0 )
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[3]);
        }
    }
    else if(!strcmp(tokens[0], "ZINTERSTORE"))
    {
        if(atoi(tokens[1]) != 0 )
        {
            sdsbuffer = sdscat(sdsbuffer,tokens[2]);
        }
    }

    sdsfree(defaultbuffer);
    sdsfree(command);
    sdsfreesplitres(tokens,count);
    return sdsbuffer;
}