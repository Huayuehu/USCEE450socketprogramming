/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define AWSPORT "25224" // the port client will be connecting to 


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/*
*PHASE #1: SET UP TCP CONNECTION
*/

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 4) {
        fprintf(stderr,"\nPlease enter the initial values.\n\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", AWSPORT, &hints, &servinfo)) != 0) { // loading address imformation
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,      // socket set error -> return -1
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { // if connet phase error
            close(sockfd);
            perror("client: connect");
            continue;
        }

        // sucessfull connect
        break;
    }


    printf("\nThe client is up and running.\n"); //client boot up


    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), // transfer address
            s, sizeof s);

    freeaddrinfo(servinfo); // all done with this structure


/*
*PHASE #2: COMMUNICATE WITH AWS BY TCP
*/
    // send three parameters to AWS
    double input[3];
    for(int i = 0; i < 3; i++){
        input[i] = atof(argv[i+1]);
    }
    
    send(sockfd, input, sizeof input, 0);
    printf("The client send ID=<%.0f>, size=<%.0f>, and power=<%.0f> to AWS.\n", input[0],input[1],input[2]);


    // reseive final calculation result from aws
    double result_buffer[1];
    recv(sockfd, result_buffer, sizeof result_buffer, 0);

    if(result_buffer[0] == -1){
        printf("Found no matches for link<%.0f>.\n\n", input[0]);
    }else{
        printf("The delay for link<%.0f> is <%.2f>ms.\n\n", input[0], result_buffer[0]);
    }

}