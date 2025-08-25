#include "__init__.h"

/*
    FFA_CLIENT_KEY_IDENIFIER: cfk_
    FFA_SERVER_KEY_IDENIFIER: skf_

    App Name in bytes

*/
// cfk_v1_0_CONNECT
// sfk_v1_0_CONNECT;HWID
// cfk_v1_0_CONNECT;app_name;HWID
#define KEY_RECV_IDENIFIER "cfk_"
#define KEY_SEND_IDENIFIER "sfk_"

#define KEY_VERSION "v1_0"

void AuthenticateBot(void **arg) {
    FFA *ffa = (FFA *)arg[0];
    sock_t bot_sock = (sock_t)arg[1];
    pthread_t tid = *(pthread_t *)arg[2];

    sock_set_read_timeout(bot_sock, 10);
    str_t bot_info = sock_read(bot_sock);
    if(!bot_info)
    {
        printf("[ - ] Error, Bot %s tried connecting\n", bot_sock->ip->data);
        sock_Destruct(bot_sock);
        pthread_exit(NULL);
        return;
    }

    // Validate bot
    if(!extract_n_parse_auth(bot_info)) {
        printf("[ - ] Error, Bot %s tried connecting with %s", bot_sock->ip->data, bot_info->data);
        sock_Destruct(bot_sock);
        pthread_exit(NULL);
        return;
    }

    arr_t args = NULL;
    if(str_Contains(bot_info, ";"))
        args = str_SplitAt(bot_info, ';');

    str_t bot_name = (str_t)args->arr[0];
    str_t HWID = (str_t)args->arr[1];

    // find owner of the bot

    client_t client = create_client(ffa, bot_sock);
    if(!client)
    {
        printf("[ - ] Error, Unable to create client instanse!\n");
        sock_Destruct(bot_sock);
        pthread_exit(NULL);
        return;
    }

    arr_Append(__FFA__->BotChannel->Clients, client);
}

int extract_n_parse_auth(str_t info) {
    if(!info)
        return 0;

    if(!str_StartsWith(info, KEY_RECV_IDENIFIER))
        return 0;

    int len = strlen(KEY_RECV_IDENIFIER);
    for(int i = 0; i < len; i++)
        str_TrimAt(info, 0);

    if(!str_StartsWith(info, KEY_VERSION))
        return 0;

    len = strlen(KEY_VERSION) + strlen("_CONNECT;") + 1;
    for(int i = 0; i < len; i++) 
        str_TrimAt(info, 0);

    return 1;
}