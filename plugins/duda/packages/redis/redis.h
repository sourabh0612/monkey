#ifndef DUDA_PACKAGE_REDIS_H
#define DUDA_PACKAGE_REDIS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "hiredis.h"
#include "async.h"
#include "duda_api.h"
#include "webservice.h"

pthread_key_t redis_key;

typedef struct duda_redis {
    
    redisAsyncContext *rc;
    duda_request_t *dr;
    struct mk_list _head_redis_fd;
    
} duda_redis_t;

struct duda_api_redis {

    /* redis functions */
    redisAsyncContext *(*connect) (const char *, int);
    void (*disconnect) (redisAsyncContext *);
    int (*attach) (redisAsyncContext *, duda_request_t *);
    int (*setConnectCallback) (redisAsyncContext *,
                               void (*)(const redisAsyncContext *, int));
    int (*setDisconnectCallback) (redisAsyncContext *,
                                  void (*)(const redisAsyncContext *, int));
    int (*command) (redisAsyncContext *, 
                    void (*) (redisAsyncContext*, void*, void*), void *, 
                    const char *,...);
};

typedef struct duda_api_redis redis_object_t;

redis_object_t *redis;

redisAsyncContext * redis_connect(const char *ip, int port);
int redis_attach(redisAsyncContext *rc, duda_request_t *dr);
int redis_init();

void redisAddRead(void *privdata);
void redisDel(void *privdata);
void redisAddWrite(void *privdata);

int redis_read(int fd, struct duda_request *dr);
int redis_write(int fd, struct duda_request *dr);
int redis_error(int fd, struct duda_request *dr);
int redis_close(int fd, struct duda_request *dr);
int redis_timeout(int fd, struct duda_request *dr);

#endif
