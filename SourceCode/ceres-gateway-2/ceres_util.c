
#include <time.h>
#include <stdlib.h>

#include "ceres_util.h"
#include "jsw_protocol.h"

#define WARN_MSG(_STR_, args...) printf("[%s] "_STR_, __FILE__, ##args);
// Modify push server address. #Isaac. 2014-11-19
const char *PUSH_SERVER = "http://apns01.omguard.com";
const char *PUSH_SERVER_HTTPS   = "https://apns01.omguard.com";
const char *PUSH_PAGE   = "apns.php";

CURL *gCurl=NULL;
CURLcode gCurlCode = -1;

struct sms_info g_sms;

char g_curl_output[DEF_CURL_OUTPUT_BUFFER_LEN];

extern pthread_t philipt_hue_handle;// = NULL;
extern int g_selftest_flag;// = 0;//self-test flag, write to running log


// ====================== GCM 消息推送 ==================================
void gcm_reg_server(const char *did) {
    DBG_PRINT("gcm_reg_server. did = %s \n", did);

    char url_cmd[256] = { 0 };

    // Modify push server address and parameter. #Isaac. 2014-11-19
    sprintf(url_cmd, "%s/%s?cmd=reg_server&did=%s&device_type=%s&device_version=%s",
                PUSH_SERVER_HTTPS, PUSH_PAGE, did, "DefaultType", "1.X.X.X");
//  printf("url_cmd : %s\n", url_cmd);

    curl_easy_setopt(gCurl, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);
//    curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);
    curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(gCurl, CURLOPT_WRITEFUNCTION, http_get_callback);
    curl_easy_setopt(gCurl, CURLOPT_WRITEDATA, (void *)g_curl_output);

    gCurlCode = curl_easy_perform(gCurl);

//  DBG_PRINT("curl_easy_perform - gCurlCode = %d\n", gCurlCode);
    if (gCurlCode != CURLE_OK) {
        DBG_PRINT("gcm_reg_server result: failed: %s\n", curl_easy_strerror(gCurlCode));
    } else {
        DBG_PRINT("gcm_reg_server result: CURLE_OK!!!\n");
    }
}

void gcmRegister(const char *did)
{
    // 判断是否已经注册过
    if (gCurl == NULL)
    {
        gCurl = curl_easy_init();
        if (gCurl)
        {
            pthread_mutex_init(&curl_mutex, NULL);
            pthread_mutex_init(&email_mutex, NULL);
            gcm_reg_server(did);
        }else
        {
            DBG_PRINT("curl_easy_init fail\n");
        }
    }
    else
    {
        DBG_PRINT("gcm already reigstered\n");
    }
}


//////////////////////////////////////////////////////////

int cal_text_checksum(char *newText)
{
    int ret = 0;
    int i;
    for(i=0;i<strlen(newText);i++)
    {
        if((i % 10) == 0)
            ret = ret+newText[i]*5+10000000;
        else if((i % 2) == 0)
            ret = ret+newText[i]*2+792;
        else if((i % 3) == 1)
            ret = ret-newText[i]*3+4106;
        else if((i % 5) == 0)
            ret -= (newText[i]+309);
        else
            ret += newText[i];
    }
    return ret;
}

int get_text_checksum(char *newText)
{
    int ret = 133;

    ret += cal_text_checksum(gDID);
    ret += cal_text_checksum(newText);
    if(ret < 0)
        ret = 0-ret;
    return ret%128;
}

int get_rand_alphanumeris()
{
    int ret = 0;

    while( !( ((ret >='0') && (ret <= '9')) || ((ret >='A') && (ret <= 'Z')) || ((ret >='a') && (ret <= 'z')) ) )
    {
        ret = rand()%128;
    }

    return ret;
}

int add_text_checksum(char *newText)
{
    int ret = 0;
    int i;
    for(i=0;i<strlen(newText);i++)
    {
        ret += newText[i];
    }
    return ret%128;
}

int gen_notification_hash(char *hash_code, char *newText)
{
    if(strlen(newText) >= 512)
        return 0;
    int hash1 = 0, hash2 = 1, hash11;
    int i, count=0;
    //srand((int)time(0));
    hash1 = get_text_checksum(newText);
    while(hash1 != hash2)
    {
        count++;
        if(count > 300)
            return 0;
        for(i=0;i<9;i++)
        {
            hash_code[i] = get_rand_alphanumeris();
        }
        hash_code[9] = 0;
        hash2 = add_text_checksum(hash_code);
        hash11 = hash1 - hash2;
        if(hash11 < 0)
            hash11 += 128;
        if( ((hash11 >= '0') && (hash11 <= '9')) ||
            ((hash11 >= 'A') && (hash11 <= 'Z')) ||
            ((hash11 >= 'a') && (hash11 <= 'z')) )
        {
            hash_code[9] = hash11;
            hash_code[10] = 0;
            hash2 = add_text_checksum(hash_code);
        }
    }
    hash_code[10] = 0;
    return 1;
}
int gcm_sound_type;

void gcm_send_event_list(const char* text) {
	static int s_errcode_7 = 0;

    if(g_selftest_flag > 0)
    {
        SetRunningLog_str("gcm_send_event_list() Start");
    }

    //DBG_PRINT("gcm_send_event_list command = %s \n", text);

    //get hash code
    char hash_code[11] = {0};
    if(gen_notification_hash(hash_code, text) == 0)
    {
        printf("Error to generate hash code!\n");
        if(g_selftest_flag > 0)
        {
            SetRunningLog_str("Error: gcm_send_event_list() gen hash code failed");
        }
        return;
    }

    CURLcode res;
    char url_cmd[512] = {0};
    char *newText = curl_easy_escape(gCurl, text, 0);

    // Modify push server address. #Isaac. 2014-11-19
    if(gcm_sound_type)
		sprintf(url_cmd,
				"%s/%s?cmd=push_event&did=%s&text=%s&hash=%s&sound=%s",
				PUSH_SERVER_HTTPS, PUSH_PAGE, gDID, newText, hash_code,"alarm.MP3");
	else
    	sprintf(url_cmd,
            "%s/%s?cmd=push_event&did=%s&text=%s&hash=%s",
            PUSH_SERVER_HTTPS, PUSH_PAGE, gDID, newText, hash_code);
    printf("url_cmd : %s\n", url_cmd);

    curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);

    //using HTTPS
    curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(gCurl, CURLOPT_WRITEFUNCTION, http_get_callback);
    curl_easy_setopt(gCurl, CURLOPT_WRITEDATA, (void *)g_curl_output);

    res = curl_easy_perform(gCurl);

    if(g_selftest_flag > 0)
    {
        long response_code;
        curl_easy_getinfo(gCurl, CURLINFO_RESPONSE_CODE, &response_code);
printf("==== CURLINFO_RESPONSE_CODE=%lu\n", response_code);
        SetRunningLog_str_int("CURLINFO_RESPONSE_CODE=", (int)response_code);
        double dl;
        curl_easy_getinfo(gCurl, CURLINFO_SIZE_DOWNLOAD, &dl);
printf("==== CURLINFO_SIZE_DOWNLOAD=%lf\n", dl);
        SetRunningLog_str_int("CURLINFO_SIZE_DOWNLOAD=", (int)dl);
    }

    curl_free(newText);

    if(res != CURLE_OK)
    {
        DBG_PRINT("gcm_send_event_list result: failed: %s\n", curl_easy_strerror(res));
        if(g_selftest_flag > 0)
        {
            SetRunningLog_str_int("Error: gcm_send_event_list() failed to call curl_easy_perform()", res);
        }
        if(res == CURLE_WRITE_ERROR) //23
        {
            printf("Reboot due to libcurl return code = 23\n");
            log_and_reboot_no_beep("Reboot due to libcurl return code = 23", 1);
        }else if(res == CURLE_COULDNT_CONNECT) //7
        {
            s_errcode_7++;
            if(s_errcode_7 >= 5)
            {
                printf("Reboot due to libcurl return code = 7\n");
                log_and_reboot_no_beep("Reboot due to libcurl return code = 7", 1);
            }
        }
    }else
    {
	    s_errcode_7 = 0;
        //DBG_PRINT("gcm_send_event_list result: CURLE_OK!!! \n");
        if(g_selftest_flag > 0)
        {
                SetRunningLog_str("Successful: the result of gcm_send_event_list() is CURLE_OK");
        }
    }

    if(g_selftest_flag > 0)
    {
        SetRunningLog_str("gcm_send_event_list() End");
    }
}

//libcurl HTTP GET callback function
static size_t http_get_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    if(g_selftest_flag > 0)
    {
        printf("==== curl callback: size=%d, nmemb=%d\n", size, nmemb);
    }
    if( (size * nmemb) < DEF_CURL_OUTPUT_BUFFER_LEN)
    {
        memset(userp, 0, DEF_CURL_OUTPUT_BUFFER_LEN);
        memcpy(userp, contents, size * nmemb);
        return size * nmemb;
    }else
    {
        memcpy(userp, contents, DEF_CURL_OUTPUT_BUFFER_LEN-1);
        char *pp = (char*)userp;
        pp[DEF_CURL_OUTPUT_BUFFER_LEN-1] = 0;
        return DEF_CURL_OUTPUT_BUFFER_LEN-1;
    }
}

//HTTP GET with response
void curl_get(char *url, char *text, char *output)
{
    CURLcode res;

    //DBG_PRINT("gcm_send_event_list command = %s \n", text);

    char url_cmd[512] = { 0 };
    char *newText = curl_easy_escape(gCurl, text, 0);

    // Modify push server address. #Isaac. 2014-11-19
    sprintf(url_cmd, "%s%s", url, newText);
    //printf("url_cmd : %s\n", url_cmd);

    curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);

    //using HTTPS
    //curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    //curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(gCurl, CURLOPT_WRITEFUNCTION, http_get_callback);
    curl_easy_setopt(gCurl, CURLOPT_WRITEDATA, (void *)output);

    curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
    res = curl_easy_perform(gCurl);

    curl_free(newText);
    //curl_free(hash_code2);

    //for test start
//  curl_version_info_data *cvid = NULL;
//  cvid = curl_version_info(CURL_VERSION_SSL);
//  printf("cvid->ssl_version=%s\n", cvid->ssl_version);
    //for test end

    if (res != CURLE_OK) {
        DBG_PRINT("curl_get() res=%d, result: failed: %s\n", res, curl_easy_strerror(res));
        if(g_selftest_flag > 0)
        {
            SetRunningLog_str_int("Error: curl_get() failed to call curl_easy_perform()", res);
            SetRunningLog_str(curl_easy_strerror(res));
        }
    } else {
        DBG_PRINT("curl_get() result: CURLE_OK!!! \n");
    }
}

// HTTP PUT function to change status of lamps
int curl_put(char *buffer)//char *ip_hue, char *hash_hue, int lamp, char *buffer)
{
    CURL *curl=NULL;
    CURLcode curl_res;
    char addr[256];
    char hue_ip[32], hue_hash[64];
    int lamp = 0;

    //read config file
    char script_buf[1024];

    memset(script_buf, 0, 1024);

    //handle config file
    FILE *fp;
    fp = fopen("./philips_hue.db", "rb");
    if (fp == NULL)
    {
        printf("No philips hue config file found, exit\n");
        return -1;
    }
    int bread = fread(script_buf, 1, 1024, fp);
    fclose(fp);

    //srtok
    char chars[] = "\n";
    char *p = NULL;
    char *c1;
    p = strtok(script_buf, chars);
    while(p)
    {
        printf("line=%s\n", p);
        if(strstr(p, "ipaddress:"))
        {//ipaddress command
            c1 = strrchr(p, ':');
            if(c1)
            {
                memset(hue_ip, 0, sizeof(hue_ip));
                memcpy(hue_ip, c1+1, strlen(p)-(c1-p));
            }
        }else if(strstr(p, "hash:"))
        {//hash command
            c1 = strrchr(p, ':');
            if(c1)
            {
                memset(hue_hash, 0, sizeof(hue_hash));
                memcpy(hue_hash, c1+1, strlen(p)-(c1-p));
            }
        }else if(strstr(p, "lamp:"))
        {//lamp command
            c1 = strrchr(p, ':');
            if(c1)
            {
                lamp = atoi(c1+1);
            }
        }

        p = strtok(NULL, chars);
    }

    printf("ipaddress:%s, hash:%s, lamp:%d\n", hue_ip, hue_hash, lamp);
    sprintf(addr, "http://%s/api/%s/lights/%d/state", hue_ip, hue_hash, lamp);//"192.168.2.20", "e40d9351259a4a71220340121e67fd3", 1);//ip_hue, hash_hue, lamp);

    printf("addr=%s\n", addr);
    printf("json=%s\n", buffer);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(NULL, "Content-Type: application/json");

    if(curl == NULL)
        curl = curl_easy_init();
    if(curl == NULL)
    {
        printf("curl_easy_init(0 failed\n!!!");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, addr);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_get_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)g_curl_output);

    curl_res = curl_easy_perform(curl);
    if(curl_res != 0) {
        printf("Error: Could not send command to lamp\n");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 0;
}

void *gcm_thread_rountine(void *ptr){
    if(g_selftest_flag > 0)
    {
        SetRunningLog_str("gcm_thread_rountine() Start");
    }

    // Set detach
    pthread_detach(pthread_self());

    //DBG_PRINT("gcm_thread_rountine running...\n");

    pthread_mutex_lock(&curl_mutex);
    const char *gcm_cmd = (const char *)ptr;
    gcm_send_event_list(gcm_cmd);
    //free((void *)gcm_cmd);
    pthread_mutex_unlock(&curl_mutex);

	if(g_selftest_flag > 0)
    {
        SetRunningLog_str("gcm_thread_rountine() End");
    }
pthread_exit(0);
}

void *gcm_thread_rountine2(void *ptr){
    // Set detach
    pthread_detach(pthread_self());

    //DBG_PRINT("gcm_thread_rountine running...\n");

    pthread_mutex_lock(&curl_mutex);
    const char *gcm_cmd2 = (const char *)ptr;
    curl_put(gcm_cmd2);
    //free((void *)gcm_cmd);
    pthread_mutex_unlock(&curl_mutex);

    pthread_exit(0);
}

char gcmdata[1024];

void pushEvent(const char *payload, int len)
{
    //DBG_PRINT("pushEvent. payload = %s \n", payload);

    if(g_selftest_flag > 0)
    {
        SetRunningLog_str("pushEvent() Start");
    }

    // 判断 curl 是否已经注册
    if (gCurl == NULL)
    {
        DBG_PRINT("curl has not registered to server! \n");
        if(g_selftest_flag > 0)
        {
            SetRunningLog_str("Error: pushEvent() return by gCurl is NULL");
        }
        return;
    }

    //const char *result = (const char *)malloc(len + 1);
    memset((void *)gcmdata, 0, sizeof(gcmdata));
    if (len > 1024)
    {
        DBG_PRINT("pushEvent.  mem overflow \n");
        if(g_selftest_flag > 0)
        {
            SetRunningLog_str_int("Error: pushEvent() return by len overflow", len);
        }
        return;
    }
    memcpy((void *)gcmdata, payload, len);
    pthread_create(&gcm_thread, NULL, gcm_thread_rountine, (void *)gcmdata);

    if(g_selftest_flag > 0)
    {
        SetRunningLog_str("pushEvent() End");
    }
}

char gcmdata2[1024];
void pushEvent2(const char *payload, int len)
{
    // 判断 curl 是否已经注册
//  if (gCurl == NULL)
//  {
//      DBG_PRINT("curl has not registered to server2! \n");
//      return;
//  }

    //const char *result = (const char *)malloc(len + 1);
    memset((void *)gcmdata2, 0, sizeof(gcmdata2));
    if (len > 1024)
    {
        DBG_PRINT("pushEvent2.  mem overflow \n");
        return;
    }
    memcpy((void *)gcmdata2, payload, len);
    pthread_create(&philipt_hue_handle, NULL, gcm_thread_rountine2, (void *)gcmdata2);
    pthread_join(philipt_hue_handle, NULL);
}

/* 20150324_victorwu: send sms message - begin */
void sms_send_event_list(const char* key_string, const char* from_string, const char* to_string, const char* message, const char *route) {
    CURLcode res;
//  DBG_PRINT("sms_send_event_list key = %s from = %s to = %s message = %s\n", key_string, from_string, to_string, message);

    char url_cmd[2000] = { 0 };
    char *newKeyString = curl_easy_escape(gCurl, key_string, 0);
    char *newFromString = curl_easy_escape(gCurl, from_string, 0);
    char *newToString = curl_easy_escape(gCurl, to_string, 0);
    char *newMessage = curl_easy_escape(gCurl, message, 0);
    char *route_data = curl_easy_escape(gCurl, route, 0);

    // Modify push server address. #Isaac. 2014-11-19
    sprintf(url_cmd,
            "%s/%s?cmd=send_sms&key=%s&from=%s&to=%s&message=%s&route=%s",
            PUSH_SERVER, PUSH_PAGE, newKeyString, newFromString, newToString, newMessage, route_data);

    printf("url_cmd : %s\n", url_cmd);

    curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);
    curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(gCurl, CURLOPT_WRITEFUNCTION, http_get_callback);
    curl_easy_setopt(gCurl, CURLOPT_WRITEDATA, (void *)g_curl_output);

    res = curl_easy_perform(gCurl);

    curl_free(newKeyString);
    curl_free(newFromString);
    curl_free(newToString);
    curl_free(newMessage);

    if (res != CURLE_OK) {
        DBG_PRINT("sms_send_event_list result: failed: %s\n", curl_easy_strerror(res));
    } else {
        //DBG_PRINT("sms_send_event_list result: CURLE_OK!!! \n");
    }
}

void *sms_thread_rountine(void *ptr){
    // Set detach
    pthread_detach(pthread_self());

    struct sms_info *data;
    data = (struct sms_info*) ptr;

//  DBG_PRINT("sms_thread_rountine running...\n");

    pthread_mutex_lock(&curl_mutex);

    //char sms_key[64] = {0};
    //char sms_to[64] = {0};
    //char sms_from[64] = {0};
    //char sms_message[256] = {0};
    //sscanf((const char *)ptr, "%s%s%s", sms_key, sms_from, sms_to);
    //memcpy(sms_message, ptr+strlen(sms_key)+1+strlen(sms_from)+1+strlen(sms_to)+1, sizeof(sms_message));


    sms_send_event_list(data->key, data->from, data->to, data->message, data->route);

    pthread_mutex_unlock(&curl_mutex);

    pthread_exit(0);
}

char smsdata[512];
void smsEvent(const char *payload, int len)
{
    //DBG_PRINT("smsEvent. payload = %s \n", payload);

    if (gCurl == NULL)
    {
        DBG_PRINT("curl has not initialized! \n");
        return;
    }
    memset(&g_sms, 0, sizeof(g_sms));

    if ( strlen(pushdata.smsnumber) < 5)
        return;

    if ( strlen(pushdata.key) < 5)
        return;

    strcpy(g_sms.message, payload);
    //strcpy(g_sms.from, "Smartvest");
    strcpy(g_sms.from, "SHC_pro");
    strcpy(g_sms.to, pushdata.smsnumber);
    strcpy(g_sms.key, pushdata.key);
    if (pushdata.route == 0)
        strcpy(g_sms.route, "basic");
    else if (pushdata.route == 1)
        strcpy(g_sms.route, "gold");
    else if (pushdata.route == 2)
        strcpy(g_sms.route, "direct");



    //const char *result = (const char *)malloc(len + 1);
    //memset((void *)smsdata, 0,sizeof(smsdata) );
    //memcpy((void *)smsdata, payload, len);
    pthread_create(&sms_thread, NULL, sms_thread_rountine, (void *)&g_sms);
}
/* 20150324_victorwu: send sms message - end */

void send_email_notification(const char* text) {
    CURLcode res;

    //DBG_PRINT("text = %s\n", text);

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
//  printf("url_cmd : %s\n", url_cmd);

    curl_easy_setopt(gCurl, CURLOPT_URL, url_cmd);

    //using HTTPS
    curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(gCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(gCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(gCurl, CURLOPT_WRITEFUNCTION, http_get_callback);
    curl_easy_setopt(gCurl, CURLOPT_WRITEDATA, (void *)g_curl_output);

    res = curl_easy_perform(gCurl);

    curl_free(newToAddress);
    curl_free(newfromAddress);
    curl_free(newSubject);
    curl_free(newContent);

    if (res != CURLE_OK) {
        DBG_PRINT("send_email_notification result: failed: %s\n", curl_easy_strerror(res));
    } else {
    //  DBG_PRINT("send_email_notification result: CURLE_OK!!! \n");
    }
}

void *email_notification_thread_rountine(void *ptr){
    // Set detach
    pthread_detach(pthread_self());

//  DBG_PRINT("email_notification_thread_rountine running...\n");

    pthread_mutex_lock(&curl_mutex);
    const char *email_notification_cmd = (const char *)ptr;
    send_email_notification(email_notification_cmd);
    //free((void *)email_notification_cmd);
    pthread_mutex_unlock(&curl_mutex);

    pthread_exit(0);
}

char *emaildata;
char emaildata2[1024];
char emaildata3[1024];
void sendEmailNotification(const char *payload, int len)
{
    //DBG_PRINT("sendEmailNotification. payload = %s \n", payload);
    static int s_emial_notif = 0;
    if(s_emial_notif % 2)
        emaildata = emaildata2;
    else
        emaildata = emaildata3;
    s_emial_notif++;

    if (gCurl == NULL)
    {
        DBG_PRINT("curl has not registered to server! \n");
        return;
    }

    //const char *result = (const char *)malloc(len + 1);
    memset((void *)emaildata, 0, sizeof(emaildata2) );
    memcpy((void *)emaildata, payload, len);
    pthread_create(&email_notification_thread, NULL, email_notification_thread_rountine, (void *)emaildata);
}


// ====================== 发送邮件 ==================================
void *email_thread_routine(void *ptr){
    // Set detach
    pthread_detach(pthread_self());

    //DBG_PRINT("email_thread_routine running...\n");

    pthread_mutex_lock(&email_mutex);
    const char *email_cmd = (const char *)ptr;

    //DBG_PRINT("email_cmd = %s\n", email_cmd);

    system(email_cmd);
//  free((void *)email_cmd);
    pthread_mutex_unlock(&email_mutex);

    pthread_exit(0);
}

char emaildata_ex[1024];
void sendEmail(const char *payload, int len)
{
    //DBG_PRINT("sendEmail payload = %s\n", payload);

    //const char *result = (const char *)malloc(len + 1);
    memset((void *)emaildata_ex, 0, sizeof(emaildata_ex) );
    memcpy((void *)emaildata_ex, payload, len);
    pthread_create(&email_thread, NULL, email_thread_routine, (void *)emaildata_ex);
}


// ====================== IP Camera 录像 ==================================
//#ifdef IPCAM_ENABLE
//void *ipc_record_thread_routine(void *ptr)
//{
//  //DBG_PRINT("trigger_camera_thread_routine \n");
//
//  if (NULL == ptr)
//      return NULL;
//
//  int startConnect = 0;   // flag for start connect
//  int sendRecordCmd = 0;  // flag for send trigger record command
//
//  int endian;
//  int now = (int) time(NULL);
//
//  CheckCPUEndian(endian);
//  Camera_Init(pszParam);
//
//  char *payload = (char *)ptr;
//
//  // 获取DID， PASSWD
//  char did[32] = { 0 };
//  char pw[32] = { 0 };
//
//  sscanf(payload, "%[^;]", did);
//  sscanf(payload + strlen(did) + 1, "%[^.]", pw);
//  //free(ptr);
//
//  DBG_PRINT("camera: did = %s. pw = %s\n", did, pw);
//
//  // 初始化摄像头
//  memset(gTriggerCamera.did, 0, CAM_DID_LENGTH);
//  memset(gTriggerCamera.passwd, 0, CAM_PASSWD_LENGTH);
//
//  gTriggerCamera.valid = 1;
//  gTriggerCamera.touch = now;
//
//  strncpy(gTriggerCamera.did, did, strlen(did));
//  strncpy(gTriggerCamera.passwd, pw, strlen(pw));
//
//  gTriggerCamera.hCamera.connState = CAMERA_CONN_IDLE;
//  gTriggerCamera.hCamera.p2pConnResult = TRIGGER_RECORDER_DEFAULT;
//
//  while (1)
//  {
//      switch (gTriggerCamera.hCamera.connState)
//      {
//      case CAMERA_CONN_IDLE:
//          // 开始连接
//          gTriggerCamera.hCamera.connState = CAMERA_CONN_CONNECTING;
//          if (0 == startConnect)
//          {
//              startConnect = 1;
//
//              DBG_PRINT("%s: begin connecting.\n", did);
//              int ret = Camera_Connect((HCAMERA *)&gTriggerCamera.hCamera,
//                                      gTriggerCamera.did,
//                                      gTriggerCamera.passwd,
//                                      0,
//                                      true);
//              DBG_PRINT("Camera_Connect ret = %d\n", ret);
//              gTriggerCamera.hCamera.p2pConnResult = ret;
//          }
//          break;
//
//      case CAMERA_CONN_CONNECTING:
//          DBG_PRINT("%s: Connecting.\n", did);
//          break;
//
//      case CAMERA_CONN_DISCONNECTING:
//          DBG_PRINT("%s: Disconnecting.\n", did);
//          break;
//
//      case CAMERA_CONN_CONNECTED:
//          if (0 == sendRecordCmd)
//          {
//              sendRecordCmd = 1;
//              DBG_PRINT("%s: Connected.\n", did);
//              DBG_PRINT("%s: Sending trigger record command.\n", did);
//              Camera_manuRecStart(&gTriggerCamera.hCamera);
//          }
//          else
//          {
//              DBG_PRINT("%s: Waiting for trigger result.\n", did);
//          }
//          break;
//      }
//
//      // p2p connect error
//      if (gTriggerCamera.hCamera.p2pConnResult < 0)
//          break;
//
//      // get trigger result
//      if (TRIGGER_RECORDER_SUCCESSFUL <= gTriggerCamera.hCamera.p2pConnResult
//              && gTriggerCamera.hCamera.p2pConnResult <= TRIGGER_RECORDER_WRONG_PASSWROD)
//      {
//          // successful
//          if (TRIGGER_RECORDER_SUCCESSFUL == gTriggerCamera.hCamera.p2pConnResult)
//              usleep(100 * 1000); // 100ms
//
//          break;
//      }
//
//
//      usleep(50 * 1000);      // 50 ms
//  }
//
//  Camera_Deinit();
//
//  // exit thread
//  if (gTriggerCamera.hCamera.p2pConnResult < 0)
//      DBG_PRINT("%s: %s.\n", did, P2PErrorMsg[-gTriggerCamera.hCamera.p2pConnResult]);
//  else if (TRIGGER_RECORDER_SUCCESSFUL <= gTriggerCamera.hCamera.p2pConnResult
//          && gTriggerCamera.hCamera.p2pConnResult <= TRIGGER_RECORDER_WRONG_PASSWROD)
//      DBG_PRINT("%s: %s.\n", did, StarxErrorMsg[gTriggerCamera.hCamera.p2pConnResult]);
//  else
//      DBG_PRINT("%s: %s.\n", did, "You should NOT see this message! Please connect us!");
//
//
//  DBG_PRINT("trigger_record result = %d \n", gTriggerCamera.hCamera.p2pConnResult);
//
//  return NULL;
//}
//
//char ipcRecdata[512];
//
//void ipcRecord(const char *payload, int len)
//{
//  DBG_PRINT("ipcRecord payload = %s \n", payload);
//
//  //char *result = (char *)malloc(len + 1);
//  memset(ipcRecdata, 0, sizeof(ipcRecdata) );
//  memcpy(ipcRecdata, payload, len);
//  pthread_create(&ipc_record_thread, NULL, ipc_record_thread_routine, (void *)ipcRecdata);
//}
//#endif


/*
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
*/
/*
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
*/

//int main(int argc, char *argv[]) {
/*
void start_push_routine(){
    DBG_PRINT("ceres_util is running. \n");

    pthread_mutex_init(&curl_mutex, NULL);
    pthread_mutex_init(&email_mutex, NULL);
//
//#ifdef IPCAM_ENABLE
//  int APIVersion = STARX_GetAPIVersion();
//  DBG_PRINT("camera_record-P2P Version: %d.%d.%d.%d\n", (APIVersion & 0xFF000000) >> 24,
//              (APIVersion & 0x00FF0000) >> 16, (APIVersion & 0x0000FF00) >> 8,
//              (APIVersion & 0x000000FF) >> 0);
//#endif


    //// MQTT线程
    //pthread_create(&mqtt_sub_pid, NULL, mqtt_client_thread_routine, NULL);
    //while (g_run)
    //{
    //  if (0 == mMQTTConnected)
    //  {
    //      DBG_PRINT("waiting for mqtt connect\n");
    //      usleep(1 * 1000 * 1000);
    //  }
    //  else
    //      break;
    //}

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
*/
