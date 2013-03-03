#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <syslog.h>

#define PORT = 4444;

int working=0;

void *simpleThread(void *socket)
{
	int n,sock = (int) socket;
	char buffer[256];
	memset(&buffer,0,256);
	do
	{
		n = recv(sock,buffer,255,0);
		if (n > 0) n = send(sock,buffer,n,0);
	}
	while (strncmp(buffer,"exit",4)!=0 && working == 0);
	close(sock);
	working=1;
	syslog(LOG_ERR,"ustawiam working");
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

	setlogmask (LOG_UPTO (LOG_NOTICE));

	openlog ("onetask", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);

	syslog(LOG_ERR,"hello");
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		syslog(LOG_ERR,"blad otwarcia master socketa");
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(4444);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		syslog(LOG_ERR,"blad otwarcia portu socketa");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	do
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd != -1)
		{
			pthread_t thread;
			int rc = pthread_create(&thread, NULL, simpleThread, (void *) newsockfd);
			syslog(LOG_ERR,"rc ma wartosc %i",rc);
		}
	} while (working == 0);
	close(sockfd);

	syslog(LOG_ERR,"cya");
	closelog();
	pthread_exit(NULL);
}
