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
#include <math.h>


#define MYPORT "23224"

void compute(double *input, double result[4]){
	double resultValue[4];
	double C; // bit rate
	double p1,p2;

	/*
	*CALCULATE FUNCTION 
	*/
	resultValue[0] = input[0];
	input[3] = input[3]*pow(10,7);
	input[2] = input[2]*pow(10,3);
	input[1] = input[1]*pow(10,6);
	resultValue[1] = (input[2]/input[3])*pow(10,3);


	input[4] = pow(10,(input[4]/10)-3);
	input[6] = pow(10,(input[6]/10)-3);

	C = input[1]*log2( 1 + input[6]/input[4] );

	resultValue[2] = (input[5]/C)*pow(10,3);
	resultValue[3] = resultValue[1] + resultValue[2];

	for (int i = 0; i < 4; i++){
		result[i] = resultValue[i];
	}

}

int main(void){
	/*
    *SET UP UDP CONNECTION WITH AWS 
    */
	int sockfd;
	int rv;
	struct addrinfo hints;  			// the struct addrinfo have already been filled with relevant info
	struct addrinfo *servinfo; 			// point out the result
	struct sockaddr_storage their_addr;
	struct addrinfo *p;  				// tempoary point
	socklen_t addr_len;


	memset(&hints, 0, sizeof hints);  	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC; 		// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; 	// UDP dgram sockets
	hints.ai_flags = AI_PASSIVE;		// use my IP

	if ((rv = getaddrinfo("localhost", MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 0;
	}

	// loop through all the results and bind to the first we can----from Beej
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("serverC: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("serverC: bind");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	printf( "\nThe Server C is up and running using UDP on port <%s>.\n", MYPORT);


 	/*
   	*COMMUNICATE WITH AWS BY UDP 
 	*/
	while(1){

		addr_len = sizeof their_addr;
		
        /*
        *calcuarray[]:
        *[0] = linkID
        *[1] = bandwith
        *[2] = length
        *[3] = velocity
        *[4] = noise power
        *[5] = file size
        *[6] = signal power 
        */
		double calcuarray[7];
    	recvfrom(sockfd, calcuarray, sizeof calcuarray, 0 , (struct sockaddr *)&their_addr, &addr_len);

    	printf("The Server C received link information of link <%.0f>, file size <%.0f>, and signal power <%.0f>\n", calcuarray[0], calcuarray[5], calcuarray[6]);


          	    
  	    double result[4]={0,0,0,0};
    	compute(calcuarray,result);
    	printf("The Server C finished the calculation for link <%.0f>.\n", calcuarray[0]);
       		
    	
		sendto(sockfd, result, sizeof result , 0,(struct sockaddr *) &their_addr, addr_len);
		printf("The Server C finished sending the output to AWS.\n\n");
		
	}
}