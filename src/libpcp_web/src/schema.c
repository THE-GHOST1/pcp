/*
 * Copyright (c) 2017-2018 Red Hat.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#include <assert.h>
#include "schema.h"
#include "util.h"
#include "maps.h"
#include <pcp/pmda.h>

#define STRINGIFY(s)	#s
#define TO_STRING(s)	STRINGIFY(s)
#define SCHEMA_VERSION	2
#define SHA1SZ		20

typedef void (*redis_callback)(redisSlots *, redisReply *, void *);

struct ClientRequestData {
    redisAsyncContext *redisAsyncContext;
    sds ip;
    uv_tcp_t *client;
    int64_t ClientID; //Internal implementaion
};


typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;


typedef struct redis_script {
    const char		*text;
    sds			hash;
} redisScript;

static int duplicates;	/* TODO: add counter to individual contexts */

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
    if (status != REDIS_OK)
    {
        printf("Error: %s\n", c->errstr);
        return;
    }
    // printf("\nDisconnected...\n");
}


static redisScript scripts[] = {
/* Script HASH_MAP_ID pcp:map:<name> , <string> -> ID
	returns a map identifier from a given key (hash) and
	value (string).  If the string has not been observed
	previously a new identifier is assigned and the hash
	updated.  This is a cache-internal identifier only.
 */
    { .text = \
	"local ID = redis.pcall('HGET', KEYS[1], ARGV[1])\n"
	"local NEW = 0\n"
	"if (ID == false) then\n"
	"    ID = redis.pcall('HLEN', KEYS[1]) + 1\n"
	"    redis.call('HSETNX', KEYS[1], ARGV[1], tostring(ID))\n"
	"    NEW = 1\n"
	"end\n"
	"return {tonumber(ID), NEW}\n"
    },
};

enum {
    HASH_MAP_ID = 0,
    NSCRIPTS
};

static void
redis_load_scripts(redisAsyncContext *redis, void *arg)
{
    redisReply		*reply;
    redisScript		*script;
    int			i, sts;
    sds			cmd;

    for (i = 0; i < NSCRIPTS; i++) {
	script = &scripts[i];
	cmd = redis_command(3);
	cmd = redis_param_str(cmd, "SCRIPT", sizeof("SCRIPT")-1);
	cmd = redis_param_str(cmd, "LOAD", sizeof("LOAD")-1);
	cmd = redis_param_str(cmd, script->text, strlen(script->text));

	/* Note: needs to be executed on all Redis instances */
//	if (redis_async_submitcb(redis, NULL, NULL,cmd, NULL, NULL, NULL) != REDIS_OK) {
//	    fprintf(stderr, "Failed to LOAD Redis LUA script[%d] setup\n%s\n",
//			    i, script->text);
//	    exit(1);	/* TODO: propogate error */
//	}
	sdsfree(cmd);
    }

    for (i = 0; i < NSCRIPTS; i++) {
	script = &scripts[i];
//	sts = redisGetReply(redis, (void **)&reply);
//	if (sts != REDIS_OK || reply->type != REDIS_REPLY_STRING) {
//	    fprintf(stderr, "Failed to LOAD Redis LUA script[%d]: %s\n%s\n",
//			    i, reply->str, script->text);
//	    exit(1);	/* TODO: propogate error */
//	}
	if ((scripts[i].hash = sdsnew(reply->str)) == NULL) {
	    fprintf(stderr, "Failed to save LUA script SHA1 hash\n");
	    exit(1);	/* TODO: propogate error */
	}


	if (pmDebugOptions.series)
	    fprintf(stderr, "Registered script[%d] as %s\n", i, script->hash);
    }

}
int
redis_async_submitcb(redisSlots *redis, const char *command, sds key, sds cmd, redis_callback callback, void *arg, void *handle)
{
    redisAsyncContext   *Asynccontext = redisAsyncGet(redis, command, key);
    redisReply		*reply;
    int			sts;

    if (UNLIKELY(pmDebugOptions.desperate))
        fputs(cmd, stderr);

    redisEventAttach(Asynccontext, uv_default_loop());
    redisAsyncSetConnectCallBack(Asynccontext, connectCallback);
    redisAsyncSetDisconnectCallBack(Asynccontext,disconnectCallback);


    redisAsyncFormattedCommand(Asynccontext, callback, arg, cmd, sdslen(cmd));


    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    if (key)
        sdsfree(key);
    sdsfree(cmd);
    if (sts != REDIS_OK)
        return -ENOMEM;

    return 0;
}


int
redis_submitcb(redisSlots *redis, const char *command, sds key, sds cmd,
		redis_callback callback, void *arg)
{
    redisContext	*context = redisGet(redis, command, key);
    redisReply		*reply;
    int			sts;

    if (UNLIKELY(pmDebugOptions.desperate))
	fputs(cmd, stderr);

    sts = redisAppendFormattedCommand(context, cmd, sdslen(cmd));
    if (key)
	sdsfree(key);
    sdsfree(cmd);
    if (sts != REDIS_OK)
	return -ENOMEM;

    /* TODO: switch to async commands and callback handling */
    if (callback) {
	redisGetReply(context, (void **)&reply);
	callback(redis, reply, arg);
    }
    return 0;
}

/* TODO: remove calls, use async commands and callback handling */
int
redis_submit(redisSlots *redis, const char *command, sds key, sds cmd)
{
    return redis_submitcb(redis, command, key, cmd, NULL, NULL);
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

static void
checkStatusOK(redisReply *reply, const char *format, ...)
{
    if (reply->type == REDIS_REPLY_STATUS &&
	(strcmp("OK", reply->str) == 0 || strcmp("QUEUED", reply->str) == 0)) {
	return;
    } else {
	va_list	argp;
	va_start(argp, format);
	checkError(reply, format, argp);
	va_end(argp);
    }
}

static int
checkStreamDup(redisReply *reply, sds stamp, const char *hash)
{
    const char dupmsg[] = \
"ERR The ID specified in XADD is smaller than the target stream top item";

    if (reply->type == REDIS_REPLY_ERROR && strcmp(dupmsg, reply->str) == 0) {
	fprintf(stderr, "Warning: dup stream %s insert at %s\n", hash, stamp);
	return 1;
    }
    return 0;
}

static void
checkStatusString(redisReply *reply, sds s, const char *format, ...)
{
    if (reply->type == REDIS_REPLY_STATUS && strcmp(s, reply->str) == 0) {
	return;
    } else {
	va_list	argp;
	va_start(argp, format);
	checkError(reply, format, argp);
	va_end(argp);
    }
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
checkMapScript(redisReply *reply, long long *add, sds s, const char *format, ...)
{
    /* on success, map script script returns two integer values via array */
    if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements == 2 &&
	reply->element[0]->type == REDIS_REPLY_INTEGER &&
	reply->element[1]->type == REDIS_REPLY_INTEGER) {
	*add = reply->element[1]->integer;	/* is this newly allocated */
	return reply->element[0]->integer;	/* the actual string mapid */
    } else {
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

static long long
redis_map(redisSlots *redis, redisMap *mapping, sds mapkey)
{
    redisMapEntry	*entry = redisMapLookup(mapping, mapkey);
    const char		*mapname = redisMapName(mapping);
    redisReply		*reply;
    long long		map, add = 0;
    sds			cmd, msg, key;
    redisContext        *c;
    sds                 error = sdsnew("NOSCRIPT No matching script. Please use EVAL.");

    if (entry != NULL)
	return redisMapValue(entry);

    key = sdscatfmt(sdsempty(), "pcp:map:%s", mapname);
    cmd = redis_command(5);
    cmd = redis_param_str(cmd, EVALSHA, EVALSHA_LEN);
    cmd = redis_param_sds(cmd, scripts[HASH_MAP_ID].hash);
    cmd = redis_param_str(cmd, "1", sizeof("1")-1);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sds(cmd, mapkey);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, EVALSHA, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    if (reply && reply->type == REDIS_REPLY_ERROR && strcmp(error, reply->str) == 0) {
        freeReplyObject(reply);
        redis_load_scripts(c, NULL);
        redis_submit(redis, EVALSHA, key, cmd);
        redisGetReply(c, (void **)&reply);
    }
    map = checkMapScript(reply, &add, "%s: %s (%s)" EVALSHA,
			"string mapping script", mapname);
    redisMapInsert(mapping, mapkey, map);
    freeReplyObject(reply);

    /* publish any newly created name mapping */
    if (add) {
	msg = sdscatfmt(sdsempty(), "%I:%s", map, mapkey);
	key = sdscatfmt(sdsempty(), "pcp:channel:%s", mapname);
	cmd = redis_command(3);
	cmd = redis_param_str(cmd, PUBLISH, PUBLISH_LEN);
	cmd = redis_param_sds(cmd, key);
	cmd = redis_param_sds(cmd, msg);
	sdsfree(msg);
        c = redisGet(redis, cmd, key);
	redis_submit(redis, PUBLISH, key, cmd);

	/* TODO: async callback function */
        redisGetReply(c, (void **)&reply);
	checkInteger(reply, "%s: %s", PUBLISH, "new %s mapping", mapname);

	freeReplyObject(reply);
    }

    return map;
}

void
redis_series_source(redisSlots *redis, context_t *context)
{
    redisReply		*reply;
    const char		*hash = pmwebapi_hash_str(context->hash);
    long long		mapid, hostid;
    sds			cmd, key, val;
    redisContext        *c;

    if ((mapid = context->mapid) <= 0) {
	mapid = redis_map(redis, contextmap, context->name);
	context->mapid = mapid;
    }
    if ((hostid = context->hostid) <= 0) {
	hostid = redis_map(redis, contextmap, context->host);
	context->hostid = hostid;
    }

    key = sdscatfmt(sdsempty(), "pcp:source:context.name:%I", mapid);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sha(cmd, context->hash);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s: %s", SADD, "mapping context to source name");
    freeReplyObject(reply);

    val = sdscatfmt(sdsempty(), "%I", mapid);
    key = sdscatfmt(sdsempty(), "pcp:context.name:source:%s", hash);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sds(cmd, val);
    sdsfree(val);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s: %s", SADD, "mapping source name to context");
    freeReplyObject(reply);

    key = sdscatfmt(sdsempty(), "pcp:source:context.name:%I", hostid);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sha(cmd, context->hash);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s: %s", SADD, "mapping context to host name");
    freeReplyObject(reply);

    val = sdscatfmt(sdsempty(), "%I", hostid);
    key = sdscatfmt(sdsempty(), "pcp:context.name:source:%s", hash);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sds(cmd, val);
    sdsfree(val);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s: %s", SADD, "mapping host name to context");
    freeReplyObject(reply);
}

void
redis_series_inst(redisSlots *redis, metric_t *metric, value_t *value)
{
    redisReply		*reply;
    const char		*hash;
    sds			cmd, key, val, id;
    redisContext        *c;

    if (!value->name)
	return;

    if (value->mapid <= 0) {
	val = sdsnew(value->name);
	value->mapid = redis_map(redis, instmap, val);
	sdsfree(val);
    }

    key = sdscatfmt(sdsempty(), "pcp:series:inst.name:%I", value->mapid);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sha(cmd, metric->hash);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s: %s", SADD, "mapping series to inst name");
    freeReplyObject(reply);

    hash = pmwebapi_hash_str(metric->hash);
    key = sdscatfmt(sdsempty(), "pcp:instances:series:%s", hash);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sha(cmd, value->hash);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s: %s", SADD, "mapping instance to series");
    freeReplyObject(reply);

    hash = pmwebapi_hash_str(value->hash);
    id = sdscatfmt(sdsempty(), "%I", value->mapid);
    val = sdscatfmt(sdsempty(), "%i", value->inst);
    key = sdscatfmt(sdsempty(), "pcp:inst:series:%s", hash);
    cmd = redis_command(8);
    cmd = redis_param_str(cmd, HMSET, HMSET_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_str(cmd, "inst", sizeof("inst")-1);
    cmd = redis_param_sds(cmd, val);
    cmd = redis_param_str(cmd, "name", sizeof("name")-1);
    cmd = redis_param_sds(cmd, id);
    cmd = redis_param_str(cmd, "series", sizeof("series")-1);
    cmd = redis_param_sha(cmd, metric->hash);
    sdsfree(val);
    sdsfree(id);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, HMSET, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkStatusOK(reply, "%s: %s", HMSET, "setting metric inst");
    freeReplyObject(reply);
}

typedef struct {
    redisSlots		*redis;
    metric_t		*metric;
    value_t		*value;
} annotate_t;

static int
annotate_metric(const pmLabel *label, const char *json, annotate_t *my)
{
    redisMap		*valuemap;
    redisSlots          *redis = my->redis;
    redisReply		*reply;
    const char		*offset;
    size_t		length;
    const char		*hash;
    long long		value_mapid, name_mapid;
    sds			cmd, key, val, name;
    redisContext        *c;

    offset = json + label->name;
    val = sdsnewlen(offset, label->namelen);
    name_mapid = redis_map(redis, labelsmap, val);
    sdsfree(val);

    offset = json + label->value;
    length = label->valuelen;

    /*
     * TODO: decode complex values ('{...}' and '[...]'),
     * using a dot-separated name for these maps, and names
     * with explicit array index suffix for array entries.
     * This is safe as JSONB names cannot present that way.
     */

    val = sdsnewlen(offset, length);
    key = sdscatfmt(sdsempty(), "label.%I.value", name_mapid);
    valuemap = redisMapCreate(key);
    value_mapid = redis_map(redis, valuemap, val);
    redisMapRelease(valuemap);
    sdsfree(key);
    sdsfree(val);

    if (my->value != NULL)
	hash = pmwebapi_hash_str(my->value->hash);
    else
	hash = pmwebapi_hash_str(my->metric->hash);

    name = sdscatfmt(sdsempty(), "%I", name_mapid);
    val = sdscatfmt(sdsempty(), "%I", value_mapid);
    key = sdscatfmt(sdsempty(), "pcp:labels:series:%s", hash);
    cmd = redis_command(4);
    cmd = redis_param_str(cmd, HMSET, HMSET_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sds(cmd, name);
    cmd = redis_param_sds(cmd, val);
    sdsfree(name);
    sdsfree(val);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, HMSET, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkStatusOK(reply, "%s: %s", HMSET, "setting series labels");
    freeReplyObject(reply);

    key = sdscatfmt(sdsempty(), "pcp:series:label.%I.value:%I",
		    name_mapid, value_mapid);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sha(cmd, my->metric->hash);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s %s", "SADD", "pcp:series:label.X.value:Y");
    freeReplyObject(reply);

    return 0;
}

static int
filter(const pmLabel *label, const char *json, void *arg)
{
    if (pmDebugOptions.series)
	fprintf(stderr, "Caching label %.*s=%.*s (optional=%s)\n",
		label->namelen, json + label->name,
		label->valuelen, json + label->value,
		(label->flags & PM_LABEL_OPTIONAL) ? "yes" : "no");

    return annotate_metric(label, json, (annotate_t *)arg);
}

void
redis_series_label(redisSlots *redis, metric_t *metric, value_t *value)
{
    annotate_t	annotate;
    char	buf[PM_MAXLABELJSONLEN];
    int		sts;

    annotate.redis = redis;
    annotate.metric = metric;
    annotate.value = value;

    sts = merge_labelsets(metric, value, buf, sizeof(buf), filter, &annotate);
    if (sts < 0) {
	fprintf(stderr, "%s: failed to merge series %s labelsets: %s\n",
		pmGetProgname(), pmwebapi_hash_str(metric->hash),
		pmErrStr(sts));
	exit(1);
    }
}

void
redis_series_metric(redisSlots *redis, context_t *context, metric_t *metric)
{
    redisReply	*reply;
    value_t	*value;
    const char	*hash = pmwebapi_hash_str(metric->hash);
    const char	*units, *indom, *pmid, *sem, *type;
    char	*name;
    int		i, map;
    sds		val, cmd, key;
    redisContext *c;

    for (i = 0; i < metric->numnames; i++) {
	if ((name = metric->names[i]) == NULL)
	    continue;
	if ((map = metric->mapids[i]) <= 0) {
	    val = sdsnew(name);
	    map = redis_map(redis, namesmap, val);
	    metric->mapids[i] = map;
	    sdsfree(val);
	}

	key = sdscatfmt(sdsempty(), "pcp:metric.name:series:%s", hash);
	val = sdscatfmt(sdsempty(), "%I", map);
	cmd = redis_command(3);
	cmd = redis_param_str(cmd, SADD, SADD_LEN);
	cmd = redis_param_sds(cmd, key);
	cmd = redis_param_sds(cmd, val);
	sdsfree(val);
        c = redisGet(redis, cmd, key);
	redis_submit(redis, SADD, key, cmd);

	/* TODO: async callback function */
	redisGetReply(c, (void **)&reply);
	checkInteger(reply, "%s %s", SADD, "map metric name to series");
	freeReplyObject(reply);

	key = sdscatfmt(sdsempty(), "pcp:series:metric.name:%I", map);
	cmd = redis_command(3);
	cmd = redis_param_str(cmd, SADD, SADD_LEN);
	cmd = redis_param_sds(cmd, key);
	cmd = redis_param_sha(cmd, metric->hash);
        c = redisGet(redis, cmd, key);
	redis_submit(redis, SADD, key, cmd);

	/* TODO: async callback function */
	redisGetReply(c, (void **)&reply);
	checkInteger(reply, "%s: %s", SADD, "map series to metric name");
	freeReplyObject(reply);
    }

    indom = indom_str(metric);
    pmid = pmid_str(metric);
    sem = semantics_str(metric);
    type = type_str(metric);
    units = units_str(metric);

    key = sdscatfmt(sdsempty(), "pcp:desc:series:%s", hash);
    cmd = redis_command(14);
    cmd = redis_param_str(cmd, HMSET, HMSET_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_str(cmd, "indom", sizeof("indom")-1);
    cmd = redis_param_str(cmd, indom, strlen(indom));
    cmd = redis_param_str(cmd, "pmid", sizeof("pmid")-1);
    cmd = redis_param_str(cmd, pmid, strlen(pmid));
    cmd = redis_param_str(cmd, "semantics", sizeof("semantics")-1);
    cmd = redis_param_str(cmd, sem, strlen(sem));
    cmd = redis_param_str(cmd, "source", sizeof("source")-1);
    cmd = redis_param_sha(cmd, context->hash);
    cmd = redis_param_str(cmd, "type", sizeof("type")-1);
    cmd = redis_param_str(cmd, type, strlen(type));
    cmd = redis_param_str(cmd, "units", sizeof("units")-1);
    cmd = redis_param_str(cmd, units, strlen(units));
    c = redisGet(redis, cmd, key);
    redis_submit(redis, HMSET, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkStatusOK(reply, "%s: %s", HMSET, "setting metric desc");
    freeReplyObject(reply);

    if (metric->desc.indom == PM_INDOM_NULL) {
	redis_series_label(redis, metric, NULL);
    } else {
	for (i = 0; i < metric->u.inst->listcount; i++) {
	    value = &metric->u.inst->value[i];
	    redis_series_inst(redis, metric, value);
	    redis_series_label(redis, metric, value);
	}
    }

    hash = pmwebapi_hash_str(context->hash);
    key = sdscatfmt(sdsempty(), "pcp:series:source:%s", hash);
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SADD, SADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sha(cmd, metric->hash);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, SADD, key, cmd);

    /* TODO: async callback function */
    redisGetReply(c, (void **)&reply);
    checkInteger(reply, "%s: %s", SADD, "mapping series to context");
    freeReplyObject(reply);
}

static sds
series_stream_append(sds cmd, sds name, sds value)
{
    unsigned int	nlen = sdslen(name);
    unsigned int	vlen = sdslen(value);

    cmd = sdscatfmt(cmd, "$%u\r\n%S\r\n$%u\r\n%S\r\n", nlen, name, vlen, value);
    sdsfree(value);
    return cmd;
}

static sds
series_stream_value(sds cmd, sds name, int type, pmAtomValue *avp)
{
    unsigned int	bytes;
    const char		*string;
    sds			value;

    if (!avp)
	return series_stream_append(cmd, name, sdsnewlen("0", 1));

    switch (type) {
    case PM_TYPE_32:
	value = sdscatfmt(sdsempty(), "%i", avp->l);
	break;
    case PM_TYPE_U32:
	value = sdscatfmt(sdsempty(), "%u", avp->ul);
	break;
    case PM_TYPE_64:
	value = sdscatfmt(sdsempty(), "%I", avp->ll);
	break;
    case PM_TYPE_U64:
	value = sdscatfmt(sdsempty(), "%U", avp->ull);
	break;

    case PM_TYPE_FLOAT:
	value = sdscatprintf(sdsempty(), "%e", (double)avp->f);
	break;
    case PM_TYPE_DOUBLE:
	value = sdscatprintf(sdsempty(), "%e", (double)avp->d);
	break;

    case PM_TYPE_STRING:
	if ((string = avp->cp) == NULL)
	    string = "<null>";
	value = sdsnew(string);
	break;

    case PM_TYPE_AGGREGATE:
    case PM_TYPE_AGGREGATE_STATIC:
	if (avp->vbp != NULL) {
	    string = avp->vbp->vbuf;
	    bytes = avp->vbp->vlen - PM_VAL_HDR_SIZE;
	} else {
	    string = "<null>";
	    bytes = strlen(string);
	}
	value = sdsnewlen(string, bytes);
	break;

    default:
	value = sdscatfmt(sdsempty(), "%i", PM_ERR_NYI);
	break;
    }

    return series_stream_append(cmd, name, value);
}

void
redis_series_stream(redisSlots *redis, sds stamp, metric_t *metric)
{
    unsigned int	count;
    redisReply		*reply;
    const char		*hash = pmwebapi_hash_str(metric->hash);
    int			i, sts, type;
    sds			cmd, key, name = sdsempty(), stream = sdsempty();
    redisContext        *c;

    count = 3;	/* XADD key stamp */
    key = sdscatfmt(sdsempty(), "pcp:values:series:%s", hash);

    if ((sts = metric->error) < 0) {
	stream = series_stream_append(stream,
			sdsnewlen("-1", 2), sdscatfmt(sdsempty(), "%i", sts));
	count += 2;
    } else {
	type = metric->desc.type;
	if (metric->desc.indom == PM_INDOM_NULL) {
	    sdsclear(name);
	    stream = series_stream_value(stream, name, type, &metric->u.atom);
	    count += 2;
	} else if (metric->u.inst->listcount <= 0) {
	    stream = series_stream_append(stream, sdsnew("0"), sdsnew("0"));
	    count += 2;
	} else {
	    for (i = 0; i < metric->u.inst->listcount; i++) {
		value_t	*v = &metric->u.inst->value[i];
		name = sdscpylen(name, (const char *)&v->hash[0], sizeof(v->hash));
		stream = series_stream_value(stream, name, type, &v->atom);
		count += 2;
	    }
	}
    }
    //    sdsfree(name);

    cmd = redis_command(count);
    cmd = redis_param_str(cmd, XADD, XADD_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_sds(cmd, stamp);
    cmd = redis_param_raw(cmd, stream);
    sdsfree(stream);
    c = redisGet(redis, cmd, key);
    redis_submit(redis, XADD, key, cmd);

    /* TODO: check return codes, use async callbacks */
    redisGetReply(c, (void**)&reply);
    if (checkStreamDup(reply, stamp, hash))
	duplicates++;
    else
	checkStatusString(reply, stamp, "status mismatch (%s)", stamp);
    freeReplyObject(reply);
}

void
redis_series_mark(redisSlots *redis, context_t *context, sds timestamp)
{
    /* TODO: inject mark records into time series */
}

static void
redis_update_version_callback(redisSlots *redis, redisReply *reply, void *arg)
{
    checkStatusOK(reply, "%s setup", "pcp:version:schema");
    redis_load_scripts(redis->Asynccontrol,arg);
}

static void
redis_update_version(redisSlots *redis, void *arg, void *handle)
{
    sds			cmd, key;
    const char		ver[] = TO_STRING(SCHEMA_VERSION);

    key = sdsnew("pcp:version:schema");
    cmd = redis_command(3);
    cmd = redis_param_str(cmd, SETS, SETS_LEN);
    cmd = redis_param_sds(cmd, key);
    cmd = redis_param_str(cmd, ver, sizeof(ver)-1);
    redis_async_submitcb(redis, SETS, key, cmd, redis_update_version_callback, arg, handle);
}

static void
redis_load_version_callback(redisSlots *redis, redisReply *reply, void *arg)
{
    unsigned int	version = 0;

    if (reply->type == REDIS_REPLY_STRING) {
	version = (unsigned int)atoi(reply->str);
	if (!version || version != SCHEMA_VERSION) {
	    /* TODO: notify through info/done callback */
	    fprintf(stderr, "%s: unsupported schema (got v%u, not v%u)\n",
		    pmGetProgname(), version, SCHEMA_VERSION);
	    exit(1);
	}
    } else if (reply->type == REDIS_REPLY_ERROR) {
	/* TODO: notify through info/done callback */
        fprintf(stderr, "%s: version check error: %s\n",
                pmGetProgname(), reply->str);
        exit(1);
    } else if (reply->type != REDIS_REPLY_NIL) {
	/* TODO: notify through info/done callback */
	fprintf(stderr, "%s: unexpected schema version reply type (%s)\n",
		pmGetProgname(), redis_reply(reply->type));
	exit(1);
    }


    /* set the version when none found (first time through) */
    if (version != SCHEMA_VERSION)
	redis_update_version(redis, arg, NULL);

    redis_load_scripts(redis->Asynccontrol,arg);
}

static void
redis_load_version(redisSlots *redis, void *arg, void *handle)
{
    sds			cmd, key;

    key = sdsnew("pcp:version:schema");
    cmd = redis_command(2);
    cmd = redis_param_str(cmd, GETS, GETS_LEN);
    cmd = redis_param_sds(cmd, key);
    redis_async_submitcb(redis, GETS, key, cmd, redis_load_version_callback, arg, handle);
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
getCallback(redisSlots *redis, redisReply *reply, void *arg , void *handle){
    struct ClientRequestData    *clientdata;
    struct ClientRequestData    *clientdata2;
    redisAsyncContext           *clientcontext;
    int                         len;
    int                         i;

    sds s = sdsempty();

    if (reply == NULL) return;

//    clientdata = (uv_stream_t *)handle->data;
//    clientcontext = clientdata->redisAsyncContext;
//    clientdata2 = clientcontext->data;

    s = ProcessRedisReply(reply);
    printf("%s",s);

//    write_req_t *wr = (write_req_t *) malloc(sizeof(*wr));
//    wr->buf = uv_buf_init(s, sdslen(s));
//    uv_write(&wr->req, clientdata2->client , &wr->buf, 1, after_write);
    //  sdsfree(s);
}


static int
decodeRedisNode(redisSlots *redis, redisReply *reply, redisSlotServer *server)
{
    redisReply		*value;
    unsigned int	port;

    /* expecting IP address and port (integer), ignore optional node ID */
    if (reply->elements < 2)
	return -EINVAL;

    value = reply->element[1];
    if (value->type != REDIS_REPLY_INTEGER)
	return -EINVAL;
    port = (unsigned int)value->integer;

    value = reply->element[0];
    if (value->type != REDIS_REPLY_STRING)
	return -EINVAL;

    server->hostspec = sdscatfmt(sdsempty(), "%s:%u", value->str, port);
    return server->hostspec ? 0 : -ENOMEM;
}

static int
decodeRedisSlot(redisSlots *redis, redisReply *reply)
{
    redisSlotServer	*servers = NULL;
    redisSlotRange	slots, *sp;
    redisReply		*node;
    int			i, n;

    /* expecting start and end slot range integers, then node arrays */
    if (reply->elements < 3)
	return -EINVAL;
    memset(&slots, 0, sizeof(slots));

    node = reply->element[0];
    if ((slots.start = (__uint32_t)checkInteger(node, "%s start", "SLOT")) < 0)
	return -EINVAL;
    node = reply->element[1];
    if ((slots.end = (__uint32_t)checkInteger(node, "%s end", "SLOT")) < 0)
	return -EINVAL;
    node = reply->element[2];
    if ((decodeRedisNode(redis, node, &slots.master)) < 0)
	return -EINVAL;

    if ((sp = calloc(1, sizeof(redisSlotRange))) == NULL)
	return -ENOMEM;
    *sp = slots;    /* struct copy */

    if ((n = reply->elements - 3) > 0)
	if ((servers = calloc(n, sizeof(redisSlotServer))) == NULL)
	    n = 0;
    sp->nslaves = n;
    sp->slaves = servers;

    for (i = 0; i < n; i++) {
	node = reply->element[i + 3];
	if (checkArray(node, "%s range %u-%u slave %d\n",
				"SLOTS", sp->start, sp->end, i) == 0)
	    decodeRedisNode(redis, node, &sp->slaves[i]);
    }

    return redisSlotRangeInsert(redis, sp);
}

static void
decodeRedisSlots(redisSlots *redis, redisReply *reply)
{
    redisReply		*slot;
    int			i;

    for (i = 0; i < reply->elements; i++) {
	slot = reply->element[i];
	if (checkArray(slot, "%s %s entry %d", CLUSTER, "SLOTS", i) == 0)
	    decodeRedisSlot(redis, slot);
    }
}
static void
redis_load_slots_callback(redisSlots *redis, redisReply *reply, void *arg , void *handle)
{

    redisAsyncContext                   *desiredcontex;
    sds                                 keys;

    keys = sdsnew("key");

    /* Case where we're dealing with a single redis intance) */
    sds single = sdsnew("ERR This instance has cluster support disabled");
    redisSlotServer	*servers = NULL;
    redisSlotRange      *slots;
    if (reply && reply->type == REDIS_REPLY_ERROR && strcmp(single, reply->str) == 0) {
        if ((servers = calloc(1, sizeof(redisSlotServer))) != NULL){
            if ((slots = calloc(1, sizeof(redisSlotRange))) == NULL)
                return;
            servers->hostspec = sdscatfmt(sdsempty(), "%s", redis->hostspec);
            slots->nslaves = 0;
            slots->start = 0;
            slots->end = MAXSLOTS;
            slots->master = *servers;
            redisSlotRangeInsert(redis, slots);
        }
    }
    else if (checkArray(reply, "%s %s", CLUSTER, "SLOTS") == 0)
	decodeRedisSlots(redis, reply);

 //   redis_load_version(redis, arg);

    desiredcontex = redisAsyncGet(redis,(const char*)arg,keys);

    if (desiredcontex->err) {
        printf("Error: %s\n", desiredcontex->errstr);
        return;
    }

    redisEventAttach(desiredcontex, uv_default_loop());
    redisAsyncSetConnectCallBack(desiredcontex, connectCallback);
    redisAsyncSetDisconnectCallBack(desiredcontex,disconnectCallback);
    redisAsyncFormattedCommand(desiredcontex , getCallback, (char*)"end-1",((const char*)arg),(strlen((const char*)arg)));
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

}

int
redis_load_slots(redisSlots *redis, void *arg, void *handle)
{
    sds			cmd;

    cmd = redis_command(2);
    cmd = redis_param_str(cmd, CLUSTER, CLUSTER_LEN);
    cmd = redis_param_str(cmd, "SLOTS", sizeof("SLOTS")-1);
    redis_async_submitcb(redis, CLUSTER, NULL, cmd, redis_load_slots_callback, arg, handle);
    return 0;
}

void
redis_init(sds server, void *command, void *handle)
{
    redisSlots          *slots;

    static int		setup;

    if (!setup) {	/* create global string map caches */
	redisMapsInit();
	setup = 1;
    }

    if ((slots = redisAsyncSlotsInit(server)) == NULL)
        exit(1);

    redis_load_slots(slots, command, handle);
}

void
redis_stop(redisContext *redis)
{
    redisFree(redis);
}
