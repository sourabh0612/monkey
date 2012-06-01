#ifndef DUDA_PACKAGE_REDIS_H
#define DUDA_PACKAGE_REDIS_H

#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>

#include "hiredis.h"
#include "async.h"
#include "duda_api.h"

pthread_key_t redis_key;

typedef struct redis_data {
    redisAsyncContext *rc;
    duda_request_t *dr;
} redis_data_t;

typedef struct duda_redis {
    
    redisAsyncContext *rc;
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
                    const char *, va_list);
    void (*listen) (int, int);

    /* epoll functions */
    
};

typedef struct duda_api_redis redis_object_t;

redis_object_t *redis;

redisAsyncContext * redis_connect(const char *ip, int port);
int redis_attach(redisAsyncContext *rc, duda_request_t *dr);
void redis_listen(int efd, int max_events);
int redis_init();

void redisAddRead(void *privdata);
void redisDelRead(void *privdata);
void redisAddWrite(void *privdata);
void redisDelWrite(void *privdata);
void redisCleanup(void *privdata);

void mk_redis_read(int fd);
void mk_redis_write(int fd);
void mk_redis_error(int fd);
void mk_redis_close(int fd);
void mk_redis_timeout(int fd);

#endif
