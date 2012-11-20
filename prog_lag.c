#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lag.h"
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int lag_file = open("/dev/lag",O_RDWR);
	struct lag_request req,res;
	req.REQID=0;
	req.pid=1394;
	req.status=0;
	write(lag_file,&req,sizeof(struct lag_request));
	req.status=0;
	read(lag_file,&res,sizeof(struct lag_request));
	printf("\n status: %i   pid %i",res.status,res.pid);
	printf("\n pauzuje proces...");
	req.REQID=1;
	req.status=4;
	write(lag_file,&req,sizeof(struct lag_request));
	read(lag_file,&res,sizeof(struct lag_request));
	printf("\n status: %i   pid %i",res.status,res.pid);
	char g;
	scanf("%s",&g);
	req.REQID=1;
	req.status=1;
	write(lag_file,&req,sizeof(struct lag_request));
	read(lag_file,&res,sizeof(struct lag_request));
	printf("\n status: %i   pid %i",res.status,res.pid);
	close(lag_file);
	return 0;
}
