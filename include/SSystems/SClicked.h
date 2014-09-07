#ifndef _SCLICKED_H
#define _SCLICKED_H

#include "properties\\PClickable.h"
#include "systems\\SSystem.h"
#include "world\\EntityManager.h"

class SClicked : public SSystem
{

private:
	bool checkMouseEvents(PPosition* widget, PClickable * click, int layer);

public:
	SClicked() : SSystem(){};
	void startUp(EntityManager * entM){};

	void update(Map * world, float frameTime);
	void shutDown(){};	//null all
};

#endif