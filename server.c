#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <assert.h>
#include <ctype.h>

#define MAXNUM 31

typedef struct user_t{
    struct pollfd pfd;
    char name[16];
    int channel_count;
    int channel_ids[MAXNUM];
} user;

typedef struct userlist_t{
    int current_number_of_user;
    user users[MAXNUM];
} userlist;

typedef struct channel_t{
    char name[16];
    int member_count;
    int member_list[MAXNUM];
} channel;

typedef struct channels_t{
    int channel_counter;
    channel channel_names[MAXNUM];
} channels;

struct recv_command{
    int argc;
    char **argv;
};

userlist* init_userlist(){
    userlist* UL = malloc(sizeof(userlist));
    assert(UL);
    UL->current_number_of_user = 1;
    return UL;
}

channels* init_channellist(){
    channels* CL = malloc(sizeof(channels));
    assert(CL);
    CL->channel_counter = 0;
    return CL;
}

void free_userlist(userlist* UL){
    free(UL);
}

void free_channellist(channels* CL){
    free(CL);
}

int serv(char* port){
    struct addrinfo hints, *gai, *ai;
    int err;
    int sfd;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, port, &hints, &gai)) != 0){
        fprintf(stderr, "./server: getaddrinfo [%s] %s\n", port, gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    for (ai = gai; ai != NULL; ai = ai->ai_next){
        if ((sfd = socket(ai->ai_family, ai->ai_socktype, 0)) < 0){
            perror("socket");
            continue;
        }

        int v = 1;

        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(int)) < 0){
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        if (bind(sfd, ai->ai_addr, ai->ai_addrlen) < 0){
            perror("bind");
            continue;
        }

        if (listen(sfd, 8) < 0){
            perror("listen");
            continue;
        }

        break;
    }

    if (ai == NULL){
        fprintf(stderr, "Something went wrong\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(gai);

    fprintf(stderr, "SERVER IS ONLINE\n");

    return sfd;
}

struct recv_command* parse_answer(const char *buf, int len){
    struct recv_command *cmd = NULL;
    char p = ' ';
    int n = 0;
    int beg = 0;
    const char* s = buf;

    while (*s){
        if (isspace(p) && !isspace(*s)) n++;
        p = *s++;
    }

    cmd = malloc(sizeof(*cmd));
    assert(cmd);

    cmd->argc = 0;

    cmd->argv = malloc((n + 1) * sizeof(*cmd->argv));
    assert(cmd->argv);

    p = ' ';

    for (int i = 0; i < len; i++){
        if (!isspace(buf[i]) && isspace(p)){
            beg = i;
        }
        else if (isspace(buf[i]) && !isspace(p)){
            cmd->argv[cmd->argc++] = strndup(buf + beg, i - beg);
        }

        p = buf[i];
    }

    cmd->argv[cmd->argc] = NULL;

    return cmd;
}

void free_answer(struct recv_command *arr){
    for (int i = 0; i < arr->argc; i++){
        free(arr->argv[i]);
    }

    free(arr->argv);
    free(arr);
}

int main(int argc, char** argv){
    int len;
    int ret;
    int userfd;

    uint8_t buf[1024];
    char tmp_buf[1024];

    struct recv_command* arr;

    userlist* UL;
    channels* CL;

    if (argc != 2){
        fprintf(stderr, "Usage: ./server.out PORT\n");
        exit(EXIT_FAILURE);
    }

    UL = init_userlist();
    CL = init_channellist();

    UL->users[0].pfd.fd = serv(argv[1]);
    UL->users[0].pfd.events = POLLIN;

    for (;;){
        struct pollfd pfds[MAXNUM];

        for (int i = 0; i < UL->current_number_of_user; i++){
            pfds[i] = UL->users[i].pfd;
        }

        ret = poll(pfds, UL->current_number_of_user, -1);

        if (ret < 0){
            perror("poll");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < UL->current_number_of_user; i++){
            UL->users[i].pfd.revents = pfds[i].revents;
        }

        bzero(buf, sizeof(buf));

        if (UL->users[0].pfd.revents & POLLIN){
            userfd = accept(UL->users[0].pfd.fd, NULL, NULL);

            if (userfd < 0){
                perror("accept");
                continue;
            }

            int COUNTER = UL->current_number_of_user;

            UL->users[COUNTER].pfd.fd = userfd;
            UL->users[COUNTER].pfd.events = POLLIN;
            UL->users[COUNTER].channel_count = 0;

            strcpy(UL->users[COUNTER].name, "");

            UL->current_number_of_user++;
        }

        for (int i = 1; i < UL->current_number_of_user; i++){
            if (UL->users[i].pfd.revents & POLLIN){
                len = recv(UL->users[i].pfd.fd, buf, sizeof(buf), 0);

                if (len < 0){
                    perror("recv");
                    continue;
                }

                if (len == 0 || (UL->users[i].pfd.revents & POLLHUP)){
                    close(UL->users[i].pfd.fd);

                    for (int j = i; j < UL->current_number_of_user - 1; j++){
                        UL->users[j] = UL->users[j + 1];
                    }

                    UL->current_number_of_user--;
                    i--;

                    continue;
                }

                arr = parse_answer((char*)buf, len);

                if (arr->argc < 1){
                    free_answer(arr);
                    continue;
                }

                if (strlen(arr->argv[0]) != 4){
                    send(UL->users[i].pfd.fd, "ERR! 01\n", 8, 0);
                    free_answer(arr);
                    continue;
                }

                else if (strcmp(arr->argv[0], "NAME") == 0){
                    if (arr->argc != 2){
                        send(UL->users[i].pfd.fd, "ERR! 10\n", 8, 0);
                        free_answer(arr);
                        continue;
                    }

                    if (strlen(arr->argv[1]) > 15){
                        send(UL->users[i].pfd.fd, "ERR! 10\n", 8, 0);
                        free_answer(arr);
                        continue;
                    }

                    strcpy(UL->users[i].name, arr->argv[1]);

                    printf("%s %d\n", UL->users[i].name, UL->users[i].pfd.fd);

                    send(UL->users[i].pfd.fd, "OKAY\n", 5, 0);

                    free_answer(arr);

                    continue;
                }

                else if (strcmp(UL->users[i].name, "") == 0){
                    send(UL->users[i].pfd.fd, "ERR! 02\n", 8, 0);
                    free_answer(arr);
                    continue;
                }

                else if (strcmp(arr->argv[0], "JOIN") == 0){
                    if (arr->argc != 2){
                        send(UL->users[i].pfd.fd, "ERR! 10\n", 8, 0);
                        free_answer(arr);
                        continue;
                    }
                    else if (strlen(arr->argv[1]) > 15){
                        send(UL->users[i].pfd.fd, "ERR! 10\n", 8, 0);
                        free_answer(arr);
                        continue;
                    }
                    
                    int FOUND_CHANNEL = 0;

                    for (int j = 0; j < CL->channel_counter; j++){
                        if (strcmp(arr->argv[1], CL->channel_names[j].name) == 0){
                            CL->channel_names[j].member_list[CL->channel_names[j].member_count++] = UL->users[i].pfd.fd;

                            UL->users[i].channel_ids[UL->users[i].channel_count++] = j;
                            
                            snprintf(tmp_buf, sizeof(tmp_buf), "MEMB %d\n", CL->channel_names[j].member_count);
                            send(UL->users[i].pfd.fd, tmp_buf, strlen(tmp_buf), 0);

                            for (int k = 0; k < CL->channel_names[j].member_count; k++){
                                for (int l = 1; l < UL->current_number_of_user; l++){
                                    if (CL->channel_names[j].member_list[k] == UL->users[l].pfd.fd){
                                        snprintf(tmp_buf, sizeof(tmp_buf), "%s\n", UL->users[l].name);
                                        //TODO: send JOIN member to other members of said channel
                                        send(UL->users[i].pfd.fd, tmp_buf, strlen(tmp_buf), 0);
                                    }
                                }
                            }

                            FOUND_CHANNEL = 1;

                            break;
                        }
                    }

                    if (!FOUND_CHANNEL){
                        int COUNTER = CL->channel_counter;

                        strcpy(CL->channel_names[COUNTER].name, arr->argv[1]);

                        CL->channel_names[COUNTER].member_count = 0;

                        CL->channel_names[COUNTER].member_list[CL->channel_names[COUNTER].member_count++] = UL->users[i].pfd.fd;

                        UL->users[i].channel_ids[UL->users[i].channel_count++] = COUNTER;

                        CL->channel_counter++;

                        printf("NEW CHANNEL -- %s\n", CL->channel_names[COUNTER].name);
                        sprintf(tmp_buf, "MEMB 1\n %s\n", UL->users[i].name);
                        send(UL->users[i].pfd.fd, tmp_buf, strlen(tmp_buf), 0);

                    }

                    free_answer(arr);

                    continue;
                }

                free_answer(arr);
            }
        }
    }

    free_userlist(UL);
    free_channellist(CL);

    return 0;
}