#include "__init__.h"

FFA *init_ffa_server(const char *ip, int port) {
    if(!ip || port <= 0 || port >= 65535)
        return NULL;

    FFA *ffa = (FFA *)malloc(sizeof(FFA));
    if(!(ffa->server = create_socket(_ipv4, new_str(ip, 0), port))) {
        free(ffa);
        return NULL;
    }

    ffa->clients = create_client_list(NULL, 0);
    ffa->bots = create_bot_list(NULL, 0);

    pthread_create(&ffa->tid, NULL, (void *)UserListener, (void *)ffa);
    return ffa;
}

void UserListener(FFA *ffa) {
    ffa->listening = 1;
    while(ffa->listening != 0) {
        sock_t client_sock = sock_Accept(ffa->server, 1024);
        if(!client_sock)
            continue;

        sock_get_client_ip(client_sock);

        // Protection
        pthread_t tid;
        void **args = (void **)malloc(sizeof(void *) * 3);
        args[0] = ffa;
        args[1] = client_sock;
        args[2] = &tid;
        pthread_create(&tid, NULL, (void *)Authentication, (void *)args);
    }
}

int clients_Remove(FFA *ffa, Client *client) {
    if(!ffa || !client)
        return -1;

    for(int i = 0; i < ffa->clients->idx; i++) {
        if(!ffa->clients->arr[i])
            break;

        Client *c = (Client *)ffa->clients->arr[i];
        if(c == client) {
            arr_Remove(ffa->clients, i, client_Destruct);
            return 1;
        }
    }

    return -1;
}