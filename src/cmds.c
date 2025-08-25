#include "__init__.h"

void ListClientCmd(client_t Client) {
    if(!__FFA__)
        return;

    str_t users_list = new_str(strdup("List of users online: "), 0);
    str_iAppend(users_list, __FFA__->UserChannel->Clients->idx);
    str_cAppend(users_list, "\r\n");
    for(int i = 0; i < __FFA__->UserChannel->Clients->idx; i++) {
        str_Append(users_list, ((client_t)__FFA__->UserChannel->Clients->arr[i])->acc->name);
        str_cAppend(users_list, "\r\n");
    }

    sock_write(Client->con, users_list->data);
    str_Destruct(users_list);
}

void UserAddCmd(client_t Client, str_t buff) {
    if(!Client)
        return;

    if(is_user_admin(Client->acc)) {
        arr_t args = str_SplitAt(buff, ' ');
        if(args->idx < 2) {
            sock_write(Client->con, "[ - ] Error, Invalid arguments provided....!\nUsage: useradd <username>");
            str_Destruct(buff);
            sock_write(Client->con, "\r\n$ ");
            return;
        }

        User *u = create_user(__FFA__, ((str_t)args->arr[1])->data);
        arr_Destruct(args, str_Destruct);
    }
}