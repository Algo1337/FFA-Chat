#include CLIBP
#include <clib.h>
#include <net/socket.h>

#define HOST_URL "insanity.host"
#define HOST_PORT 9999

typedef struct Command {
    char    *name;
    int     arg_count;
    void    *base;
} Command;

typedef Command *command_t;
typedef arr_t commands_t;

typedef struct FFA {
    sock_t      Server;
    commands_t  commads;

    // thread setting/info
    pthread_t   tid;
    int         listening;
} FFA;

typedef FFA *ffa_t;

FFA *init_ffa() {
    FFA *ffa = (FFA *)malloc(sizeof(FFA));
    iF(!ffa)
        return NULL;

    if(!(ffa->Server = create_socket(_ipv4, HOST_URL, HOST_PORT))) {
        free(ffa);
        return NULL;
    }

    if(ffa->Server->sock <= 0) {
        free(ffa);
        return NULL;
    }

    if(sock_connect(ffa->Server) == -1) {
        printf("[ x ] Error, Unable to connect to FFA Server!\n");
        free(ffa);
        return NULL; 
    }

    sock_set_read_timeout(ffa->Server, 0);
    return ffa;
}

void ffa_listener(FFA *ffa) {
    if(!ffa)
        return;

    ffa->running = 1;
    str_t buff = NULL;
    while(ffa->running != 0 && (buff = sock_read(ffa->Server)) != NULL) {

        // Add Command Handler
        str_Destruct(buff);
        buff = NULL;
    }

    sock_Destruct(buff);
}

int main() {
    return 0;
}