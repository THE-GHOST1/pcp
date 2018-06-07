#ifndef PMPROXY2_COMMANDKEYS_H
#define PMPROXY2_COMMANDKEYS_H

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

sds GetCommandKey(char *command);

#endif //PMPROXY2_COMMANDKEYS_H
