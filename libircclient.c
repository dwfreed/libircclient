#include "global.h"

struct irc_session *irc_create_session(struct irc_callbacks *callbacks){
	g_thread_init(NULL);
	struct irc_session *session = (struct irc_session *)calloc(1, sizeof(struct irc_session));
	if( !session ){
		return NULL;
	}
	session->socket = -1;
	session->outgoing_queue = g_async_queue_new_full(g_free);
	memcpy(&session->callbacks, callbacks, sizeof(struct irc_callbacks));
	return session;
}

void irc_destroy_session(struct irc_session *session){
	if( session->socket >= 0 ){
		close(session->socket);
		session->socket = -1;
	}
	g_async_queue_unref(session->outgoing_queue);
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
		char *str_port = g_strdup_printf("%hu", port);
		struct addrinfo hints, *result;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
		if( !getaddrinfo(server, str_port, &hints, &result) ){
			g_free(str_port);
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
			g_free(str_port);
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
		char *str_port = g_strdup_printf("%hu", port);
		struct addrinfo hints, *result;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		hints.ai_flags = AI_NUMERICSERV | AI_V4MAPPED | AI_ALL | AI_ADDRCONFIG;
		if( !getaddrinfo(server, str_port, &hints, &result) ){
			g_free(str_port);
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
			g_free(str_port);
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
