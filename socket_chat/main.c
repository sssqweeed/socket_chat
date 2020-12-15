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
static const int LEN_BUF = 7;

list_usr clear_closed(list_usr list){
    list_usr root = list;
    while (list != NULL) {
        if(list->is_named == 2){
            int fd_delete = list->val;
            list = list->next;
            delete_elem(&root, fd_delete);
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

void fill_buffer(list_usr user){
    if(user->is_init_buf == false){
        init_str(&user->buffer);
        user->is_init_buf = true;
    }
    char buf[LEN_BUF];
    size_t count_read = read(user->val, buf, sizeof(buf));
    for (size_t i = 0; i < count_read; i++) {
        if(buf[i] == '\r')
            continue;
        add_str_char(&user->buffer, buf[i]);
    }
}

bool ready_to_send(list_usr user){
    int i = 0;
    while (user->buffer.p[i] != 0) {
        if (user->buffer.p[i] == '\n') {
            return true;
        }
        i++;
    }
    return false;
}

bool correct_name(list_usr users, const char* name){
    unsigned long str_len = strlen(name);
    if(str_len < 3 || str_len > 16){
        return false;
    }

    if (strcmp(name, "bye") == 0) {
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

struct _string get_message_from_buf(list_usr user){
    struct _string message;
    init_str(&message);
    while(user->buffer.p[0] != '\n'){
        add_str_char(&message, user->buffer.p[0]);
        delete_first(&user->buffer);
    }
    delete_first(&user->buffer);
    return message;
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
                fill_buffer(cur_usr);
                //if(cur_usr->ready_to_send){
                while (ready_to_send(cur_usr)) {
                    struct _string message = get_message_from_buf(cur_usr);
                    if(!cur_usr->is_named){
                        const char mes3[] = "<SM>: OK!\n";
                        const char error[] = "<SM>: invalid name\n";
                        if(correct_name(users, message.p)){
                            strcpy(cur_usr->name, message.p);
                            write(fd, mes3, sizeof(mes3));
                            char mes[128];
                            sprintf(mes, "<SM>: id %d is %s\n", fd, cur_usr->name);
                            server_message(users, mes, fd);
                            cur_usr->is_named = true;
                        }
                        else{
                            write(fd, error, sizeof(error));
                            cur_usr->is_named = false;
                        }
                    }
                    else{
                        if(strcmp(message.p, "bye") == 0){
                            server_message(users, "<SM>: user ", fd);
                            server_message(users, cur_usr->name, fd);
                            server_message(users, " left\n", fd);
                            cur_usr->is_named = 2;
                        }
                        else{
                            server_message(users, cur_usr->name, fd);
                            server_message(users, ": ", fd);
                            server_message(users, message.p, fd);
                            server_message(users, "\n", fd);
                        }
                    }
                    dispose_str(&message);
                }
            }
            cur_usr = cur_usr->next;
        }
        users = clear_closed(users);
    }
    return 0;
}

// gcc -Wall main.c list_int.c dynstr.c -o chat && ./chat
