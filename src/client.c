#include "__init__.h"

Client *create_client(FFA *ffa, sock_t con) {
    Client *c = (Client *)malloc(sizeof(Client));

    c->base = (void *)__FFA__;
    c->con = con;
    c->color = NULL;
    c->mute_reason = NULL;
    c->last_buff = NULL;

    return c;
}

void Authentication(void **args) {
    FFA *ffa = (FFA *)args[0];
    sock_t con = (sock_t)args[1];
    pthread_t tid = *(pthread_t *)args[2];

    free(args);
    if(!__FFA__ || !con)
        return;

    sock_set_read_timeout(con, 0);

    client_t client = create_client(__FFA__, con);
    sock_write(client->con, "Username: ");
    str_t username = sock_read(client->con);
    str_StripInput(username);
    if(!username) {
        printf("[ x ] Error %s:%d, Unable to get username input from socket...!\n", __FILE__, __LINE__);
        client_Destruct(client);
        return;
    }

    User *user = find_user(__FFA__, username);
    if(!user) {
        printf("[ AUTHENTICATION ]: Unable to find user %s\n", username->data);
        sock_Destruct(con);
        str_Destruct(username);
        pthread_exit(NULL);
        return;
    }

    if(!strcmp(user->password->data, "none")) {
        sock_write(client->con, "[ NEW ACCOUNT ]: New Password for your account?: ");
        str_t new_pw = sock_read(con);
        str_StripInput(new_pw);
        change_password(user, new_pw);
        SaveDatabase(__FFA__);
        str_Destruct(new_pw);
    }

    sock_write(client->con, "Password: ");
    str_t passwd = sock_read(client->con);
    str_StripInput(passwd);
    if(!passwd || !passwd->data || passwd->idx < 3) {
        printf("[ x ] Error %s:%d, Unable to get password input from socket...!\n", __FILE__, __LINE__);
        client_Destruct(client);
        return;
    }
    
    int check;
    if((check = is_user_signed_in(user)) > -1) {
        sock_write(client->con, "Your account is already signed in, Do you want to log it out for this session?: ");
        str_t answer = sock_read(client->con);
        str_StripInput(answer);
        if(!strcmp(answer->data, "Y")) {
            clients_Remove(__FFA__, __FFA__->UserChannel->Clients->arr[check]);
        } else {
            client_Destruct(client);
            return;
        }
    }

    if(!user_Login(user, username, passwd)) {
        str_Destruct(username);
        str_Destruct(passwd);
        sock_Destruct(con);
        return;
    }
    
    client->acc = user;
    if(!strcmp(user->ipaddr->data, "none")) {
        change_ip(client->acc, client->con->ip);
        SaveDatabase(__FFA__);
    } else if(strcmp(user->ipaddr->data, "0.0.0.0") && strcmp(client->con->ip->data, "none")) {
        if(strcmp(user->ipaddr->data, client->con->ip->data)) {
            printf("HERE AUTH\n");
            str_Destruct(username);
            str_Destruct(passwd);
            client_Destruct(client);
            return;
        }
    }
    
    printf("[ SUCCESSFULL_LOGIN ]: New user connected => %s:%s:%s\n", username->data, passwd->data, con->ip->data);

    str_t buff = new_str(strdup("User: "), 0);
    str_Append(buff, user->name);
    str_cAppend(buff, " has successfully joined FFA chat!");

    broadcast_message(__FFA__, client, buff->data);

    str_Destruct(buff);
    str_Destruct(username);
    str_Destruct(passwd);
    
    arr_Append(__FFA__->UserChannel->Clients, client);
    handle_client(__FFA__, client);
}

void handle_client(FFA *ffa, client_t client) {
    str_t buff;
    client->running = 1;
    sock_write(client->con, "Welcome to The FFA Chatroom\r\n$ ");
    while(client->running != 0 && (buff = sock_read(client->con)) != NULL) {
        if(!buff || !buff->data)
            continue;
            
        str_StripInput(buff);
        if(!strcmp(buff->data, "users")) {
            ListClientCmd(client);
        } else if(!strcmp(buff->data, "clear")) {
            sock_write(client->con, "\x1b[2J\x1b[1;1H");
        } else if(strstr(buff->data, "useradd")) {
            
            continue;
        } else {
            broadcast_message(__FFA__, client, buff->data);
        }

        str_Destruct(buff);
        sock_write(client->con, "$ ");
    }

    clients_Remove(client->base, client);
    pthread_exit(NULL);
}


int broadcast_message(FFA *ffa, Client *c, const char *data) {
    if(!ffa || !c)
        return -1;

    time_t now = time(NULL);
    struct tm local;
    localtime_r(&now, &local);

    char *TIME_BUFF = (char *)malloc(512);
    sprintf(TIME_BUFF, "%02d/%02d/%04d-%02d:%02d", local.tm_mon + 1, local.tm_mday, local.tm_year + 1900, local.tm_hour, local.tm_min);
    
    str_t buff = new_str(strdup("\a[ "), 0);
    if(c->acc->color > 0) {
        char* color = get_color_ansi(c->acc);
        str_cAppend(buff, color);
        str_Append(buff, c->acc->name);
        str_cAppend(buff, ANSI_DEFAULT);
    } else {
        str_Append(buff, c->acc->name);
    }

    str_cAppend(buff, " - ");
    str_cAppend(buff, TIME_BUFF);
    str_cAppend(buff, " ]\r\n");
    str_cAppend(buff, (char *)data);

    for(int i = 0; i < ffa->UserChannel->Clients->idx; i++) {
        if(!ffa->UserChannel->Clients->arr[i])
            break;

        if(ffa->UserChannel->Clients->arr[i] != c) {
            sock_write(((Client *)__FFA__->UserChannel->Clients->arr[i])->con, buff->data);
            sock_write(((Client *)__FFA__->UserChannel->Clients->arr[i])->con, "\r\n$ ");
        }
    }

    /* Send to bots */
    str_t bot_buff = new_str(strdup("new_msg: "), 0);
    str_cAppend(bot_buff, (char *)data);
    for(int i = 0; i < ffa->BotChannel->Clients->idx; i++) {
        if(!ffa->BotChannel->Clients->arr[i])
            break;

        sock_write(((Client *)ffa->BotChannel->Clients->arr[i])->con, bot_buff->data);
    }

    str_Destruct(bot_buff);
    str_Destruct(buff);
    free(TIME_BUFF);
    return 1;
}


void replace_colors(str_t buff) {
    for(int i = 0; i < ANSI_COLOR_COUNT; i++) {
        int pos = 0;
        if((pos = strstr(buff->data, ANSI_COLORS[i][0]) - buff->data) > 0) {}
            // str_ReplaceString()
    }
}

str_t get_user_list() {
    if(!__FFA__)
        return NULL;

    str_t user_list = new_str(strdup("A List of Online Users | Count: "), 0);
    str_iAppend(user_list, __FFA__->UserChannel->Clients->idx);
    str_cAppend(user_list, "\r\n");

    for(int i = 0; i < __FFA__->UserChannel->Clients->idx; i++) {
        client_t c = __FFA__->UserChannel->Clients->arr[i];

        str_cAppend(user_list, "[ ");
        str_iAppend(user_list, i);
        str_cAppend(user_list, " ]: ");
        str_Append(user_list, c->acc->name);
        str_cAppend(user_list, "\r\n");
    }

    return user_list;
}

int is_user_signed_in(User *u) {
    if(!__FFA__)
        return -1;

    for(int i = 0; i < __FFA__->UserChannel->Clients->idx; i++) {
        if(!__FFA__->UserChannel->Clients->arr[i])
            break;

        client_t client = (client_t)__FFA__->users->arr[i];
        if(client->acc == u)
            return i;
    }

    return -1;
}


str_t get_last_buffer(Client *c) {
    if(!c)
        return NULL;

    return c->last_buff;
}

int get_cool_down(Client *c) {
    if(!c)
        return -1;

    return c->cooldown;
}

int is_user_muted(Client *c) {
    if(!c)
        return -1;

    return c->muted;
}

str_t get_mute_reason(Client *c) {
    if(!c)
        return NULL;

    return c->mute_reason;
}

void client_Destruct(Client *client) {
    if(!client)
        return;

    if(client->color)
        str_Destruct(client->color);

    if(client->con)
        sock_Destruct(client->con);

    if(client->last_buff)
        str_Destruct(client->last_buff);

    free(client);
}