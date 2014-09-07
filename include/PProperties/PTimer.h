#ifndef _PTIMER_H
#define _PTIMER_H

#include "properties\\PProperty.h"
#include <vector>

class PTimer : public PProperty
{
typedef bool (*Task)(void *);

private:
	float endTime;
	float elapsedTime;
	bool active;

public:

	PTimer() : active(false), elapsedTime(0.0f), endTime(0.0f) {}
	~PTimer(){}
	
	// functions to call once elapsed time reached
	std::vector<Task> tasks;
	// callers, passed into Task funcs, as arguments
	std::vector<void*> callers;

	//lua accessibility functions
	float getElapsedTime() const {return elapsedTime;}
	float getEndTime() const {return endTime;}

	bool isActive() const {return active;}
	void setActive(bool active) {this->active = active;}

	void addTask(Task func, void * caller){ tasks.push_back(func); callers.push_back(caller); }

	void setElapsedTime(float x){elapsedTime = x;}
	void setEndTime(float y){endTime = y;}
};

#endif