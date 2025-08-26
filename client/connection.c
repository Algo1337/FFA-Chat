#include "connection.h"

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
        sock_Destruct(ffa->Server);
        return NULL;
    }

    ffa->commands = new_arr(NULL, 0);
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

char *get_hwid() {
    FILE *fd = fopen("/sys/class/dmi/id/product_uuid", "r");
    if(!fd)
        return NULL;

    fseek(fd, 0L, SEEK_END);
    long sz = ftell(fd);
    fseek(fd, 0L, SEEK_SET);

    char BUFF[sz];
    int rbytes = fread(&BUFF, 1, sz, fd);
    if(rbytes != sz)
        printf("[ - ] Error, Unable to cmd respnse correctly %s:%d\n", __FILE__, __LINE__);

    fclose(fd);
    if(strlen(BUFF) > 0)
        return strdup(BUFF);

    return NULL;
}

void start_bot(FFA *ffa, const char *appname) {
    if(!ffa)
        return;

    // send auth
    char abuff[500] = {0};
    strcat(abuff, "cfk_v1_0_CONNECT;");
    strncat(abuff, appname, strlen(appname));
    strcat(abuff, ";");
    char *hwid = get_hwid();
    if(!hwid)
        printf("[ - ] HWID Error\n");

    strncat(abuff, hwid, strlen(hwid));
    sock_write(ffa->Server, abuff);


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
            ffa->buffer = strdup(buff->data);
            memset(buff->data, 0, buff->idx);
            buff->idx = 0;
            continue;
        }

        if(str_StartsWith(buff, "new_msg") || str_StartsWith(buff, "new_dm"))
        {
            if(ffa->OnMessage)
                ((handler_t)(void *)ffa->OnMessage)(buff);
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

int set_onmessage_handler(FFA *ffa, void *handler) {
    if(!ffa || !handler)
        return 0;

    ffa->OnMessage = handler;
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

void *send_data(FFA *ffa, cmd_t action, const char *data) {
    switch(action) {
        case __send_msg__:
            str_t msg_buff = new_str(strdup("new_msg: "), 0);
            str_cAppend(msg_buff, (char *)data);
            sock_write(ffa->Server, msg_buff->data);
            str_Destruct(msg_buff);

            return (void *)1;
        case __send_dm__:
            str_t dm_buff = new_str(strdup("new_dm: "), 0);
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