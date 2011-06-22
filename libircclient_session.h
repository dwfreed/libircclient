#ifndef LIBIRCCLIENT_SESSION_H
#define LIBIRCCLIENT_SESSION_H
#ifndef IN_LIBIRCCLIENT_H
	#error This file should not be included directly; include just libircclient.h
#endif
#include <glib.h>
#include <netinet/in.h>
#include "libircclient_params.h"
struct irc_session {
	void *ctx;
	int options;
	int last_error;
	char *incoming_buffer;
	unsigned int incoming_offset;
	GAsyncQueue *outgoing_queue;
	int socket;
	int state;
	int motd_received;
	char *server;
	char *server_password;
	char *nick;
	char *username;
	char *hostname;
	char *realname;
	struct in6_addr local_address;
	struct irc_callbacks callbacks;
};
#endif /* LIBIRCCLIENT_SESSION_H */
