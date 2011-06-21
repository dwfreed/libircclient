#ifndef LIBIRCCLIENT_ERRORS_H
#define LIBIRCCLIENT_ERRORS_H
#ifndef IN_LIBIRCCLIENT_H
	#error This file should not be included directly; include just libircclient.h
#endif
#define LIBIRCCLIENT_ERR_OK			0
#define LIBIRCCLIENT_ERR_INVAL		1
#define LIBIRCCLIENT_ERR_RESOLV		2
#define LIBIRCCLIENT_ERR_SOCKET		3
#define LIBIRCCLIENT_ERR_CONNECT		4
#define LIBIRCCLIENT_ERR_CLOSED		5
#define LIBIRCCLIENT_ERR_NOMEM		6
#define LIBIRCCLIENT_ERR_STATE		12
#define LIBIRCCLIENT_ERR_TERMINATED	15
#define LIBIRCCLIENT_ERR_MAX			17
#endif /* LIBIRCCLIENT_ERRORS_H */
