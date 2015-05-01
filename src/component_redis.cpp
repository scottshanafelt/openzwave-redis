#include "component_redis.h"

#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
	using std::string;
#include <hiredis/hiredis.h>



Component_Redis::Component_Redis()
{
	redisContext* redis = redisConnect("localhost", 6379);

	if (redis == NULL || redis->err)
	{
		std::cout << "Error Connecting to Redis Server" << std::endl;
		exit(1);
	}
    redisReply *reply;
    reply = (redisReply*) redisCommand(redis,"PING");
    if (strcmp(reply->str, "PONG") == 0) std::cout << "Connected to Redis Server!" << std::endl;




	//Clean up Redis
	redisFree(redis);
}