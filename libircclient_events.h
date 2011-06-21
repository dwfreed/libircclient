#ifndef LIBIRCCLIENT_EVENTS_H
#define LIBIRCCLIENT_EVENTS_H
#ifndef IN_LIBIRCCLIENT_H
	#error This file should not be included directly; include just libircclient.h
#endif
typedef struct irc_session irc_session_struct;
typedef void (*irc_event_callback)(irc_session_struct *session, const char *event, const char *origin, const char **params, unsigned int count);
typedef void (*irc_eventcode_callback)(irc_session_struct *session, unsigned int event, const char *origin, const char **params, unsigned int count);
struct irc_callbacks {
	irc_event_callback event_channel;
	irc_event_callback event_channel_notice;
	irc_event_callback event_connect;
	irc_event_callback event_ctcp_action;
	irc_event_callback event_ctcp_rep;
	irc_event_callback event_ctcp_req;
	irc_event_callback event_invite;
	irc_event_callback event_join;
	irc_event_callback event_kick;
	irc_event_callback event_mode;
	irc_event_callback event_nick;
	irc_event_callback event_notice;
	irc_eventcode_callback event_numeric;
	irc_event_callback event_part;
	irc_event_callback event_privmsg;
	irc_event_callback event_quit;
	irc_event_callback event_topic;
	irc_event_callback event_umode;
	irc_event_callback event_unknown;
};
#endif /* LIBIRCCLIENT_EVENTS_H */
