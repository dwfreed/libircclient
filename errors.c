#include "global.h"

int irc_errno(struct irc_session *session){
	return session->lasterror;
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
