/*
 * ceres.c
 *
 *  Created on: 2014年1月26日
 *      Author: yulin
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "mongoose.h"


#define MAX_USER_LEN  20
#define MAX_SESSIONS 5
#define SESSION_TTL 60 * 5

extern char gDID[256];
extern char gPassword[256];

void generate_session_id(char *buf, const char *random, const char *user);

const char *authorize_url = "/authorize";
const char *login_url = "/login.html";
const char *index_url = "/index.html";

// Describes web session.
struct session {
  char session_id[33];      // Session ID, must be unique
  char random[20];          // Random data used for extra user validation
  char user[MAX_USER_LEN];  // Authenticated user
  time_t expire;            // Expiration timestamp, UTC
};

struct session sessions[MAX_SESSIONS];  // Current sessions

// Protects messages, sessions, last_message_id
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// webserver runtime thread
pthread_t webserver_thread;
void *webserver_sub_thread_result;

void my_strlcpy(char *dst, const char *src, size_t len) {
  strncpy(dst, src, len);
  dst[len - 1] = '\0';
}

// Get session object for the connection. Caller must hold the lock.
struct session *get_session(const struct mg_connection *conn) {
  int i;
  char session_id[33]={0};
  time_t now = time(0);

  mg_parse_header(mg_get_header(conn, "Cookie"), "session", session_id, sizeof(session_id));

  for (i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].expire != 0 &&
        sessions[i].expire > now &&
        strncmp(sessions[i].session_id, session_id, strlen(sessions[i].session_id)) == 0) {
    	sessions[i].expire = now + SESSION_TTL;
      break;
    }
  }
  return i == MAX_SESSIONS ? NULL : &sessions[i];
}

void redirect_to_login(struct mg_connection *conn) {
  mg_printf(conn, "HTTP/1.1 302 Found\r\n"
      "Set-Cookie: original_url=%s\r\n"
      "Location: %s\r\n\r\n",
      conn->uri, login_url);
}

int ws_logout(struct mg_connection *conn){
    int i;
    char session_id[33]={0};
    time_t now = time(NULL);

    mg_parse_header(mg_get_header(conn, "Cookie"), "session", session_id, sizeof(session_id));

    for (i = 0; i < MAX_SESSIONS; i++) {
        if (strcmp(sessions[i].session_id, session_id) == 0) {
            sessions[i].expire = 0;
            break;
        }
    }
    redirect_to_login(conn);

    return 0;
}

int is_authorized(const struct mg_connection *conn);
int ws_index(struct mg_connection *conn) {

	if(!is_authorized(conn)) {
		redirect_to_login(conn);
		return -1;
	}

	char session_id[33]={0};
	mg_parse_header(mg_get_header(conn, "Cookie"), "session", session_id, sizeof(session_id));

    mg_printf(conn, "HTTP/1.1 302 Found\r\n"
            "Set-Cookie: session=%s; max-age=3600; http-only\r\n"  // Session ID
            "Set-Cookie: original_url=/; max-age=0\r\n"  // Delete original_url
            "Location: /index.html\r\n\r\n",
            session_id);

	return 0;
}

// Return 1 if request is authorized, 0 otherwise.
int is_authorized(const struct mg_connection *conn) {
	  struct session *session;
  char valid_id[33];
  int authorized = 0;

  // Always authorize accesses to login page and to authorize URI
  if (!strcmp(conn->uri, login_url) ||
      !strcmp(conn->uri, authorize_url)) {
    return 1;
  }

  pthread_mutex_lock(&mutex);
  if ((session = get_session(conn)) != NULL) {
    generate_session_id(valid_id, session->random, session->user);
    if (strcmp(valid_id, session->session_id) == 0) {
      session->expire = time(0) + SESSION_TTL;
      authorized = 1;
    }
  }
  pthread_mutex_unlock(&mutex);

//  return authorized;
  return 1;		// for debug
}

int check_password(const char* user, const char* password) {
    if(strcmp("admin", user) == 0 && strcmp(gPassword, password) == 0){
	    return 1;
	}else{
	    return 0;
	}
}

// Allocate new session object
struct session *new_session(void) {
  int i;
  time_t now = time(NULL);
  pthread_mutex_lock(&mutex);
  for (i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].expire == 0 || sessions[i].expire < now) {
      sessions[i].expire = time(0) + SESSION_TTL;
      break;
    }
  }
  pthread_mutex_unlock(&mutex);
  return i == MAX_SESSIONS ? NULL : &sessions[i];
}

// Generate session ID. buf must be 33 bytes in size.
// Note that it is easy to steal session cookies by sniffing traffic.
// This is why all communication must be SSL-ed.
void generate_session_id(char *buf, const char *random,
                                const char *user) {
  mg_md5(buf, random, user, NULL);
}

// A handler for the /authorize endpoint.
// Login page form sends user name and password to this endpoint.
void authorize(struct mg_connection *conn) {
	char user[MAX_USER_LEN]={0}, password[MAX_USER_LEN]={0};
  struct session *session;

  // Fetch user name and password.
    mg_get_var(conn, "user", user, MAX_USER_LEN);
    mg_get_var(conn, "password", password, MAX_USER_LEN);

  if (check_password(user, password) && (session = new_session()) != NULL) {
    // Authentication success:
    //   1. create new session
    //   2. set session ID token in the cookie
    //   3. remove original_url from the cookie - not needed anymore
    //   4. redirect client back to the original URL
    //
    // The most secure way is to stay HTTPS all the time. However, just to
    // show the technique, we redirect to HTTP after the successful
    // authentication. The danger of doing this is that session cookie can
    // be stolen and an attacker may impersonate the user.
    // Secure application must use HTTPS all the time.
    my_strlcpy(session->user, user, sizeof(session->user));
    snprintf(session->random, sizeof(session->random), "%d", rand());
    generate_session_id(session->session_id, session->random, session->user);
    // send_server_message("<%s> joined", session->user);
    mg_printf(conn, "HTTP/1.1 302 Found\r\n"
        "Set-Cookie: session=%s; max-age=3600; http-only\r\n"  // Session ID
        "Set-Cookie: user=%s; max-age=604800\r\n"  // Set user, needed by Javascript code
        "Set-Cookie: original_url=/; max-age=0\r\n"  // Delete original_url
        "Location: /index.html\r\n\r\n",
        session->session_id, session->user);

  } else {
    // Authentication failure, redirect to login.
    redirect_to_login(conn);
  }
}

