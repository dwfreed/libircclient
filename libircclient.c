#include "global.h"

struct irc_session *irc_create_session(struct irc_callbacks *callbacks){
	g_thread_init(NULL);
	struct irc_session *session = (struct irc_session *)calloc(1, sizeof(struct irc_session));
	if( !session ){
		return NULL;
	}
	session->socket = -1;
	session->outgoing_queue = g_async_queue_new_full(free);
	memcpy(&session->callbacks, callbacks, sizeof(struct irc_callbacks));
	return session;
}

void irc_destroy_session(struct irc_session *session){
	if( session->socket >= 0 ){
		close(session->socket);
		session->socket = -1;
	}
	g_async_queue_unref(session->outgoing_queue);
	if( session->incoming_buffer ){
		free(session->incoming_buffer);
	}
	if( session->server ){
		free(session->server);
	}
	if( session->server_password ){
		free(session->server_password);
	}
	if( session->nick ){
		free(session->nick);
	}
	if( session->username ){
		free(session->username);
	}
	if( session->hostname ){
		free(session->hostname);
	}
	if( session->realname ){
		free(session->realname);
	}
	free(session);
}

int irc_connect(struct irc_session *session, const char *server, unsigned short port, const char *server_password, const char *nick, const char *username, const char *realname){
	struct sockaddr_in saddr;
	if( !server || !nick ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	if( session->state != LIBIRCCLIENT_STATE_INIT ){
		session->last_error = LIBIRCCLIENT_ERR_STATE;
		return 1;
	}
	if( username ){
		session->username = strdup(username);
	}
	if( server_password ){
		session->server_password = strdup(server_password);
	}
	if( realname ){
		session->realname = strdup(realname);
	}
	session->nick = strdup(nick);
	session->server = strdup(server);
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	if( (session->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1 || fcntl(session->socket, F_SETFL, fcntl(session->socket, F_GETFL, 0) | O_NONBLOCK) ){
		session->last_error = LIBIRCCLIENT_ERR_SOCKET;
		return 1;
	}
	if( !inet_aton(server, &saddr.sin_addr) ){
		char *str_port;
		if( asprintf(&str_port, "%hu", port) < 0 ){
			session->last_error = LIBIRCCLIENT_ERR_NOMEM;
			return 1;
		}
		struct addrinfo hints, *result;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
		if( !getaddrinfo(server, str_port, &hints, &result) ){
			free(str_port);
			struct addrinfo *rp;
			char im_done = FALSE;
			for( rp = result; rp != NULL; rp = rp->ai_next ){
				while( TRUE ){
					if( connect(session->socket, rp->ai_addr, rp->ai_addrlen) ){
						if( errno == EINTR ){
							continue;
						}
						if( errno != EINPROGRESS && errno != EWOULDBLOCK ){
							break;
						}
						im_done = TRUE;
						break;
					}
				}
				if( im_done ){
					break;
				}
			}
			freeaddrinfo(result);
			if( !im_done ){
				session->last_error = LIBIRCCLIENT_ERR_CONNECT;
				return 1;
			}
		} else {
			free(str_port);
			session->last_error = LIBIRCCLIENT_ERR_RESOLV;
			return 1;
		}
	} else {
		while( TRUE ){
			if( connect(session->socket, (struct sockaddr *)&saddr, sizeof(saddr)) ){
				if( errno == EINTR ){
					continue;
				}
				if( errno != EINPROGRESS && errno != EWOULDBLOCK ){
					session->last_error = LIBIRCCLIENT_ERR_CONNECT;
					return 1;
				}
				break;
			}
		}
	}
	session->state = LIBIRCCLIENT_STATE_CONNECTING;
	session->motd_received = 0;
	return 0;
}

int irc_connect6(struct irc_session *session, const char *server, unsigned short port, const char *server_password, const char *nick, const char *username, const char *realname){
	struct sockaddr_in6 saddr;
	if( !server || !nick ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	if( session->state != LIBIRCCLIENT_STATE_INIT ){
		session->last_error = LIBIRCCLIENT_ERR_STATE;
		return 1;
	}
	if( username ){
		session->username = strdup(username);
	}
	if( server_password ){
		session->server_password = strdup(server_password);
	}
	if( realname ){
		session->realname = strdup(realname);
	}
	session->nick = strdup(nick);
	session->server = strdup(server);
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin6_family = AF_INET6;
	saddr.sin6_port = htons(port);
	if( (session->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1 || fcntl(session->socket, F_SETFL, fcntl(session->socket, F_GETFL, 0) | O_NONBLOCK) ){
		session->last_error = LIBIRCCLIENT_ERR_SOCKET;
		return 1;
	}
	if( inet_pton(AF_INET6, server, &saddr.sin6_addr) < 1 ){
		char *str_port;
		if( asprintf(&str_port, "%hu", port) < 0 ){
			session->last_error = LIBIRCCLIENT_ERR_NOMEM;
			return 1;
		}
		struct addrinfo hints, *result;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		hints.ai_flags = AI_NUMERICSERV | AI_V4MAPPED | AI_ALL | AI_ADDRCONFIG;
		if( !getaddrinfo(server, str_port, &hints, &result) ){
			free(str_port);
			struct addrinfo *rp;
			char im_done = FALSE;
			for( rp = result; rp != NULL; rp = rp->ai_next ){
				while( TRUE ){
					if( connect(session->socket, rp->ai_addr, rp->ai_addrlen) ){
						if( errno == EINTR ){
							continue;
						}
						if( errno != EINPROGRESS && errno != EWOULDBLOCK ){
							break;
						}
						im_done = TRUE;
						break;
					}
				}
				if( im_done ){
					break;
				}
			}
			freeaddrinfo(result);
			if( !im_done ){
				session->last_error = LIBIRCCLIENT_ERR_CONNECT;
				return 1;
			}
		} else {
			free(str_port);
			session->last_error = LIBIRCCLIENT_ERR_RESOLV;
			return 1;
		}
	} else {
		while( TRUE ){
			if( connect(session->socket, (struct sockaddr *)&saddr, sizeof(saddr)) ){
				if( errno == EINTR ){
					continue;
				}
				if( errno != EINPROGRESS && errno != EWOULDBLOCK ){
					session->last_error = LIBIRCCLIENT_ERR_CONNECT;
					return 1;
				}
				break;
			}
		}
	}
	session->state = LIBIRCCLIENT_STATE_CONNECTING;
	session->motd_received = 0;
	return 0;
}

void irc_disconnect(struct irc_session *session){
	if( session->socket > -1 ){
		close(session->socket);
	}
	session->socket = -1;
	session->state = LIBIRCCLIENT_STATE_INIT;
}

int irc_is_connected(struct irc_session *session){
	return session->state == LIBIRCCLIENT_STATE_CONNECTED || session->state == LIBIRCCLIENT_STATE_CONNECTING;
}

static int irc_add_select_descriptors(struct irc_session *session, fd_set *in_set, fd_set *out_set, int *maxfd){
	if( session->state == LIBIRCCLIENT_STATE_INIT || session->state == LIBIRCCLIENT_STATE_DISCONNECTED ){
		session->last_error = LIBIRCCLIENT_ERR_STATE;
		return 1;
	}
	if( session->state == LIBIRCCLIENT_STATE_CONNECTING ){
		FD_SET(session->socket, out_set);
		if( session->socket > *maxfd ){
			*maxfd = session->socket;
		}
	} else if( session->state == LIBIRCCLIENT_STATE_CONNECTED ){
		FD_SET(session->socket, in_set);
		if( g_async_queue_length(session->outgoing_queue) > 0 ){
			FD_SET(session->socket, out_set);
		}
		if( session->socket > *maxfd ){
			*maxfd = session->socket;
		}
	}
	return 0;
}

static int irc_findcrlf(const char *buffer, int length){
	int offset;
	for( offset = 0; offset < length - 1; ++offset ){
		if( buffer[offset] == '\r' && buffer[offset + 1] == '\n' ){
			return offset + 2;
		}
	}
	return 0;
}

static void irc_process_incoming_data(struct irc_session *session, size_t process_length){
	char buffer[511], *p, *s, *command = 0, *prefix = 0;
	const char **params;
	int code = 0, param_index = 0;
	memcpy(buffer, session->incoming_buffer, process_length);
	buffer[process_length] = '\0';
	params = (const char **)calloc(16, sizeof(char *));
	p = buffer;
	if( buffer[0] == ':' ){
		for( ; *p && *p != ' '; ++p );
		*p++ = '\0';
		prefix = buffer + 1;
		if( session->options & LIBIRCCLIENT_OPTION_STRIPNICKS ){
			for( s = prefix; *s; ++s ){
				if( *s == '@' || *s == '!' ){
					*s = '\0';
					break;
				}
			}
		}
	}
	if( isdigit(p[0]) && isdigit(p[1]) && isdigit(p[2]) ){
		p[3] = '\0';
		code = atoi(p);
		p += 4;
	} else {
		s = p;
		for( ; *p && *p != ' '; ++p );
		*p++ = '\0';
		command = s;
	}
	while( *p && param_index < 16 ){
		if( *p == ':' ){
			params[param_index++] = ++p;
			break;
		}
		for( s = p; *p && *p != ' '; ++p );
		params[param_index++] = s;
		if( *p ){
			*p++ = '\0';
		}
	}
	if( code ){
		if( (code == IRC_RFC_RPL_ENDOFMOTD || code == IRC_RFC_ERR_NOMOTD) && !session->motd_received ){
			session->motd_received = 1;
			irc_cmd_whois(session, session->nick);
			if( session->callbacks.event_connect ){
				session->callbacks.event_connect(session, "CONNECT", prefix, params, param_index);
			}
		} else if( code == IRC_RFC_RPL_WHOISUSER ){
			if(!strcmp(session->nick, params[1])){
				if(session->hostname){
					free(session->hostname);
				}
				if(session->username){
					free(session->username);
				}
				session->hostname = strdup(params[3]);
				session->username = strdup(params[2]);
			}
		}
		if( session->callbacks.event_numeric ){
			session->callbacks.event_numeric(session, code, prefix, params, param_index);
		}
	} else {
		if( !strcmp(command, "PING") && params[0] ){
			irc_send_raw(session, "PONG %s", params[0]);
		} else if( !strcmp(command, "NICK") ){
			char *nick = irc_target_get_nick(prefix);
			if( !strcmp(nick, session->nick) && params[0] ){
				free(session->nick);
				session->nick = strdup(params[0]);
			}
			free(nick);
			if( session->callbacks.event_nick ){
				session->callbacks.event_nick(session, command, prefix, params, param_index);
			}
		} else if( !strcmp(command, "QUIT") ){
			if( session->callbacks.event_quit ){
				session->callbacks.event_quit(session, command, prefix, params, param_index);
			}
		} else if( !strcmp(command, "JOIN") ){
			if( session->callbacks.event_join ){
				session->callbacks.event_join(session, command, prefix, params, param_index);
			}
		} else if( !strcmp(command, "PART") ){
			if( session->callbacks.event_part ){
				session->callbacks.event_part(session, command, prefix, params, param_index);
			}
		} else if( !strcmp(command, "MODE") ){
			if( params[0] && !strcmp(params[0], session->nick) ){
				params[0] = params[1];
				param_index = 1;
				if( session->callbacks.event_umode ){
					session->callbacks.event_umode(session, command, prefix, params, param_index);
				}
			} else {
				if( session->callbacks.event_mode ){
					session->callbacks.event_mode(session, command, prefix, params, param_index);
				}
			}
		} else if( !strcmp(command, "TOPIC") ){
			if( session->callbacks.event_topic ){
				session->callbacks.event_topic(session, command, prefix, params, param_index);
			}
		} else if( !strcmp(command, "KICK") ){
			if( session->callbacks.event_kick ){
				session->callbacks.event_kick(session, command, prefix, params, param_index);
			}
		} else if( !strcmp(command, "PRIVMSG") ){
			int message_length = strlen(params[1]);
			if( params[1][0] == '\1' && params[1][message_length - 1] == '\1' ){
				char ctcp_buffer[512];
				message_length -= 2;
				memcpy(ctcp_buffer, params[1] + 1, message_length);
				ctcp_buffer[message_length] = '\0';
				params[1] = ctcp_buffer;
				if( !strcasecmp(params[1], "ACTION ") && session->callbacks.event_ctcp_action ){
					params[1] = params[1] + 7;
					param_index = 2;
					session->callbacks.event_ctcp_action(session, "ACTION", prefix, params, param_index);
				} else {
					params[0] = params[1];
					param_index = 1;
					if( session->callbacks.event_ctcp_req ){
						session->callbacks.event_ctcp_req(session, "CTCP", prefix, params, param_index);
					}
				}
			} else if( !strcasecmp(params[0], session->nick) ){
				if( session->callbacks.event_privmsg ){
					session->callbacks.event_privmsg(session, command, prefix, params, param_index);
				}
			} else {
				if( session->callbacks.event_channel ){
					session->callbacks.event_channel(session, command, prefix, params, param_index);
				}
			}
		} else if( !strcmp(command, "NOTICE") ){
			int message_length = strlen(params[1]);
			if( params[1][0] == '\1' && params[1][message_length - 1] == '\1' ){
				char ctcp_buffer[512];
				message_length -= 2;
				memcpy(ctcp_buffer, params[1] + 1, message_length);
				ctcp_buffer[message_length] = '\0';
				params[0] = ctcp_buffer;
				param_index = 1;
				if( session->callbacks.event_ctcp_rep ){
					session->callbacks.event_ctcp_rep(session, "CTCP", prefix, params, param_index);
				}
			} else if( !strcasecmp(params[0], session->nick) ){
				if( session->callbacks.event_notice ){
					session->callbacks.event_notice(session, command, prefix, params, param_index);
				}
			} else {
				if( session->callbacks.event_channel_notice ){
					session->callbacks.event_channel_notice(session, command, prefix, params, param_index);
				}
			}
		} else if( !strcmp(command, "INVITE") ){
			if( session->callbacks.event_invite ){
				session->callbacks.event_invite(session, command, prefix, params, param_index);
			}
		} else {
			if( session->callbacks.event_unknown ){
				session->callbacks.event_unknown(session, command, prefix, params, param_index);
			}
		}
	}
	free(params);
}

static int irc_process_select_descriptors(struct irc_session *session, fd_set *in_set, fd_set *out_set){
	if( session->state == LIBIRCCLIENT_STATE_INIT || session->state == LIBIRCCLIENT_STATE_DISCONNECTED ){
		session->last_error = LIBIRCCLIENT_ERR_STATE;
		return 1;
	}
	if( session->state == LIBIRCCLIENT_STATE_CONNECTING && FD_ISSET(session->socket, out_set) ){
		struct sockaddr_storage laddr, saddr;
		socklen_t llen = sizeof(laddr);
		socklen_t slen = llen;
		if( getsockname(session->socket, (struct sockaddr *)&laddr, &llen) || getpeername(session->socket, (struct sockaddr *)&saddr, &slen) ){
			session->last_error = LIBIRCCLIENT_ERR_CONNECT;
			session->state = LIBIRCCLIENT_STATE_DISCONNECTED;
			return 1;
		}
		if( saddr.ss_family == AF_INET ){
			memcpy(&session->local_address, &((struct sockaddr_in *)&laddr)->sin_addr, sizeof(struct in_addr));
			if( session->options & LIBIRCCLIENT_OPTION_DEBUG ){
				char local_address[INET_ADDRSTRLEN];
				fprintf(stderr, "[DEBUG] Detected local address: %s\n", inet_ntop(AF_INET, &session->local_address, local_address, INET_ADDRSTRLEN));
			}
		} else if( saddr.ss_family == AF_INET6 ){
			memcpy(&session->local_address, &((struct sockaddr_in6 *)&laddr)->sin6_addr, sizeof(struct in6_addr));
			if( session->options & LIBIRCCLIENT_OPTION_DEBUG ){
				char local_address[INET6_ADDRSTRLEN];
				fprintf(stderr, "[DEBUG] Detected local address: %s\n", inet_ntop(AF_INET6, &session->local_address, local_address, INET6_ADDRSTRLEN));
			}
		}
		session->state = LIBIRCCLIENT_STATE_CONNECTED;
		if( session->server_password ){
			irc_send_raw(session, "PASS %s", session->server_password);
		}
		irc_send_raw(session, "NICK %s", session->nick);
		if( !session->username ){
			session->username = "nobody";
		}
		if( !session->realname ){
			session->realname = "noname";
		}
		irc_send_raw(session, "USER %s unknown unknown :%s", session->username, session->realname);
	}
	if( session->state != LIBIRCCLIENT_STATE_CONNECTED ){
		return 1;
	}
	if( FD_ISSET(session->socket, in_set) ){
		int length, readable_length, offset;
		unsigned int amount;
		ioctl(session->socket, FIONREAD, &readable_length);
		session->incoming_buffer = realloc(session->incoming_buffer, session->incoming_offset + readable_length + 1);
		memset(session->incoming_buffer + session->incoming_offset, 0, readable_length + 1);
		amount = readable_length;
		while( (length = recv(session->socket, session->incoming_buffer + session->incoming_offset, amount, 0)) < 0 && errno == EINTR ){
			continue;
		}
		if( length <= 0 ){
			session->last_error = length == 0 ? LIBIRCCLIENT_ERR_CLOSED : LIBIRCCLIENT_ERR_TERMINATED;
			session->state = LIBIRCCLIENT_STATE_DISCONNECTED;
			return 1;
		}
		session->incoming_offset += length;
		while( (offset = irc_findcrlf(session->incoming_buffer, session->incoming_offset)) > 0 ){
			if( session->options & LIBIRCCLIENT_OPTION_DEBUG ){
				int i;
				printf("RECV: ");
				for( i = 0; i < offset; ++i ){
					printf("%c", session->incoming_buffer[i]);
				}
			}
			irc_process_incoming_data(session, offset - 2);
			if( session->incoming_offset - offset ){
				memmove(session->incoming_buffer, session->incoming_buffer + offset, session->incoming_offset - offset);
			}
			session->incoming_offset -= offset;
			session->incoming_buffer = realloc(session->incoming_buffer, session->incoming_offset);
		}
	}
	if( FD_ISSET(session->socket, out_set) ){
		int queue_length = g_async_queue_length(session->outgoing_queue);
		if( session->options & LIBIRCCLIENT_OPTION_DEBUG ){
			printf("[DEBUG] queue_length: %d\n", queue_length);
		}
		while(queue_length > 0){
			int length;
			char *outgoing_message = g_async_queue_pop(session->outgoing_queue);
			while( (length = send(session->socket, outgoing_message, strlen(outgoing_message), 0)) < 0 && errno == EINTR ){
				continue;
			}
			if( length <= 0 ){
				session->last_error = length == 0 ? LIBIRCCLIENT_ERR_CLOSED : LIBIRCCLIENT_ERR_TERMINATED;
				session->state = LIBIRCCLIENT_STATE_DISCONNECTED;
				return 1;
			}
			if( session->options & LIBIRCCLIENT_OPTION_DEBUG ){
				printf("SEND: %s", outgoing_message);
			}
			free(outgoing_message);
			queue_length = g_async_queue_length(session->outgoing_queue);
		}
	}
	return 0;
}

int irc_run(struct irc_session *session){
	if( session->state != LIBIRCCLIENT_STATE_CONNECTING ){
		session->last_error = LIBIRCCLIENT_ERR_STATE;
		return 1;
	}
	while( irc_is_connected(session) ){
		fd_set in_set, out_set;
		int maxfd = 0;
		struct timeval wait_time = {0L, 1000L};
		FD_ZERO(&in_set);
		FD_ZERO(&out_set);
		irc_add_select_descriptors(session, &in_set, &out_set, &maxfd);
		if( select(maxfd + 1, &in_set, &out_set, 0, &wait_time) < 0 ){
			if( errno == EINTR ){
				continue;
			}
			session->last_error = LIBIRCCLIENT_ERR_TERMINATED;
			return 1;
		}
		if( irc_process_select_descriptors(session, &in_set, &out_set) ){
			return 1;
		}
	}
	return 0;
}

int irc_send_raw(struct irc_session *session, const char *format, ...){
	if( session->state != LIBIRCCLIENT_STATE_CONNECTED ){
		session->last_error = LIBIRCCLIENT_ERR_STATE;
		return 1;
	}
	char *new_format = strdup(format), *command;
	int format_length = strlen(format);
	va_list va_arg_list;
	new_format = (char *)realloc(new_format, format_length + 3);
	strcat(new_format, "\r\n");
	va_start(va_arg_list, format);
	if( vasprintf(&command, new_format, va_arg_list) < 0 ){
		session->last_error = LIBIRCCLIENT_ERR_NOMEM;
		free(new_format);
		return 1;
	}
	va_end(va_arg_list);
	g_async_queue_push(session->outgoing_queue, command);
	free(new_format);
	return 0;
}

int irc_cmd_channel_mode(struct irc_session *session, const char *channel, const char *mode){
	if( !channel ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
	}
	if( mode ){
		return irc_send_raw(session, "MODE %s %s", channel, mode);
	} else {
		return irc_send_raw(session, "MODE %s", channel);
	}
}

int irc_cmd_ctcp_reply(struct irc_session *session, const char *nick, const char *reply){
	if( !nick || !reply ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "NOTICE %s :\1%s\1", nick, reply);
}

int irc_cmd_ctcp_request(struct irc_session *session, const char *nch, const char *request){
	if( !nch || !request ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "PRIVMSG %s :\1%s\1", nch, request);
}

int irc_cmd_invite(struct irc_session *session, const char *nick, const char *channel){
	if( !channel || !nick ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "INVITE %s %s", nick, channel);
}

int irc_cmd_join(struct irc_session *session, const char *channel, const char *key){
	if( !channel ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	if( key ){
		return irc_send_raw(session, "JOIN %s :%s", channel, key);
	} else {
		return irc_send_raw(session, "JOIN %s", channel);
	}
}

int irc_cmd_kick(struct irc_session *session, const char *nick, const char *channel, const char *reason){
	if( !channel || !nick ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	if( reason ){
		return irc_send_raw(session, "KICK %s %s :%s", channel, nick, reason);
	} else {
		return irc_send_raw(session, "KICK %s %s", channel, nick);
	}
}

int irc_cmd_list(struct irc_session *session, const char *channel){
	if( channel ){
		return irc_send_raw(session, "LIST %s", channel);
	} else {
		return irc_send_raw(session, "LIST");
	}
}

int irc_cmd_me(struct irc_session *session, const char *nch, const char *text){
	if( !nch || !text ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "PRIVMSG %s :\1ACTION %s\1", nch, text);
}

int irc_cmd_msg(struct irc_session *session, const char *nch, const char *text){
	if( !nch || !text ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	char *temp_text = strdup(text), *new_text = temp_text;
	int length = strlen(new_text), max_text_length = 512 - (strlen(session->nick) + strlen(session->username) + strlen(session->hostname) + strlen(nch) + 17), ret_val = 0;
	while( length > 0 ){
		ret_val = irc_send_raw(session, "PRIVMSG %s :%s", nch, new_text);
		new_text = &new_text[max_text_length];
		length -= max_text_length;
	}
	free(temp_text);
	return ret_val;
}

int irc_cmd_msg_to(struct irc_session *session, const char *channel, const char *to, const char *text){
	if( !channel || !to || !text ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	char *new_text;
	if( asprintf(&new_text, "%s: %s", to, text) < 0 ){
		session->last_error = LIBIRCCLIENT_ERR_NOMEM;
		return 1;
	}
	int ret_val = irc_cmd_msg(session, channel, new_text);
	free(new_text);
	return ret_val;
}

int irc_cmd_names(struct irc_session *session, const char *channel){
	if( !channel ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "NAMES %s", channel);
}

int irc_cmd_nick(struct irc_session *session, const char *newnick){
	if( !newnick ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "NICK %s", newnick);
}

int irc_cmd_notice(struct irc_session *session, const char *nch, const char *text){
	if( !nch || !text ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "NOTICE %s :%s", nch, text);
}

int irc_cmd_part(struct irc_session *session, const char *channel, const char *reason){
	if( !channel ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	if( reason ){
		return irc_send_raw(session, "PART %s :%s", channel, reason);
	} else {
		return irc_send_raw(session, "PART %s", channel);
	}
}

int irc_cmd_quit(struct irc_session *session, const char *reason){
	if( reason ){
		return irc_send_raw(session, "QUIT :%s", reason);
	} else {
		return irc_send_raw(session, "QUIT");
	}
}

int irc_cmd_topic(struct irc_session *session, const char *channel, const char *topic){
	if( !channel ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	if( topic ){
		return irc_send_raw(session, "TOPIC %s :%s", channel, topic);
	} else {
		return irc_send_raw(session, "TOPIC %s", channel);
	}
}

int irc_cmd_user_mode(struct irc_session *session, const char *mode){
	if( mode ){
		return irc_send_raw(session, "MODE %s %s", session->nick, mode);
	} else {
		return irc_send_raw(session, "MODE %s", session->nick);
	}
}

int irc_cmd_whois(struct irc_session *session, const char *nick){
	if( !nick ){
		session->last_error = LIBIRCCLIENT_ERR_INVAL;
		return 1;
	}
	return irc_send_raw(session, "WHOIS %1$s %1$s", nick);
}

char *irc_target_get_nick(const char *target){
	char *split_point = strstr(target, "!");
	if( split_point ){
		return strndup(target, split_point - target);
	} else {
		return strdup(target);
	}
}

char *irc_target_get_username(const char *target){
	char *split_point1 = strstr(target, "!");
	char *split_point2 = strstr(target, "@");
	if( split_point1 && split_point2 ){
		return strndup(split_point1 + 1, split_point2 - split_point1 - 1);
	} else if( split_point2 ){
		return strndup(target, split_point2 - target);
	} else if( split_point1 ){
		return strdup(split_point1 + 1);
	} else {
		return strdup(target);
	}
}

char *irc_target_get_hostname(const char *target){
	char *split_point = strstr(target, "@");
	if( split_point ){
		return strdup(split_point + 1);
	} else {
		return strdup(target);
	}
}

void irc_set_ctx(struct irc_session *session, void *ctx){
	session->ctx = ctx;
}

void *irc_get_ctx(struct irc_session *session){
	return session->ctx;
}

int irc_errno(struct irc_session *session){
	return session->last_error;
}

const char *irc_strerror(int ircerrno){
	static const char *libircclient_strerror[LIBIRCCLIENT_ERR_MAX] = {
		"No error",
		"Invalid argument",
		"Host not resolved",
		"Socket error",
		"Could not connect",
		"Remove connection closed",
		"Out of memory",
		"Could not accept new connection",
		"Object not found",
		"Could not DCC send this object",
		"Read error",
		"Write error",
		"Illegal operation for this state",
		"Timeout error",
		"Could not open file",
		"IRC session terminated",
		"IPv6 not supported"
	};
	if( ircerrno >= 0 && ircerrno < LIBIRCCLIENT_ERR_MAX )
		return libircclient_strerror[ircerrno];
	else
		return "Invalid irc_errno value";
}

void irc_option_set(struct irc_session *session, unsigned int option){
	session->options |= option;
}

void irc_option_reset(struct irc_session *session, unsigned int option){
	session->options &= ~option;
}
