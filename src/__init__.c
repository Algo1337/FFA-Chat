#include "__init__.h"

char *ANSI_COLORS[][2] = {
    {"{BLACK}", ANSI_BLACK},
    {"{RED}", ANSI_RED},
    {"{GREEN}", ANSI_GREEN},
    {"{YELLOW}", ANSI_YELLOW},
    {"{DEFULT}", ANSI_DEFAULT},
    NULL
};

FFA *__FFA__ = NULL;

FFA *init_ffa_server(const char *ip, int port) {
    if(!ip || port <= 0 || port >= 65535)
        return NULL;

    __FFA__ = (FFA *)malloc(sizeof(FFA));

    /* Set up User and Bot Channel */
    setup_user_channel(ip, port);
    setup_bot_channel(ip, port);

    __FFA__->users = new_arr(NULL, 0);
    if(read_user_database(__FFA__) == -1) {
        printf("[ - ] Error, Unable to read file or no users in database....!\n");
        return NULL;
    }

    printf("[ START_UP ]: User db loaded with %d users\n", __FFA__->users->idx);

    return __FFA__;
}

void setup_user_channel(const char *ip, int port) {
    __FFA__->UserChannel = (Channel *)malloc(sizeof(Channel));
    __FFA__->UserChannel->Clients = new_arr(NULL, 0);
    __FFA__->UserChannel->Server = create_socket(_ipv4, new_str((const str)ip, 0), port);
    if(__FFA__->UserChannel->Server->sock <= 0) {
        printf("[ - ] Error, Unable to start FFA Chat User Server....!\n");
        sock_Destruct(__FFA__->UserChannel->Server);
        return;
    }

    sock_bind(__FFA__->UserChannel->Server);
    sock_listen(__FFA__->UserChannel->Server, 9999);
    pthread_create(&__FFA__->tid, NULL, (void *)UserListener, (void *)__FFA__);
}

void setup_bot_channel(const char *ip, int port) {
    __FFA__->BotChannel = (Channel *)malloc(sizeof(Channel));
    __FFA__->BotChannel->Clients = new_arr(NULL, 0);
    __FFA__->BotChannel->Server = create_socket(_ipv4, new_str((const str)ip, 0), 9999);
    if(__FFA__->BotChannel->Server->sock <= 0) {
        printf("[ - ] Error, Unable to start FFA Chat Bot Server....!\n");
        sock_Destruct(__FFA__->BotChannel->Server);
        return;
    }

    sock_bind(__FFA__->BotChannel->Server);
    sock_listen(__FFA__->BotChannel->Server, 9999);

    pthread_create(&__FFA__->BotChannel->tid, NULL, (void *)BotListener, (void *)__FFA__);
}

void UserListener(FFA *ffa) {
    ffa->listening = 1;
    while(ffa->listening != 0) {
        printf("[ USER_CHANNEL_LISTENER ]: Listening for users....!\n");
        sock_t client_sock = sock_accept(ffa->UserChannel->Server, 1024);
        if(!client_sock)
            continue;

        sock_get_client_ip(client_sock);
        if(is_ip_to_user(client_sock->ip->data) == -1 && validate_free_socket_spots() < 0) {
            sock_Destruct(client_sock);
            continue;
        }

        pthread_t tid;
        void **args = (void **)malloc(sizeof(void *) * 3);
        args[0] = ffa;
        args[1] = client_sock;
        args[2] = &tid;
        pthread_create(&tid, NULL, (void *)Authentication, (void *)args);
    }
}

void BotListener(FFA *ffa) {
    __FFA__->BotChannel->running = 1;
    while(__FFA__->BotChannel->running != 0) {
        printf("[ BOT_CHANNEL_LISTENER ] Listening for bots....!\n");
        sock_t bot_sock = sock_accept(__FFA__->BotChannel->Server, 1024);
        if(!bot_sock)
            continue;

        sock_get_client_ip(bot_sock);

        pthread_t tid;
        void **args = (void **)malloc(sizeof(void *) * 3);
        args[0] = ffa;
        args[1] = bot_sock;
        args[2] = &tid;
        pthread_create(&tid, NULL, (void *)AuthenticateBot, (void *)args);
    }
}

int is_ip_to_user(const char *ip) {
    if(!__FFA__)
        return -1;

    for(int i = 0; i < __FFA__->users->idx; i++) {
        if(!__FFA__->users->arr[i])
            break;

        if(!strcmp(((User *)__FFA__->users->arr[i])->ipaddr->data, ip))
            return i;
    }

    return -1;
}

int validate_free_socket_spots() {
    if(!__FFA__)
        return -1;

    int count = 0;
    for(int i = 0; i < __FFA__->users->idx; i++) {
        if(!__FFA__->users->arr[i])
            break;
        
        User *u = (User *)__FFA__->users->arr[i];
        if(!strcmp(u->ipaddr->data, "none") || !strcmp(u->ipaddr->data, "0.0.0.0"))
            count++;
    }

    return count;
}

int clients_Remove(FFA *ffa, Client *client) {
    if(!ffa || !client)
        return -1;

    for(int i = 0; i < ffa->BotChannel->Clients->idx; i++) {
        if(!ffa->BotChannel->Clients->arr[i])
            break;

        Client *c = (Client *)ffa->BotChannel->Clients->arr[i];
        if(c == client) {
            arr_Remove(ffa->BotChannel->Clients, i, client_Destruct);
            return 1;
        }
    }

    return -1;
}