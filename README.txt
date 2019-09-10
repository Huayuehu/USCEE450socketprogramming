a. Name: Huayue Hua
b. Student ID: 9817961224
c. What have I done in this assignment:
	I wrote code for serverA, serverB, serverC, AWS, monitor, client using C language to realize all the requirement as described in the Project requirement. 
	In Phase1, all four server programs (AWS, Back-Server A, B,& C) boot up and run first, then monitor and client boot up and run. The client code will take input argument (LINK_ID, SIZE, POWER) from the command line. In Phases2A AWS server receives <LINK_ID> from client and sends it to storage sever A and B. After receiving <LINK_ID> from AWS, server A and B perform the {search} operation on their respective databases for an exact match of the given <LINK_ID> and send the result back to AWS.In Phases2B once the AWS server receives replies from both storage servers, the AWS server will send all the information it has to computing server C if storage servers do find the result. Then server C begins calculating. In Phases3 back-server C will make the calculation and send the final result back to AWS. At last, the AWS will send those results to client and monitor respectively as required. 

d. What each file does:
	serverA.c: (1) receives the LINK_ID from AWS; (2) reads data from database_a.csv file; (3) sends to aws server.	
	serverB.c: (1) receives the LINK_ID from AWS; (2) reads data from database_b.csv file; (3) sends to aws server.
	serverC.c: (1) receives link information, file size, and signal power from AWS; (2) makes the calculation; (3) sends back the result to AWS.
	aws.c: (1) receives link id, file size, signal power from client; (2) sends LINK_ID, FILEZ_SIZE, SIGANL POWER to monitor; (3) sends LINK_ID to server A and B and receives the searching result; (4) sends link information, file size, and signal power to server C and receives the computing result; (5) sends those results to client and monitor respectively as required.
	client.c: (1) takes input argument (LINK_ID, SIZE, POWER) from the command line and sends them to AWS; (2) receives end-to-end delay from AWS and display it.
	monitor.c: (1) receives link id, file size, signal power from AWS and display them; (2) receives the final results and display them.
 
e. The format of all the messages exchanged:
	1. serverA:
	The Server A is up and running using UDP on port <21224>. 
	The Server A received input <148>.
	The Server A has found <1> matches.
	The Server A finished sending the output to AWS.

	2. serverB:
	The Server B is up and running using UDP on port <22224>. 
	The Server B received input <148>.
	The Server B has found <0> matches.
	The Server B finished sending the output to AWS.

	3. serverC:
	The Server C is up and running using UDP on port <23224>.
	The Server C received link information of link <148>, file size <10000>, and signal power <-30>.
	The Server C finished the calculation for link <148>.
	The Server C finished sending the output to AWS.

	4. aws:
   	The AWS is up and running.
	The AWS received link ID=<148>, size=<10000>, and power=<-30> from the client using TCP over port <25224>.
	The AWS sent link ID=<148>, size=<10000>, and power=<10> to the monitor using TCP over port <26224>.
	The AWS sent link ID=<148> to Backend-Server A using UDP over port <24224>.
	The AWS received <1> matches from Backend-Server < A > using UDP over port <24224>.
	The AWS sent link ID=<148> to Backend-Server B using UDP over port <24224>.
	The AWS received <0> matches from Backend-Server < B > using UDP over port <24224>.
	The AWS sent link ID=<148>, size=<10000>, power=<-30>, and link information to Backend-Server C using UDP over port <24224>.
	The AWS received outputs from Backend-Server C using UDP over port <24224>.
	The AWS sent delay=<0.04>ms to the client using TCP over port <25224>.
	The AWS sent detailed results to the monitor using TCP over port <26224>.
	(The AWS sent No Match to the monitor and the client using TCP over ports <26224> and <25224>, respectively.)

	5. client:
	The client is up and running.
	The client sent link ID=<148>, size=<10000>, and power=<-30> to AWS.
	The delay for link <148> is <0.04>ms.
	(Found no matches for link <330>.)

	6. monitor:
	The monitor is up and running.
	The monitor received input=<148>, size=<10000>, and power=<-30> from the AWS.
	The result for link <148>:
	Tt = <0.01>ms
	Tp = <0.03>ms
	Delay = <0.04>ms
	(Found no matches for link <330>.)

g. Idiosyncrasy or fail situation:
	Under my test case, it should be no fail if obeying the order as follows: four servers(AWS, Back-Server A, B, C), monitor, client.

h. Reused code:
	Some code block for setting TCP and UDP are from "Beej's Guide to Network Programming -- Using Internet Sockets". They are marked in the program. 