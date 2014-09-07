#ifndef _SSYSTEM_H
#define _SSYSTEM_H

#include "world\\EntityManager.h"

class Game;

class SSystem
{

protected:
	unsigned int sys_id;

public:
	SSystem() { sys_id = -1; }

	virtual void update(Map * world, float frameTime) = 0;

	/*
	template <class T>
	void remove(std::vector<T*>& props, unsigned int i)
	{
		props[i] = props[props.size()-1];
		props.erase(--props.end());
	}
	*/
	virtual void shutDown() = 0;
};

#endif