#ifndef NEST_RAELTIME_H
#define NEST_RAELTIME_H
//------------------------------------------------------------------------------
#define DEVICE_ID_LEN     32
#define DEVICE_NAME_LEN   32
#define MAX_DEVICE_FILTER 6
//------------------------------------------------------------------------------
typedef enum
{
    HVAC_MODE_HEATCOOL = 0,
    HVAC_MODE_HEAT,
    HVAC_MODE_COOL,
    HVAC_MODE_OFF,
    HVAC_MODE_ECO,
    HVAC_MODE_LAST

} HVAC_MODE_ENUM;
//------------------------------------------------------------------------------
typedef enum
{
    HVAC_STATE_OFF = 0,
    HVAC_STATE_HEATING,
    HVAC_STATE_COOLING,
    HVAC_STATE_LAST

} HVAC_STATE_ENUM;
//------------------------------------------------------------------------------
typedef struct
{
    char device_id[DEVICE_ID_LEN];
    char reserved1[4]; // string null terminate for device_id
    char name[DEVICE_NAME_LEN];
    char reserved2[4]; // string null terminate for name

    char is_online;
    char hvac_mode;  // HVAC_MODE_ENUM
    char hvac_state; // HVAC_STATE_ENUM
    char reserved3[2];

} thermo_data_t;
//------------------------------------------------------------------------------
typedef enum
{
    SMOKE_STATE_OK = 0,
    SMOKE_STATE_WARNING,
    SMOKE_STATE_EMERGENCY,
    SMOKE_STATE_LAST
} SMOKE_STATE_ENUM;
//------------------------------------------------------------------------------
typedef struct
{
    char device_id[DEVICE_ID_LEN];
    char reserved1[4];
    char name[DEVICE_NAME_LEN];
    char reserved2[4];

    char is_online;
    char is_manual_test_active;
    char battery_health;        // ok=1, replace=0
    char co_alarm_state;        // SMOKE_STATE_ENUM
    char smoke_alarm_state;     // SMOKE_STATE_ENUM
    char ui_color_state;        // SMOKE_STATE_ENUM: 0/1/2 (green/yellow/red)
    char reserved3[2];
} smoke_data_t;
//------------------------------------------------------------------------------
typedef void (thermo_data_h)(thermo_data_t *before, thermo_data_t *after);
typedef void (smoke_data_h)(smoke_data_t *before, smoke_data_t *after);
typedef void (auth_revoked_h)(void);
//------------------------------------------------------------------------------
int  nrt_init(thermo_data_h *tdh, smoke_data_h *sdh, auth_revoked_h *arh);
void nrt_destroy(void);
int  nrt_start(const char *access_token);
void nrt_stop(void);
//------------------------------------------------------------------------------
int  nrt_add_thermo_id(const char *device_id);
void nrt_del_thermo_id(const char *device_id);
int  nrt_add_smoke_id(const char *device_id);
void nrt_del_smoke_id(const char *device_id);
//------------------------------------------------------------------------------
const char *nrt_smoke_state_str(SMOKE_STATE_ENUM val);
//------------------------------------------------------------------------------
#endif
