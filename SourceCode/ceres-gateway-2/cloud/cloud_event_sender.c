#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <cJSON.h>
#include "queue.h"
#include "cloud_event_sender.h"

#define JS_DEBUG	1
//------------------------------------------------------------------------------


// Define 
#define CLOUD_SERVER_URL "http://atthings.us-east-1.elasticbeanstalk.com/matthings/event/addEvent"

//structure

CURL* m_pcurl;
//bool m_bSendDataToCloud;
pthread_t m_send_thread = NULL;


//---------------------------------------------------------------------------------
// 返回http header回調函數
static size_t header_this_callback(char  *ptr, size_t size, size_t nmemb, void *data)  
{  
	//CFoxCurl* req = static_cast<CFoxCurl*>(data);
	//return req->HeadWrite(ptr, size, nmemb);

	return 0;
}


//------------------------------------------------------------------------------
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t sz = (size * nmemb);
	char *begin = ptr;
	char *end = begin + sz;
	int len = 0;

	return len;
}

//------------------------------------------------------------------------------
static void *thread_proc_cmd(void *arg)
{
    struct timeval tv;
    int tick = 0;

    while(1)
    {
		pop(void)
    }

    return NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//Funtions

int JS_cloud_init()
{
	if( NULL == m_pcurl )
		return FALSE ;

	printf("JS_Init_CURL \n");

	//Init queue
	initstack();

	m_pcurl = curl_easy_init();
	curl_easy_setopt(m_pcurl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(m_pcurl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_pcurl, CURLOPT_URL, CLOUD_SERVER_URL);
	curl_easy_setopt(m_pcurl, CURLOPT_HEADERFUNCTION, header_this_callback );  
	curl_easy_setopt(m_pcurl, CURLOPT_FOLLOWLOCATION, 1);

	return TRUE ;
}

//------------------------------------------------------------------------------
static void *thread_proc_cmd(void *arg)
{
    struct timeval tv;
    int tick = 0;

    while(1)
    {
		//Send event to cloud server
    }

    return NULL;
}



//------------------------------------------------------------------------------
int  JS_cloud_start()
{
	struct curl_slist* pHeader = NULL;
	int err = 0;

#if JS_DEBUG
	printf("####  JS_cloud_start Enter \n");
#endif
	
    err = pthread_create(&m_send_thread, NULL, thread_proc_cmd, NULL);
    if(err)
    {
        return errno;
    }

#if JS_DEBUG
		printf("####  JS_cloud_start Leave \n");
#endif

	return 0;
}

//------------------------------------------------------------------------------
void  JS_cloud_stop()
{
#if JS_DEBUG
	printf("####  JS_cloud_stop Enter \n");
#endif


#if JS_DEBUG
	printf("####  JS_cloud_stop Leave\n");
#endif

}

int JS_add_event(jswevent* pEvent)
{

#if JS_DEBUG
	printf("####  JS_add_event Enter \n");
#endif

	if(pEvent == NULL)
		return 0;

	push(pEvent);
	printf("####  JS_add_event ary size = %d \n", get_count());

#if JS_DEBUG
	printf("####  JS_cloud_start Leave \n");
#endif

	return 1;
}

bool JS_enable_upload_data()
{
#if JS_DEBUG
	printf("####  JS_enable_upload_data Enter \n");
#endif

//packege data, Json format





#if JS_DEBUG
	printf("####  JS_enable_upload_data Leave \n");
#endif

	return 1;
}


//////////////////////////////////////////////////////
// sample

/*
	{
		"sensor_id": " 317815650"
	      	    "sensor_name" : " front door",
		  	    "device_type" : " JSW-PIR-B090",
		  	    "sensor_status" : "ON",
		  	    "device_did " : "WGXX-000281-ETRDX",
				"device_status " : "Arm",
		  		"items":[
					{
				 	    "sensor_id": " 317815650"
			      	    "sensor_name" : " front door",
				  	    "device_type" : " JSW-PIR-B090",
				  	    "sensor_status" : "ON",
						"location" : "Living room",
						"armlink" : "" ,
						"scenario_target" : "" ,
						"Schedule_list" : "",
						"Nat type" :  "1"   
			  	    }
			  	    .....
		  	    ]
	}
	
*/



