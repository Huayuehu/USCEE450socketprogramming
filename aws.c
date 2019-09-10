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
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10// connections queue

#define PORT_FOR_CLIENT "25224"           
#define PORT_FOR_MONITOR "26224"
#define PORT_FOR_OUTPUT "24224"
#define PORTA "21224"
#define PORTB "22224"
#define PORTC "23224"



// get sockaddr, IPv4 or IPv6----from Beej
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



/*
*SET UP UDP CONNECTION WITH BACK_SERVER A & B 
*/
int search(int firstParameter, char port, double search_Result[5]){
    int udp_sock;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    // set up UDP----from Beej
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;


    const char* backserver_port;

        if (port == 'A'){
            backserver_port = PORTA;
        }else if (port == 'B'){
            backserver_port = PORTB;
        }

    if ((rv = getaddrinfo("localhost", backserver_port, &hints, &servinfo))
            != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }


    // loop through all the results and make a socket----from Beej
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((udp_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
                == -1) {
            perror("Backend-Server: socket");
            continue;
        }
        break;
    }


    
    /*
    *COMMUNICATE WITH BACK_SERVER A & B
    */
    // send LinkID to backServer-A and backServer-B
    int LinkID [1] = {firstParameter};
    sendto(udp_sock, LinkID, sizeof LinkID, 0, p->ai_addr,p->ai_addrlen);
    if(port == 'A'){
        printf("The AWS sent link ID = <%d> to Backend-Server A using UDP over port <%s>.\n", firstParameter, PORT_FOR_OUTPUT);
    }else{
        printf("The AWS sent link ID = <%d> to Backend-Server B using UDP over port <%s>.\n", firstParameter, PORT_FOR_OUTPUT);
    }

    // receive searching result from backServer-A or backServer-B
    double result[5];
    recvfrom(udp_sock, result, sizeof result, 0, NULL, NULL);


    // successfully search   
    if(result[0] != -1){
        if(port == 'A'){
            printf("The AWS received <1> matches from Backend-Server <A> using UDP over port <%s>.\n", PORT_FOR_OUTPUT);
        }else{
            printf("The AWS received <1> matches from Backend-Server <B> using UDP over port <%s>.\n", PORT_FOR_OUTPUT);
        }
    } 
    // search failed
    else{
        if(port == 'A'){
            printf("The AWS received <0> matches from Backend-Server <A> using UDP over port <%s>.\n", PORT_FOR_OUTPUT);
        }else{
            printf("The AWS received <0> matches from Backend-Server <B> using UDP over port <%s>.\n", PORT_FOR_OUTPUT);
        }
    }

    // pass the result to the main function
    for(int i = 0; i < 5; i++){
        search_Result[i] = result[i];
    }
   
}



/*
*SET UP UDP CONNECTION WITH BACK_SERVER C 
*/
int calculation(double calcu_Parameter[7], double calcu_Result[4]){
    int udp_sock;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    //set up UDP----from Beej
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4
    hints.ai_socktype = SOCK_DGRAM;



    if ((rv = getaddrinfo("localhost", PORTC, &hints, &servinfo))
            != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }


    // loop through all the results and make a socket----Beej
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((udp_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
                == -1) {
            perror("Backend-Server: socket");
            continue;
        }
        break;
    }


    /*
    *COMMUNICATE WITH BACK_SERVER C BY UDP
    */
    // send calculation parameters to backServer-C
    double  sendArray[7];

    for(int i = 0; i < 7; i++){

        sendArray[i]= calcu_Parameter[i];
    }
    sendto(udp_sock, sendArray, sizeof sendArray, 0, p->ai_addr,p->ai_addrlen);
    
    printf("The AWS sent link ID = <%.0f>, size = <%.0f>, power = <%.0f>, and link information to Backend-Server C using UDP over port <%s>.\n",
            sendArray[0], sendArray[5], sendArray[6], PORT_FOR_OUTPUT);
    

    // receive calculation result from backServer-C
    double result[4];
    recvfrom(udp_sock, result, sizeof result, 0, NULL, NULL);
   

    printf("The AWS received outputs from Backend-Server C using UDP over port <%s>.\n", PORT_FOR_OUTPUT);

    // pass the result to the main function
    for(int i = 0; i < 4; i++){
        calcu_Result[i] = result[i];
    }

}





int main(){

    /*
    *SET UP TCP CONNECTION WITH CLIENT 
    */    
    int sockfd, child_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    int sock1fd_recv, numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo("localhost", PORT_FOR_CLIENT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
                == -1) {
            perror("AWS: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
                == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("AWS: bind");
            continue;
        }


        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    //listen phase
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("\nThe AWS is up and running.\n");




    /*
    *SET UP TCP CONNECTION WITH MONITOR 
    */
    int sock2fd, child2_fd;  // listen on sock2fd, new connection on child2_fd
    struct addrinfo hints2, *servinfo2, *p2;
    struct sockaddr_storage their2_addr; // connector's address information
    socklen_t sin_size2; // IPv4 size
    struct sigaction sa;
    int yes2=1;
    char s2[INET6_ADDRSTRLEN];
    int rv2;


    memset(&hints2, 0, sizeof hints2);   // load address structure
    hints2.ai_family = AF_UNSPEC;
    hints2.ai_socktype = SOCK_STREAM;
    hints2.ai_flags = AI_PASSIVE; // use my IP

    if ((rv2 = getaddrinfo("localhost", PORT_FOR_MONITOR, &hints2, &servinfo2)) != 0) {       // loading address imformation
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
        if ((sock2fd = socket(p2->ai_family, p2->ai_socktype,              // if socket set error
                p2->ai_protocol)) == -1) {
            perror("AWS: socket");
            continue;
        }


        if (setsockopt(sock2fd, SOL_SOCKET, SO_REUSEADDR, &yes2, sizeof(int))
                == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sock2fd, p2->ai_addr, p2->ai_addrlen) == -1) {
            close(sock2fd);
            perror("AWS: bind");
            continue;
        }


        // sucessfull connect
        break;
    }


    freeaddrinfo(servinfo2); // all done with this structure

    // listen phase
    if (listen(sock2fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // accept monitor
    sin_size2 = sizeof their2_addr;   
    child2_fd = accept(sock2fd, (struct sockaddr *) &their2_addr, &sin_size2);
    if (child2_fd == -1) {
        perror("accept");           
        exit(1);
    }  

    // get the port of monitor
    inet_ntop(their2_addr.ss_family, get_in_addr((struct sockaddr *) &their2_addr), s2, sizeof s2);
    struct sockaddr_in addrTheir2;
    memset(&addrTheir2, 0, sizeof(addrTheir2));
    int len2 = sizeof(addrTheir2);
    getpeername(child2_fd, (struct sockaddr *) &addrTheir2, (socklen_t *) &len2);
    int for_monitor_port = addrTheir2.sin_port;  


    
    while(1){
         // accept client
        sin_size = sizeof their_addr;   
        child_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (child_fd == -1) {
            perror("accept");           
            exit(1);
        }

        // get the port of client
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        struct sockaddr_in addrTheir;
        memset(&addrTheir, 0, sizeof(addrTheir));
        int len = sizeof(addrTheir);
        getpeername(child_fd, (struct sockaddr *) &addrTheir, (socklen_t *) &len);
        int for_client_port = addrTheir.sin_port;



        /*
        *COMMUNICATE WITH CLIENT & MONITOR BY TCP
        */
        // receive input parameters from client
        double input_buffer[3];
        recv(child_fd, input_buffer, sizeof input_buffer, 0);
        printf("The AWS received link ID = <%.0f>, size = <%.0f>, and power = <%.0f> from the client using TCP over port <%s>.\n",
            input_buffer[0], input_buffer[1], input_buffer[2], PORT_FOR_CLIENT);

        // send three received parameters to monitor
        send(child2_fd, input_buffer, sizeof input_buffer, 0);
        printf("The AWS sent link ID = <%.0f>, size = <%.0f>, and power = <%.0f> to the monitor using TCP over port <%s>.\n",
            input_buffer[0], input_buffer[1], input_buffer[2], PORT_FOR_MONITOR);


        // call search function to communicate with Server A and B
        double search_Result[5] = {0};
        double search_Result_A[5] = {0};
        double search_Result_B[5] = {0};

        search(input_buffer[0], 'A', search_Result_A);      // input_buffer[0] = linkID
        search(input_buffer[0], 'B', search_Result_B);

        
        // either A or B finds the result, put the result in search_Result
        if (search_Result_A[0] != -1){
            for(int i = 0; i < 5; i++){
                search_Result[i] = search_Result_A[i];
            }
        }else if (search_Result_B[0] != -1){
            for(int i = 0; i < 5; i++){
                search_Result[i] = search_Result_B[i];
            }
        }


        // if both A and B can't find result
        if(search_Result_A[0] == -1 && search_Result_B[0] == -1){

            // send No match result to client
            send(child_fd, search_Result_A, sizeof search_Result_A, 0);     //child_fd is TCP socket connecting to client
            send(child2_fd, search_Result_A, sizeof search_Result_A, 0);     //sock2fd is TCP socket connecting to monitor
            printf("The AWS sent No Matches to the monitor and the client using TCP over ports<%s> and <%s>, respectively.\n\n", PORT_FOR_MONITOR, PORT_FOR_CLIENT);
        
        }else{ 
            // if do find result, send them to C
            // first 5 are searching result from Server A and B
            double calcu_Array[7];
            for(int i = 0; i < 5; i++){
                 calcu_Array[i] = search_Result[i];
            }

            // last 2 are 'input parameter from client'
            calcu_Array[5] = input_buffer[1];
            calcu_Array[6] = input_buffer[2];


            /*
            *calcu_Array[]:
            *[0] = linkID
            *[1] = bandwith
            *[2] = length
            *[3] = velocity
            *[4] = noise power
            *[5] = file size
            *[6] = signal power 
            */
            // call calculation function
            double calcu_Result[4];
            calculation(calcu_Array, calcu_Result);



            // send final calculation result to client
            double final_Result[1];
            final_Result[0] = calcu_Result[3];      // put the last number(delay) into final_result[0] ---> send to client

            send(child_fd, final_Result, sizeof final_Result, 0);
            printf("The AWS sent delay = <%.2f>ms to the client using TCP over port <%s>.\n", final_Result[0], PORT_FOR_CLIENT);

            // send final calculation result to monitor
            send(child2_fd, calcu_Result, sizeof calcu_Result, 0);
            printf("The AWS sent detailed results to the monitor using TCP over port <%s>.\n\n", PORT_FOR_MONITOR);
        }
    
    }
    
}