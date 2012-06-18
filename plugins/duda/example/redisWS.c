/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "webservice.h"
#include "packages/redis/redis.h"

DUDA_REGISTER("Service Example", "service");

duda_global_t my_data_mem;
duda_global_t my_data_empty;

/*
 *
 * URI Map example
 * +--------------------------------------------------------------+
 * |  Interface         Method     Param Name  Param Max Length   |
 * +--------------------------------------------------------------+
 * |  redis            version                       0            |
 * +--------------------------------------------------------------+
 * |                   write_key    key_name         6            |
 * |                                key_value        6            |
 * +--------------------------------------------------------------+
 * |                   read_key     key_name         6            |
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
        exit(1);
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void versionCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    
    const char *field = "redis_version:";
    char *p, *eptr,*version;
    int major, minor;

    p = strstr(reply->str,field);
    major = strtol(p+strlen(field),&eptr,10);
    p = eptr+1;
    minor = strtol(p,&eptr,10);
    
    printf("Version:%d.%d\n",major,minor);
//    response->body_print((duda_request_t *)privdata, major+minor+"\n", st);
    redis->disconnect(c);
}

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    
    printf("%s: %s\n", (char*)privdata, reply->str);

    /* Disconnect after receiving the reply to GET */
    redis->disconnect(c);
}

void setCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    
    printf("%s\n", reply->str);

    /* Disconnect after receiving the reply to GET */
    redis->disconnect(c);

}

void cb_version(duda_request_t *dr)
{
    response->http_status(dr, 200);
    response->http_header(dr, "Content-Type: text/plain", 24);

    redisAsyncContext *rc = redis->connect("127.0.0.1", 6379);
    redis->attach(rc,dr);
    redis->setConnectCallback(rc,connectCallback);
    redis->setDisconnectCallback(rc, disconnectCallback);
    redis->command(rc, versionCallback, dr, "INFO");

    response->body_print(dr, "Redis test successful\n", 22);

    response->end(dr, cb_end);
}

void cb_read_key(duda_request_t *dr)
{
    char *key;
    response->http_status(dr, 200);
    response->http_header(dr, "Content-Type: text/plain", 24);

    redisAsyncContext *rc = redis->connect("127.0.0.1", 6379);
    redis->attach(rc,dr);
    redis->setConnectCallback(rc,connectCallback);
    redis->setDisconnectCallback(rc, disconnectCallback);
    key = param->get(dr, 0);
    printf("%p\n",key);
    redis->command(rc, getCallback, key, "GET %b", key, strlen(key));

    response->body_print(dr, "Redis test successful\n", 22);

    response->end(dr, cb_end);
}

void cb_write_key(duda_request_t *dr)
{
    char *key,*value;
    response->http_status(dr, 200);
    response->http_header(dr, "Content-Type: text/plain", 24);

    redisAsyncContext *rc = redis->connect("127.0.0.1", 6379);
    redis->attach(rc,dr);
    redis->setConnectCallback(rc,connectCallback);
    redis->setDisconnectCallback(rc, disconnectCallback);
    key = param->get(dr, 0);
    value = param->get(dr,1);
    redis->command(rc, setCallback, NULL, "SET %b %b", key, strlen(key), value, strlen(value));

    response->body_print(dr, "Redis test successful\n", 22);

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
    duda_param_t *params;

    duda_service_init();

    session->init();

    duda_load_package(redis, "redis");

    /* An empty global variable */
    duda_global_init(my_data_empty, NULL);

    /* A global variable with the value returned by the callback */
    duda_global_init(my_data_mem, cb_global_mem);

    /* archive interface */
    if_system = map->interface_new("hiredis");

    /* URI: /redisWS/redis/version */
    method = map->method_new("version", "cb_version", 0);
    map->interface_add_method(method, if_system);

    /* URI: /redisWS/redis/write_key/key_name/key_value */
    method = map->method_new("write_key", "cb_write_key", 2);
    params = map->param_new("key_name", 6);
    map->method_add_param(params, method);
    params = map->param_new("key_value", 6);
    map->method_add_param(params, method);
    map->interface_add_method(method, if_system);

    /* URI: /redisWS/redis/read_key/key_name */
    method = map->method_new("read_key", "cb_read_key", 1);
    params = map->param_new("key_name", 6);
    map->method_add_param(params, method);
    map->interface_add_method(method, if_system);

    /* Add interface to map */
    duda_service_add_interface(if_system);

    duda_service_ready();
}
