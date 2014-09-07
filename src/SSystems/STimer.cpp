#include "systems\\STimer.h"
#include "game\\LuaAPI.h"
#include "game\\game.h"
#include <list>

void STimer::startUp()
{
    // attempt to set up high resolution timer
	BOOL check = QueryPerformanceFrequency(&t_freq);
    LogAssert(check, consoleNS::ERR, "Failed to initialize high resolution timer.");

    check = QueryPerformanceCounter(&t);        // get starting time
	LogAssert(check, consoleNS::ERR, "Failed to query performance counter.");

	t_prev = t;
}

// querys high res timer and computes the current frame time
bool STimer::updateTime(bool paused)
{
    // calculate elapsed time of last frame, save in frameTime
    BOOL check = QueryPerformanceCounter(&t);
	LogAssert( check, consoleNS::ERR, "Failed to query performance counter." );

    this->frameTime = static_cast<float>( ( t.QuadPart - t_prev.QuadPart ) / static_cast<float>( t_freq.QuadPart ) );

   if(frameTime < stimerNS::MIN_FRAME_TIME)		// if not enough time has elapsed for desired frame rate, return
		return false;					

    if(frameTime > stimerNS::MAX_FRAME_TIME)		// if frame rate is very slow...
        frameTime = stimerNS::MAX_FRAME_TIME;		// ...limit maximum frameTime 

	fuAccumulator += frameTime;

	t_prev = t;

	if( paused )
	{
		// if paused, set dt and fixed_dt to 0 so that systems that rely on this value will come to a halt.
		// for instance, SMovement uses dt to compute the position of each individual entity on a fixed time interval.
		// if the time interval (dt, fixed_dt) is zero, than no simulation will occur.
		dt = 0;
		fixed_dt = 0;
	}
	else
	{
		dt = frameTime;
		fixed_dt = gameNS::FIXEDUPDATE_TIMESTEP;
	}

	return true;
}

void STimer::update(Map * world, float frameTime)
{
	EntityManager * entM = world->getEntityManager();

	// retrieve all PTimer properties from entity manager
	auto list = entM->getProperties<PTimer>();
	// get entities registered for a callback
	auto notifyees = entM->getNotifyees();

	for(auto it = list.begin(); it != list.end(); it++)
	{
		updatePTimer( it->second, frameTime );
	}

	auto iter = notifyees->begin();
	while( iter != notifyees->end() )
	{
		PTimer * it = &iter->timer;
		// returns true when timer is exhausted ( reaches endTime )
		if( updatePTimer( it, frameTime, iter->id, true ) )
		{
			// remove iter from notifyees
			iter = notifyees->erase( iter );
			continue;
		}
		else
			++iter;
	}
}

// returns true on event fire
bool STimer::updatePTimer( PTimer * it, float frameTime, std::string callerId, bool onWaitEvent )
{
	if(it->isActive())
	{
		float endTime = it->getEndTime();

		// hack for making snare image fade in/out through pause. change this.
		if( it->tasks.empty() )
			it->setElapsedTime(it->getElapsedTime() + frameTime);
		else
			it->setElapsedTime(it->getElapsedTime() + this->frameTime);

		if(it->getElapsedTime() >= endTime)
		{
			it->setElapsedTime(it->getElapsedTime() - endTime );

			if( it->tasks.empty() )
			{
				// this timer has no tasks, therefore it exists for the purpose of
				// notifying caller when endTime has elapsed, so we call onWaitComplete event.
				if( onWaitEvent )
					LuaDoa::onWaitComplete( callerId );
				else
					LuaDoa::fireEvent(ON_ALARM, it->entity_id);

				return true;
			}
			else
			{
				auto func = it->tasks.begin();
				auto caller = it->callers.begin();
				while( func != it->tasks.end() )
				{
					// if task function returns true, task has completed; remove task from vec
					if( (*func)( (*caller) ) )
					{
						it->tasks.erase(func);
						it->callers.erase(caller);
						LuaDoa::fireEvent(ON_TASK_COMPLETE, it->entity_id);
						break;
					}

					func++;
					caller++;
				}
				if( it->tasks.empty() )
				{
					return true;
				}
			}
		}
	}

	return false;
}