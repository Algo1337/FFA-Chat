#include "__init__.h"

User *new_user(str_t *args) {
    if(!args)
        return NULL;

    User *u = (User *)malloc(sizeof(User));
    if(!u)
        return NULL;

    u->name = str_Copy(args[0]);
    u->ipaddr = str_Copy(args[1]);
    u->password = str_Copy(args[2]);
    u->color = atoi(((str_t)args[3])->data);
    u->rank = atoi(((str_t)args[4])->data);
    u->bot = str_Copy(args[5]);
    u->hwid = str_Copy(args[6]);

    return u;
}

int user_Login(User *u, str_t username, str_t passwd) {
    if(!u || !username || !passwd)
        return 0;

    return ((!strcmp(u->name->data, username->data) && !strcmp(u->password->data, passwd->data)) ? 1 : 0);
}

int change_password(User *u, str_t new_password) {
    if(!u || !new_password)
        return -1;

    if(u->password)
        str_Destruct(u->password);

    u->password = str_Copy(new_password);
    return 1;
}

int change_ip(User *u, str_t new_ip) {
    if(!u || !new_ip)
        return -1;

    if(u->ipaddr)
        str_Destruct(u->ipaddr);
        
    u->ipaddr = str_Copy(new_ip);
    return 1;
}

int has_perms(User *u) {
    if(!u)
        return 0;

    return (u->rank > 1 ? 1 : 0);
}

int is_user_og(User *u) {
    if(!u)
        return 0;

    return (u->rank == 1 ? 1 : 0);
}

int is_user_moderator(User *u) {
    if(!u)
        return 0;

    return (u->rank == 2 ? 1 : 0);
}

int is_user_admin(User *u) {
    if(!u)
        return 0;

    return (u->rank == 3 ? 1 : 0);
}

int is_user_owner(User *u) {
    if(!u)
        return 0;

    return (u->rank == 4 ? 1 : 0);
}

char *get_color_ansi(User *u) {
    if(!u || u->color > ANSI_COLOR_COUNT)
        return NULL;

    return ANSI_COLORS[u->color][1];
}

void user_Destruct(User *u) {
    if(!u)
        return;

    if(u->name)
        str_Destruct(u->name);

    if(u->ipaddr)
        str_Destruct(u->ipaddr);

    if(u->password)
        str_Destruct(u->password);

    free(u);
}