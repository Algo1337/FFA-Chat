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

static char *get_hwid() {
    FILE *fd = fopen("/sys/class/dmi/id/product_uuid", "r");
    if(!fd)
        return NULL;

    fseek(fd, 0L, SEEK_END);
    long sz = ftell(fd);
    fseek(fd, 0L, SEEK_SET);

    char BUFF[sz];
    int rbytes = fread(&BUFF, 1, sz, fd);

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
    if(!hwid) {
        printf("[ - ] HWID Error\n");
		return;
	}

    strncat(abuff, hwid, strlen(hwid));
    sock_write(ffa->Server, abuff);


    ffa->listening = 1;
    str_t buff = NULL;
    while(ffa->listening != 0 && (buff = sock_read(ffa->Server)) != NULL) { 
        if(ffa->get_next_buffer && !str_StartsWith(buff, "new_msg") && !str_StartsWith(buff, "new_dm") && !str_StartsWith(buff, "on_join"))
        {
            int cpos = str_FindChar(buff, ' ', 1);
            str_t nbuff = new_str(str_GetSub(buff, cpos, buff->idx), 0);

            ffa->buffer = strdup(nbuff->data);
            memset(buff->data, 0, buff->idx);
            buff->idx = 0;
            continue;
        }

        if(str_StartsWith(buff, "new_msg") || str_StartsWith(buff, "new_dm"))
        {
            Message *m = (Message *)malloc(sizeof(Message));
            m->is_dm = str_StartsWith(buff, "new_dm") ? 1 : 0;

            int cpos = str_FindChar(buff, ';', 0);
            int msg_pos = str_FindChar(buff, ';', 1);

            m->content = new_str(str_GetSub(buff, msg_pos, buff->idx), 0);
            m->author = (User *)malloc(sizeof(User));

            str_t author = new_str(str_GetSub(buff, cpos, msg_pos), 0);
            arr_t author_info = str_SplitAt(author, ",");

            m->author->name = new_str(strdup(((str_t)author_info->arr[0])->data), 0);
            m->author->color = atoi(((str_t)author_info->arr[1])->data);
            m->author->rank = atoi(((str_t)author_info->arr[2])->data);
            
            if(ffa->OnMessage)
                ((handler_t)(void *)ffa->OnMessage)(m);

            arr_t args = str_SplitAt(m->data, ' ');
            int pos = 0;
            if((pos = is_command_valid(ffa, (str_t)args->arr[1])) != -1) 
                ((void *(*)(message_t))((Command *)ffa->commands->arr[pos])->handler)(m);

            str_Destruct(author);
            str_Destruct(author_info);
            arr_Destruct(args, str_Destruct);
            message_Destruct(m);
        }

        str_Destruct(buff);
        buff = NULL;
    }

    str_Destruct(buff);
    sock_Destruct(ffa->Server);
}

int reset_buffer(FFA *ffa) {
    if(!ffa)
        return 0;

    if(ffa->buffer) {
        free(ffa->buffer);
        ffa->buffer = NULL;
    }

    return 1;
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

    c->handler = cmd.handler;

    arr_Append(ffa->commands, c);
    return 1;
}

void *send_data(FFA *ffa, event_t action, const char *data) {
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
        case __get_all_members__:
            ffa->get_next_buffer = 1;
            sock_write(ffa->Server, "get_all_members");

            str_t buff = sock_read(ffa->Server);

            users_t users = extract_all_members(buff->data);
            if(!users)
                return NULL;

            if(users->idx == 0) {
                free(users);
                reset_buffer(ffa);
                return NULL;
            }

            return users;
        case __get_role_memers__:
            str_t mbuff = new_str(strdup("get_role_memers: "), 0);
            str_cAppend(mbuff, (char *)data);
            sock_write(ffa->Server, mbuff->data);
            str_Destruct(mbuff);
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


users_t extract_all_members(const char *buffer) {
    if(!buffer)
        return NULL;

    str_t buff = new_str(strdup(buffer), 0);
    str_t membs = new_str(str_GetSub(buff, strlen("all_members: "), buff->idx), 0);
    arr_t members = str_SplitAt(membs, ';');

    users_t users = new_arr(NULL, 0);
    for(int i = 0; i < members->idx; i++) {
        arr_t args = str_SplitAt((str_t)members->arr[i], ',');
        if(args->idx != 3) {
            arr_Destruct(args, str_Destruct);
            continue;
        }

        User *u = (User *)malloc(sizeof(User));
        u->name = strdup(((str_t)args->arr[0])->data);
        u->color = strdup(((str_t)args->arr[1])->data);
        u->rank = atoi(((str_t)args->arr[1])->data);

        arr_Append(users, u);
        arr_Destruct(args, str_Destruct);
    }

    str_Destruct(membs);
    str_Destruct(buff);
    arr_Destruct(members, str_Destruct);

    return users;
}

void user_Destruct(User *u) {
    if(!u)
        return;

    if(u->name)
        str_Destruct(u->name);

    if(u->color)
        str_Destruct(u->color);
}

void message_Destruct(Message *m) {
    if(!m)
        return;

    if(m->content)
        str_Destruct(m->content);

    if(m->timestamp)
        str_Destruct(m->content);

    if(m->author)
        user_Destruct(m->author);

    free(m);
}