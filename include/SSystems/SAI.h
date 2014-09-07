#ifndef _SAI_H
#define _SAI_H

#include "systems\\SSystem.h"
#include "properties\\PAI.h"
#include "world\\EntityManager.h"

class SAI : public SSystem
{

public:
	SAI() : SSystem() {}

	void startUp(EntityManager * entM){};

	void update(Map * world, float frameTime);
	void shutDown(){};	//null all
};

#endif