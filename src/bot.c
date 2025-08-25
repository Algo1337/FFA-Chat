#include "__init__.h"

void AuthenticateBot(void **args) {
    FFA *ffa = (FFA *)args[0];
    sock_t bot_sock = (sock_t)args[1];
    pthread_t tid = *(pthread_t *)args[2];

    sock_set_read_timeout(bot_sock, 10);
    str_t bot_info = sock_read(bot_sock);
    
    client_t client = create_client(ffa, bot_sock);
}