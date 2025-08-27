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
        str_Destruct(bot_info);
        sock_Destruct(bot_sock);
        pthread_exit(NULL);
        return;
    }

    str_t sub = new_str(str_GetSub(bot_info, 17, bot_info->idx), 0);
    arr_t args = str_SplitAt(sub, ';');
    if(!args || args->idx < 2) {
        printf("[ - ] Unable to get app info\n");
        str_Destruct(bot_info);
        sock_Destruct(bot_sock);
        pthread_exit(NULL);
        return;
    }

    str_t bot_name = (str_t)args->arr[0];
    str_t HWID = (str_t)args->arr[1];

    printf("%s : %s\n", bot_name->data, HWID->data);
    User *check = find_bot(ffa, bot_name, HWID);
    if(!check)
    {
        printf("[ - ] Error, Unable to find bot owner!\n");
        str_Destruct(bot_info);
        sock_Destruct(bot_sock);
        pthread_exit(NULL);
        return;
    }

    client_t client = create_client(ffa, bot_sock);
    if(!client)
    {
        printf("[ - ] Error, Unable to create client instanse!\n");
        str_Destruct(bot_info);
        sock_Destruct(bot_sock);
        pthread_exit(NULL);
        return;
    }
    
    str_Destruct(bot_info);
    arr_Append(__FFA__->BotChannel->Clients, client);
    handle_bot(ffa, client);
}

void handle_bot(FFA *ffa, Client *bot) {
    bot->running = 1;
    str_t buff = NULL;
    arr_t args = NULL;
    while(bot->running != 0 && (buff = sock_read(bot->con)) != NULL) {
        // Get commands from bot

        if(strstr(buff->data, " ")) {
            args = str_SplitAt(buff, ' ');
        }

        /* send_msg: <msg> */
        /* send_dm: <user> <msg> */
        /* get_role_members: <role> */
        if(str_StartsWith(buff, "send_dm")) {

        } else if(str_StartsWith(buff, "send_msg")) {
            
        } else if(!strcmp(buff->data, "get_all_members")) {
            printf("fetched\n");
            arr_t membs = get_all_members(ffa);
            if(!membs || membs == 0)
                printf("[ - ] All members error\n");

            str_t buff = new_str(strdup("all_members: "), 0);
            for(int i = 0; i < membs->idx; i++) {
                str_cAppend(buff, (char *)membs->arr[i]);
                str_cAppend(buff, ";");
            }

            printf("SENDING: %s\n", buff->data);
            sock_write(bot->con, buff->data);
            str_Destruct(buff);
            arr_Destruct(membs, free);
        } else if(str_StartsWith(buff, "get_role_members: ")) {
            if(args->idx != 2) {
                sock_write(bot->con, "error");
            } else {
                arr_t membs = get_role_members(ffa, atoi(((str_t)args->arr[1])->data));
                str_t buff = new_str(strdup("role_members: "), 0);
                str_t member_list = arr_Join(membs, ", ");
                str_Append(buff, member_list);
                sock_write(bot->con, buff->data);

                str_Destruct(member_list);
                str_Destruct(buff);
                arr_Destruct(membs, free);
            }
        }

        str_Destruct(buff);
        buff = NULL;
        if(args) {
            arr_Destruct(args, str_Destruct);
            args = NULL;
        }
    }
}

int extract_n_parse_auth(str_t info) {
    if(!info)
        return 0;

    char *data = "cfk_v1_0_CONNECT;";
    size_t len = strlen("cfk_v1_0_CONNECT;");
    for(int i = 0; i < len; i++) {
        if(info->data[i] != data[i])
            return 0;
    }

    return 1;
}