#include "client/connection.h"

FFA *__FFA__ = NULL;

void on_join(User *user) {

}

void on_message(str_t buff) {
    printf("New Message: %s\n", buff->data);
}

void help_cmd(str_t buffer) {
    char *get_members = (char *)send_data(__FFA__, __get_role_memers__, "1");
    if(!get_members)
    {
        printf("[ - ] Error, Unable to get members!\n");
        return;
    }

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