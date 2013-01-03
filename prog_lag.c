#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lag.h"
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int lag_file = open("/dev/lag",O_RDWR);
	struct lag_request req,res;
	if (argc < 3) printf("za malo argumentow!\n");
	req.REQID=atoi(argv[2]);
	req.pid=atoi(argv[1]);
	printf("%i %i\n",req.pid,req.REQID);
	req.status=0;
	write(lag_file,&req,sizeof(struct lag_request));
	close(lag_file);
	return 0;
}
