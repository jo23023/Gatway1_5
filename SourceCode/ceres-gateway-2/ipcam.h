#ifndef _IPCHDR
#define _IPCHDR

#include "PPPP/PPPP_Type.h"
#define  CHANNEL_IOCTRL 0

typedef enum {
  SIO_TYPE_UNKN = 0,
  SIO_TYPE_AUTH,
  SIO_TYPE_VIDEO,
  SIO_TYPE_AUDIO,
  SIO_TYPE_IOCTRL,
  SIO_TYPE_FILE,

} ENUM_STREAM_IO_TYPE;

typedef enum {
  CODECID_UNKN,
  CODECID_V_MJPEG,
  CODECID_V_MPEG4,
  CODECID_V_H264,

  CODECID_A_PCM =0x4FF,
  CODECID_A_ADPCM,
  CODECID_A_SPEEX,
  CODECID_A_AMR,
  CODECID_A_AAC
} ENUM_CODECID;

typedef enum
{
  VFRAME_FLAG_I	= 0x00,	// Video I Frame
  VFRAME_FLAG_P	= 0x01,	// Video P Frame
  VFRAME_FLAG_B	= 0x02	// Video B Frame
} ENUM_VFRAME;

typedef enum
{
  ASAMPLE_RATE_8K	= 0x00,
  ASAMPLE_RATE_11K= 0x01,
  ASAMPLE_RATE_12K= 0x02,
  ASAMPLE_RATE_16K= 0x03,
  ASAMPLE_RATE_22K= 0x04,
  ASAMPLE_RATE_24K= 0x05,
  ASAMPLE_RATE_32K= 0x06,
  ASAMPLE_RATE_44K= 0x07,
  ASAMPLE_RATE_48K= 0x08
} ENUM_AUDIO_SAMPLERATE;

typedef enum
{
  ADATABITS_8   = 0,
  ADATABITS_16	= 1
} ENUM_AUDIO_DATABITS;

typedef enum
{
  ACHANNEL_MONO	= 0,
  ACHANNEL_STERO= 1
} ENUM_AUDIO_CHANNEL;

/*typedef enum {
	IOCTRL_TYPE_UNKN,

	IOCTRL_TYPE_VIDEO_START,
	IOCTRL_TYPE_VIDEO_STOP,
	IOCTRL_TYPE_AUDIO_START,
	IOCTRL_TYPE_AUDIO_STOP

}ENUM_IOCTRL_TYPE;*/

// IOCTRL PTZ Command Value
typedef enum
{
    IOCTRL_PTZ_STOP,
    IOCTRL_PTZ_UP,
    IOCTRL_PTZ_DOWN,
    IOCTRL_PTZ_LEFT,
    IOCTRL_PTZ_RIGHT,
    IOCTRL_PTZ_LEFT_UP,
    IOCTRL_PTZ_LEFT_DOWN,
    IOCTRL_PTZ_RIGHT_UP,
    IOCTRL_PTZ_RIGHT_DOWN,

    IOCTRL_LENS_ZOOM_IN,
    IOCTRL_LENS_ZOOM_OUT,

    IOCTRL_PTZ_AUTO,
    IOCTRL_PTZ_SET_PRESET_POINT,
    IOCTRL_PTZ_PRESET_POINT,
}ENUM_PTZCMD;



//NOTE: struct below is all Little Endian
typedef struct {
  union {
    struct {
      UCHAR  nDataSize[3];
      UCHAR  nStreamIOType; //refer to ENUM_STREAM_IO_TYPE
    }uionStreamIOHead;
    UINT32 nStreamIOHead;
  };
}st_AVStreamIOHead;

//for Video and Audio
typedef struct {
  UINT16 nCodecID;	// refer to ENUM_CODECID
  UCHAR  nOnlineNum;
  UCHAR  flag;		// Video:=ENUM_VFRAME; Audio:=(ENUM_AUDIO_SAMPLERATE << 2) | (ENUM_AUDIO_DATABITS << 1) | (ENUM_AUDIO_CHANNEL)
  UCHAR  XorKey;
  UCHAR  codecPara1;  // For ADPCM, codecParam1 = nAudioIndex, codecParam2 = nAudioPreSample.
  UINT16 codecPara2;

  UINT32 nDataSize;
  UINT32 nTimeStamp;	//system tick
} st_AVFrameHead;

//for IO Control
typedef struct {
  unsigned short nIOCtrlType;//refer to ENUM_IOCTRL_TYPE
  unsigned short nIOCtrlDataSize;
  unsigned char  XorKey;
  unsigned char  reserve[10];
} st_AVIOCtrlHead;

typedef struct {
  short AuthType;
  short nAuthDataSize;
  unsigned char reserve[12];
} st_AuthHead;

typedef struct {
  unsigned char cam_index;
  unsigned char bit_field;
  unsigned char quality;
  unsigned char orientation;
  unsigned char environment;
  unsigned char reserve[3];
} IOCTRLGetVideoParameterResp;

/*
IOCTRL_TYPE_PTZ_COMMAND// P2P Ptz Command Msg
** @struct IOCTRLPtzCmd
*/
typedef struct
{
    unsigned char channel; //camera index
    unsigned char control; // ptz control command, refer to ENUM_PTZCMD
    unsigned char step;  // ptz control step
    unsigned char reserve[5];
} IOCTRLPtzCmd;

///


enum AUTH_TYPE
{
  AUTH_TYPE_UNKN = 0,
  AUTH_TYPE_REQ = 0x1,
  AUTH_TYPE_RESP,
  AUTH_TYPE_OK,
  AUTH_TYPE_FAILED
};

enum IOCTL_TYPE
{
  IOCTRL_TYPE_PUSH_CamIndex,
  IOCTRL_TYPE_VIDEO_START,
  IOCTRL_TYPE_VIDEO_STOP,
  IOCTRL_TYPE_AUDIO_START,
  IOCTRL_TYPE_AUDIO_STOP,

  // --- special -------------------
  IOCTRL_TYPE_DEVINFO_REQ,
  IOCTRL_TYPE_DEVINFO_RESP,
  IOCTRL_TYPE_RECORD_PLAYCONTROL_REQ,
  IOCTRL_TYPE_RECORD_PLAYCONTROL_RESP,
  IOCTRL_TYPE_PTZ_COMMAND,
  IOCTRL_TYPE_LISTEVENT_REQ,
  IOCTRL_TYPE_LISTEVENT_RESP,
  IOCTRL_TYPE_EVENT_NOTIFY,

  IOCTRL_TYPE_EMAIL_ON_OFF_REQ,         // Alarm Email enable / disable
  IOCTRL_TYPE_EMAIL_ON_OFF_RESP,

  IOCTRL_TYPE_EVENT_NOTIFY_ON_OFF_REQ,  // Device Event Notify enable / disable
  IOCTRL_TYPE_EVENT_NOTIFY_ON_OFF_RESP,

  IOCTRL_TYPE_GET_ON_OFF_VALUE_REQ,
  IOCTRL_TYPE_GET_ON_OFF_VALUE_RESP,

  IOCTRL_TYPE_SPEAKER_START,
  IOCTRL_TYPE_SPEAKER_STOP,

  IOCTRL_TYPE_SETPASSWORD_REQ,
  IOCTRL_TYPE_SETPASSWORD_RESP,

  IOCTRL_TYPE_SET_VIDEO_PARAMETER_REQ,
  IOCTRL_TYPE_SET_VIDEO_PARAMETER_RESP,
  IOCTRL_TYPE_GET_VIDEO_PARAMETER_REQ,
  IOCTRL_TYPE_GET_VIDEO_PARAMETER_RESP,

  IOCTRL_TYPE_LISTWIFIAP_REQ,
  IOCTRL_TYPE_LISTWIFIAP_RESP,
  IOCTRL_TYPE_SETWIFI_REQ,
  IOCTRL_TYPE_SETWIFI_RESP,

  IOCTRL_TYPE_SETMOTIONDETECT_REQ,
  IOCTRL_TYPE_SETMOTIONDETECT_RESP,
  IOCTRL_TYPE_GETMOTIONDETECT_REQ,
  IOCTRL_TYPE_GETMOTIONDETECT_RESP,

  IOCTRL_TYPE_SETRECORD_REQ,            // no use
  IOCTRL_TYPE_SETRECORD_RESP,           // no use
  IOCTRL_TYPE_GETRECORD_REQ,            // no use
  IOCTRL_TYPE_GETRECORD_RESP,           // no use

  IOCTRL_TYPE_FORMATEXTSTORAGE_REQ,     // Format external storage
  IOCTRL_TYPE_FORMATEXTSTORAGE_RESP,

  IOCTRL_TYPE_MANU_REC_START,           // start manual recording
  IOCTRL_TYPE_MANU_REC_STOP,            // stop manual recording

  IOCTRL_TYPE_SET_EMAIL_REQ,            // set alarm Email settings
  IOCTRL_TYPE_SET_EMAIL_RESP = 0x2C,
  IOCTRL_TYPE_GET_EMAIL_REQ,            // get alarm Email settings
  IOCTRL_TYPE_GET_EMAIL_RESP,

  IOCTRL_TYPE_AUTH_ADMIN_PASSWORD_REQ,  // authenticate admin password
  IOCTRL_TYPE_AUTH_ADMIN_PASSWORD_RESP,
  IOCTRL_TYPE_SET_ADMIN_PASSWORD_REQ,   // set admin password
  IOCTRL_TYPE_SET_ADMIN_PASSWORD_RESP,

  IOCTRL_TYPE_GETWIFI_REQ,
  IOCTRL_TYPE_GETWIFI_RESP,

  IOCTRL_TYPE_PUSH_APP_UTC_TIME,

  IOCTRL_TYPE_SET_TIMEZONE_REQ,
  IOCTRL_TYPE_SET_TIMEZONE_RESP,
  IOCTRL_TYPE_GET_TIMEZONE_REQ,
  IOCTRL_TYPE_GET_TIMEZONE_RESP,

  IOCTRL_TYPE_AUTO_DEL_REC_ON_OFF_REQ,

  IOCTRL_TYPE_SETDETECTMODE_REQ,
  IOCTRL_TYPE_SETDETECTMODE_RESP,
  IOCTRL_TYPE_GETDETECTMODE_REQ,
  IOCTRL_TYPE_GETDETECTMODE_RESP,

  /***************** Extra Control Cmd defined for Onet *******************/
  IOCTRL_TYPE_GET_ONET_DEVINFO_REQ,
  IOCTRL_TYPE_GET_ONET_DEVINFO_RESP,
  IOCTRL_TYPE_SET_ONET_DEVINFO_REQ,
  IOCTRL_TYPE_SET_ONET_DEVINFO_RESP,
  IOCTRL_TYPE_SET_ONET_STATUS_REQ,
  IOCTRL_TYPE_SET_ONET_STATUS_RESP,

  IOCTRL_TYPE_REMOVE_EVENTLIST_REQ,     // Remove event list
  IOCTRL_TYPE_REMOVE_EVENTLIST_RESP,
  IOCTRL_TYPE_REMOVE_EVENT_REQ,         // Remove one event
  IOCTRL_TYPE_REMOVE_EVENT_RESP,

  IOCTRL_TYPE_UPGRADE_FIRMWARE_REQ,
  IOCTRL_TYPE_UPGRADE_FIRMWARE_RESP = 0x6F,
  IOCTRL_TYPE_SET_RVDP_GATE_DOOR_REQ = 0x8E,
  IOCTRL_TYPE_SET_RVDP_GATE_DOOR_RESP = 0x8F
};

//for ipcam
enum
{
    REC_BY_MANUALLY = 0x00,
    REC_BY_PIR = 0x01,
    REC_BY_PIR_WITH_SOFTWARE = 0x02, //don't use
    REC_BY_SOFTWARE = 0x03,
    REC_BY_VOICE = 0x04,
    REC_BY_VOICE_PIR = 0x05, //don't use
    REC_BY_SOFTWAR_VOICE_TEMP_HUMIDITY = 0x06,   //don't use
    REC_BY_TEMP_HUMDITY = 0x07,
    REC_BY_TEMP_HUMIDITY_VOICE = 0x08,  //don't use
    REC_BY_HUMIDITY  = 0x09,
    REC_BY_TEMP = 0x0A,

    REC_BUT_SD_FULL = 0x0B,
    REC_BUT_SD_EMPTY = 0x0C,
    REC_BUT_SD_BAD = 0x0D,
    REC_BY_DOORKEY = 0x0E,
    EVENT_ARM = 0x0F,
    EVENT_DISARM=0x10,
    REC_BY_EXT_SENSOR = 0x20,
    // REC event reserve to 0x3F
    DOORKEY_ANSWERING = 0x40,
    // SHC local push event from 0x80
    SHC_DISARM = 0x80,
    SHC_ARM =0x81,
	SHC_INEVITABILITY =0x82,
};

typedef struct
{
    UINT16 nYear;
    UCHAR nMonth;
    UCHAR nDay;
    UCHAR nWday;
    UCHAR nHour;
    UCHAR nMinute;
    UCHAR nSecond;
} st_EvnetResDate;

typedef struct
{
    UCHAR n_CAmID[4];
    INT32 evtType;
    st_EvnetResDate UTC_time;
    INT32 UTC_sec;
    UCHAR sensor_type;
    UCHAR reserve[3];
} event_notify_data;

typedef struct
{
    UCHAR n_CAmID[4];
    INT32 evtType;
    st_EvnetResDate UTC_time;
    INT32 UTC_sec;
    INT32 RF_TAG_ID;
} event_notify_data_2SHC;

#endif
