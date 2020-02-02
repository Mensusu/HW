//Source code from: Socket Programming Reference - (Beej's-Guide)---start
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//Source code from: Socket Programming Reference - (Beej's-Guide)---end

#define MYPORT "22819"	// the port connect to AWS
#define SIZE 10    
#define MAXDATASIZE 1024
using namespace std;

typedef struct _inf// struct used to store information come from AWS
{
int vexs[SIZE];
int start_index;
long long int file_size;
double prop_speed;
double trans_speed;
int len[SIZE];
int pre_nodes[SIZE];
int vexnum; 
}MSE;

// calculate the transmitting delay and propagation delay
void calculation(MSE message,double delay[SIZE][3]){
	for(int i=0;i<message.vexnum;i++){
		delay[i][0]=(double)message.file_size/(8*message.trans_speed);
		delay[i][1]=(double)message.len[i]/message.prop_speed;
		delay[i][2]=delay[i][0]+delay[i][1];
	}
}


// Source code from: Socket Programming Reference - (Beej's-Guide)--start
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	// UDP socket initialization
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	
	// variable used to send or recieve data
	char buf[MAXDATASIZE];
	MSE recv_aws;
	double send[SIZE][3];

	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(servinfo);
//Source code from: Socket Programming Reference - (Beej's-Guide)---------end


	printf("The Server B is up and running using UDP on port "MYPORT".\n");

	
	//main loop: 
	while(1) {
		//recieve from aws
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, &buf, MAXDATASIZE , 0,	(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		memset(&recv_aws,0,sizeof(recv_aws));
		memcpy(&recv_aws,buf,sizeof(recv_aws));

		printf("The Server B has received data for calculation: \n");
		printf("* Propagation speed: <%f> km/s; \n",recv_aws.prop_speed);
		printf("* Transmission speed <%f> Bytes/s; \n",recv_aws.trans_speed);
		for (int i=0;i<recv_aws.vexnum;i++)
			{
			if(recv_aws.len[i]!=0)
			printf("* Path length for destination %d : %d  \n",recv_aws.vexs[i],recv_aws.len[i]);

			}
		//calculate delay
		calculation(recv_aws,send);

		printf("The Server B has finished the calculation of the delays: \n");
		printf("------------------------\n");
		printf("Destination    Delay\n");
		printf("------------------------\n");
		for (int i=0;i<recv_aws.vexnum;i++)
			{
			if(recv_aws.len[i]!=0)
			printf(" %-12d %-10.2f\n",recv_aws.vexs[i],send[i][2]);
			}

		//send to aws
		if ((numbytes = sendto(sockfd, &send, 240, 0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
			perror("senderr: sendto");
			exit(1);
		}

		printf("The Server B has finished sending the output to AWS\n");
	}

	return 0;
}
