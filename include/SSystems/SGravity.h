#ifndef _SGRAVITY_H
#define _SGRAVITY_H

#include "properties\\PPosition.h"
#include "properties\\PMobile.h"
#include "systems\\SSystem.h"
#include "world\\EntityManager.h"

class Game;

namespace SGravityNS
{
	const float FORCE = 1000.0f;
}

class SGravity : public SSystem
{
private:
	int frameCount;
	float force;

public:
	SGravity() : SSystem(){force = SGravityNS::FORCE; frameCount = 0;}

	void startUp(float force){this->force = force;}
	void startUp(EntityManager * entM){}

	void update(Map * world, float frameTime);
	void shutDown(){};
};

#endif