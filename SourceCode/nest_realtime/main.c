#include "nest_realtime.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
//------------------------------------------------------------------------------
static void thermo_data_handler(thermo_data_t *before, thermo_data_t *after)
{
    printf("%s | [%s] | %s\n", __func__, before->device_id,
        after->device_id[0] ? "changed" : "deleted");
}
//------------------------------------------------------------------------------
static void smoke_data_handler(smoke_data_t *before, smoke_data_t *after)
{
    printf("%s | [%s] | %s\n", __func__, before->device_id,
        after->device_id[0] ? "changed" : "deleted");
}
//------------------------------------------------------------------------------
static void auth_revoked_handler(void)
{
    printf("%s\n", __func__);
}
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    printf("sizeof(float)=%d\n", sizeof(float));
    printf("sizeof(double)=%d\n", sizeof(double));

    char *access_token = NULL;
    int i = 0;

    if(argc < 2)
        return EINVAL;

    access_token = argv[1];

    nrt_init(thermo_data_handler, smoke_data_handler, auth_revoked_handler);
    nrt_add_thermo_id("acdbTaCI3GiG0Z0BjYf2FLn_6kUwZs0M");
    nrt_add_thermo_id("acdbTaCI3GhLkl-bdUg3KLn_6kUwZs0M");
    nrt_add_smoke_id("uNI9Pt4XDzPy5nCa9PnAo7n_6kUwZs0M");
    nrt_add_smoke_id("uNI9Pt4XDzMmGDM4Yfqn47n_6kUwZs0M");
    nrt_start(access_token);

    while(++i < 6000)
    {
        sleep(1);
    }

    nrt_destroy();
    return 0;
}
//------------------------------------------------------------------------------
