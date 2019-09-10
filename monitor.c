#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define AWSPORT "26224" // the port monitor will be connecting to 
#define BACKLOG 10     // pending connections queue
#define MAXDATASIZE 100 // max number of bytes we can get at once


// get sockaddr, IPv4 or IPv6----from Beej
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
int main()
{
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);           // load address structure
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", AWSPORT, &hints, &servinfo)) != 0) {   // check if loading address is success
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,      // socket set
                p->ai_protocol)) == -1) {
            perror("monitor: socket");
            continue;
        }


        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {    // bind failed
            close(sockfd);
            perror("monitor: connect");
            continue;
        }

        // sucessful connect
        break;
    }


    printf("\nThe monitor is up and running.\n"); //monitor boot up

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), // transfer address
        s, sizeof s);

    freeaddrinfo(servinfo); // all done with this structure


/*
*PHASE #2: COMMUNICATE WITH AWS BY TCP
*/
    while(1){  
        // receive input parameter from aws
        double input_Parameter[3];
        recv(sockfd, input_Parameter, sizeof input_Parameter, 0);

        printf("The monitor received input=<%.0f>, size=<%.0f>, and power=<%.0f> from the AWS.\n",
            input_Parameter[0],input_Parameter[1],input_Parameter[2]);

        
        // receive final calculation result from aws
        double final_result[5];
        recv(sockfd, final_result, sizeof final_result, 0);

        if(final_result[0] != -1){         
            printf("The result for link<%.0f>:\n", final_result[0]);
            printf("Tt = <%.2f>ms.\n", final_result[2]);
            printf("Tp = <%.2f>ms.\n", final_result[1]);
            printf("Delay = <%.2f>ms.\n\n", final_result[3]);

        }else{
            printf("Found no matches for link<%.0f>.\n\n",input_Parameter[0]);
            continue;    
        }

     }

}
