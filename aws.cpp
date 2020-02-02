//Source code from: Socket Programming Reference - (Beej's-Guide)--start
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
//end

#define PORT "24819"  // the port users will be connecting to
#define UDP_PORT "23819"
#define SERVER_A_PORT "21819"
#define SERVER_B_PORT "22819"

#define MAXDATASIZE 1024
#define BACKLOG 10
#define SIZE 10    

using namespace std;

typedef struct _inf //struct uesd to store information to be sent or recieved
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

typedef struct _sendtoclient//struct uesd to store information to be sent or recieved
{
int vexs[SIZE];
int len[SIZE];
int vexnum; 
double delay[SIZE][3];
}RESULT;


//Source code from: Socket Programming Reference - (Beej's-Guide)--start
void sigchld_handler(int s)
{
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{

	int sockfd, new_fd;  			
	struct addrinfo hints, *servinfo, *p;	
	struct sockaddr_storage their_addr; 	
	socklen_t sin_size;			
	struct sigaction sa;			
	int yes=1;	
	int i;
	int port_number;
	char s[INET6_ADDRSTRLEN];		
	int rv;					
	int numbytes;				
	char snd_buf[MAXDATASIZE];
//Source code from: Socket Programming Reference - (Beej's-Guide)---end	

//variable used to send or recieve data
	long long int client_recv[3];
	int sendto_A[2];
	MSE recv_A;
	MSE sendto_B;
	double recv_B[SIZE][3];
	RESULT sendto_client;
	 
//Source code from: Socket Programming Reference - (Beej's-Guide)--start
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 	// use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {	
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can (from Beej's-Guide: server.c)
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}
	freeaddrinfo(servinfo); 	

	if (p == NULL)  {		
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) {	
		perror("listen");		
		exit(1);
	}
	sa.sa_handler = sigchld_handler;	
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

// UDP socket initialization: (from Beej's-Guide: listener.c)

	int udp_sockfd;
	struct addrinfo udp_hints, *udp_servinfo, *udp_p;
	int udp_rv;
	struct sockaddr_storage udp_their_addr;
	char udp_buf[MAXDATASIZE];
	socklen_t udp_addr_len;
	char udp_s[INET6_ADDRSTRLEN];

	memset(&udp_hints, 0, sizeof udp_hints);
	udp_hints.ai_family = AF_UNSPEC; 	
	udp_hints.ai_socktype = SOCK_DGRAM;
	udp_hints.ai_flags = AI_PASSIVE;	 

	if ((udp_rv = getaddrinfo(NULL, UDP_PORT, &udp_hints, &udp_servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_rv));
		return 1;
	}

	// loop through all the results and bind to the first we can (from Beej's-Guide: server.c)
	for(udp_p = udp_servinfo; udp_p != NULL; udp_p = udp_p->ai_next) {
		if ((udp_sockfd = socket(udp_p->ai_family, udp_p->ai_socktype,
				udp_p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(udp_sockfd, udp_p->ai_addr, udp_p->ai_addrlen) == -1) {
			close(udp_sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (udp_p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(udp_servinfo);

   // UDP server A's addr initialization (from Beej's Guide: talker.c)

	struct addrinfo udp_A_hints, *udp_A_servinfo, *udp_A_p;
	int udp_A_rv;

	memset(&udp_A_hints, 0, sizeof udp_A_hints);
	udp_A_hints.ai_family = AF_UNSPEC;
	udp_A_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_A_rv = getaddrinfo("127.0.0.1", SERVER_A_PORT, &udp_A_hints, &udp_A_servinfo)) != 0) {	
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_A_rv));
		return 1;
	}

	udp_A_p = udp_A_servinfo; 

   // UDP server B's addr initialization (from Beej's Guide: talker.c)

	struct addrinfo udp_B_hints, *udp_B_servinfo, *udp_B_p;
	int udp_B_rv;

	memset(&udp_B_hints, 0, sizeof udp_B_hints);
	udp_B_hints.ai_family = AF_UNSPEC;
	udp_B_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_B_rv = getaddrinfo("127.0.0.1", SERVER_B_PORT, &udp_B_hints, &udp_B_servinfo)) != 0) {	
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_B_rv));
		return 1;
	}

	udp_B_p = udp_B_servinfo;
	printf("The AWS is up and running.\n"); 
	
	// main loop:
	while(1) { 
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		
		inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
	
		if (!fork()) { 				
			close(sockfd); 		
//Source code from: Socket Programming Reference - (Beej's-Guide)--end			
			
			//receive mapid, start_index and file_size from client's input 
			if ((numbytes = recv(new_fd, client_recv, 24, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			//store data recieved
			char mapid = 'a'+client_recv[0];
			int start_node = (int)client_recv[1];
			long long int file_size=client_recv[2];

			printf("The AWS has received map ID <%c>, start vertex <%d> and file size <%lld> from the client using TCP over port <"PORT">. \n" , mapid, start_node, file_size);

			//send mapid and start_indexto server A			
			sendto_A[0]=client_recv[0];
			sendto_A[1]=client_recv[1];

			if ((numbytes = sendto(udp_sockfd, &sendto_A, 8, 0,udp_A_p->ai_addr, udp_A_p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			printf("The AWS has sent map ID and starting vertex to server A using UDP over port <"UDP_PORT">. \n");
	
			// receive result from A
			memset(udp_buf,0,1024);
			udp_addr_len = sizeof udp_their_addr;
			if ((numbytes = recvfrom(udp_sockfd, &udp_buf, 1024, 0,(struct sockaddr *)&udp_their_addr, &udp_addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
				}
			
			memset(&recv_A,0,sizeof(recv_A));
			memcpy(&recv_A,udp_buf,sizeof(recv_A)); //transfer struct to buffer string 
			
			printf("The AWS has received shortest path from server A:\n");
			printf("-----------------------------\n");
			printf("Destination   Min Length\n");
			printf("-----------------------------\n");
			for (int i=0;i<recv_A.vexnum;i++)
				{
				if(recv_A.len[i]!=0)
				printf(" %-10d  %-10d  \n",recv_A.vexs[i],recv_A.len[i]);
				}
			printf("-----------------------------\n");
			
			memcpy(sendto_client.vexs, recv_A.vexs,  sizeof(recv_A.vexs));
			memcpy(sendto_client.len, recv_A.len,  sizeof(recv_A.len));
			sendto_client.vexnum=recv_A.vexnum; 

			//send to server B
			sendto_B=recv_A;
			sendto_B.file_size=file_size;	

			memset(snd_buf,0,1024);
			memcpy(snd_buf,&sendto_B,sizeof(sendto_B)); //transfer struct to string
		
			if ((numbytes = sendto(udp_sockfd, snd_buf, MAXDATASIZE, 0,udp_B_p->ai_addr, udp_B_p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			printf("The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <"UDP_PORT">.\n");

			// receive from B
			udp_addr_len = sizeof udp_their_addr;
			if ((numbytes = recvfrom(udp_sockfd, &recv_B, MAXDATASIZE, 0,(struct sockaddr *)&udp_their_addr, &udp_addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
				}	

			close(udp_sockfd);
			memcpy(sendto_client.delay, recv_B, sizeof(recv_B));

			printf("The AWS has received delays from server B:\n");
			printf("--------------------------------------------\n");
			printf("Destination     Tt        Tp       Delay\n");
			printf("--------------------------------------------\n");
			for (int i=0;i<sendto_client.vexnum;i++)
			{
			if(sendto_client.len[i]!=0)
			printf("  %-10d %-10.2f %-10.2f  %-10.2f\n",sendto_client.vexs[i], sendto_client.delay[i][0],sendto_client.delay[i][1],sendto_client.delay[i][2]);
			}

			// send result to client
			memset(snd_buf,0,1024);
			memcpy(snd_buf,&sendto_client,sizeof(sendto_client)); 
					
			if (send(new_fd, snd_buf, 1024, 0) == -1)		
				perror("send");
			else	
				printf("The AWS has sent calculated delay to client using TCP over port <"PORT"> .\n");
				close(new_fd);
				exit(0);
		}         
		close(new_fd);  
	}

	return 0;
}
