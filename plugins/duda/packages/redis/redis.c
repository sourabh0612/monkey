#include <stdio.h>
#include <stdlib.h>

#include "hiredis.h"
#include "async.h"
#include "duda_api.h"
#include "redis.h"

void mk_redis_read(int fd)
{
    MK_TRACE("[FD %i] Connection Handler / read", fd);
    struct mk_list *list_redis_fd,*head;
    duda_redis_t *dr_entry;
    redisAsyncContext *rc;
    list_redis_fd = duda_global_get(redis_global);

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
    list_redis_fd = duda_global_get(redis_global);

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
    list_redis_fd = duda_global_get(redis_global);

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
    list_redis_fd = duda_global_get(redis_global);

    mk_list_foreach(head, list_redis_fd) {
        dr_entry = mk_list_entry(head, duda_redis_t, _head_redis_fd);
        if(dr_entry->rc->c.fd == fd){
            mk_list_del(&dr_entry->_head_redis_fd);
            duda_global_set(redis_global, list_redis_fd);
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
    struct mk_list *list_redis_fd,*new,*prev,*next;
    duda_redis_t *dr;
    redisAsyncContext *c = redisAsyncConnect(ip, port);
    if (c->err) {
        printf("REDIS: Can't connect: %s\n", c->errstr);
        exit(EXIT_FAILURE);
    }
    dr = malloc(sizeof(duda_redis_t));
    dr->rc = c;
    printf("redis : %p pid:%u\n",redis_global.key, (unsigned int)pthread_self());
    list_redis_fd = duda_global_get(redis_global);
    mk_list_add(&dr->_head_redis_fd, list_redis_fd);
    return c;
}

int redis_attach(int efd, redisAsyncContext *ac)
{

    /* Nothing should be attached when something is already attached */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* Register functions to start/stop listening for events */
    ac->ev.addRead = redisAddRead;
    ac->ev.delRead = redisDelRead;
    ac->ev.addWrite = redisAddWrite;
    ac->ev.delWrite = redisDelWrite;
    ac->ev.cleanup = redisCleanup;
    ac->ev.data = &efd;

    return REDIS_OK;

}

void redisAddRead(void *privdata) {
    printf("In addread\n");
/*    int *e = (int *)privdata;
    mk_epoll_add(&e, e->fd, MK_EPOLL_READ,
                      MK_EPOLL_LEVEL_TRIGGERED);
*/
}

void redisDelRead(void *privdata) {
    printf("In delread\n");
/*    redisEvents *e = (redisEvents*)privdata;
    mk_epoll_del(efd, fd);
*/
}

void redisAddWrite(void *privdata) {
    printf("In addwrite\n");
/*    redisEvents *e = (redisEvents*)privdata;
    mk_epoll_add(e->efd, e->fd, MK_EPOLL_WRITE,
                      MK_EPOLL_LEVEL_TRIGGERED);
*/
}

void redisDelWrite(void *privdata) {
    printf("In delwrite\n");
/*    redisEvents *e = (redisEvents*)privdata;
    mk_epoll_del(efd, fd);
*/
}

void redisCleanup(void *privdata) {
    printf("In clean\n");
/*    redisEvents *e = (redisEvents*)privdata;
    redisDelRead(privatedata);
    redisDelWrite(privatedata);
    free(e);
*/
}

void redis_listen(int efd, int max_events)
{
    mk_epoll_handlers *handler;
    handler = mk_epoll_set_handlers((void *) mk_redis_read,
                                    (void *) mk_redis_write,
                                    (void *) mk_redis_error,
                                    (void *) mk_redis_close,
                                    (void *) mk_redis_timeout);

    mk_epoll_init(efd, handler, max_events);

}

int redis_init(){
    return 1;
}
