#ifndef LIBIRCCLIENT_SESSION_H
#define LIBIRCCLIENT_SESSION_H
#ifndef IN_LIBIRCCLIENT_H
	#error This file should not be included directly; include just libircclient.h
#endif
#include <glib.h>
#include "libircclient_params.h"
struct irc_session {
	void *ctx;
	int options
	int lasterror
	char incoming_buffer[LIBIRC_BUFFER_SIZE];
	unsigned int incoming_offset;
	GAsyncQueue *outgoing_queue;
	int socket;
	int state;
	int motd_received;
	char *server;
	char *server_password;
	char *realname;
	char *username;
	char *nick;
	struct in6_addr local_address;
	struct irc_callbacks callbacks;
}
#endif /* LIBIRCCLIENT_SESSION_H */
