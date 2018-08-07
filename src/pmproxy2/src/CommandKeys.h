#ifndef PMPROXY2_COMMANDKEYS_H
#define PMPROXY2_COMMANDKEYS_H

#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <redis.h>
#include <net.h>
#include <sdsalloc.h>
#include <slots.h>
#include <libuv.h>

typedef struct keystruct {
    sds keysinposone;
    sds keysinposx;
} keystruct;

extern void loadkeystruct(redisReply *reply);
extern sds GetCommandKey(redisReply *reply);
extern sds ProcessRedisReply(redisReply *reply);

#endif //PMPROXY2_COMMANDKEYS_H
