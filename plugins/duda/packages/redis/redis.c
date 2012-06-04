#include <stdio.h>
#include <stdlib.h>

#include "hiredis.h"
#include "async.h"
#include "duda_api.h"
#include "redis.h"
#include "MKPlugin.h"

void mk_redis_read(int fd)
{
    MK_TRACE("[FD %i] Connection Handler / read", fd);
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
        MK_TRACE("Error");
    else
        redisAsyncHandleRead(rc);
}

void mk_redis_write(int fd)
{
    MK_TRACE("[FD %i] Connection Handler / write", fd);
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
        MK_TRACE("Error");
    else
        redisAsyncHandleWrite(rc);
}

void mk_redis_error(int fd)
{
    MK_TRACE("[FD %i] Connection Handler / error", fd);
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
        MK_TRACE("Error");
    else
        redisCleanup(rc);
}

void mk_redis_close(int fd)
{
    MK_TRACE("[FD %i] Connection Handler / close", fd);
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
}

void mk_redis_timeout(int fd)
{
    MK_TRACE("[FD %i] Connection Handler / timeout", fd);

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
    dr = mk_api->mem_alloc(sizeof(duda_redis_t));
    dr->rc = c;
    printf("in thread context redis : %p pid:%p\n",redis_key, (unsigned int)pthread_self());
    list_redis_fd = pthread_getspecific(redis_key);
    if(list_redis_fd == NULL)
    {
        list_redis_fd = malloc(sizeof(struct mk_list));
        mk_list_init(list_redis_fd);
        pthread_setspecific(redis_key, (void *) list_redis_fd);    
    }

    printf("list:%p, list->next:%p, list->prev:%p\n",list_redis_fd, list_redis_fd->next, list_redis_fd->prev);
    mk_list_add(&dr->_head_redis_fd, list_redis_fd);
    return c;
}

int redis_attach(redisAsyncContext *ac, duda_request_t *dr)
{
    redis_data_t *rd;
    printf("mk_api: %p\n",mk_api);
    rd = mk_api->mem_alloc(sizeof(redis_data_t));
    
    rd->rc = ac;
    rd->dr = dr;

    /* Nothing should be attached when something is already attached */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* Register functions to start/stop listening for events */
    ac->ev.addRead = redisAddRead;
    ac->ev.delRead = redisDel;
    ac->ev.addWrite = redisAddWrite;
    ac->ev.delWrite = redisDel;
    ac->ev.cleanup = redisDel;
    ac->ev.data = rd;

    return REDIS_OK;

}

void redisAddRead(void *privdata) {
    printf("In addread\n");
    redis_data_t *rd = (redis_data_t *) privdata;
    mk_api->event_add(rd->rc->c.fd, MK_EPOLL_READ, rd->dr->plugin,
                           rd->dr->cs, rd->dr->sr, MK_EPOLL_LEVEL_TRIGGERED);
    duda_event_register_write(rd->dr);
}

void redisDel(void *privdata) {
    printf("In del\n");
    redis_data_t *rd = (redis_data_t *) privdata;
    mk_api->event_del(rd->rc->c.fd);
}

void redisAddWrite(void *privdata) {
    printf("In addwrite\n");
    redis_data_t *rd = (redis_data_t *) privdata;
    printf("fd:%i\n",rd->rc->c.fd);
    mk_api->event_add(rd->rc->c.fd, MK_EPOLL_WRITE, rd->dr->plugin,
                           rd->dr->cs, rd->dr->sr, MK_EPOLL_LEVEL_TRIGGERED);
    duda_event_register_write(rd->dr);
}

int redis_init(){
    struct mk_list *list_redis_fd;

    pthread_key_create(&redis_key, NULL);
    
    printf("in process context redis : %p pid:%p\n",redis_key, (unsigned int)pthread_self());
    return 1;
}
