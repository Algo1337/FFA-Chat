#define CLIBP
#include <clibp.h>
#include <net/socket.h>

#define HOST_URL "insanity.host"
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

    // thread setting/info
    pthread_t   tid;
    int         listening;
    int         get_next_buffer;
    char        *buffer;
} FFA;

typedef FFA *ffa_t;

typedef void *(*handler_t)(str_t);

FFA *init_ffa(void) {
    FFA *ffa = (FFA *)malloc(sizeof(FFA));
    if(!ffa)
        return NULL;

    if(!(ffa->Server = create_socket(_ipv4, new_str(HOST_URL, 1), HOST_PORT))) {
        free(ffa);
        return NULL;
    }

    if(ffa->Server->sock <= 0) {
        free(ffa);
        return NULL;
    }

    if(sock_connect(ffa->Server) == -1) {
        printf("[ x ] Error, Unable to connect to FFA Server!\n");
        free(ffa);
        return NULL; 
    }

    sock_set_read_timeout(ffa->Server, 0);
    return ffa;
}

static int is_command_valid(FFA *ffa, str_t cmd) {
    if(!ffa || !cmd)
        return -1;

    for(int i = 0; i < ffa->commands->idx; i++) {
        if(!ffa->commands->arr[i])
            break;

        command_t c = (command_t)ffa->commands->arr[i];
        if(c->prefix != 0) {
            if(c->prefix == cmd->data[0] && !strcmp(c->name, cmd->data + 1))
                return i;
        } else if(!strcmp(c->name, cmd->data)) {
            return i;
        }
    }

    return -1;
}

void start_bot(FFA *ffa) {
    if(!ffa)
        return;

    // send auth

    ffa->listening = 1;
    str_t buff = NULL;
    while(ffa->listening != 0 && (buff = sock_read(ffa->Server)) != NULL) { 
        if(ffa->buffer && !str_StartsWith(buff, "new_msg") && !str_StartsWith(buff, "new_dm") && !str_StartsWith(buff, "on_join"))
        {
            arr_t args = str_SplitAt(buff, ' ');
            size_t rm_len = strlen(((str_t)args->arr[0])->data) + 1;
            for(int i = 0; i < rm_len; i++)
                str_TrimAt(buff, 0);

            arr_Destruct(args, str_Destruct);
            memcpy(ffa->buffer, buff->data, 1024);
            memset(buff->data, 0, buff->idx);
            buff->idx = 0;
            continue;
        }

        if(str_StartsWith(buff, "new_msg") || str_StartsWith(buff, "new_dm"))
        {
            arr_t args = str_SplitAt(buff, ' ');
            size_t rm_len = ((str_t)args->arr[0])->idx + 1;
            for(int i = 0; i < rm_len; i++)
                str_TrimAt(buff, 0);

            int pos = 0;
            if((pos = is_command_valid(ffa, (str_t)args->arr[1])) != -1)
                ((handler_t)((Command *)ffa->commands->arr[pos])->handler)(buff);

            arr_Destruct(args, str_Destruct);
        }

        str_Destruct(buff);
        buff = NULL;
    }

    str_Destruct(buff);
    sock_Destruct(ffa->Server);
}

int set_onjoin_handler(FFA *ffa, void *handler) {
    if(!ffa || !handler)
        return 0;

    ffa->OnJoin = handler;
    return 1;
}

int add_command(FFA *ffa, Command cmd) {
    if(!ffa)
        return 0;

    Command *c = (Command *)malloc(sizeof(Command));
    if(cmd.name)
        c->name = strdup(cmd.name);

    c->arg_count = cmd.arg_count;
    if(cmd.err_msg)
        c->err_msg = strdup(cmd.err_msg);

    arr_Append(ffa->commands, c);
    return 1;
}

FFA *__FFA__ = NULL;
void *send_data(FFA *ffa, cmd_t action, const char *data) {
    
    switch(action) {
        case __send_msg__:
            str_t dm_buff = new_str(strdup("new_msg: "), 0);
            str_cAppend(dm_buff, (char *)data);
            sock_write(ffa->Server, dm_buff->data);
            str_Destruct(dm_buff);

            return (void *)1;
        case __get_role_memers__:
            str_t buff = new_str(strdup("get_role_memers: "), 0);
            str_cAppend(buff, (char *)data);
            sock_write(ffa->Server, buff->data);
            str_Destruct(buff);
            ffa->get_next_buffer = 1;

            while(ffa->buffer == NULL) {
                printf("[ - ] Waiting for Role Members....!\n");
            }

            return (void *)ffa->buffer;
        default:
            return NULL;
    }

    return NULL;
}

void help_cmd(str_t buffer) {
    char *get_members = (char *)send_data(__FFA__, __get_role_memers__, "1");
    if(!get_members)
    {
        printf("[ - ] Error, Unable to get members!\n");
        return;
    }

    if(!send_data(__FFA__, __send_msg__, "Welcome!")) 
    {
        printf("[ - ] Error, Unable to send data to FFA server!\n");
        exit(0);
    }
}

void on_join(User *user) {

}

int main() {
    __FFA__ = init_ffa();
    set_onjoin_handler(__FFA__, on_join);
    add_command(__FFA__, (Command){ .prefix = '/', .name = "help", .arg_count = 0, .err_msg = NULL, .handler = help_cmd });
    start_bot(__FFA__);
    return 0;
}