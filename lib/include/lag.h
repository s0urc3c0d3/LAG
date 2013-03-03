
//LAG uses pthread to manage comunication between lagd and the agents
#include <pthread.h>

int processPID; //this is used to ensure possible PID change. Agent should use this as its PID. Using system functions make break application after migration

(* void)(errorFunction)(int errCode, cher *errMsg);
#define THIS_PID processPID

#define LAGINIT(f) 	\
			lag_create_threads();		\
			errorFunction=f;		\
			lag_register_agent(THIS_PID);	

#define LAGSTOP()	\
			lag_deregister_agent();		\
			lag_stop_threads();
//lagd details. Can be changed using LAGCRED macro
int lagdPort=4657;



pthread_t lag_main;	//this thread talks to lagd
pthread_t lag_network;	//this thread gets messages from sockets

void *lagMain(void *threadid)
{
	int lagdSock;
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;

	char buffer[256];
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error(1,"ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
		error(2,"inet_pton failed");
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error(3,"ERROR connecting");
	//n = write(sockfd,buffer,strlen(buffer));
	close(lagdSock);
}

int lag_create_threads()
{
	int errorCode;
	errorCode = pthread_create(&lag_main, NULL, lagMain, NULL);
	//if error call error function provides in LAGINIT
}
