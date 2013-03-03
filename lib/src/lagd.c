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

#define PORT = 4567;

int working=0;

int parseCommand(b, n)
{
	// niektorzy moga uznac te funkcje za bezuzyteczna ale tak szczerze? Wole nie bawic sie w switch na lancuchac w C
	//cel jest prosty: przelicz polecenia biblioteki na int a parametry do zmiennych globalnych

	if (strncpy(b,"dreg",4)==0) return 0; //wyrejestruj agenta. Agenci dostaja ID od lagd. Podobnie user moze odpytac sie lagd o obecne agenty. 
	if (strncpy(b,"rega",4)==0) return 1; //az chcialo sie dodac jeszcze jedna literke i stworzyc nowy regał...
	if (strncpy(b,"mige",4)==0) return 2; //od migrate entry - pierwszy etap migracji. Gdzie lagd sprawdza czy docelowy host jest zdolny do migracji. W tym czasie agent nie musi jeszcze sie szykowac. Odpowiedz "yes" lub "no"
	if (strncpy(b,"migs",4)==0) return 3; //od migrate start - drugi etap migracji. W tym momencie agent powinien byc gotowy do migracji. Przy tym komunikacie lagd zapauzuje proces i zacznie migracje. Po zakonczeniu odpauzuje go i napisze "ok" lub "nope". W przypadku "nope" wystapil blad w migracji i nie jestesmy na nowej maszynie. Oryginalny proces zostal odpauzowany. Jezeli udalo sie zmigrowac to oryginalny proces dostaje SIGKILL i zostaje odmrozony aby wykonal sygnal.
	//lagv - wersja lagd
	//stat - status agenta
	//rega - AID - Agent ID. Da ok jezeli udalo sie lub no jezeli AID nie udalo sie przydzielic. AID moze byc literami i cyframi. Proponuje nie uzywac bialych znakow i wylacznie ASCII. Drugi parametr to numer portu. IP pobierze LAG z systemu. Agent zostanie zarejestrowany w serwerze LDAP (dalszy etap).
	//dreg - bez argumentow. Kazdy agent dostaje swoj watek.
	//mige - jako argument powinien byc IP
	//migs - poniewaz mige ustawia zmienne migracji nie musi podawac parametru. W LDAP status agenta zmieniany jest na "migrating". Funkcje biblioteki blokuja sie cyklicznie sprawdzajac czy status zmienil sie na "running". Dopiero wtedy probuja nawiazac polaczenie pobierajac dane agenta
}	

void *agentThread(void *socket)
{
	int n,sock = (int) socket;
	char buffer[256], command[4]; //command ma 4 znaki - taka nostalgiczna dla mnie wartosc. Polecenia biblioteki moglby by byc wieksze lub mniejsze. Ale 4 to taka ladna i okragla liczba...
	memset(&buffer,0,256);
	do
	{
		n = recv(sock,buffer,255,0);
		if (n > 0)
		{
			command = getCommand(buffer,n);
			switch (command)
			{

			}
				
	}
	while (strncmp(command,"exit",4)!=0 && working == 0);
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

	openlog ("lagd", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);

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
	serv_addr.sin_port = htons(4567);
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
