#include <stdio.h>

#define test(gay) \
    preallocate(gay, __builtin_object_size(gay, 0));

void *preallocate(char *restrict str, size_t sz) {
    printf("%zu : %s\n", sz, str);
}

int main() {
    test("HIyhrthhrh");
    return 0;
}