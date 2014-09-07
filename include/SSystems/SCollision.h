#ifndef _SCOLLISION_H
#define _SCOLLISION_H

#include "properties\\PBoundingBox.h"
#include "properties\\PPosition.h"
#include "properties\\PImage.h"
#include "systems\\SSystem.h"
#include "world\\EntityManager.h"
#include "world\\map.h"

namespace SCollisionNS
{
	const unsigned int ARBITRARY_RESERVE = 5000;
}

/*
	SCollision is the system responsible for detecting collision between entities which have a PBoundingBox property.
	SCollision is the only system which makes use of a caching system due to its quadratic nature ( for each entity, must
	check collision against each other entity, and so forth, making it n^2 bound ). as such, it's update() method is slightly more 
	complicated than the other systems.
*/
class SCollision : public SSystem
{
private:
	std::vector<PPosition*> pposes;
	std::vector<PBoundingBox*> pboxes;
	std::vector<PImage*> images;

	// pointer to the entity manager for caching purposes.
	EntityManager * entM;

public:
	
	static const unsigned int ARBITRARY_RESERVE = 2500;

	SCollision() : SSystem()
	{
		entM = nullptr;
		sys_id = 0;
	};

	void startUp(EntityManager * entM)
	{ 
		this->entM = entM;

		if(pboxes.size() < 1)
		{
			pboxes.reserve( SCollision::ARBITRARY_RESERVE );
			pposes.reserve( SCollision::ARBITRARY_RESERVE );
			images.reserve( SCollision::ARBITRARY_RESERVE );
		}
		else
		{
			pboxes.clear();
			pposes.clear();
			images.clear();
		}
	}

	void update(Map * world, float frameTime);
	void shutDown(){};

	//============================================================================
	// collision detection methods and helper methods
	//============================================================================

	void acknowledgeInserts( std::list<unsigned int> &newlist, EntityManager * entM );

	bool removeDelEnt( std::list<unsigned int> &deletedList, unsigned int id, size_t cache_index );

	//returns true if collision is occurring between ent1 and ent2
	bool collisionOccurring(EntityManager * entM,
							PPosition* pos1, 
							PBoundingBox* ent1, 
							PPosition* pos2, 
							PBoundingBox* ent2);
	
	bool lineCircleColliding(PPosition* pos1,
							 PBoundingBox* ent1,
							 PPosition* pos2,
							 PBoundingBox* ent2,
							 VECTOR2& collisionVector);

	bool boxCircleColliding(PPosition* pos1, PBoundingBox* ent1, PPosition* pos, PBoundingBox* ent2, VECTOR2& collisionVector);

	
	bool circlesColliding(PPosition* pos1, PBoundingBox* ent1, PPosition* pos2, PBoundingBox* ent2, VECTOR2& collisionVector);

	bool boxesColliding(PPosition* pos1, PBoundingBox* ent1, PPosition* pos2, PBoundingBox* ent2, VECTOR2& collisionVector);
};

#endif