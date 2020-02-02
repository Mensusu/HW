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
#include <cstring>
#include <string>
#include<vector>
#include<iostream>
#include <fstream>
//Source code from: Socket Programming Reference - (Beej's-Guide)---end
#define MYPORT "21819"	// port connect to AWS
#define SIZE 10    
#define INF 10000000
#define MAXDATASIZE 1024

using namespace std;

typedef struct _inf //structure used to store information sent to AWS 
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

typedef struct _graph // structure used to store information of every map
{
    char mapid;
    int vexs[SIZE];       
    int vexnum;           
    int edgnum;          
    int matrix[SIZE][SIZE]; 
    double propSpeed;
    double transSpeed;
}Graph;

void mapConstruction(vector<Graph> &mapList){ // read data from map.txt
	fstream f;
  	string c;
	double t,p;
	int k,m,n;
    	f.open("map.txt",ios::in);
	
	if(!f.eof()){
	f>>c;
	}
	while (!f.eof()) 
        { 
		Graph thisMap={'0',{0},0,0,{0},0,0};
		thisMap.edgnum=0;
		for(int i=0;i<SIZE;i++){
			thisMap.vexs[i]=-1;
			for(int j=0;j<SIZE;j++){
				thisMap.matrix[i][j]=INF;
			}
		}
				
		if(isalpha(c[0])!=0){ //start from mapid
			thisMap.mapid=c[0];
			f>>p; //store propagation speed
			f>>t; // store transmitting speed
			thisMap.propSpeed=p;
			thisMap.transSpeed=t;
			while(!f.eof()){
				f>>c;
				if(isalpha(c[0])!=0)
				{break;}	
				m=atoi(c.c_str());
				f>>n;
				f>>k;
				int i=0;
				for(i=0;i<thisMap.vexnum;i++){ // exam if this node already exist in the array 
					if(thisMap.vexs[i]==m){
						break;
					}
				}
				if(thisMap.vexs[i]!=m){  
					thisMap.vexs[i]=m;
					thisMap.vexnum++;
				}
				
                                int j=0;
				for(j=0;j<thisMap.vexnum;j++){ // exam if this node already exist in the array 
					if(thisMap.vexs[j]==n){
						break;
					}
				}
				if(thisMap.vexs[j]!=n){ 
					thisMap.vexs[j]=n;
					thisMap.vexnum+=1;
				}
				 
				thisMap.matrix[i][j]=k; 
                                thisMap.matrix[j][i]=k;
		
				thisMap.edgnum++;
					
			}
		}
		mapList.push_back(thisMap);		
	}
    	f.close();
}

/* 
** Dijkstra Algorithm used to find the shortest path to every destination node
** Source code from: https://www.includehelp.com/cpp-tutorial/dijkstras-algorithm.aspx
*/
void dijkstra(Graph G, int start, int pre_nodes[], int len[]){	 
	int pre[SIZE];
	int i; 
	int visit[SIZE];  //state of every node  
	for(int i = 0; i < G.vexnum; i++){ 
        	visit[i] = 0;	//every node is not be visited
 		pre[i] = -1;
        	len[i] = G.matrix[start][i];	//the initial distance from start node to destination node 
	}  
	visit[start] = 1;
	len[start] = 0;
	int j;  
	int pos;//record the node has min len 
	int min;  // min len
    	for(i = 0; i < G.vexnum; i++){	 
        	min = INF; 
		for(j = 0; j < G.vexnum; j++){	//find the min direct len
            		if(!visit[j] && min > len[j]){  
                		pos = j;  
                		min = len[j];  
            		}  
        	}  
        	visit[pos] = 1;  
  
        	for (j = 0; j < G.vexnum; j++){
 			int tmp = (G.matrix[pos][j]==INF ? INF : (min + G.matrix[pos][j])); // avoid segmentation fault
            		if (visit[j] == 0 && (tmp  < len[j] ) ){
                		len[j] = tmp;
                		pre[j] = pos;
				}  
        		}  
    		} 
		
		for(int i=0;i<G.vexnum;i++){ //calculate the number of nodes in the path
			int j,num=0;
			j=i;
       			while(pre[j]!=-1){
				num++;          
            			j=pre[j];          
        		}   
			pre_nodes[i]=num;	
		} 
}
///** Dijkstra Algorithm end///


///Source code from: Socket Programming Reference - (Beej's-Guide)---start
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	vector<Graph> mapList;   
	mapConstruction(mapList);	
	// UDP socket initialization: (from Beej's-Guide: listener.c)
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	
	char buf[MAXDATASIZE];
	int aws_rcv[2]; 
	MSE sendto_aws;

	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

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

//Source code from: Socket Programming Reference - (Beej's-Guide)--end

	cout<<"The Server A is up and running using UDP on port <"MYPORT">\n";
	cout<<"The Server A has constructed a list of < "<< mapList.size() <<" > maps:\n";
	cout<<"---------------------------------------------------------------------------\n";
	cout<<" Map ID   Num Vertices   Num Edges \n";
	cout<<"---------------------------------------------------------------------------\n";
	for(int i=0;i<mapList.size();i++){
		if(i==mapList.size()-1){
			mapList[i].edgnum--;
			}
		printf(" %-10c %-10d   %-10d \n",mapList[i].mapid, mapList[i].vexnum, mapList[i].edgnum);
		}
	cout<<"---------------------------------------------------------------------------\n";


	//main loop: 
	while (1) {
		
		// receive from AWS
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, &aws_rcv, 16, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		char mapid=aws_rcv[0]+'a';
		int start_node=aws_rcv[1];
		printf("The Server A has received input for finding shortest paths:starting vertex %d of map %c. \n",start_node, mapid);
		
		Graph map;
		int node_index;		
		int len[SIZE];
		int pre_nodes[SIZE]; 

		for(int i=0;i<mapList.size();i++){
			if(mapList[i].mapid==mapid){
				map=mapList[i];
				if(i==mapList.size()-1){
					map.edgnum--;
					}
			}
		}

		for(int i=0;i<SIZE;i++){
			if(map.vexs[i]==start_node){
				node_index=i;
			}
		}
		//find the shortest path	
		dijkstra(map,node_index,pre_nodes,len);
		
		//sort according to node's number
		int temp_vex,temp_len,temp_pre;    
		for (int i = 0; i < map.vexnum-1;i++) {          
			for (int j = 0; j < map.vexnum -1 - i;j++) {      
				if (map.vexs[j] > map.vexs[j + 1]) {				
					temp_vex = map.vexs[j];				
					map.vexs[j] = map.vexs[j + 1];				
					map.vexs[j + 1] = temp_vex;
				
					temp_len =len[j];				
					len[j] = len[j + 1];				
					len[j + 1] = temp_len;

					temp_pre = pre_nodes[j];				
					pre_nodes[j] = pre_nodes[j + 1];				
					pre_nodes[j + 1] = temp_pre;			
					}		
				}	
			}

		cout<<"The Server A has identified the following shortest paths:\n";
		cout<<"-----------------------------\n";
		cout<<"Destination  Min Length \n";
		cout<<"-----------------------------\n";
		for (int i=0;i<map.vexnum;i++)
			{
			if(len[i]!=0){
				printf("%-14d %-10d \n",map.vexs[i],len[i]);
				//cout <<map.vexs[i]<< "             "<<len[i]<< "            "<<"\n"<< endl;
				}
			}
		cout<<"-----------------------------\n";

		// send result to aws
		sendto_aws.start_index=node_index;
		sendto_aws.prop_speed=map.propSpeed;
		sendto_aws.trans_speed=map.transSpeed;
		sendto_aws.vexnum=map.vexnum;
		memcpy( sendto_aws.vexs,  map.vexs,  sizeof(map.vexs) );
		memcpy( sendto_aws.len, len, sizeof(len));
		memcpy( sendto_aws.pre_nodes, pre_nodes, sizeof(pre_nodes));


		memset(buf,'z',MAXDATASIZE);//refresh the buffer
		memcpy(buf,&sendto_aws,sizeof(sendto_aws)); //transfer struct to string

		if ((numbytes = sendto(sockfd, &buf, MAXDATASIZE, 0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
			perror("senderr: sendto");
			exit(1);
		}
		printf("The Server A has sent shortest paths to AWS.\n");

	}

	return 0;
}
