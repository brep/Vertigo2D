#ifndef _STIMER_H
#define _STIMER_H

#include "properties\\PTimer.h"
#include "systems\\SSystem.h"

class Game;

namespace stimerNS
{
	// frame rate related constants
	const float FRAME_RATE = 120.0f;					// the target/maximum frame rate
	const float MIN_FRAME_RATE = 10.0f;					// the minimum frame rate
	const float MIN_FRAME_TIME = 1.0f/FRAME_RATE;		// minimum desired time for 1 frame 
	const float MAX_FRAME_TIME = 1.0f/MIN_FRAME_RATE;	// maximum time used in calculations 
};

class STimer : public SSystem
{

private:
	LARGE_INTEGER t;		// total time since game started
	LARGE_INTEGER t_prev;	// time at last frame
	LARGE_INTEGER t_freq;	// frequency of timer
	LARGE_INTEGER vt;		// total virtual time (in other words, not counting time paused) since game started
	LARGE_INTEGER vt_prev;	// virtual time at last frame
	float frameTime;		// in any given frame, frameTime = t - t_prev
	float dt;				// in any given frame, dt = vt - vt_prev
	float fixed_dt;

public:
	STimer() : SSystem(), frameTime(0.0f), dt(0.0f), fixed_dt(0.0f), fuAccumulator(0.0f) {};

	float fuAccumulator;	// fixed update accumulator - make public access 

	void startUp();
	bool startUp(EntityManager * entM){};

	float getFrameTime() const		{ return frameTime; }
	float getDeltaTime() const		{ return dt; }
	float getFixedDeltaTime() const { return fixed_dt; }

	// update t and vt, as well as compute new frameTime and dt.
	// must call every rendered frame.
	// if paused is true, then vt is not updated, thus dt for that frame will be zero.
	// RETURNS:
	//	- false if frameTime < MIN_FRAME_TIME
	//	- true otherwise
	bool updateTime(bool paused);

	// update PTimers and execute any associated Tasks.
	void update(Map * world, float frameTime);

	bool updatePTimer(PTimer * it, float frameTime, std::string callerId = "", bool onWait = false);

	void shutDown(){};
};

#endif