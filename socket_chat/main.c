#include "list_usr.h"
#include <stdio.h>
#include <netinet/in.h>
#include <sys/_select.h>
#include <unistd.h>
#include "dynstr.h"
#include <string.h>
#include <stdlib.h>

static const int EXPT_SERV = -1;
static const int ONLY_SERV = -2;
static const int MAX_USR = 2;

bool correct_name(list_usr users, const char* name){
    unsigned long str_len = strlen(name);
    if(str_len < 3 || str_len > 16){
        return false;
    }

    if (strcmp(users->name, "bye") == 0) {
        return false;
    }

    while (users != NULL && users->is_named) {
        if(strcmp(users->name, name) == 0){
            return false;
        }
        users = users->next;
    }
    return true;
}


list_usr clear_closed(list_usr list){
    list_usr root = list;
    while (list != NULL) {
        if(list->is_named == 2){
            int fd_delete = list->val;
            list = list->next;
            deletelem(&root, fd_delete);
            close(fd_delete);
        }
        else{
            list = list->next;
        }
    }
    return root;
}


void send_all(list_usr fds, const char* mes, const int fd){
    while (fds != NULL) {
        if(fd != fds->val && fds->is_named == 1){ // == 1 будет отправлять только именованным
            size_t err = write(fds->val, mes, strlen(mes));
            if(err == -1){
                printf("Error in write");
            }
        }
        fds = fds->next;
    }
}


void server_message(list_usr fds, const char* mes, const int fd){
    if (fd != EXPT_SERV) {
        printf("%s", mes);
    }
    if (fd != ONLY_SERV) {
        send_all(fds, mes, fd);
    }
}


int init_ls_port(void){
    int ls = socket(AF_INET, SOCK_STREAM, 0);
    if(ls == -1){
        printf("Failed to create socket\n");
    }
    int port;
    scanf("%d", &port);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (0 != bind(ls, (struct sockaddr *) &addr, sizeof(addr))){
        printf("Failed to bind\n");
        exit(0);
    }

    if(-1 == listen(ls, 5)){
        printf("Failed to listen\n");
    }
    return ls;
}

void connect_member(list_usr* list, int fd){
    add_list(list, fd);
    server_message(*list, "<SM>: Someone joined with id: ", fd);
    // посылаем id
    char id_string[17];
    sprintf(id_string, "%d\n", fd);
    server_message(*list, id_string, fd);
    // -----------
    const char mes1[] = "<SM>: please enter your name: \n";
    if(write(fd, mes1, sizeof(mes1)) == -1){
        printf("Error write in get_name\n");
    }
}

void get_message(list_usr user){
    if(user->init_mes == false){
        init_str(&user->message);
        user->init_mes = true;
    }
    char buf[7];
    size_t count_read = read(user->val, buf, sizeof(buf));
    for (size_t i = 0; i < count_read; i++) {
        if(buf[i] == '\r')
            continue;
        if(buf[i] == '\n'){
            user->ready_to_send = true;
            // '\0' уже есть в конце
            return;
        }
        add_str_char(&user->message, buf[i]);
    }
}

void request_name(list_usr users, list_usr cur_user, int fd){
    const char mes3[] = "<SM>: OK!\n";
    const char error[] = "<SM>: invalid name\n";

    get_message(cur_user);
    if(cur_user->ready_to_send){
        cur_user->is_named = correct_name(users, cur_user->message.p);
        if(cur_user->is_named == false || strcmp(cur_user->message.p, "bye") == 0){
            write(fd, error, sizeof(error));
            cur_user->is_named = false;
            clear_message(cur_user);
            return;
        }
        write(fd, mes3, sizeof(mes3));
        strcpy(cur_user->name, cur_user->message.p);
        clear_message(cur_user);
    
        char message[128];
        sprintf(message, "<SM>: id %d is %s\n", fd, cur_user->name);
        server_message(users, message, fd);
    }
}

int main(int argc, const char* argv[]) {
    struct timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;
    printf("Port: ");
    int ls = init_ls_port();
    printf("Server is running\n");
    list_usr users = NULL;
    list_usr cur_usr = users;
    for(;;){
        cur_usr = users;
        fd_set readfds;
        int max_d = ls;
        int fd;
        FD_ZERO(&readfds);
        FD_SET(ls, &readfds);

        //  добавляем все ф.д. из списка сохраненных в множество
        cur_usr = users;
        while(!is_empty_list(cur_usr)){
            fd = cur_usr->val;
            FD_SET(fd, &readfds);
            if(fd > max_d){
                max_d = fd;
            }
            cur_usr = cur_usr->next;
        }

        int res = select(max_d+1, &readfds, NULL, NULL, &timeout);
        if(res < 1) {
            printf("<SM>: chat server is running... (%d users connected)\n", len_list(users));
        }

        // обрабатываем новое подключение
        if(FD_ISSET(ls, &readfds)){
            fd = accept(ls, NULL, NULL);
            if(fd == -1){
                printf("Error\n");
            }
            else{
                if(len_list(users) >= MAX_USR){
                    const char mes[] = "<SM>: server is full\n";
                    write(fd, mes, sizeof(mes));
                    close(fd);
                }
                else{
                    connect_member(&users, fd);
                }
            }
        }
        // обрабатываем ф д клиентов
        cur_usr = users;
        while(!is_empty_list(cur_usr)){
            fd = cur_usr->val;
            if(FD_ISSET(fd, &readfds)){
                if(cur_usr->is_named == false){
                    request_name(users, cur_usr, fd);
                }
                else{
                    get_message(cur_usr);
                    if(cur_usr->ready_to_send){
                        if(strcmp(cur_usr->message.p, "bye") == 0){
                            server_message(users, "<SM>: user ", fd);
                            server_message(users, cur_usr->name, fd);
                            server_message(users, " left\n", fd);
                            cur_usr->is_named = 2;
                        }
                        else{
                            server_message(users, cur_usr->name, fd);
                            server_message(users, ": ", fd);
                            server_message(users, cur_usr->message.p, fd);
                            server_message(users, "\n", fd);
                        }
                        clear_message(cur_usr);
                    }
                }
            }
            cur_usr = cur_usr->next;
        }
        users = clear_closed(users);
    }
    return 0;
}

// gcc -Wall main.c list_int.c dynstr.c -o chat && ./chat
