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
    int sfd;
    char name[16];
} user;

typedef struct userlist_t{
    struct pollfd pfd[MAXNUM];
    int current_number_of_user;
    user* user_list;
} userlist;

typedef struct channel_t{
    char name[16];
    int member_count;
    user* member_list;
} channel;

typedef struct channels_t{
    int max_channels;
    int channel_counter;
    channel* channel_names;
    
} channels;

struct recv_command {
    int argc;
    char **argv;
};


userlist* init_userlist(){
    userlist* UL;
    UL = (userlist*) malloc(sizeof *UL);
    assert(UL);
    
    UL->user_list = (user*) malloc(sizeof *UL->user_list);
    assert(UL->user_list);
    UL->current_number_of_user = 1;
    return UL;
}

void free_userlist(userlist* UL){
    free(UL->user_list);
    free(UL);
}

int serv(char* port){
    struct addrinfo hints, *gai, *ai;
    int err;
    int sfd; 
    int acc;
    
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((err = getaddrinfo(NULL, port, &hints, &gai)) != 0){
        fprintf(stderr, "./server: getaddrinfo [%s] %s", port, gai_strerror(err));
        exit(EXIT_FAILURE);
    } 
    
    for (ai = gai; ai != NULL; ai = ai->ai_next){
        if ((sfd = socket(ai->ai_family, ai->ai_socktype, 0)) < 0){
            perror("socket");
            continue;
        }
        int v = 1;
        if ((setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(int))) < 0){
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
    return sfd;
}

struct recv_command * parse_answer(const char *buf, int len)
{
    struct recv_command *cmd = NULL;
    char p = ' ';
    int n = 0, beg = 0;
    int isAQuote = 0;
    const char* s = buf;
    
    while(*s){
        if (isspace(p) && !isspace(*s)) n++;
        p = *s++;
    }

    if ((cmd = malloc(sizeof(*cmd))) == NULL)
        perror("malloc cmd");
    
    cmd->argc = 0;
    if ((cmd->argv = malloc((n + 1) * sizeof(*cmd->argv))) == NULL)
        perror("malloc cmd->argv");
    
    for (int i = 0; i < len; i++) {
        if (!isspace(buf[i]) && (isspace(p))) 
        beg = i; 
        
        else if ((isspace(buf[i]))) {
            cmd->argv[cmd->argc++] = strndup(buf + beg, i - beg);
        }
        p = buf[i];
    }
    strcat(cmd->argv[cmd->argc-1], "\n");
    cmd->argv[cmd->argc] = NULL;
    return cmd;
}

void free_answer(struct recv_command * arr){
    for(int i = 0; i<arr->argc; i++){
        free(arr->argv[i]);
    }
    free(arr->argv);
    free(arr);
}

int main(int argc, char** argv){
    int sfd; 
    int len;
    int ret;
    int userfd;
    userlist* UL;
    uint8_t buf[1024];
    struct recv_command* arr;
    
    if(argc == 1 || argc > 3){
        fprintf(stderr, "Usage: ./server.out PORT\n");
        exit(EXIT_FAILURE);
    }
    UL = init_userlist();
    UL->pfd[0].fd = serv(argv[1]);
    UL->pfd[0].events = POLLIN;
    for(;;){
        ret = poll(UL->pfd, UL->current_number_of_user, -1);
        
        if (ret > 0){
            bzero(&buf, sizeof(buf));
            
            if(UL->pfd[0].revents & POLLIN){
                userfd = accept(UL->pfd[0].fd, NULL, NULL);
                
                UL->pfd[UL->current_number_of_user].fd = userfd;
                UL->pfd[UL->current_number_of_user].events = POLLIN;
                UL->user_list[UL->current_number_of_user++].sfd = userfd;
                strcpy(UL->user_list[UL->current_number_of_user++].name, "");
            }
            else{
                for(int i = 1; i < UL->current_number_of_user; i++){
                    if(UL->pfd[i].revents & POLLIN){
                        if ((len = recv(UL->pfd[i].fd, buf, sizeof(buf), 0)) < 0){
                            perror("recv");
                            exit(EXIT_FAILURE);
                        }
                        if (len == 0 || UL->pfd[i].revents & POLLHUP){
                            for (int j = i; j < UL->current_number_of_user; j++){
                                UL->user_list[j].sfd = UL->user_list[j+1].sfd; 
                                UL->pfd[j].fd = UL->pfd[j+1].fd;

                            }
                            UL->current_number_of_user--;
                        }
                        else{    
                            arr = parse_answer((char*) buf, len);
                            
                            if (strlen(arr->argv[0]) != 5){
                                send(UL->pfd[i].fd, "ERR! 01\n", 8, 0);
                                continue;
                            }
                            free_answer(arr);
                        }
                    }
                }
            }
        }
    }

    return 0;
}