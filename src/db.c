#include "__init__.h"

#define USER_DB_PATH "assets/users.db"

int read_user_database(FFA *ffa) {
    if(!ffa)
        return -1;

    FILE *fp = fopen(USER_DB_PATH, "r"); 
    if(!fp)
        return -1;

    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char *data = (char *)malloc(sz);
    memset(data, 0, sz);

    int bytes = fread(data, 1, sz, fp);

    fclose(fp);
    str_t rdb = new_str(strdup(data), 0);
    str_Trim(rdb, '(');
    str_Trim(rdb, ')');
    str_Trim(rdb, '\'');

    arr_t lines = str_SplitAt(rdb, '\n');
    for(int i = 0; i < lines->idx; i++) {
        str_t line = (str_t)lines->arr[i];
        arr_t args = str_SplitAt(line, ',');

        if(args->idx != 7) {
            arr_Destruct(args, str_Destruct);
            continue;
        }

        User *u = new_user((str_t *)args->arr);
        arr_Append(ffa->users, u);
        arr_Destruct(args, str_Destruct);
    }

    str_Destruct(rdb);
    arr_Destruct(lines, str_Destruct);

    return ffa->users->idx;
}

User *find_user(FFA *ffa, str_t name) {
    if(!ffa || !name)
        return NULL;

    for(int i = 0; i < ffa->users->idx; i++) {
        if(!ffa->users->arr[i])
            break;

        if(!strcmp(((User *)ffa->users->arr[i])->name->data, name->data))
            return (User *)ffa->users->arr[i];
    }

    return NULL;
}

User *find_bot(FFA *ffa, str_t bot_name, str_t hwid) {
    if(!ffa || !bot_name || !hwid)
        return NULL;

    for(int i = 0; i < ffa->users->idx; i++) {
        if(!ffa->users->arr[i])
            break;

        if(!strcmp(((user_t)ffa->users->arr[i])->bot->data, bot_name->data) && (!strcmp(((user_t)ffa->users->arr[i])->hwid->data, hwid->data) || !strcmp(((user_t)ffa->users->arr[i])->hwid->data, "none")))
            return ((user_t)ffa->users->arr[i]);
    }

    return NULL;
}

arr_t get_role_members(FFA *ffa, int rank) {
    if(!ffa)
        return NULL;

    arr_t users = new_arr(NULL, 0);
    for(int i = 0; i < ffa->users->idx; i++) {
        if(!ffa->users->arr[i])
            break;

        if(((user_t)ffa->users->arr[i])->rank == rank)
            arr_Append(users, ((user_t)ffa->users->arr[i])->name->data);
    }

    if(users->idx > 0)
        return users;

    arr_Destruct(users, NULL);
    return NULL;
}

/* Create new user */
User *create_user(FFA *ffa, const char *username) {
    if(!ffa || !username)
        return NULL;
    
    char *ARR[] = {(char *)username, "none", "none", "0", "0", "none", NULL};
    User *u = (User *)malloc(sizeof(User));
    if(!u)
        return NULL;

    u->name         = new_str(strdup(ARR[0]), 0);
    u->ipaddr       = new_str(strdup(ARR[1]), 0);
    u->password     = new_str(strdup(ARR[2]), 0);
    u->color        = atoi(ARR[3]);
    u->rank         = atoi(ARR[4]);
    u->bot          = new_str(strdup(ARR[5]), 0);

    arr_Append(ffa->users, u);
    SaveDatabase(ffa);

    return u;
}

/* Remove User From Database */
int user_Remove(FFA *ffa, user_t acc) {
    if(!ffa)
        return 0;

    for(int i = 0; i < ffa->users->idx; i++) {
        if(!ffa->users->arr[i])
            break;

        User *u = (User *)ffa->users->arr[i];
        if(u == acc)
            arr_Remove(ffa->users, i, user_Destruct);
    }

    SaveDatabase(ffa);
    return 1;
}

/* Save User Database */
int SaveDatabase(FFA *ffa) {
    if(!ffa || ffa->users->idx == 0)
        return 0;

    str_T new_db = { .data = malloc(1), .idx = 0 };
    new_db.data[0] = '\0';

    for(int i = 0; i < ffa->users->idx; i++) {
        User *u = (User *)ffa->users->arr[i];

        str_cAppend(&new_db, "('");
        str_Append(&new_db, u->name);
        str_cAppend(&new_db, "','");
        str_Append(&new_db, u->ipaddr);
        str_cAppend(&new_db, "','");
        str_Append(&new_db, u->password);
        str_cAppend(&new_db, "','");
        str_iAppend(&new_db, u->color);
        str_cAppend(&new_db, "','");
        str_iAppend(&new_db, u->rank);
        str_cAppend(&new_db, "','");
        str_Append(&new_db, u->bot);
        str_cAppend(&new_db, "','");
        str_Append(&new_db, u->hwid);
        str_cAppend(&new_db, "')\n");
    }

    file_t db = open_file(USER_DB_PATH, "w");
    if(!db || !db->stream)
        printf("[ - ] Error\n");

    if(!file_Write(db, new_db.data))
        printf("ERROR\n");

    file_Destruct(db);

    return 1;
}