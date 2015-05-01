#include "component_ozw.h"

#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
	using std::string;
#include <list>
	using std::list;
#include <pthread.h>
#include <openzwave/Options.h>
#include <openzwave/Manager.h>
#include <openzwave/Driver.h>
#include <openzwave/Node.h>
#include <openzwave/Group.h>
#include <openzwave/Notification.h>
	using OpenZWave::Notification;
#include <openzwave/value_classes/ValueStore.h>
#include <openzwave/value_classes/Value.h>
	using OpenZWave::ValueID;
#include <openzwave/value_classes/ValueBool.h>

string port = "/dev/ttyUSB0";
bool temp = false;
static uint32 g_homeId = 0;
static bool   g_initFailed = false;
typedef struct
{
	uint32			m_homeId;
	uint8			m_nodeId;
	bool			m_polled;
	list<ValueID>	m_values;
} NodeInfo;

static list<NodeInfo*> g_nodes;
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------------
// <GetNodeInfo>
// Return the NodeInfo object associated with this notification
//-----------------------------------------------------------------------------
NodeInfo* GetNodeInfo
(
	Notification const* _notification
)
{
	uint32 const homeId = _notification->GetHomeId();
	uint8 const nodeId = _notification->GetNodeId();
	for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
	{
		NodeInfo* nodeInfo = *it;
		if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
		{
			return nodeInfo;
		}
	}

	return NULL;
}

void OnNotification (OpenZWave::Notification const* _notification, void* _context)
{
	pthread_mutex_lock( &g_criticalSection );

	//std::cout << "OnNotification Running: " << _notification->GetType() << std::endl;
	switch( _notification->GetType() )
	{
		case Notification::Type_ValueAdded:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				// Add the new value to our list
				nodeInfo->m_values.push_back( _notification->GetValueID() );
			}
			break;
		}

		case Notification::Type_ValueRemoved:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				// Remove the value from out list
				for( list<ValueID>::iterator it = nodeInfo->m_values.begin(); it != nodeInfo->m_values.end(); ++it )
				{
					if( (*it) == _notification->GetValueID() )
					{
						nodeInfo->m_values.erase( it );
						break;
					}
				}
			}
			break;
		}

		case Notification::Type_ValueChanged:
		{
			// One of the node values has changed
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo = nodeInfo;		// placeholder for real action
			}
			break;
		}

		case Notification::Type_Group:
		{
			// One of the node's association groups has changed
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo = nodeInfo;		// placeholder for real action
			}
			break;
		}

		case Notification::Type_NodeAdded:
		{
			// Add the new node to our list
			NodeInfo* nodeInfo = new NodeInfo();
			nodeInfo->m_homeId = _notification->GetHomeId();
			nodeInfo->m_nodeId = _notification->GetNodeId();
			nodeInfo->m_polled = false;		
			g_nodes.push_back( nodeInfo );
	        if (temp == true)
	        {
		    	OpenZWave::Manager::Get()->CancelControllerCommand( _notification->GetHomeId() );
            }
			break;
		}

		case Notification::Type_NodeRemoved:
		{
			// Remove the node from our list
			uint32 const homeId = _notification->GetHomeId();
			uint8 const nodeId = _notification->GetNodeId();
			for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
			{
				NodeInfo* nodeInfo = *it;
				if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
				{
					g_nodes.erase( it );
					delete nodeInfo;
					break;
				}
			}
			break;
		}

		case Notification::Type_NodeEvent:
		{
			// We have received an event from the node, caused by a
			// basic_set or hail message.
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo = nodeInfo;		// placeholder for real action
			}
			break;
		}

		case Notification::Type_PollingDisabled:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo->m_polled = false;
			}
			break;
		}

		case Notification::Type_PollingEnabled:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo->m_polled = true;
			}
			break;
		}

		case Notification::Type_DriverReady:
		{
			g_homeId = _notification->GetHomeId();
			break;
		}

		case Notification::Type_DriverFailed:
		{
			g_initFailed = true;
			pthread_cond_broadcast(&initCond);
			break;
		}

		case Notification::Type_AwakeNodesQueried:
		case Notification::Type_AllNodesQueried:
		case Notification::Type_AllNodesQueriedSomeDead:
		{
			pthread_cond_broadcast(&initCond);
			break;
		}

		case Notification::Type_DriverReset:
		case Notification::Type_Notification:
		case Notification::Type_NodeNaming:
		case Notification::Type_NodeProtocolInfo:
		case Notification::Type_NodeQueriesComplete:
		default:
		{
		}
	}
	pthread_mutex_unlock( &g_criticalSection );
}

Component_OZW::Component_OZW()
{
	pthread_mutexattr_t mutexattr;

	pthread_mutexattr_init ( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &g_criticalSection, &mutexattr );
	pthread_mutexattr_destroy( &mutexattr );

	pthread_mutex_lock( &initMutex );

	std::cout << "Using OpenZWave Version: " << OpenZWave::Manager::getVersionAsString().c_str() << std::endl;

	OpenZWave::Options::Create("","./local/","");
	OpenZWave::Options::Get()->Lock();

	OpenZWave::Manager::Create();
	
	OpenZWave::Manager::Get()->AddWatcher(OnNotification, NULL);
	
	
	OpenZWave::Manager::Get()->AddDriver(port);
	
	pthread_cond_wait(&initCond, &initMutex);
	if (!g_initFailed)
	{
		// The section below demonstrates setting up polling for a variable.  In this simple
		// example, it has been hardwired to poll COMMAND_CLASS_BASIC on the each node that 
		// supports this setting.
		pthread_mutex_lock( &g_criticalSection );
		for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
		{
			NodeInfo* nodeInfo = *it;

			// skip the controller (most likely node 1)
			if( nodeInfo->m_nodeId == 1) continue;

			for( list<ValueID>::iterator it2 = nodeInfo->m_values.begin(); it2 != nodeInfo->m_values.end(); ++it2 )
			{
				ValueID v = *it2;
				if( v.GetCommandClassId() == 0x20 )
				{
					std::cout << "enabling polling on " << std::endl;
					OpenZWave::Manager::Get()->EnablePoll( v, 2 );		// enables polling with "intensity" of 2, though this is irrelevant with only one value polled
					break;
				} //else { std::cout << "did enable polling on command class id: " << v.GetCommandClassId() << std::endl; }
			}
		}
		pthread_mutex_unlock( &g_criticalSection );

		// If we want to access our NodeInfo list, that has been built from all the
		// notification callbacks we received from the library, we have to do so
		// from inside a Critical Section.  This is because the callbacks occur on other 
		// threads, and we cannot risk the list being changed while we are using it.  
		// We must hold the critical section for as short a time as possible, to avoid
		// stalling the OpenZWave drivers.
		// At this point, the program just waits for 3 minutes (to demonstrate polling),
		// then exits

		// for( int i = 0; i < 60*1; i++ )
		// {
		// 	pthread_mutex_lock( &g_criticalSection );
		// 	// but NodeInfo list and similar data should be inside critical section
		// 	std::cout << i << "s,";
		// 	pthread_mutex_unlock( &g_criticalSection );
		// 	sleep(1);
		// }
	} else { std::cout << "OZW: Init Failed!" << std::endl; }


}
void Component_OZW::Finish()
{
	OpenZWave::Driver::DriverData data;
	OpenZWave::Manager::Get()->GetDriverStatistics( g_homeId, &data );
	printf("SOF: %d ACK Waiting: %d Read Aborts: %d Bad Checksums: %d\n", data.m_SOFCnt, data.m_ACKWaiting, data.m_readAborts, data.m_badChecksum);
	printf("Reads: %d Writes: %d CAN: %d NAK: %d ACK: %d Out of Frame: %d\n", data.m_readCnt, data.m_writeCnt, data.m_CANCnt, data.m_NAKCnt, data.m_ACKCnt, data.m_OOFCnt);
	printf("Dropped: %d Retries: %d\n", data.m_dropped, data.m_retries);

	//Clean up OpenZWave
	OpenZWave::Manager::Get()->RemoveDriver(port);
	OpenZWave::Manager::Get()->RemoveWatcher(OnNotification, NULL);
	OpenZWave::Manager::Destroy();
	OpenZWave::Options::Destroy();


	pthread_mutex_destroy( &g_criticalSection );
	exit(0);

}