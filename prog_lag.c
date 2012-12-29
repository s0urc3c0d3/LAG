#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lag.h"
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int lag_file = open("/dev/lag",O_RDWR);
	struct lag_request req,res;
	req.REQID=1;
	req.pid=1772;
	req.status=0;
	write(lag_file,&req,sizeof(struct lag_request));
	close(lag_file);
	return 0;
}
