#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define CLIBP
#include <clibp.h>
#include <file.h>
#include <net/socket.h>

#define ANSI_BLACK "\x1b[30m"
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_DEFAULT "\x1b[39m"

#define ANSI_COLOR_COUNT 5
extern char *ANSI_COLORS[][2];

typedef struct {
    str_t       name;
    str_t       ipaddr;
    str_t       password;
    int         color;
    int         rank;
    str_t       bot;
    str_t       hwid;
} User;

typedef User *user_t;

typedef struct {
    user_t      acc;
    sock_t      con;

    str_t       last_buff;
    str_t       color;
    pthread_t   tid;
    int         listening;
    void        *base;
    int         cooldown;
    int         muted;
    str_t       mute_reason;
    int         running;
} Client;

typedef Client *client_t;
typedef arr_t clients_t;
typedef arr_t bots_t;
typedef arr_t users_t;

typedef struct {
    sock_t      Server;
    clients_t   Clients;
    int         running;
    pthread_t   tid;
} Channel;

typedef Channel *channel_t;

typedef struct {
    channel_t   UserChannel;
    channel_t   BotChannel;
    users_t     users;

    pthread_t   tid;
    int         listening;
} FFA;

typedef void *(*handler_t)(str_t);

extern FFA *__FFA__;

// __init__.h
FFA     *init_ffa_server(const char *ip, int port);
void    UserListener(FFA *ffa);
void    BotListener(FFA *ffa);
void    setup_user_channel(const char *ip, int port);
void    setup_bot_channel(const char *ip, int port);
int     is_ip_to_user(const char *ip);
int     validate_free_socket_spots();
int     clients_Remove(FFA *ffa, Client *client);

// db.c
int     read_user_database(FFA *ffa);
User    *find_user(FFA *ffa, str_t name);
User    *find_bot(FFA *ffa, str_t bot_name, str_t hwid);
arr_t   get_all_members(FFA *ffa);
arr_t   get_role_members(FFA *ffa, int rank);
User    *create_user(FFA *ffa, const char *username);
int     user_Remove(FFA *ffa, user_t acc);
int     SaveDatabase(FFA *ffa);

// client.c
Client  *create_client(FFA *ffa, sock_t con);
void    Authentication(void **args);
int     broadcast_message(FFA *ffa, Client *c, const char *data);
void    handle_client(FFA *ffa, client_t client);
int     is_user_signed_in(User *u);
str_t   get_user_list();
str_t   get_last_buffer(Client *c);
int     get_cool_down(Client *c);
int     is_user_muted(Client *c);
str_t   get_mute_reason(Client *c);
void    client_Destruct(Client *client);

// bot.c
void    AuthenticateBot(void **arg);
void    handle_bot(FFA *ffa, Client *bot);
int     extract_n_parse_auth(str_t info);

// user.c
User    *new_user(str_t *args);
int     user_Login(User *u, str_t username, str_t passwd);
int     change_password(User *u, str_t new_password);
int     change_ip(User *u, str_t new_ip);
int     has_perms(User *u);
int     is_user_og(User *u);
int     is_user_moderator(User *u);
int     is_user_admin(User *u);
int     is_user_owner(User *u);
char    *get_color_ansi(User *u);
void    user_Destruct(User *u);

// utils.c
void    str_StripInput(str_t buff);

// cmd.c
void    ListClientCmd(client_t Client);
void    UserAddCmd(client_t Client, str_t buff);