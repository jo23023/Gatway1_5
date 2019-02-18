#include "ceres_util.h"

#ifdef DEBUG
	#define DBG_PRINT(_STR_, args...) printf("[ Ceres_util ] "_STR_, ##args);
#else
	#define DBG_PRINT(_STR_, args...) ;
#endif

#define WARN_MSG(_STR_, args...) printf("[%s] "_STR_, __FILE__, ##args);

// ====================== GCM 消息推送 ==================================
void gcm_reg_server(const char *did) {
	DBG_PRINT("gcm_reg_server. did = %s \n", did);

	char url_cmd[256] = { 0 };

	// Modify push server address and parameter. #Isaac. 2014-11-19
	sprintf(url_cmd, "%s/%s?cmd=reg_server&did=%s&device_type=%s&device_version=%s",
				PUSH_SERVER_HTTPS, PUSH_PAGE, did, "DefaultType", "1.X.X.X");
	printf("url_cmd : %s\n", url_cmd);

	curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);
	curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
	gCurlCode = curl_easy_perform(gCurl);

	DBG_PRINT("curl_easy_perform - gCurlCode = %d\n", gCurlCode);
	if (gCurlCode != CURLE_OK) {
		DBG_PRINT("gcm_reg_server result: failed: %s\n", curl_easy_strerror(gCurlCode));
	} else {
		DBG_PRINT("gcm_reg_server result: CURLE_OK!!!\n");
	}
}

void gcmRegister(const char *payload, int len)
{
	DBG_PRINT("gcmRegister. payload = %s\n", payload);

	const char *did = (const char *)malloc(len + 1);
	memset((void *)did, 0, len + 1);
	memcpy((void *)did, payload, len);

	// 判断是否已经注册过
	if (gCurl == NULL)
	{
		memset(gDID, 0, sizeof(gDID));
		memcpy(gDID, did, strlen(did));

		gCurl = curl_easy_init();
		if (gCurl)
		{
			gcm_reg_server(did);
		}
	}
	else
	{
		DBG_PRINT("%s: gcm already reigstered\n", did);
	}

	free((void *) did);
}

void gcm_send_event_list(const char* text) {
	CURLcode res;

	DBG_PRINT("gcm_send_event_list command = %s \n", text);

	char url_cmd[512] = { 0 };

	char *newText = curl_easy_escape(gCurl, text, 0);

	// Modify push server address. #Isaac. 2014-11-19
	sprintf(url_cmd,
			"%s/%s?cmd=push_event&did=%s&text=%s",
			PUSH_SERVER_HTTPS, PUSH_PAGE, gDID, newText);
	printf("url_cmd : %s\n", url_cmd);

	curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);
	curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_perform(gCurl);

	curl_free(newText);

	if (res != CURLE_OK) {
		DBG_PRINT("gcm_send_event_list result: failed: %s\n", curl_easy_strerror(res));
	} else {
		DBG_PRINT("gcm_send_event_list result: CURLE_OK!!! \n");
	}
}

void *gcm_thread_rountine(void *ptr){
	// Set detach
	pthread_detach(pthread_self());

	DBG_PRINT("gcm_thread_rountine running...\n");

	pthread_mutex_lock(&curl_mutex);
	const char *gcm_cmd = (const char *)ptr;
	gcm_send_event_list(gcm_cmd);
	free((void *)gcm_cmd);
	pthread_mutex_unlock(&curl_mutex);
	
	pthread_exit(0);
}

void pushEvent(const char *payload, int len)
{
	DBG_PRINT("pushEvent. payload = %s \n", payload);

	// 判断 curl 是否已经注册
	if (gCurl == NULL)
	{
		DBG_PRINT("curl has not registered to server! \n");
		return;
	}

	const char *result = (const char *)malloc(len + 1);
	memset((void *)result, 0, len + 1);
	memcpy((void *)result, payload, len);
	pthread_create(&gcm_thread, NULL, gcm_thread_rountine, (void *)result);
}

/* 20150324_victorwu: send sms message - begin */
void sms_send_event_list(const char* key_string, const char* from_string, const char* to_string, const char* message) {
	CURLcode res;

	DBG_PRINT("sms_send_event_list key = %s from = %s to = %s message = %s\n", key_string, from_string, to_string, message);

	char url_cmd[512] = { 0 };
	char *newKeyString = curl_easy_escape(gCurl, key_string, 0);
	char *newFromString = curl_easy_escape(gCurl, from_string, 0);
	char *newToString = curl_easy_escape(gCurl, to_string, 0);
	char *newMessage = curl_easy_escape(gCurl, message, 0);

	// Modify push server address. #Isaac. 2014-11-19
	sprintf(url_cmd,
			"%s/%s?cmd=send_sms&key=%s&from=%s&to=%s&message=%s",
			PUSH_SERVER_HTTPS, PUSH_PAGE, newKeyString, newFromString, newToString, newMessage);
	printf("url_cmd : %s\n", url_cmd);

	curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);
	curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_perform(gCurl);

	curl_free(newKeyString);
	curl_free(newFromString);
	curl_free(newToString);
	curl_free(newMessage);

	if (res != CURLE_OK) {
		DBG_PRINT("sms_send_event_list result: failed: %s\n", curl_easy_strerror(res));
	} else {
		DBG_PRINT("sms_send_event_list result: CURLE_OK!!! \n");
	}
}

void *sms_thread_rountine(void *ptr){
	// Set detach
	pthread_detach(pthread_self());

	DBG_PRINT("sms_thread_rountine running...\n");

	pthread_mutex_lock(&curl_mutex);
	char sms_key[64] = {0};
	char sms_to[64] = {0};
	char sms_from[64] = {0};
	char sms_message[256] = {0};
	
	sscanf((const char *)ptr, "%s%s%s", sms_key, sms_from, sms_to);
	memcpy(sms_message, ptr+strlen(sms_key)+1+strlen(sms_from)+1+strlen(sms_to)+1, sizeof(sms_message));
	sms_send_event_list(sms_key, sms_from, sms_to, sms_message);
	free((void *)ptr);
	pthread_mutex_unlock(&curl_mutex);

	pthread_exit(0);
}

void smsEvent(const char *payload, int len)
{
	DBG_PRINT("smsEvent. payload = %s \n", payload);

	if (gCurl == NULL)
	{
		DBG_PRINT("curl has not initialized! \n");
		return;
	}

	const char *result = (const char *)malloc(len + 1);
	memset((void *)result, 0, len + 1);
	memcpy((void *)result, payload, len);
	pthread_create(&sms_thread, NULL, sms_thread_rountine, (void *)result);
}
/* 20150324_victorwu: send sms message - end */

void send_email_notification(const char* text) {
	CURLcode res;

	DBG_PRINT("text = %s\n", text);

	char url_cmd[2048] = {0};
	char toAddress[1024] = {0};
	char fromAddress[128] = {0};
	char subject[128] = {0};
	char content[512] = {0};

	sscanf(text, "%[^;]", toAddress);
	sscanf(text + strlen(toAddress) + 1, "%[^;]", fromAddress);
	sscanf(text + strlen(toAddress) + strlen(fromAddress) + 2, "%[^;]", subject);
	sscanf(text + strlen(toAddress) + strlen(fromAddress) + strlen(subject) + 3 , "%[^;]", content);

	char *newToAddress = curl_easy_escape(gCurl, toAddress, 0);
	char *newfromAddress = curl_easy_escape(gCurl, fromAddress, 0);
	char *newSubject = curl_easy_escape(gCurl, subject, 0);
	char *newContent = curl_easy_escape(gCurl, content, 0);

	sprintf(url_cmd,
			"%s/%s?cmd=send_mail&did=%s&to=%s&from=%s&subject=%s&content=%s",
			PUSH_SERVER_HTTPS, PUSH_PAGE, gDID, newToAddress, newfromAddress, newSubject, newContent);
	printf("url_cmd : %s\n", url_cmd);

	curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);
	curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_perform(gCurl);

	curl_free(newToAddress);
	curl_free(newfromAddress);
	curl_free(newSubject);
	curl_free(newContent);

	if (res != CURLE_OK) {
		DBG_PRINT("send_email_notification result: failed: %s\n", curl_easy_strerror(res));
	} else {
		DBG_PRINT("send_email_notification result: CURLE_OK!!! \n");
	}
}

void *email_notification_thread_rountine(void *ptr){
	// Set detach
	pthread_detach(pthread_self());

	DBG_PRINT("email_notification_thread_rountine running...\n");

	pthread_mutex_lock(&curl_mutex);
	const char *email_notification_cmd = (const char *)ptr;
	send_email_notification(email_notification_cmd);
	free((void *)email_notification_cmd);
	pthread_mutex_unlock(&curl_mutex);

	pthread_exit(0);
}

void sendEmailNotification(const char *payload, int len)
{
	DBG_PRINT("sendEmailNotification. payload = %s \n", payload);

	if (gCurl == NULL)
	{
		DBG_PRINT("curl has not registered to server! \n");
		return;
	}

	const char *result = (const char *)malloc(len + 1);
	memset((void *)result, 0, len + 1);
	memcpy((void *)result, payload, len);
	pthread_create(&email_notification_thread, NULL, email_notification_thread_rountine, (void *)result);
}


// ====================== 发送邮件 ==================================
void *email_thread_routine(void *ptr){
	// Set detach
	pthread_detach(pthread_self());

	DBG_PRINT("email_thread_routine running...\n");

	pthread_mutex_lock(&email_mutex);
	const char *email_cmd = (const char *)ptr;

	DBG_PRINT("email_cmd = %s\n", email_cmd);

	system(email_cmd);
	free((void *)email_cmd);
	pthread_mutex_unlock(&email_mutex);

	pthread_exit(0);
}

void sendEmail(const char *payload, int len)
{
	DBG_PRINT("sendEmail payload = %s\n", payload);

	const char *result = (const char *)malloc(len + 1);
	memset((void *)result, 0, len + 1);
	memcpy((void *)result, payload, len);
	pthread_create(&email_thread, NULL, email_thread_routine, (void *)result);
}


// ====================== IP Camera 录像 ==================================
#ifdef IPCAM_ENABLE
void *ipc_record_thread_routine(void *ptr)
{
	DBG_PRINT("trigger_camera_thread_routine \n");

	if (NULL == ptr)
		return NULL;

	int startConnect = 0;	// flag for start connect
	int sendRecordCmd = 0;	// flag for send trigger record command

	int endian;
	int now = (int) time(NULL);

	CheckCPUEndian(endian);
	Camera_Init(pszParam);

	char *payload = (char *)ptr;

	// 获取DID， PASSWD
	char did[32] = { 0 };
	char pw[32] = { 0 };

	sscanf(payload, "%[^;]", did);
	sscanf(payload + strlen(did) + 1, "%[^.]", pw);
	free(ptr);

	DBG_PRINT("camera: did = %s. pw = %s\n", did, pw);

	// 初始化摄像头
	memset(gTriggerCamera.did, 0, CAM_DID_LENGTH);
	memset(gTriggerCamera.passwd, 0, CAM_PASSWD_LENGTH);

	gTriggerCamera.valid = 1;
	gTriggerCamera.touch = now;

	strncpy(gTriggerCamera.did, did, strlen(did));
	strncpy(gTriggerCamera.passwd, pw, strlen(pw));

	gTriggerCamera.hCamera.connState = CAMERA_CONN_IDLE;
	gTriggerCamera.hCamera.p2pConnResult = TRIGGER_RECORDER_DEFAULT;

	while (1)
	{
		switch (gTriggerCamera.hCamera.connState)
		{
		case CAMERA_CONN_IDLE:
			// 开始连接
			gTriggerCamera.hCamera.connState = CAMERA_CONN_CONNECTING;
			if (0 == startConnect)
			{
				startConnect = 1;

				DBG_PRINT("%s: begin connecting.\n", did);
				int ret = Camera_Connect((HCAMERA *)&gTriggerCamera.hCamera,
										gTriggerCamera.did,
										gTriggerCamera.passwd,
										0,
										true);
				DBG_PRINT("Camera_Connect ret = %d\n", ret);
				gTriggerCamera.hCamera.p2pConnResult = ret;
			}
			break;

		case CAMERA_CONN_CONNECTING:
			DBG_PRINT("%s: Connecting.\n", did);
			break;

		case CAMERA_CONN_DISCONNECTING:
			DBG_PRINT("%s: Disconnecting.\n", did);
			break;

		case CAMERA_CONN_CONNECTED:
			if (0 == sendRecordCmd)
			{
				sendRecordCmd = 1;
				DBG_PRINT("%s: Connected.\n", did);
				DBG_PRINT("%s: Sending trigger record command.\n", did);
				Camera_manuRecStart(&gTriggerCamera.hCamera);
			}
			else
			{
				DBG_PRINT("%s: Waiting for trigger result.\n", did);
			}
			break;
		}

		// p2p connect error
		if (gTriggerCamera.hCamera.p2pConnResult < 0)
			break;

		// get trigger result
		if (TRIGGER_RECORDER_SUCCESSFUL <= gTriggerCamera.hCamera.p2pConnResult
				&& gTriggerCamera.hCamera.p2pConnResult <= TRIGGER_RECORDER_WRONG_PASSWROD)
		{
			// successful
			if (TRIGGER_RECORDER_SUCCESSFUL == gTriggerCamera.hCamera.p2pConnResult)
				usleep(100 * 1000);	// 100ms

			break;
		}


		usleep(50 * 1000);		// 50 ms
	}

	Camera_Deinit();

	// exit thread
	if (gTriggerCamera.hCamera.p2pConnResult < 0)
		DBG_PRINT("%s: %s.\n", did, P2PErrorMsg[-gTriggerCamera.hCamera.p2pConnResult]);
	else if (TRIGGER_RECORDER_SUCCESSFUL <= gTriggerCamera.hCamera.p2pConnResult
			&& gTriggerCamera.hCamera.p2pConnResult <= TRIGGER_RECORDER_WRONG_PASSWROD)
		DBG_PRINT("%s: %s.\n", did, StarxErrorMsg[gTriggerCamera.hCamera.p2pConnResult]);
	else
		DBG_PRINT("%s: %s.\n", did, "You should NOT see this message! Please connect us!");


	DBG_PRINT("trigger_record result = %d \n", gTriggerCamera.hCamera.p2pConnResult);

	return NULL;
}

void ipcRecord(const char *payload, int len)
{
	DBG_PRINT("ipcRecord payload = %s \n", payload);

	char *result = (char *)malloc(len + 1);
	memset(result, 0, len + 1);
	memcpy(result, payload, len);
	pthread_create(&ipc_record_thread, NULL, ipc_record_thread_routine, (void *)result);
}
#endif

void mqttCheck(const char *payload, int len)
{
	DBG_PRINT("mqttCheck payload %s(%d)\n", payload, len);

	if(strcmp(payload, "CHECK") == 0)
	{
		char sMsg[16] = {"OK"};

		mosquitto_publish(connection, 0, TOPIC_CHECK_CONNECT, strlen(sMsg), sMsg, 2, 0);
	}
}

void connect_callback(struct mosquitto *mosq, void *obj, int rc) {
	if (rc == 0)
	{
		mMQTTConnected = 1;
		DBG_PRINT("%s connect to mqtt bus, succeed!\n", CLIENT_NAME);

		mosquitto_subscribe(connection, NULL, TOPIC_GCM_PUSHEVENT, 2);
		mosquitto_subscribe(connection, NULL, TOPIC_GCM_REGISTER, 2);
		mosquitto_subscribe(connection, NULL, TOPIC_SMS_PUSHEVENT, 2);
		mosquitto_subscribe(connection, NULL, TOPIC_SEND_EMIAL, 2);
#ifdef IPCAM_ENABLE
		mosquitto_subscribe(connection, NULL, TOPIC_IPC_RECORD, 2);
#endif
		mosquitto_subscribe(connection, NULL, TOPIC_CHECK_CONNECT, 2);
		mosquitto_subscribe(connection, NULL, TOPIC_SEND_EMAIL_NOTIFICATION, 2);
	}
	else
	{
		DBG_PRINT("%s connect to mqtt bus, failed!\n", CLIENT_NAME);
	}
}

void message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message) {
	DBG_PRINT("ceres_util message_callback(). topic = %s\n", message->topic);

	if (strcmp(message->topic, TOPIC_GCM_PUSHEVENT) == 0)
	{
		pushEvent(message->payload, message->payloadlen);
	}
	else if (strcmp(message->topic, TOPIC_GCM_REGISTER) == 0)
	{
		gcmRegister(message->payload, message->payloadlen);
	}
	else if (strcmp(message->topic, TOPIC_SMS_PUSHEVENT) == 0)
	{
		smsEvent(message->payload, message->payloadlen);
	}
	else if (strcmp(message->topic, TOPIC_SEND_EMIAL) == 0)
	{
		sendEmail(message->payload, message->payloadlen);
	}
#ifdef IPCAM_ENABLE
	else if ( strcmp(message->topic, TOPIC_IPC_RECORD)== 0)
	{
		ipcRecord(message->payload, message->payloadlen);
	}
#endif
	else if(strcmp(message->topic, TOPIC_CHECK_CONNECT) == 0)
	{
		mqttCheck(message->payload, message->payloadlen);
	}
	else if(strcmp(message->topic, TOPIC_SEND_EMAIL_NOTIFICATION) == 0)
	{
		sendEmailNotification(message->payload, message->payloadlen);
	}
}

void* mqtt_client_thread_routine(void *ptr) {
	int rc = 0;
	int nFailCount = 0;

	mosquitto_lib_init();

	connection = mosquitto_new(CLIENT_NAME, true, NULL);
	if (connection) {
		mosquitto_connect_callback_set(connection, connect_callback);
		mosquitto_message_callback_set(connection, message_callback);

		rc = mosquitto_connect(connection, MQTT_HOST, MQTT_PORT, 60);

		while (1)
		{
			if (mosquitto_loop(connection, -1, 1) != MOSQ_ERR_SUCCESS)	/* 20150225_victorwu: fix mqtt crash issue */
			{
				if (nFailCount > 5)	
				{
					WARN_MSG("%s is disconnected with mqtt, exit and restart ceres_util process......\n", CLIENT_NAME);
					sleep(2);
					system("killall -9 ceres_util&");
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
		mosquitto_disconnect(connection);
		mosquitto_destroy(connection);
	}
	mosquitto_lib_cleanup();

	g_run = 0;	// End main thread
	DBG_PRINT("End thread\n");
	return NULL;
}


int main(int argc, char *argv[]) {
	DBG_PRINT("ceres_util is running. \n");

	pthread_mutex_init(&curl_mutex, NULL);
	pthread_mutex_init(&email_mutex, NULL);

#ifdef IPCAM_ENABLE
	int APIVersion = STARX_GetAPIVersion();
	DBG_PRINT("camera_record-P2P Version: %d.%d.%d.%d\n", (APIVersion & 0xFF000000) >> 24,
				(APIVersion & 0x00FF0000) >> 16, (APIVersion & 0x0000FF00) >> 8,
				(APIVersion & 0x000000FF) >> 0);
#endif

	// Save pid itself
	{
		char sCmd[128] = "";

		snprintf(sCmd, sizeof(sCmd), "echo %d > /var/run/ceres_util.pid", getpid());
		system(sCmd);
	}

	// MQTT线程
	pthread_create(&mqtt_sub_pid, NULL, mqtt_client_thread_routine, NULL);
	while (g_run)
	{
		if (0 == mMQTTConnected)
		{
			DBG_PRINT("waiting for mqtt connect\n");
			usleep(1 * 1000 * 1000);
		}
		else
			break;
	}

	if(g_run)
	{
		// Sleep 30 sec and check gCurl
		sleep(30);
		if(gCurl == NULL)
		{
			char sMsg[16] = {"RETRY"};

			DBG_PRINT("Send mqtt to ceres, do gcm register\n");
			mosquitto_publish(connection, 0, TOPIC_CHECK_CONNECT, strlen(sMsg), sMsg, 2, 0);
		}
	}

	// 主循环
	while(g_run)
	{
		usleep(10 * 1000 * 1000);
	}

	curl_easy_cleanup(gCurl);
	pthread_join(mqtt_sub_pid, NULL);
	pthread_mutex_destroy(&curl_mutex);
	pthread_mutex_destroy(&email_mutex);

	DBG_PRINT("gcm_pushevent exit application\n");
	return 0;
}
