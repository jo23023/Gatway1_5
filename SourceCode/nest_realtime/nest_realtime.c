#include "nest_realtime.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <cJSON.h>
//------------------------------------------------------------------------------
#define MAX_ACCESS_TOKEN_LEN  255
#define NEST_URL        "https://developer-api.nest.com/devices"
#define STR_KEEPALIVE   "event: keep-alive"
#define STR_REVOKED     "event: auth_revoked"
#define STR_PUT         "event: put"
#define STR_DATA        "data: "
//------------------------------------------------------------------------------
typedef struct
{
    thermo_data_h *tdh;
    smoke_data_h *sdh;
    auth_revoked_h *arh;
    struct curl_slist *header;
    CURL *easy;
    curl_socket_t fd;
    pthread_t thread;
    pthread_mutex_t mutex;
    char run;
    char reserved[3];

} nrt_t;
//------------------------------------------------------------------------------
static nrt_t *nrt = NULL;
static thermo_data_t thermo_store[MAX_DEVICE_FILTER];
static smoke_data_t smoke_store[MAX_DEVICE_FILTER];
//------------------------------------------------------------------------------
static smoke_data_t *remove_deleted_smoke(cJSON *devices)
{
    cJSON *dev;
    int i;
    smoke_data_t empty;

    memset(&empty, 0x0, sizeof(smoke_data_t));

    for(i = 0; i < MAX_DEVICE_FILTER; ++i)
    {
        if(!strlen(smoke_store[i].device_id))
            continue;

        dev = cJSON_GetObjectItem(devices, smoke_store[i].device_id);
        if(dev && dev->child)
            continue;

        nrt->sdh(smoke_store + i, &empty);
        memset(smoke_store + i, 0x0, sizeof(smoke_data_t));
    }
}
//------------------------------------------------------------------------------
static thermo_data_t *remove_deleted_thermo(cJSON *devices)
{
    cJSON *dev;
    int i;
    thermo_data_t empty;

    memset(&empty, 0x0, sizeof(thermo_data_t));

    for(i = 0; i < MAX_DEVICE_FILTER; ++i)
    {
        if(!strlen(thermo_store[i].device_id))
            continue;

        dev = cJSON_GetObjectItem(devices, thermo_store[i].device_id);
        if(dev && dev->child)
            continue;

        nrt->tdh(thermo_store + i, &empty);
        memset(thermo_store + i, 0x0, sizeof(thermo_data_t));
    }
}
//------------------------------------------------------------------------------
static smoke_data_t *find_smoke(const char *id)
{
    int i = 0;
    for(; i < MAX_DEVICE_FILTER; ++i)
    {
        if(0 == smoke_store[i].device_id[0])
            continue;

        if(0 == strncmp(id, smoke_store[i].device_id, DEVICE_ID_LEN))
            return (smoke_store + i);
    }

    return NULL;
}
//------------------------------------------------------------------------------
static thermo_data_t *find_thermo(const char *id)
{
    int i = 0;
    for(; i < MAX_DEVICE_FILTER; ++i)
    {
        if(0 == thermo_store[i].device_id[0])
            continue;

        if(0 == strncmp(id, thermo_store[i].device_id, DEVICE_ID_LEN))
            return (thermo_store + i);
    }

    return NULL;
}
//------------------------------------------------------------------------------
static double json_get_double(cJSON *obj, const char *name)
{
    cJSON *child = cJSON_GetObjectItem(obj, name);
    if(child)
    {
        if(cJSON_Number == child->type)
            return (child->valuedouble);
    }

    return 0.0f;
}
//------------------------------------------------------------------------------
static char json_get_boolean(cJSON *obj, const char *name)
{
    cJSON *child = cJSON_GetObjectItem(obj, name);
    if(child)
    {
        switch(child->type)
        {
            case cJSON_False:
                return 0;
            case cJSON_True:
                return 1;
            case cJSON_NULL:
                return 0;
            case cJSON_Number:
                return (child->valueint != 0);
            case cJSON_String:
                if(strcmp(child->string, "ok"))
                    return 1;
                if(strcmp(child->string, "on"))
                    return 1;
                if(strcmp(child->string, "yes"))
                    return 1;
                break;
            default:
                break;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
static void update_thermo_data(cJSON *dev, thermo_data_t *data)
{
    thermo_data_t new_data;
    memset(&new_data, 0x0, sizeof(thermo_data_t));
    strcpy(new_data.device_id, dev->string);
    strcpy(new_data.name, cJSON_GetStringItem(dev, "name"));
    new_data.is_online = json_get_boolean(dev, "is_online");

    char *str = cJSON_GetStringItem(dev, "hvac_mode");
    if(0 == strcmp(str, "heat-cool"))
        new_data.hvac_mode = HVAC_MODE_HEATCOOL;
    else if(0 == strcmp(str, "heat"))
        new_data.hvac_mode = HVAC_MODE_HEAT;
    else if(0 == strcmp(str, "cool"))
        new_data.hvac_mode = HVAC_MODE_COOL;
    else if(0 == strcmp(str, "off"))
        new_data.hvac_mode = HVAC_MODE_OFF;
    else if(0 == strcmp(str, "eco"))
        new_data.hvac_mode = HVAC_MODE_ECO;

    str = cJSON_GetStringItem(dev, "hvac_state");
    if(0 == strcmp(str, "off"))
        new_data.hvac_state = HVAC_STATE_OFF;
    else if(0 == strcmp(str, "heating"))
        new_data.hvac_state = HVAC_STATE_HEATING;
    else if(0 == strcmp(str, "cooling"))
        new_data.hvac_state = HVAC_STATE_COOLING;

    if(0 != memcmp(data, &new_data, sizeof(thermo_data_t)))
    {
        char first_update = (0 == data->name[0] ? 1 : 0);

        if(!first_update)
            nrt->tdh(data, &new_data);

        memcpy(data, &new_data, sizeof(thermo_data_t));
    }
}
//------------------------------------------------------------------------------
static void parse_thermo(cJSON *json)
{
    cJSON *devices = cJSON_GetObjectItem(json, "thermostats");
    if(devices && devices->child)
    {
        thermo_data_t *data = NULL;
        cJSON *dev = devices->child;

        pthread_mutex_lock(&nrt->mutex);
        remove_deleted_thermo(devices);
        while(dev)
        {
            data = find_thermo(dev->string);
            if(data)
            {
                update_thermo_data(dev, data);
            }
            dev = dev->next;
        }
        pthread_mutex_unlock(&nrt->mutex);
    }
}
//------------------------------------------------------------------------------
static void update_smoke_data(cJSON *dev, smoke_data_t *data)
{
    smoke_data_t new_data;
    memset(&new_data, 0x0, sizeof(smoke_data_t));
    strcpy(new_data.device_id, dev->string);
    strcpy(new_data.name, cJSON_GetStringItem(dev, "name"));
    new_data.is_online = json_get_boolean(dev, "is_online");
    new_data.is_manual_test_active = json_get_boolean(dev, "is_manual_test_active");
    new_data.battery_health = json_get_boolean(dev, "battery_health");

    char *str = cJSON_GetStringItem(dev, "co_alarm_state");
    if(0 == strcmp(str, "emergency"))
        new_data.co_alarm_state = SMOKE_STATE_EMERGENCY;
    else if(0 == strcmp(str, "warning"))
        new_data.co_alarm_state = SMOKE_STATE_WARNING;
    else
        new_data.co_alarm_state = SMOKE_STATE_OK;

    str = cJSON_GetStringItem(dev, "smoke_alarm_state");
    if(0 == strcmp(str, "emergency"))
        new_data.smoke_alarm_state = SMOKE_STATE_EMERGENCY;
    else if(0 == strcmp(str, "warning"))
        new_data.smoke_alarm_state = SMOKE_STATE_WARNING;
    else
        new_data.smoke_alarm_state = SMOKE_STATE_OK;

    str = cJSON_GetStringItem(dev, "ui_color_state");
    if(0 == strcmp(str, "red"))
        new_data.ui_color_state = SMOKE_STATE_EMERGENCY;
    else if(0 == strcmp(str, "yellow"))
        new_data.ui_color_state = SMOKE_STATE_WARNING;
    else
        new_data.ui_color_state = SMOKE_STATE_OK;

    if(0 != memcmp(data, &new_data, sizeof(smoke_data_t)))
    {
        char first_update = (0 == data->name[0] ? 1 : 0);
        
        if(!first_update)
            nrt->sdh(data, &new_data);

        memcpy(data, &new_data, sizeof(smoke_data_t));
    }
}
//------------------------------------------------------------------------------
static void parse_smoke(cJSON *json)
{
    cJSON *devices = cJSON_GetObjectItem(json, "smoke_co_alarms");
    if(devices && devices->child)
    {
        smoke_data_t *data = NULL;
        cJSON *dev = devices->child;

        pthread_mutex_lock(&nrt->mutex);
        remove_deleted_smoke(devices);
        while(dev)
        {
            data = find_smoke(dev->string);
            if(data)
            {
                update_smoke_data(dev, data);
            }
            dev = dev->next;
        }
        pthread_mutex_unlock(&nrt->mutex);
    }
}
//------------------------------------------------------------------------------
static int parse_json(const char *str)
{
    cJSON *obj = cJSON_Parse(str);
    if(!obj)
    {
        printf("cJSON_Parse failed | %s\n", str);
        return -1;
    }
    else
    {
        //printf("cJSON_Parse ok | %s\n", str);
    }

    cJSON *data = cJSON_GetObjectItem(obj, "data");
    if(data && data->child)
    {
        if(nrt->tdh)
            parse_thermo(data);

        if(nrt->sdh)
            parse_smoke(data);
    }

    cJSON_Delete(obj);
    return 0;
}
//------------------------------------------------------------------------------
static curl_socket_t socket_callback(void *p,
                                     curlsocktype t,
                                     struct curl_sockaddr *addr)
{
    nrt->fd = socket(addr->family, addr->socktype, addr->protocol);
    printf("%s | fd=%d\n", __func__, nrt->fd);
    return nrt->fd;
}
//------------------------------------------------------------------------------
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t sz = (size * nmemb);
    char *begin = ptr;
    char *end = begin + sz;
    int len = 0;

    while(*end <= 0x20)
    {
        *end = 0x0;
        --end;
    }

    len = strlen(STR_KEEPALIVE);
    if(0 == strncmp(begin, STR_KEEPALIVE, len))
    {
        printf("%s | [%s]\n", __func__, STR_KEEPALIVE);
        return (nrt->run ? sz : 0);
    }

    len = strlen(STR_PUT);
    if(0 == strncmp(begin, STR_PUT, len))
    {
        char *data = strstr(begin + len, STR_DATA);
        if(data)
        {
            parse_json(data + strlen(STR_DATA));
        }
    }

    len = strlen(STR_REVOKED);
    if(0 == strncmp(begin, STR_REVOKED, len))
    {
        printf("%s | [%s]\n", __func__, STR_REVOKED);
        if(nrt->arh)
            nrt->arh();
        return 0;
    }

    return (nrt->run ? sz : 0);
}
//------------------------------------------------------------------------------
static void send_nest_realtime_req(void)
{
    printf("%s\n", __func__);

    nrt->fd = -1;
    nrt->easy = curl_easy_init();
    curl_easy_setopt(nrt->easy, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(nrt->easy, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(nrt->easy, CURLOPT_OPENSOCKETFUNCTION, socket_callback);
    curl_easy_setopt(nrt->easy, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(nrt->easy, CURLOPT_URL, NEST_URL);
    curl_easy_setopt(nrt->easy, CURLOPT_HTTPHEADER, nrt->header);
#if 1
    long enable = 1;
    curl_easy_setopt(nrt->easy, CURLOPT_FOLLOWLOCATION, enable);
    curl_easy_perform(nrt->easy);
#else
    curl_easy_perform(nrt->easy);

    char *location = NULL;
    CURLcode res = curl_easy_getinfo(nrt->easy, CURLINFO_REDIRECT_URL, &location);
    if(CURLE_OK == res)
    {
        curl_easy_setopt(nrt->easy, CURLOPT_URL, location);
        curl_easy_perform(nrt->easy);
    }
#endif

    if(nrt->easy)
    {
        curl_easy_cleanup(nrt->easy);
        nrt->easy = NULL;
    }
}
//------------------------------------------------------------------------------
static void *thread_exec(void *arg)
{
    struct timeval tv;
    int tick = 0;

    while(nrt->run)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 10000; // 10 msec per tick
        select(0, NULL, NULL, NULL, &tv);
        ++tick;

        if(++tick > 500) // delay 5 seconds between two http requests
        {
            send_nest_realtime_req();
            tick = 0;
        }
    }

    return NULL;
}
//------------------------------------------------------------------------------
int  nrt_init(thermo_data_h *tdh, smoke_data_h *sdh, auth_revoked_h *arh)
{
    if(nrt)
        return EALREADY;

    memset(smoke_store, 0x0, sizeof(smoke_data_t) * MAX_DEVICE_FILTER);
    memset(thermo_store, 0x0, sizeof(thermo_data_t) * MAX_DEVICE_FILTER);

    nrt = (nrt_t *)malloc(sizeof(nrt_t));
    memset(nrt, 0x0, sizeof(nrt_t));
    pthread_mutex_init(&nrt->mutex, NULL);
    nrt->tdh = tdh;
    nrt->sdh = sdh;
    nrt->arh = arh;

    return 0;
}
//------------------------------------------------------------------------------
void nrt_destroy(void)
{
    if(!nrt)
        return;

    nrt_stop();
    pthread_mutex_destroy(&nrt->mutex);
    free(nrt);
    nrt = NULL;
}
//------------------------------------------------------------------------------
int  nrt_start(const char *access_token)
{
    if(!nrt)
        return EFAULT;

    int err = 0;
    int access_token_len = (access_token ? strlen(access_token) : 0);

    if(access_token_len < 1 ||
       access_token_len > MAX_ACCESS_TOKEN_LEN)
        return EINVAL;

    nrt_stop();

    char buf[320] = {0};
    sprintf(buf, "Authorization: Bearer %s", access_token);
    nrt->header = curl_slist_append(nrt->header, buf);
    nrt->header = curl_slist_append(nrt->header, "Accept: text/event-stream");

    nrt->run = 1;
    err = pthread_create(&nrt->thread, NULL, thread_exec, NULL);
    if(err)
    {
        nrt->run = 0;
        nrt->thread = 0;
        return errno;
    }

    return 0;
}
//------------------------------------------------------------------------------
void nrt_stop(void)
{
    if(!nrt)
        return;

    if(nrt->run)
        nrt->run = 0;

    if(nrt->fd > 0)
    {
        close(nrt->fd);
        nrt->fd = -1;
    }

    if(nrt->thread)
    {
        pthread_join(nrt->thread, NULL);
        nrt->thread = 0;
    }

    if(nrt->header)
    {
        curl_slist_free_all(nrt->header);
        nrt->header = NULL;
    }

    memset(smoke_store, 0x0, sizeof(smoke_data_t) * MAX_DEVICE_FILTER);
    memset(thermo_store, 0x0, sizeof(thermo_data_t) * MAX_DEVICE_FILTER);
}
//------------------------------------------------------------------------------
int  nrt_add_thermo_id(const char *device_id)
{
    if(!nrt)
        return EFAULT;

    thermo_data_t *data = NULL;
    int empty = -1;
    int err = 0;
    int i = 0;
    pthread_mutex_lock(&nrt->mutex);

    for(i = 0; i < MAX_DEVICE_FILTER; ++i)
    {
        data = (thermo_store + i);
        if(0 == strncmp(device_id, data->device_id, DEVICE_ID_LEN))
        {
            err = EEXIST;
            break;
        }

        if(0 == data->device_id[0])
            empty = i;
    }

    if(!err)
    {
        if(empty < 0)
        {
            err = EMFILE;
        }
        else
        {
            data = (thermo_store + empty);
            memset(data, 0x0, sizeof(thermo_data_t));
            strncpy(data->device_id, device_id, DEVICE_ID_LEN);
        }
    }

    pthread_mutex_unlock(&nrt->mutex);
    return err;
}
//------------------------------------------------------------------------------
void nrt_del_thermo_id(const char *device_id)
{
    if(!nrt)
        return;

    thermo_data_t *data = NULL;
    int i = 0;
    pthread_mutex_lock(&nrt->mutex);

    for(i = 0; i < MAX_DEVICE_FILTER; ++i)
    {
        data = (thermo_store + i);
        if(0 == strncmp(device_id, data->device_id, DEVICE_ID_LEN))
        {
            memset(data, 0x0, sizeof(thermo_data_t));
            break;
        }
    }

    pthread_mutex_unlock(&nrt->mutex);
}
//------------------------------------------------------------------------------
int  nrt_add_smoke_id(const char *device_id)
{
    if(!nrt)
        return EFAULT;

    smoke_data_t *data = NULL;
    int empty = -1;
    int err = 0;
    int i = 0;
    pthread_mutex_lock(&nrt->mutex);

    for(i = 0; i < MAX_DEVICE_FILTER; ++i)
    {
        data = (smoke_store + i);
        if(0 == strncmp(device_id, data->device_id, DEVICE_ID_LEN))
        {
            err = EEXIST;
            break;
        }

        if(0 == data->device_id[0])
            empty = i;
    }

    if(!err)
    {
        if(empty < 0)
        {
            err = EMFILE;
        }
        else
        {
            data = (smoke_store + empty);
            memset(data, 0x0, sizeof(smoke_data_t));
            strncpy(data->device_id, device_id, DEVICE_ID_LEN);
        }
    }

    pthread_mutex_unlock(&nrt->mutex);
    return err;
}
//------------------------------------------------------------------------------
void nrt_del_smoke_id(const char *device_id)
{
    if(!nrt)
        return;

    smoke_data_t *data = NULL;
    int i = 0;
    pthread_mutex_lock(&nrt->mutex);

    for(i = 0; i < MAX_DEVICE_FILTER; ++i)
    {
        data = (smoke_store + i);
        if(0 == strncmp(device_id, data->device_id, DEVICE_ID_LEN))
        {
            memset(data, 0x0, sizeof(smoke_data_t));
            break;
        }
    }

    pthread_mutex_unlock(&nrt->mutex);
}
//------------------------------------------------------------------------------
const char *nrt_smoke_state_str(SMOKE_STATE_ENUM val)
{
    static char *smoke_state_str[SMOKE_STATE_LAST] = {
        "ok",
        "warning",
        "emergency"
    };

    if(val < SMOKE_STATE_LAST)
        return smoke_state_str[val];

    return "";
}
//------------------------------------------------------------------------------
