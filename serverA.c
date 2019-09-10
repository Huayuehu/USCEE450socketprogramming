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


#define MYPORT "21224"
#define BUFFER_SIZE 1024

/*
search()
*/
int search(double p[5]){

	char buf[BUFFER_SIZE];
	FILE *fp; 
	fp = fopen("database_a.csv", "r"); // reading only open file "database_a.csv"

	if (!fp) { 
		fprintf(stderr, "failed to open file for reading\n"); 
		exit(1); 
	}

	while(fgets(buf, sizeof(buf), fp) != NULL) {// while there is next line, go into loop and get all LINK_ID row
		char *tok = strtok(buf, ",");// get LINK_ID
		double linkid = atof(tok);

		/*
		if it is exactly the LINK_ID that inputarray[0] refer to 
		-> get that linkid, bandwidth, length, velocity and noisepower 
		*/
		if(linkid == p[0]){
			tok = strtok(NULL, ",");
			double bandwidth = atof(tok);
	        tok = strtok(NULL, ",");
	        double length = atof(tok);
	        tok = strtok(NULL, ",");
	        double velocity = atof(tok);
	        tok = strtok(NULL, ",");
	        double noisepower = atof(tok);
  
	        p[0] = linkid;
	        p[1] = bandwidth;
	        p[2] = length;
	        p[3] = velocity;
	        p[4] = noisepower;

	        
	        return 1;
	    }
	}

	// if at the end of database, we still can't find linkid -> set p[0] = -1
	p[0] = -1;
	return 1;
}



int main(void){
	/*
    *SET UP UDP CONNECTION WITH AWS 
    */

	int sockfd;
	int rv;
	struct addrinfo hints;  				// the struct addrinfo have already been filled with relevant info
	struct addrinfo *servinfo; 				// point out the result
	struct sockaddr_storage their_addr;
	struct addrinfo *p; 					// tempoary point
	socklen_t addr_len;



	memset(&hints, 0, sizeof hints);  	// set struct to all zero
	hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM;	 	// UDP dgram sockets
	hints.ai_flags = AI_PASSIVE; 		// use my IP

	if ((rv = getaddrinfo("localhost", MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 0;
	}
// loop through all the results and bind to the first we can----from Beej
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("serverA: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("serverA: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);
	printf( "\nThe Server A is up and running using UDP on port <%s>.\n", MYPORT);



	/*
    *COMMUNICATE WITH AWS 
    */

	while(1){
		addr_len = sizeof their_addr;
		
		// receive input from AWS
		int LinkID[1];
    	recvfrom(sockfd, LinkID, sizeof LinkID, 0 , (struct sockaddr *)&their_addr, &addr_len);
    	printf("The Server A received input <%d>.\n", *LinkID);


    	// after receiving LinkID, put that number into inputarray[0], then calling search()
    	double inputarray[5] = {0,0,0,0,0};
		double result[5] = {0,0,0,0,0};
    	inputarray[0] = LinkID[0];
    	search(inputarray);

	    // if we do find the result
	    if (inputarray[0] != -1){

	    	for (int i = 0; i < 5; i++){

		    	result[i] = inputarray[i];

	        }
	    
	    }else {
	    	result[0] = -1;		// if we do not find the result, set result[0] = -1	
	    }
    

    	if(result[0] == -1){
    		printf("The Server A has found <0> matches.\n");
    	}else{
			printf("The Server A has found <1> matches.\n");
		}


    	// send result to AWS
		sendto(sockfd, result, sizeof result , 0,(struct sockaddr *) &their_addr, addr_len);

		printf("The Server A finished sending the output to AWS.\n");

	}
}