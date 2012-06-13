#include <stdio.h>
#include <stdlib.h>

#include "hiredis.h"
#include "async.h"
#include "duda_api.h"
#include "redis.h"

int redis_read(int fd, struct duda_request *dr)
{
    printf("[FD %i] Redis Handler / read\n", fd);
    struct mk_list *list_redis_fd,*head;
    duda_redis_t *dr_entry;
    redisAsyncContext *rc;
    list_redis_fd = pthread_getspecific(redis_key);

    mk_list_foreach(head, list_redis_fd) {
        dr_entry = mk_list_entry(head, duda_redis_t, _head_redis_fd);
        if(dr_entry->rc->c.fd == fd){
            rc = dr_entry->rc;
            break;
        }
    }
    if(rc == NULL)
        printf("Error\n");
    else
        redisAsyncHandleRead(rc);
    return 1;
}

int redis_write(int fd, struct duda_request *dr)
{
    printf("[FD %i] Redis Handler / write\n", fd);
    struct mk_list *list_redis_fd,*head;
    duda_redis_t *dr_entry;
    redisAsyncContext *rc;
    list_redis_fd = pthread_getspecific(redis_key);

    mk_list_foreach(head, list_redis_fd) {
        dr_entry = mk_list_entry(head, duda_redis_t, _head_redis_fd);
        if(dr_entry->rc->c.fd == fd){
            rc = dr_entry->rc;
            break;
        }
    }
    if(rc == NULL)
        printf("Error\n");
    else
        redisAsyncHandleWrite(rc);
    return 1;
}

int redis_error(int fd, struct duda_request *dr)
{
    printf("[FD %i] Redis Handler / error\n", fd);
    struct mk_list *list_redis_fd,*head;
    duda_redis_t *dr_entry;
    redisAsyncContext *rc;
    list_redis_fd = pthread_getspecific(redis_key);

    mk_list_foreach(head, list_redis_fd) {
        dr_entry = mk_list_entry(head, duda_redis_t, _head_redis_fd);
        if(dr_entry->rc->c.fd == fd){
            rc = dr_entry->rc;
            break;
        }
    }
    if(rc == NULL)
        printf("Error\n");
    else
        redisDel(rc);
    return 1;
}

int redis_close(int fd, struct duda_request *dr)
{
    printf("[FD %i] Redis Handler / close\n", fd);
    struct mk_list *list_redis_fd,*head;
    duda_redis_t *dr_entry;
    list_redis_fd = pthread_getspecific(redis_key);

    mk_list_foreach(head, list_redis_fd) {
        dr_entry = mk_list_entry(head, duda_redis_t, _head_redis_fd);
        if(dr_entry->rc->c.fd == fd){
            mk_list_del(&dr_entry->_head_redis_fd);
            pthread_setspecific(redis_key, (void *) list_redis_fd);
            break;
        }
    }
    return 1;
}

int redis_timeout(int fd, struct duda_request *dr)
{
    printf("[FD %i] Redis Handler / timeout\n", fd);
    
    return 1;

}

redisAsyncContext * redis_connect(const char *ip, int port)
{
    struct mk_list *list_redis_fd;
    duda_redis_t *dr;
    redisAsyncContext *c = redisAsyncConnect(ip, port);
    if (c->err) {
        printf("REDIS: Can't connect: %s\n", c->errstr);
        exit(EXIT_FAILURE);
    }
    dr = monkey->mem_alloc(sizeof(duda_redis_t));
    dr->rc = c;
    
    list_redis_fd = pthread_getspecific(redis_key);
    if(list_redis_fd == NULL)
    {
        list_redis_fd = malloc(sizeof(struct mk_list));
        mk_list_init(list_redis_fd);
        pthread_setspecific(redis_key, (void *) list_redis_fd);    
    }

    mk_list_add(&dr->_head_redis_fd, list_redis_fd);
    return c;
}

int redis_attach(redisAsyncContext *ac, duda_request_t *dr)
{
    event->add(ac->c.fd, dr, redis_read, redis_write, redis_error, redis_close, redis_timeout);

    return REDIS_OK;

}

int redis_init(){
    pthread_key_create(&redis_key, NULL);
    
    return 1;
}
