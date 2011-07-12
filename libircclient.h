#ifndef LIBIRCCLIENT_H
#define LIBIRCCLIENT_H
#ifdef __cplusplus
extern "C" {
#endif
#define IN_LIBIRCCLIENT_H
#include "libircclient_options.h"
#include "libircclient_events.h"
#include "libircclient_session.h"
#include "libircclient_errors.h"
#include "libircclient_rfcnumerics.h"
struct irc_session *irc_create_session(struct irc_callbacks *callbacks) __attribute__((__nonnull__));
void irc_destroy_session(struct irc_session *session) __attribute__((__nonnull__));
int irc_connect(struct irc_session *session, const char *server, unsigned short port, const char *server_password, const char *nick, const char *username, const char *realname) __attribute__((__nonnull__(1, 2, 5)));
int irc_connect6(struct irc_session *session, const char *server, unsigned short port, const char *server_password, const char *nick, const char *username, const char *realname) __attribute__((__nonnull__(1, 2, 5)));
void irc_disconnect(struct irc_session *session) __attribute__((__nonnull__));
int irc_is_connected(struct irc_session *session) __attribute__((__nonnull__));
int irc_run(struct irc_session *session) __attribute__((__nonnull__));
int irc_send_raw(struct irc_session *session, const char *format, ...) __attribute((__format__(printf, 2, 3))) __attribute__((__nonnull__(1, 2)));
int irc_cmd_channel_mode(struct irc_session *session, const char *channel, const char *mode) __attribute__((__nonnull__(1, 2)));
int irc_cmd_ctcp_reply(struct irc_session *session, const char *nick, const char *reply) __attribute__((__nonnull__));
int irc_cmd_ctcp_request(struct irc_session *session, const char *nch, const char *request) __attribute__((__nonnull__));
int irc_cmd_invite(struct irc_session *session, const char *nick, const char *channel) __attribute__((__nonnull__));
int irc_cmd_join(struct irc_session *session, const char *channel, const char *key) __attribute__((__nonnull__(1, 2)));
int irc_cmd_kick(struct irc_session *session, const char *nick, const char *channel, const char *reason) __attribute__((__nonnull__(1, 2)));
int irc_cmd_list(struct irc_session *session, const char *channel) __attribute__((__nonnull__(1)));
int irc_cmd_me(struct irc_session *session, const char *nch, const char *text) __attribute__((__nonnull__));
int irc_cmd_msg(struct irc_session *session, const char *nch, const char *text) __attribute__((__nonnull__));
int irc_cmd_msg_to(struct irc_session *session, const char *channel, const char *to, const char *text) __attribute__((__nonnull__));
int irc_cmd_names(struct irc_session *session, const char *channel) __attribute__((__nonnull__));
int irc_cmd_nick(struct irc_session *session, const char *newnick) __attribute__((__nonnull__));
int irc_cmd_notice(struct irc_session *session, const char *nch, const char *text) __attribute__((__nonnull__));
int irc_cmd_part(struct irc_session *session, const char *channel, const char *reason) __attribute__((__nonnull__(1, 2)));
int irc_cmd_quit(struct irc_session *session, const char *reason) __attribute__((__nonnull__(1)));
int irc_cmd_topic(struct irc_session *session, const char *channel, const char *topic) __attribute__((__nonnull__(1, 2)));
int irc_cmd_user_mode(struct irc_session *session, const char *mode) __attribute__((__nonnull__(1)));
int irc_cmd_whois(struct irc_session *session, const char *nick) __attribute__((__nonnull__));
char *irc_target_get_nick(const char *target) __attribute__((__nonnull__));
char *irc_target_get_username(const char *target) __attribute__((__nonnull__));
char *irc_target_get_hostname(const char *target) __attribute__((__nonnull__));
void irc_set_ctx(struct irc_session *session, void *ctx) __attribute__((__nonnull__(1)));
void *irc_get_ctx(struct irc_session *session) __attribute__((__nonnull__));
int irc_errno(struct irc_session *session) __attribute__((__nonnull__));
const char *irc_strerror(int ircerrno);
void irc_option_set(struct irc_session *session, unsigned int option) __attribute__((__nonnull__));
void irc_option_reset(struct irc_session *session, unsigned int option) __attribute__((__nonnull__));
#undef IN_LIBIRCCLIENT_H
#ifdef __cplusplus
}
#endif
#endif /* LIBIRCCLIENT_H */
