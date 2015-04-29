#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <string>
#include <stdexcept>
#include <platform/Log.h>

#include <hiredis/hiredis.h>

#include <openzwave/Options.h>
#include <openzwave/Manager.h>
#include <openzwave/Notification.h>

static pthread_mutex_t g_criticalSection;
//static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

void OnNotification (OpenZWave::Notification const* _notification, void* _context)
{
	pthread_mutex_lock( &g_criticalSection );

	std::cout << "OnNotification Running" << std::endl;

	pthread_mutex_unlock( &g_criticalSection );
}

int main( int argc, char* argv[] )
{
	pthread_mutexattr_t mutexattr;

	pthread_mutexattr_init ( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &g_criticalSection, &mutexattr );
	pthread_mutexattr_destroy( &mutexattr );

	pthread_mutex_lock( &initMutex );




	std::cout << "Starting up..." << std::endl;




	redisContext* redis = redisConnect("localhost", 6379);

	if (redis == NULL || redis->err)
	{
		std::cout << "Error Connecting to Redis Server" << std::endl;
		exit(1);
	}
        redisReply *reply;
        reply = (redisReply*) redisCommand(redis,"PING");
        std::cout << reply->str << std::endl;



	std::cout << "Using OpenZWave Version: " << OpenZWave::Manager::getVersionAsString().c_str() << std::endl;
	
	OpenZWave::Options::Create("./config/", "", "");
	OpenZWave::Options::Get()->AddOptionInt( "SaveLogLevel", LogLevel_Detail );
	OpenZWave::Options::Get()->AddOptionInt( "QueueLogLevel", LogLevel_Debug );
	OpenZWave::Options::Get()->AddOptionInt( "DumpTrigger", LogLevel_Error );
	OpenZWave::Options::Get()->AddOptionInt( "PollInterval", 500 );
	OpenZWave::Options::Get()->AddOptionBool( "IntervalBetweenPolls", true );
	OpenZWave::Options::Get()->AddOptionBool("ValidateValueChanges", true);
	OpenZWave::Options::Get()->Lock();

	OpenZWave::Manager::Create();
	OpenZWave::Manager::Get()->AddWatcher( OnNotification, NULL);
	




	redisFree(redis);
	pthread_mutex_destroy( &g_criticalSection );
	exit(0);
}
