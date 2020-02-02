//Source code from: Socket Programming Reference - (Beej's-Guide)--start
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<iostream>
#include <arpa/inet.h>
#include <iomanip>
//Source code from: Socket Programming Reference - (Beej's-Guide)--end
#define PORT "24819" // the port client will be connecting to 
#define SIZE 10 
#define MAXDATASIZE 1024 // max number of bytes we can get at once 

using namespace std;

typedef struct _sendtoclient
{
int vexs[SIZE];
int len[SIZE];
int vexnum; 
double delay[SIZE][3];
}RESULT;

//Source code from: Socket Programming Reference - (Beej's-Guide)--start
void *get_in_addr(struct sockaddr *sa)// get sockaddr, IPv4 or IPv6:
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
//variable used to send or recieve data
	long long int send_buf[3];						
	RESULT recv_buf;			
	char buff[MAXDATASIZE];

	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc == 4 && strcmp(argv[0], "./client") == 0 );
	else {	
	    exit(1);
	}

	send_buf[0] = *argv[1]-'a';     //store input information
	send_buf[1] = atoi(argv[2]);
	send_buf[2] = strtoll(argv[3], NULL, 10);		
//** Source code from: Socket Programming Reference - (Beej's-Guide)
	// socket initialization
	memset(&hints, 0, sizeof hints);
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
	printf("The client is up and running.\n");

	freeaddrinfo(servinfo); 			
//Source code from: Socket Programming Reference - (Beej's-Guide)--end

	// send client input to server A
	if (send(sockfd, &send_buf, 24, 0) == -1)	
		 perror("send");

	printf("The client has sent query to AWS using TCP over port <"PORT"> start vertex <%lld>; map <%s>; file size <%lld>.\n",send_buf[1],argv[1],send_buf[2]);

	// receive result from AWS
	if ((numbytes = recv(sockfd, &buff, MAXDATASIZE-1, 0)) == -1) {		
	    perror("recv");
	    exit(1);
	}
	memset(&recv_buf,0,sizeof(recv_buf)); 
	memcpy(&recv_buf,buff,sizeof(recv_buf));//transfer buffer string to struct 
			
	printf("The client has received results from AWS:\n");
	printf("---------------------------------------------------------------------------\n");
	printf("Destination  Min Length     Tt       Tp       Delay \n");
	printf("---------------------------------------------------------------------------\n");
	for (int i=0;i<recv_buf.vexnum;i++)
		{
		if(recv_buf.len[i]!=0)
		printf("   %-10d %-10d %-10.2f  %-10.2f %-10.2f\n",recv_buf.vexs[i],recv_buf.len[i],recv_buf.delay[i][0],recv_buf.delay[i][1],recv_buf.delay[i][2]);
		}
	cout<<"---------------------------------------------------------------------------\n";


	return 0;
}
