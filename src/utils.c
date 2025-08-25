#include "__init__.h"

void str_StripInput(str_t buff) {
    if(!buff)
        return;

    buff->idx = strlen(buff->data);
    if(buff->data[buff->idx - 1] == '\r' || buff->data[buff->idx - 1] == '\n')
        buff->data[buff->idx - 1] = '\0';

    if(buff->data[buff->idx - 2] == '\r' || buff->data[buff->idx - 2] == '\n')
        buff->data[buff->idx - 2] = '\0';
}