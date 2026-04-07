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
        printf("Server online!\n");

        while(1){
        if (listen(sfd, 8) < 0){
            perror("listen");
            continue;
        }
        if ((acc = (accept(sfd, ai->ai_addr, &ai->ai_addrlen))) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Connected!\n");}
        break;
    }
    
    if (ai == NULL){
        fprintf(stderr, "Something went wrong\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(gai);
    close(sfd);
    return acc;
    
    
    
}


int main(int argc, char** argv){
    int sfd; 
    int len;
    int ret;
    
    uint8_t buf[1024];
    struct pollfd pfd[30];
    if(argc == 1 || argc > 3){
        fprintf(stderr, "Usage: ./nc [HOST] PORT\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 2){
       sfd = serv(argv[1]); 
    }
    
    
    
    // for (;;){
        
    //     // if ((ret = poll(pfd, 2, 20000)) < 0){
    //     //     perror("poll");
    //     // }

    //     if (ret > 0){
    //         bzero(&buf, sizeof(buf));
            
    //         if (pfd[0].revents & POLLIN){
    //             if ((len = recv(sfd, buf, sizeof(buf), 0)) < 0){
    //                 perror("recv");
    //                 exit(EXIT_FAILURE);
    //             }
    //             fprintf(stderr, (char*) buf, len);
                
                
    //         }
    //     }
    // }
        
    return 0;
}