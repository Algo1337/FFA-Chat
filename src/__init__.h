#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLIBP
#include <clibp.h>

#define ANSI_BLACK "\x1b[30m"
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_DEFAULT "\x1b[39m"

typedef struct {
    str_t       name;
    str_t       ipaddr;
    str_t       password;
    int         color;
    int         rank;
} User;

typedef User *user_t;

typedef struct {
    user_t      acc;
    sock_t      Con;

    str_t       last_buff;
    ANSI_COLOR  color;
    pthread_t   tid;
    int         listening;
    void        *base;
    int         cooldown;
    int         muted;
    str_t       mute_reason;
} Client;

typedef Client *client_t;
typedef arr_t clients_t;
typedef arr_t bots_t;

typedef struct {
    sock_t      server;
    clients_t   clients;
    bots_t      bots;

    pthread_t   tid;
    int         listening;
} FFA;

// client_t functions
#define new_arr create_clients_list
#define arr_Append clients_Append

// bots_t functions
#define new_arr create_bot_list
#define arr_Append bot_Append

// __init__.h
FFA *init_ffa_server(const char *ip, int port);
void UserListener(FFA *ffa);
int clients_Remove(FFA *ffa, Client *client);

// client.c
Client *create_client(FFA *ffa, sock_t con);
void Authentication(void **args);
void handle_client(client_t client);