#include "client/connection.h"

FFA *__FFA__ = NULL;


void on_join(User *user) {

}

void on_message(str_t buff) {
    printf("New Message: %s\n", buff->data);
}

void help_cmd(str_t buffer) {
    /* Get all OG members */
    users_t members = (users_t)send_data(__FFA__, __get_all_members__, NULL);
    if(!members)
    {
        printf("[ - ] Error, Unable to get members!\n");
        return;
    }

    /* debug members */
    for(int i = 0; i < members->idx; i++) {
        if(!members->arr[i])
            break;

        user_t member = (user_t)members->arr[i];
        printf("Member: %s\n", member->name);
    }

    /* Send Hello In Chat */
    if(!send_data(__FFA__, __send_msg__, "Hello!")) 
    {
        printf("[ - ] Error, Unable to send data to FFA server!\n");
        exit(0);
    }
}

int main() {
    __FFA__ = init_ffa();
    if(!__FFA__)
        return 1;

    set_onjoin_handler(__FFA__, on_join);
    set_onmessage_handler(__FFA__, on_message);
    add_command(__FFA__, (Command){ .prefix = '/', .name = "help", .arg_count = 0, .err_msg = NULL, .handler = help_cmd });
    start_bot(__FFA__, "test_app");
    return 0;
}