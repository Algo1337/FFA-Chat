#include "src/__init__.h"

int main() {
    FFA *ffa = init_ffa_server("127.0.0.1", 666);
    if(!ffa) {
        printf("[ - ] Error, Cannot start...!\n");
        return 1;
    }

    char BUFF[5];
    fgets(BUFF, 5, stdin);
    return 0;
}