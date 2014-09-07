#ifndef _PGRAVITY_H
#define _PGRAVITY_H

#include "properties\\PProperty.h"
#include "graphics\\textDX.h"

struct PGravity : public PProperty
{

	PGravity() : grounded(false){}

	bool grounded;		//whetehr or not the entity is on the ground. set to true whenever
						//collision occurs with the ground. set to false whenever the entity changes
						//position
};

#endif