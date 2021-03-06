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
#include <signal.h>

#include <getopt.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define __NR_checkpoint		304
#define CHECKPOINT_SUBTREE 	0x1
#define PORT 		= 	4567;

/*
 *  checkpoint.c: checkpoint one or multiple processes
 *
 *  Copyright (C) 2008-2009 Oren Laadan
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License.  See the file COPYING in the main directory of the Linux
 *  distribution for more details.
 */

#include <linux/checkpoint.h>

#include "../include/checkpoint.h"
#include "../include/common.h"

static int global_uerrfd = -1;

inline static int checkpoint(pid_t pid, int fd, unsigned long flags, int logfd)
{
	return syscall(__NR_checkpoint, pid, fd, flags, logfd);
}

int cr_checkpoint(int pid, struct cr_checkpoint_args *args)
{
	int ret;

	global_uerrfd = args->uerrfd;

	if (!args->container)
		args->flags |= CHECKPOINT_SUBTREE;

	ret = checkpoint(pid, args->outfd, args->flags, args->logfd);

	if (ret < 0) {
		ckpt_perror("checkpoint");
		ckpt_err("(you may use 'ckptinfo -e' for more info)\n"); 
	} else if (args->verbose) {
		ckpt_err("checkpoint id %d\n", ret);
	}

	return ret;
}

int working=0;

char *ARG1 = NULL, *ARG2 = NULL, *ARG3 = NULL;

struct __agent_t {
	char *name;
	char *IP;
	int port;
	char *AID;
	struct __agent_t *next;
	struct __agent_t *prev;
};

typedef struct __agent_t agent_t;

agent_t *agent_list = NULL;

void agent_add(agent_t *ent, agent_t *list)
{
        agent_t *list_next = list->next;
        list_next->prev = ent;
        ent->next=list_next;
        ent->prev=list;
        list->next=ent;
}

void agent_del(agent_t *ent)
{
        ent->next->prev = ent->prev;
        ent->prev->next = ent->next;
        ent->next=NULL;
        ent->prev=NULL;
}

int isRegAgent(char *AID, int port)
{
	if (!agent_list) return 0;
	int n;
	agent_t *y=agent_list;
	do
	{
		n=strlen(y->AID);
		if (strncmp(y->AID,AID,n) == 0 || y->port == port) return 1;
		y=y->next;
	}
	while (y!=agent_list);
	return 0;
}

int regAgent(char *AID, int port)
{
	if (isRegAgent(AID,port) == 1) return 1;
	agent_t *tmp = (agent_t *) malloc(sizeof(agent_t));
	if (!tmp) return 1;
	tmp->AID = (char *)malloc(strlen(AID)+1);
	if (!tmp->AID) return 1;
	memset(tmp->AID,0,sizeof(strlen(AID)+1));
	memcpy(tmp->AID,AID,strlen(AID));

	if (port == 0 ) return 1;
		tmp->port = port;

	if (agent_list == NULL)
	{
		agent_list = tmp;
		agent_list->next=agent_list;
		agent_list->prev=agent_list;
	}
	else
		agent_add(tmp, agent_list);
	return 0;
}

int deregAgent(char *AID)
{
	if (agent_list == NULL) return 1;
	agent_t *y=agent_list;
	do
	{
		if (strncmp(y->AID,AID,strlen(AID)) == 0) {
			agent_del(y);
			free(y->AID);
			free(y->IP);
			free(y);
			return 0;
		}
		y=y->next;
	}
	while (y!=agent_list);
	return 1;
}

int parseCommand(char *b, int n)
{
	// niektorzy moga uznac te funkcje za bezuzyteczna ale tak szczerze? Wole nie bawic sie w switch na lancuchac w C
	//cel jest prosty: przelicz polecenia biblioteki na int a parametry do zmiennych globalnych

	if (strncmp(b,"dreg",4)==0) return 0; //wyrejestruj agenta. Agenci dostaja ID od lagd. Podobnie user moze odpytac sie lagd o obecne agenty. 
	if (strncmp(b,"rega",4)==0) //az chcialo sie dodac jeszcze jedna literke i stworzyc nowy regał...
	{
		if (ARG1)
			free(ARG1);
		if (ARG2)
			free(ARG2);
		char *tmp = (char *)malloc(n-5);
		memset(tmp,0,n-5);
		memcpy(tmp,b+5,n-7);
		char *tmp2 = strchr(tmp,32);
		int c = tmp2-tmp+1;
		ARG1 = (char *)malloc(c);
		memset(ARG1,0,c);
		memcpy(ARG1,tmp,c-1);
		c = n-strlen(tmp)+strlen(tmp2)-5-1;
		ARG2 = (char *) malloc(c);
		memset(ARG2,0,c);
		memcpy(ARG2,tmp2+1,c);
		
		return 1;
	}
	if (strncmp(b,"mige",4)==0) return 2; //od migrate entry - pierwszy etap migracji. Gdzie lagd sprawdza czy docelowy host jest zdolny do migracji. W tym czasie agent nie musi jeszcze sie szykowac. Odpowiedz "yes" lub "no"
	if (strncmp(b,"migs",4)==0) return 3; //od migrate start - drugi etap migracji. W tym momencie agent powinien byc gotowy do migracji. Przy tym komunikacie lagd zapauzuje proces i zacznie migracje. Po zakonczeniu odpauzuje go i napisze "ok" lub "nope". W przypadku "nope" wystapil blad w migracji i nie jestesmy na nowej maszynie. Oryginalny proces zostal odpauzowany. Jezeli udalo sie zmigrowac to oryginalny proces dostaje SIGKILL i zostaje odmrozony aby wykonal sygnal.
	//lagv - wersja lagd
	//stat - status agenta
	//rega - AID - Agent ID. Da ok jezeli udalo sie lub no jezeli AID nie udalo sie przydzielic. AID moze byc literami i cyframi. Proponuje nie uzywac bialych znakow i wylacznie ASCII. Drugi parametr to numer portu. IP pobierze LAG z systemu. Agent zostanie zarejestrowany w serwerze LDAP (dalszy etap).
	//dreg - bez argumentow. Kazdy agent dostaje swoj watek.
	//mige - jako argument powinien byc IP
	//migs - poniewaz mige ustawia zmienne migracji nie musi podawac parametru. W LDAP status agenta zmieniany jest na "migrating". Funkcje biblioteki blokuja sie cyklicznie sprawdzajac czy status zmienil sie na "running". Dopiero wtedy probuja nawiazac polaczenie pobierajac dane agenta
	if (strncmp(b,"alis",4)==0) return 10;
}	

void listAgents(int s)
{
	if (agent_list == NULL) return;
	char buffer[256];
	int n;
	agent_t *y=agent_list;
	do
	{
		memset(&buffer,0,256);
		n=strlen(y->AID);
		memcpy(&buffer,y->AID,n);
		n = send(s,buffer,n,0);
		y=y->next;
	}
	while (y!=agent_list);
}

void *agentThread(void *socket)
{
	int n,sock = (int) socket, command, err, isRegistered=0;
	char buffer[256];
	char *AID;
	do
	{
		memset(&buffer,0,256);
		n = recv(sock,buffer,255,0);
		if (n > 0)
		{
			err=-1;
			command = parseCommand(buffer,n);
			switch (command)
			{
				case 0: if (isRegistered == 1) err = deregAgent(AID);
					if (err == 0) memcpy(buffer,"exit",4); 
					break;
				case 1: if (isRegistered == 0) err = regAgent(ARG1,atoi(ARG2));
						else {
							err=1;
							break;
						}
						if (err == 0) {
							isRegistered=1;
							AID=malloc(strlen(ARG1)+1);
							memset(AID,0,strlen(ARG1)+1);
							memcpy(AID,ARG1,strlen(ARG1));	
						}
					break;
				case 10: listAgents(sock);break;
			}
			if (err == 0)  send(sock,"ok",2,0);
			if (err > 0)  send(sock,"no",2,0);
		}
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
			int rc = pthread_create(&thread, NULL, agentThread, (void *) newsockfd);
			syslog(LOG_ERR,"rc ma wartosc %i",rc);
		}
	} while (working == 0);
	close(sockfd);

	syslog(LOG_ERR,"cya");
	closelog();
	pthread_exit(NULL);
}

