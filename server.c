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

#define MAXNUM 30


typedef struct user_t{
    int sfd;
    char name[16];
} user;

typedef struct userlist_t{
    int max_num;
    int current_num;
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

userlist* init_userlist(){
    userlist* UL;
    UL = (userlist*) malloc(sizeof *UL);
    UL->current_num = 0;
    UL->max_num = MAXNUM;

    UL->user_list = (user*) malloc(sizeof *UL->user_list);
    for(int i = 0; i < MAXNUM; i++){
    }

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


int main(int argc, char** argv){
    int sfd; 
    int len;
    int ret;
    int userfd;
    uint8_t buf[1024];
    struct pollfd pfd[31];
    if(argc == 1 || argc > 3){
        fprintf(stderr, "Usage: ./nc [HOST] PORT\n");
        exit(EXIT_FAILURE);
    }
    pfd[0].fd = serv(argv[1]);
    pfd[0].events = POLLIN;
    for(;;){
        
    }
    
    
        
    return 0;
}