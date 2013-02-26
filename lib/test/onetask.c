#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT = 4444;

int working=0;

void *simpleThread(void *socket)
{
	int n,sock = (int) socket;
	char buffer[256];
	memset(&buffer,0,256);
	do
	{
		n = read(sock,buffer,255);
		if (n < 0) { } //error
		n = write(sock,buffer,n);
		if (n < 0) { } //error
	}
	while (strncmp(buffer,"exit",4)!=0 && working == 0);
	close(sock);
	working=1;
	pthread_exit(NULL);
}	

int main(int argc, char *argv[])
{
	FILE *file;
	float a;
	int i;

	close(0);
	close(1);
	close(2);

	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		{ //jakis blad
		}
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(4444);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{ //jakis blad 
	}
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	do
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
		{ //jakis blad
		}
		pthread_t thread;
		int rc = pthread_create(&thread, NULL, simpleThread, (void *) newsockfd);
	} while (working == 0);
	close(sockfd);

	pthread_exit(NULL);
}
