#include "src/__init__.h"

int main() {
    FFA *ffa = init_ffa_server("127.0.0.1", 666);
    if(!ffa) {
        printf("[ - ] Error, Cannot start...!\n");
        return 1;
    }
    sleep(900000);
    return 0;
}