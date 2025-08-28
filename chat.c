#include "client/connection.h"

FFA *__FFA__ = NULL;


void on_join(User *user) {

}

void on_message(message_t m) {
    printf("New Message: %s\n", m->content->data);
}

handler_t help_cmd(message_t message) {
    printf("HELP TRIGGERS: %s -> %s\n", message->author->name->data, message->content->data);
    
    /* Get all members */
    users_t members = (users_t)send_data(__FFA__, __get_all_members__, NULL);
    if(!members)
    {
        printf("[ - ] Error, Unable to get members!\n");
        return NULL;
    }

    /* debug members */
    for(int i = 0; i < members->idx; i++) {
        if(!members->arr[i])
            break;

        user_t member = (user_t)members->arr[i];
        printf("Member: %s\n", member->name->data);
    }

    /* Send Hello In Chat */
    if(!send_data(__FFA__, __send_msg__, "Hello!")) 
    {
        printf("[ - ] Error, Unable to send data to FFA server!\n");
        exit(0);
    }

    return (void *)1;
}

#define COMMAND_COUNT 1
Command Commands[] = {
    { "help", 0, NULL, help_cmd },
    NULL
};

int main() {
    __FFA__ = init_ffa();
    if(!__FFA__)
        return 1;

    set_onjoin_handler(__FFA__, on_join);
    set_onmessage_handler(__FFA__, on_message);
    for(int i = 0; i < COMMAND_COUNT; i++)
        add_command(__FFA__, (Command){ Commands[i].name, Commands[i].arg_count, Commands[i].err_msg, Commands[i].handler });
    start_bot(__FFA__, "test_app");
    return 0;
}