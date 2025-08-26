#pragma once 

#define CLIBP
#include <clibp.h>
#include <net/socket.h>

#define HOST_URL "127.0.0.1"
#define HOST_PORT 9999

typedef enum {
    __ffa_null__ = 0,
    __send_msg__ = 1,
    __send_dm__ = 2,
    __get_role_memers__ = 3
} cmd_t;

typedef struct {
    char    *name;
    char    *color;
    char    *rank;
} User;

typedef struct {
    char    prefix;
    char    *name;
    int     arg_count;
    char    *err_msg;
    void    *handler;
} Command;

typedef Command *command_t;
typedef arr_t commands_t;

typedef struct {
    sock_t      Server;
    commands_t  commands;
    void        *OnJoin;
    void        *OnMessage;
    void        *Handler;

    // thread setting/info
    pthread_t   tid;
    int         listening;
    int         get_next_buffer;
    char        *buffer;
} FFA;

typedef FFA *ffa_t;

typedef void *(*handler_t)(str_t);

FFA *init_ffa(void);
static int is_command_valid(FFA *ffa, str_t cmd);
char *get_hwid();
void start_bot(FFA *ffa, const char *appname);
int reset_buffer(FFA *ffa);
int set_onjoin_handler(FFA *ffa, void *handler);
int set_onmessage_handler(FFA *ffa, void *handler);
int add_command(FFA *ffa, Command cmd);
void *send_data(FFA *ffa, cmd_t action, const char *data);