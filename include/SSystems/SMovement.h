#ifndef _SMOVEMENT_H
#define _SMOVEMENT_H

#include "properties\\PMobile.h"
#include "properties\\PGravity.h"
#include "properties\\PPosition.h"
#include "properties\\PAI.h"
#include "systems\\SSystem.h"
#include "world\\EntityManager.h"
#include "world\\map.h"

class Game;

class SMovement : public SSystem
{

public:
	SMovement() : SSystem(){};
	void startUp(EntityManager& entM){};
	void update(Map * world, float frameTime);
	void shutDown(){};

	void firePosEvents( PPosition * pos, PMobile * mobile );

	void computeDirection( PMobile * mobile, float dt );

	void computeDeltas( PMobile * mobile, float dt, float &dx, float &dy );

	void moveAI( PPosition * pos, PMobile * mobile, PAI * pai, float dt, float &dx, float &dy );

	// moves the entity, taking into account factors such as whether a camera is attached
	// to the entity or not. any repositioning/movement of an entity in worldspace should
	// call this method rather than manually set the position of an entity - the only
	// exception to this is in Map::update(), which moves all entities by some dx and dy
	// that is the world's change in x and y for that frame.
	static void moveEntity(PPosition * pos, float deltaX, float deltaY);

	// similar to moveEntity, except for entities whose movement is tile-bound ( such
	// as AI that use A* pathing )
	static void doPathFinding(PPosition * pos, PAI * pai, PMobile * mobile, float frameTime);
};

#endif