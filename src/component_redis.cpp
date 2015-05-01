#include "component_redis.h"

#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
	using std::string;
#include <hiredis/hiredis.h>

redisContext* _redis;

Component_Redis::Component_Redis()
{
	_redis = redisConnect("localhost", 6379);

	if (_redis == NULL || _redis->err)
	{
		std::cout << "Error Connecting to Redis Server" << std::endl;
		exit(1);
	}
    redisReply *reply;
    reply = (redisReply*) redisCommand(_redis,"PING");
    if (strcmp(reply->str, "PONG") == 0) std::cout << "Connected to Redis Server!" << std::endl;
}

void Component_Redis::Finish()
{
	//Clean up Redis
	redisFree(_redis);
}