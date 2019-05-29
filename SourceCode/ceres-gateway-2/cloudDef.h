/*
 * cloudDef.h
 *
 *  Created on: 2018/02/18
 *      Author: Apple
 */


#define CLOUD_MQTT_PORT   		3600
#define CLOUD_MAX_EVENT_CNT      120
#define KEEPALVE_TIME			  30	

char m_szToken[200];
char m_szAppID[40];
char m_szServerURL[100];
char m_szEnterprise[40];
char m_szDAReportURL[100];
char m_szMQTTHost[100];

//EU cloud Server

	#define EU_APP_ID 					"RWT3578Y-I2P8KZZH-HP54EJ9N"
	#define EU_CLOUD_TOKEN  			"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJFbnRlcnByaXNlSWQiOiJGRFAwVDIxQi0yRERUVEw1Ty04TERQN1ZFMiJ9.G6O06dXCdJO75ydFov6paIEG91KnPQ-MFGbAA6tKaGE"
	#define EU_MQTT_HOST     			"jcloud-backend-eu.eu-west-1.elasticbeanstalk.com"  
	#define EU_ENTERPRISE_ID 			"FDP0T21B-2DDTTL5O-8LDP7VE2"
	#define EU_SERVER_URL 				"http://jcloud-backend-eu.eu-west-1.elasticbeanstalk.com/matthings/event/addEvent"
	#define EU_DA_REPORT_URL			"http://jcloud-backend-eu.eu-west-1.elasticbeanstalk.com/matthings/smarthome/requestReportStateDA"
	#define EU_KEEPALIVE_URL			"http://jcloud-backend-eu.eu-west-1.elasticbeanstalk.com/jcloud/keepAliveData/updateDeviceKA"
	
	
// USA, JSW
	#define UA_APP_ID 					"R9200CBJ-SE19FLQW-5WA03KEH"
	#define UA_CLOUD_TOKEN  		    "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJFbnRlcnByaXNlSWQiOiJIS0U4N081WC1YUDBINUtWTC02Mjk1S0VNRCJ9.oNsyr3tt8sdG1kxZqhlzKLjnCaakVpnOqX6IQ22pGxQ"
	#define UA_MQTT_HOST     	    	"ec2-18-233-153-193.compute-1.amazonaws.com"  
	#define UA_ENTERPRISE_ID 			"FDP0T21B-2DDTTL5O-8LDP7VE2"
	#define UA_SERVER_URL 				"https://www.members-cloud.com/matthings/event/addEvent"
	#define UA_DA_REPORT_URL 			"https://www.members-cloud.com/matthings/smarthome/requestReportStateDA"
	#define UA_KEEPALIVE_URL 			"https://www.members-cloud.com/jcloud/keepAliveData/updateDeviceKA"

//AUBS
	#define ABUS_APP_ID					"P20LTL8T-RUD5L6EW-Q9DPK0C5"
	#define ABUS_CLOUD_TOKEN  		    "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJFbnRlcnByaXNlSWQiOiIyWVBIU0FaQy03STZWT0tOSy02U1U4UlozWCJ9.L_ECzUH0KNx9xZjignh1RWY3cwYunbD9FqcFaoum5YY"
	#define ABUS_MQTT_HOST     	    	"ssw-cloud-backend.eu-west-1.elasticbeanstalk.com"  
	#define ABUS_ENTERPRISE_ID 			"2YPHSAZC-7I6VOKNK-6SU8RZ3X"
	#define ABUS_SERVER_URL 			"http://ssw-cloud-backend.eu-west-1.elasticbeanstalk.com/matthings/event/addEvent"
	#define ABUS_DA_REPORT_URL 			"http://ssw-cloud-backend.eu-west-1.elasticbeanstalk.com/matthings/smarthome/requestReportStateDA"
	#define ABUS_KEEPALIVE_URL 			"http://ssw-cloud-backend.eu-west-1.elasticbeanstalk.com/jcloud/keepAliveData/updateDeviceKA"

	
	

