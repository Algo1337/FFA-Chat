#pragma once 

#define CLIBP
#include <clibp.h>
#include <net/socket.h>

#define HOST_URL "127.0.0.1"
#define HOST_PORT 9999

typedef enum {
    __ffa_null__            = 0,
    __send_msg__            = 1,
    __send_dm__             = 2,
    __get_all_members__     = 3,
    __get_role_memers__     = 4,
    __new_msg__             = 5,
    __new_dm__              = 6,
} event_t;

typedef struct {
    str_t    name;
    str_t    color;
    int      rank;
} User;

typedef struct {
    int     is_dm;
    str_t   content;
    str_t   timestamp;
    User    *author;
} Message;

typedef User *user_t;
typedef Message *message_t;

typedef arr_t users_t;
typedef arr_t USER_CACHE_T;
typedef arr_t messages_t;
typedef arr_t MESSAGE_CACHE_T;

typedef struct {
    file_t          fd;
    USER_CACHE_T    users;
    MESSAGE_CACHE_T messages;
} Cache;

typedef Cache *cache_t;

typedef struct {
    char    *name;
    int     arg_count;
    char    *err_msg;
    void    *handler;
} Command;

typedef Command *command_t;
typedef arr_t commands_t;
typedef void *(*handler_t)(message_t);

typedef struct {
    sock_t      Server;
    commands_t  commands;
    char        prefix;
    void        *OnJoin;
    void        *OnMessage;
    handler_t   *Handler;

    // thread setting/info
    pthread_t   tid;
    int         listening;
    int         get_next_buffer;
    char        *buffer;
} FFA;

typedef FFA *ffa_t;

FFA         *init_ffa(void);

/* Check if command is valid in cogs */
static int  is_command_valid(FFA *ffa, str_t cmd);

/* Get HWID ID for Auth */
static char *get_hwid();

// Start Bot
void        start_bot(FFA *ffa, const char *appname);

// Reset Internal Buffer
int         reset_buffer(FFA *ffa);

// Set OnJoin Event Handler
int         set_onjoin_handler(FFA *ffa, void *handler);

// Set OnMessage Event Handler
int         set_onmessage_handler(FFA *ffa, void *handler);

// Set a command prefix
int         set_prefix(FFA *ffa, const char p);

// Add a command event handler to cogs
int         add_command(FFA *ffa, Command cmd);

// send data to FFA Server
void        *send_data(FFA *ffa, event_t action, const char *data);

// Parse and extract all members
users_t     extract_all_members(const char *buffer);

// Destruct user struct
void        user_Destruct(User *u);

// Destruct message struct
void        message_Destruct(Message *m);