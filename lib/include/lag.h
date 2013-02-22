
//LAG uses pthread to manage comunication between lagd and the agents
#include <pthread.h>

int processPID; //this is used to ensure possible PID change. Agent should use this as its PID. Using system functions make break application after migration

#define THIS_PID processPID

#define LAGINIT() 	\
			lag_create_threads();		\
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
	
}

int lag_create_threads()
{
	
}
