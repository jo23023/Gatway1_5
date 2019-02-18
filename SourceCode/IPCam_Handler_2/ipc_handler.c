#include "ipc_handler.h"

#define WARN_MSG(_STR_, args...) printf("<%s> "_STR_, __FILE__, ##args);

void printAllIPCams()
{
	LOG_DEBUG(g_log, "printAllIPCams. ");

	int i = 0;
	st_camera_info *ipc;

	pthread_mutex_lock(&ipc_mutex);
	for (i = 0; i < CAM_MAX_NUMBER; i++)
	{
		ipc = gAllIPCams[i];
		if (!ipc) continue;

		LOG_DEBUG(g_log, "index = %d", i);
		LOG_DEBUG(g_log, "	valid = %d", ipc->valid);
		LOG_DEBUG(g_log, "	did = %s", ipc->did);
		LOG_DEBUG(g_log, "	passwd = %s", ipc->passwd);
		LOG_DEBUG(g_log, "	hCamera = 0x%x", ipc->hCamera);
	}
	pthread_mutex_unlock(&ipc_mutex);
}

void stopIPCam(const char *did)
{
	LOG_WARN(g_log, "ipc_handler stopIPCam. did = %s", did);

	int i = 0;
	pthread_mutex_lock(&ipc_mutex);
	for (i = 0; i < CAM_MAX_NUMBER; i++)
	{
		if (gAllIPCams[i]->valid)
		{
			if (0 == strcmp(did, gAllIPCams[i]->did))
				break;
		}
	}

	if (i < CAM_MAX_NUMBER)
	{
		st_camera_info *ipc = gAllIPCams[i];
		ipc->running = 0;
		if (NULL == ipc->hCamera)
		{
			LOG_WARN(g_log, "	Camera is null\n");
		}
		else
			Camera_Disconnect(ipc->hCamera);
	}
	else
	{
		LOG_WARN(g_log, "	Can not find this camera");
	}

	pthread_mutex_unlock(&ipc_mutex);
}



void on_alarm(char *cam_did, uint8_t *buf, uint32_t size, int result)
{
	LOG_WARN(g_log, "ipc_handler - on_alarm - cam_did = %s", cam_did);

	// msg: did;camIndex;eventType;eventTime
	// msg = CGXX-000200-NYBDZ;0;1;1413860907
	const char *format = "{\"callid\":123, \"method\":\"updateitemtrigger\",\"param\":{\"action\":\"OpenCamera\", \"value\":\"detect\",\"bind\":\"%s\"}}";

	// "bind":"p2p=CGXX-000200-NYBDZ"

/*	char did[32] = { 0 };
	char *p = buf;
	while (p == ';')break;
	strncpy(did, (unsigned char *)buf, p-buf);*/

	char bind[32] = { 0 };
	sprintf(bind, "%s%s", "p2p=", cam_did);

	char message[256] = { 0 };
	sprintf(message, format, bind);

	LOG_DEBUG(g_log, "	message = %s", message);

	if (connection)
	{
		LOG_WARN(g_log, "	ipc_handler send MQTT message to ceres: %s", message);
		mosquitto_publish(connection, 0, TOPIC_IPC_HANDLER_MOTION, strlen(message), message, 2, 0);
	}
}

void on_connect(char *cam_did, uint8_t *buf, uint32_t size, int result)
{
  switch (result) {
    case CONN_INFO_SUCCESSED:
      LOG_DEBUG(g_log, "ipc_handler %s : Connect Successed", cam_did);
      break;
    case CONN_INFO_FAILED:
      LOG_DEBUG(g_log, "ipc_handler %s : Connect Faild", cam_did);
      break;
    case CONN_INFO_AUTHOK:
      LOG_DEBUG(g_log, "ipc_handler %s : Auth OK", cam_did);
      break;
    case CONN_INFO_AUTHFAILED:
      LOG_DEBUG(g_log, "ipc_handler %s : Auth Failed", cam_did);
      break;
    case CONN_INFO_RUNNING:
      break;
    case  CONN_INFO_CLOSED:
        LOG_DEBUG(g_log, "ipc_handler %s : Close Camera", cam_did);
        break;
	}
}

void *pthread_record_routine(void *ptr)
{
	int ret;

	st_camera_info *ipc = (st_camera_info *)ptr;
	Camera *hCamera = ipc->hCamera;
	LOG_WARN(g_log, "pthread_record_routine: %s start", ipc->did);

	while (ipc->running)
	{
		LOG_DEBUG(g_log, "need send record: %d , move to preset: %d, IsAuthOk: %d ", ipc->sendRecord, ipc->toPresetPoint, Camera_IsAuthOk(hCamera));
		if (ipc->sendRecord)
		{
			if (Camera_IsAuthOk(hCamera))
			{
				LOG_WARN(g_log, "ipc_handler: %s send record command", ipc->did);
				Camera_ManuRecStart(hCamera);
				ipc->sendRecord = 0;
			}
			else{
				LOG_WARN(g_log, "ipc_handler: %s need command but AuthOK failed", ipc->did);
			}
		}

		if (ipc->toPresetPoint != -1)
		{
			Camera_MoveToPresetPoint(hCamera, ipc->toPresetPoint);
			ipc->toPresetPoint = -1;
		}

		sleep(2);
	}

	LOG_WARN(g_log, "pthread_record_routine stop");
}

void *pthread_connect_routine(void *ptr)
{
	int ret;

	st_camera_info *ipc = (st_camera_info *)ptr;
	Camera *hCamera;
	LOG_WARN(g_log, "pthread_connect_routine: %s start", ipc->did);

	if (ipc->valid == 0 || ipc->running == 0)
		return NULL;

	// Camera_Connect 中,会为 hCamera 分配地址
	ret = Camera_Connect(&hCamera, ipc->did,  ipc->passwd, 0, true);

	// 记录分配的 hCamera 地址
	ipc->hCamera = hCamera;

	Camera_Connect_CallBack_set(hCamera, on_connect);
	Camera_Alarm_CallBack_set(hCamera, on_alarm);

	pthread_create(&ipc->pthread_record, NULL, &pthread_record_routine, (void *)ipc);

	// loop 循环
	Camera_Detection(hCamera);

	pthread_mutex_lock(&ipc_mutex);
	memset(ipc, 0, sizeof(st_camera_info));
	pthread_mutex_unlock(&ipc_mutex);

	LOG_WARN(g_log, "pthread_connect_routine stop");
	return NULL;
}

int ipcExist(const char *did)
{
	int index = -1;

	int i = 0;
	st_camera_info *ipc;
	pthread_mutex_lock(&ipc_mutex);

	for (i = 0; i < CAM_MAX_NUMBER; i++)
	{
		ipc = gAllIPCams[i];
		if (1 == ipc->valid && 0 == strcmp(did, ipc->did))
		{
			index = i;
			break;
		}
	}

	pthread_mutex_unlock(&ipc_mutex);

	return index;
}

void ipcAdd(const char *payload, int len)
{
	LOG_WARN(g_log, "ipc_handler: ipcAdd payload = %s ", payload);

	int i = 0;
	char did[32] = { 0 };
	char pw[32] = { 0 };
	char preset_point[32] = { 0 };

	sscanf(payload, "%[^;]", did);
	sscanf(payload + strlen(did) + 1, "%[^;]", pw);
	sscanf(payload + strlen(did) + strlen(pw) + 2, "%[^.]", preset_point);
	printf("[%s:%d] did=%s pw=%s preset_point=%s\n", __FUNCTION__, __LINE__, did, pw, preset_point);

	int index = ipcExist(did);
	LOG_DEBUG(g_log, "	Exist index = %d ", index);
	if (-1 != index)
		return;

	pthread_mutex_lock(&ipc_mutex);

	// 判断是否有空间记录此IPC
	for (i = 0; i < CAM_MAX_NUMBER; i++)
	{
		if (0 == gAllIPCams[i]->valid)
			break;
	}

	LOG_DEBUG(g_log, "i = %d", i);
	if (CAM_MAX_NUMBER == i)
	{
		LOG_WARN(g_log, "	no room for this IPCamera");
	}
	else
	{
		LOG_WARN(g_log, "	add this IPCamera to index: %d", i);

		st_camera_info *ipc = gAllIPCams[i];
		ipc->valid = 1;
		strncpy(ipc->did, did, strlen(did));
		strncpy(ipc->passwd, pw, strlen(pw));

		ipc->running = 1;

		LOG_WARN(g_log, "	create IPCamera connect thread");
		pthread_create(&ipc->pthread_connect, NULL, &pthread_connect_routine, (void *)ipc);
	}

	pthread_mutex_unlock(&ipc_mutex);
}

void ipcRemove(const char *payload, int len)
{
	LOG_WARN(g_log, "ipc_handler: ipcRemove payload = %s ", payload);
	// 获取DID， PASSWD
	char did[32] = { 0 };
	char pw[32] = { 0 };

	sscanf(payload, "%[^;]", did);
	sscanf(payload + strlen(did) + 1, "%[^.]", pw);

	stopIPCam(did);
}

void ipcRecord(const char *payload, int len)
{
	LOG_WARN(g_log, "ipc_handler: ipcRecord payload = %s ", payload);

	// 获取DID， PASSWD
	char did[32] = { 0 };
	char pw[32] = { 0 };
	char preset_point[32] = { 0 };

	sscanf(payload, "%[^;]", did);
	sscanf(payload + strlen(did) + 1, "%[^;]", pw);
	sscanf(payload + strlen(did) + strlen(pw) + 2, "%[^.]", preset_point);

	int index = ipcExist(did);
	if (-1 != index)	// exist
	{
		LOG_WARN(g_log, "	IPCam exist.  index = %d ", index);
		pthread_mutex_lock(&ipc_mutex);
		st_camera_info *ipc = gAllIPCams[index];
		ipc->sendRecord = 1;

		if (preset_point != NULL)
		{
			ipc->toPresetPoint = atoi(preset_point);
		}
		
		pthread_mutex_unlock(&ipc_mutex);
	}
	else	// not exist
	{
		LOG_DEBUG(g_log, "	IPCam not-exist. call ipcAdd");
		ipcAdd(payload, len);
		index = ipcExist(did);
		if (-1 == index)
		{
			LOG_WARN(g_log, "	ipcAdd error!");
		}
		else{
			LOG_WARN(g_log, "	ipcAdd ok! set flag: sendRecrod");

			pthread_mutex_lock(&ipc_mutex);

			st_camera_info *ipc = gAllIPCams[index];
			ipc->sendRecord = 1;

			if (preset_point != NULL)
			{
				ipc->toPresetPoint = atoi(preset_point);
			}

			pthread_mutex_unlock(&ipc_mutex);
		}
	}
}

void ipcRecordStop(const char *payload, int len)
{
	LOG_WARN(g_log, "ipc_handler: ipcRecordStop payload = %s ", payload);

	// TODO
}

void connect_callback(struct mosquitto *mosq, void *obj, int rc) {
	if (rc == 0)
	{
		mMQTTConnected = 1;
		LOG_WARN(g_log, "ipc_handler: connect to mqtt bus, succeed!");

		mosquitto_subscribe(connection, NULL, TOPIC_IPC_HANDLER_ADD, 2);
		mosquitto_subscribe(connection, NULL, TOPIC_IPC_HANDLER_REMOVE, 2);
		mosquitto_subscribe(connection, NULL, TOPIC_IPC_HANDLER_RECORD, 2);
		mosquitto_subscribe(connection, NULL, TOPIC_IPC_HANDLER_RECORD_STOP, 2);
	}
	else
	{
		LOG_WARN(g_log, "ipc_handler: connect to mqtt bus, failed!");
	}
}

void disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	LOG_WARN(g_log, "ip_handler: disconnect_callback. rc = %d", rc);
}

void message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message) {
	LOG_WARN(g_log, "ipc_handler: received MQTT messate: topic = %s, payload = %s",
			message->topic, message->payload);

	if (strcmp(message->topic, TOPIC_IPC_HANDLER_ADD)== 0)
	{
		ipcAdd(message->payload, message->payloadlen);
	}
	else if (strcmp(message->topic, TOPIC_IPC_HANDLER_REMOVE)== 0)
	{
		ipcRemove(message->payload, message->payloadlen);
	}
	else if (strcmp(message->topic, TOPIC_IPC_HANDLER_RECORD)== 0)
	{
		ipcRecord(message->payload, message->payloadlen);
	}
	else if (strcmp(message->topic, TOPIC_IPC_HANDLER_RECORD_STOP)== 0)
	{
		ipcRecordStop(message->payload, message->payloadlen);
	}
}

// ----------------------- MQTT --------------------------
void* mqtt_client_thread_routine(void *ptr) {
	int rc = 0;
	int nFailCount = 0;

	mosquitto_lib_init();

	connection = mosquitto_new(CLIENT_NAME, true, NULL);
	if (connection) {
		mosquitto_connect_callback_set(connection, connect_callback);
//		mosquitto_disconnect_callback_set(connection, disconnect_callback);
		mosquitto_message_callback_set(connection, message_callback);

		rc = mosquitto_connect(connection, MQTT_HOST, MQTT_PORT, 60);

		while (run) {
			if (mosquitto_loop(connection, -1, 1) != MOSQ_ERR_SUCCESS)	/* 20150225_victorwu: fix mqtt crash issue */
			{	
				if (run)
				{
					if (nFailCount > 5)	
					{
						WARN_MSG("%s is disconnected with mqtt, exit and restart ceres process......\n", CLIENT_NAME);
						sleep(2);
						system("killall -9 ipc_handler&");
					}
					else
					{
						WARN_MSG("%s is disconnected with mqtt, reconnect mqtt......\n", CLIENT_NAME);
						sleep(2);
						nFailCount++;
						mosquitto_reconnect(connection);
					}
				}
			}
		}
		mosquitto_disconnect(connection);
		mosquitto_destroy(connection);
	}
	mosquitto_lib_cleanup();

	return NULL;
}

void testIPCamera()
{
	char *DID = "CGXX-000257-ZVMUW";
	char *passwd = "1111";

	char *testIPC1 = "CGXX-000257-ZVMUW;1234";
	char *testIPC2 = "CGXX-000200-NYBDZ;1111";
	char *testIPC3 = "AHXX-000099-EMMMN;q";
	char *testIPC4 = "CGXX-000171-REYWK;0000";

//	ipcRecord(testIPC1, strlen(testIPC1));
	ipcRecord(testIPC1, strlen(testIPC1));
//	ipcRecord(testIPC2, strlen(testIPC2));
}

int main(int argc, char *argv[]) {
	int i;
	int ret;

	LOG_WARN(g_log, "ipc_handler is starting...");

	// Save pid itself
	{
		char sCmd[128] = "";

		snprintf(sCmd, sizeof(sCmd), "echo %d > /var/run/ipc_handler.pid", getpid());
		system(sCmd);
	}

	// 打印P2P版本信息
//	unsigned int APIVersion = STARX_GetAPIVersion();
//	LOG_DEBUG(g_log, "ipc_handler P2P Version: %d.%d.%d.%d", (APIVersion & 0xFF000000) >> 24,
//				(APIVersion & 0x00FF0000) >> 16, (APIVersion & 0x0000FF00) >> 8,
//				(APIVersion & 0x000000FF) >> 0);

	// 初始化全局数据结构
	pthread_mutex_init(&ipc_mutex, NULL);

	gAllIPCams = (unsigned char **) malloc(CAM_MAX_NUMBER * sizeof(unsigned char *));
	for (i = 0; i < CAM_MAX_NUMBER; i++) {
		gAllIPCams[i] = (unsigned char *) malloc(sizeof(st_camera_info));
		memset(gAllIPCams[i], 0, sizeof(st_camera_info));
	}

	// MQTT线程
	pthread_create(&mqtt_sub_pid, NULL, mqtt_client_thread_routine, NULL);

	AES_Init();

	while (1)
	{
		ret = Camera_Init(pszParam);

		if (!ret)
		{
			LOG_WARN(g_log, "Camera_Init OK");
			break;
		}

		LOG_WARN(g_log, "Camera_Init Failed. sleep 3s");
		sleep(3);
	}

//	testIPCamera();

	while (1)
	{
		if (0 == mMQTTConnected)
		{
			LOG_WARN(g_log, "ipc_handler: waiting for mqtt connect");
			sleep(3);
		}
		else
			break;
	}

	// 主循环
	for (;;)
	{
		sleep(3);
	}

	AES_Deinit();
	Camera_Deinit();

	pthread_join(mqtt_sub_pid, NULL);

	if (NULL != gAllIPCams) {
		for (i = 0; i < CAM_MAX_NUMBER; i++) {
			if (NULL != gAllIPCams[i])
				free((void *)gAllIPCams[i]);
		}
		free((void *)gAllIPCams);
	}

	LOG_WARN(g_log, "ipc_handler exit application");
	return 0;
}
