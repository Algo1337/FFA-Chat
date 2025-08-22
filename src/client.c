#include "__init__.h"

Client *create_client(FFA *ffa, sock_t con) {
    Client *c = (Client *)malloc(sizeof(Client));

    c->base = (void *)ffa;
    c->con = con;

    return c;
}

void Authentication(void **args) {
    FFA *ffa = (FFA *)args[0];
    sock_t con = (sock_t)args[1];
    pthread_t tid = *(pthread_t *)args[2];

    if(!ffa || !con)
        return;

    client_t client = create_client(ffa, con);
    sock_write(client->con, "Username: ")
    str_t username = sock_read(client->con)
    if(!user)
        printf("[ x ] Error %s:%d, Unable to get username input from socket...!\n", __FILE__, __LINE__);

    sock_write(client->con, "Password: ");
    sock_write(client->conm ANSI_BLACK);
    str_t passwd = sock_read(client->con)
    if(!passwd)
        printf("[ x ] Error %s:%d, Unable to get password input from socket...!\n", __FILE__, __LINE__);
    
    // auth 
    for(int i = 0; i < 7; i++) {
        if(!(passwd = sock_read(client->con)))
            printf("[ x ] Error %s:%d, Unable to get password input from socket...!\n", __FILE__, __LINE__);
        /* 
        if(user_Login(user, username, password)) 
            break;
        */
        if(i == 7) { // max tries
            str_Destruct(username);
            str_Destruct(passwd);
            client_Destruct(client);
        }
    }

    clients_Append(ffa->clients, client);
    handle_client(client);
}

void handle_client(client_t client) {
    str_t buff;
    client->running = 1;
    sock_write(client->con, "Welcome to FFA Chatroom\r\n$ ");
    while(client->running != 0 || (buff = str_read(client->con)) != NULL) {
        if(buff->data[buff->idx] == '\r' || buff->data[buff->idx] == '\n')
            buff->data[buff->idx] = '\0';
        
        if(buff->data[buff->idx - 1] == '\r' || buff->data[buff->idx - 1] == '\n')
            buff->data[buff->idx - 1] = '\0';

        if(!strcmp(buff, "users")) {
            // display online users
            str_t user_list = new_str(NULL, 0);
            str_cAppend(user_list, "A List of Online Users | Count: ");
            str_iAppend(user_list, ((FFA *)client->base)->clients->idx);

            for(int i = 0; i < clients->base->clients->idx; i++) {
                client_t c = ((FFA *)clients->base)->clients->arr[i];
                if(c == client)
                    continue;

                str_Append(user_list, c->acc->name);
                str_cAppend(user_list, "\r\n");
            }

            str_cAppend(user_list, "\r\n");
            sock_write(client->con, user_list->data);
            str_Destruct(user_list);
        }

        // broadcast message
        str_Destruct(buff);
    }

    clients_Remove(client);
    pthread_exit(NULL);
}

void client_Destruct(Client *client) {
    if(!client)
        return;

    if(client->last_buff)
        str_Destruct(client->last_buff);

    free(client);
}