#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "component_redis.h"
#include "component_ozw.h"

Component_Redis *redis;
Component_OZW *ozw;

void my_handler(int s){
	printf("Caught signal %d\n",s);
	ozw->Finish();
	redis->Finish();
	exit(1); 
}

int main( int argc, char* argv[] )
{
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	
	redis = new Component_Redis();
	ozw = new Component_OZW();

	std::cout << "Startup Complete" << std::endl;
	pause();
}
