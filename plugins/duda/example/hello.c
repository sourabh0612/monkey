/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "webservice.h"
#include "packages/redis/redis.h"

#define INCORRECT_PARAMETERS "Incorrect Parameters\n"
#define FORMATTED_OUT "==Formatted JSON output==\n"
#define UNFORMATTED_OUT "\n\n==Unformatted JSON output==\n"

/* Allowed parameter values */
#define CREATE "create"
#define PARSE "parse"
#define FORMATTED "formatted"
#define UNFORMATTED "unformatted"

DUDA_REGISTER("Service Example", "service");

duda_global_t my_data_mem;
duda_global_t my_data_empty;

/*
 *
 * URI Map example
 * +--------------------------------------------------------------+
 * |  Interface         Method     Param Name  Param Max Length   |
 * +--------------------------------------------------------------+
 * |  examples         hello_world                   0            |
 * +--------------------------------------------------------------+
 * |                   sendfile                      0            |
 * +--------------------------------------------------------------+
 * |                   json_first                    0            |
 * +--------------------------------------------------------------+
 * |                   json_second     action        6            |
 * |                               (create/parse)                 |
 * |                                   format        11           |
 * |                          (formatted/unformatted)             |
 * +--------------------------------------------------------------+
 * |                   sha1_test                     0            |
 * +--------------------------------------------------------------+
 *
 */

void cb_end(duda_request_t *dr)
{
    msg->info("my end callback");
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void cb_hello_world(duda_request_t *dr)
{
    msg->warn("my global key: %p", global->get(my_data_mem));
    printf("REDIS:%d\n",duda_redis_fds);
    response->http_status(dr, 200);
    response->http_header(dr, "Content-Type: text/plain", 24);

    response->body_print(dr, "hello world!\n", 13);
    int efd,max_events=10;
    efd = epoll_create(max_events);

    redisAsyncContext *rc = redis->connect("127.0.0.1", 6379);
    redis->attach(efd,rc);
    redis->setConnectCallback(rc,connectCallback);
    redis->disconnect(rc);
    redis->listen(efd,max_events);
    response->end(dr, cb_end);
}


void *cb_global_mem()
{
    void *mem = malloc(16);
    return mem;
}

int duda_main(struct duda_api_objects *api)
{
    duda_interface_t *if_system;
    duda_method_t    *method;
    duda_param_t *param;

    duda_service_init();

    session->init();

    duda_load_package(redis, "redis");

    /* An empty global variable */
    duda_global_init(my_data_empty, NULL);

    /* A global variable with the value returned by the callback */
    duda_global_init(my_data_mem, cb_global_mem);

    /* archive interface */
    if_system = map->interface_new("examples");

    /* URI: /hello/examples/hello_word */
    method = map->method_new("hello_world", "cb_hello_world", 0);
    map->interface_add_method(method, if_system);

   /* Add interface to map */
    duda_service_add_interface(if_system);

    if_system = map->interface_new("test");
    duda_service_add_interface(if_system);

    duda_service_ready();
}
