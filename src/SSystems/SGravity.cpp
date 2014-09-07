#include "systems\\SGravity.h"
#include "game\\game.h"
#include "properties\\PCamera.h"

void SGravity::update(Map * world, float frameTime)
{
	EntityManager * entM = world->getEntityManager();
	auto list = entM->getGunitProperties<PGravity>();
	
	PCamera * cam = world->getPrimaryCamera();
	
	for(auto it = list.begin(); it != list.end(); it++)
	{
		PGravity* gobj = static_cast<PGravity*>(it->second);
		PPosition* pos = entM->getProperty<PPosition>(gobj->entity_id);
		PMobile* mobile = entM->getProperty<PMobile>(gobj->entity_id);

		if(mobile != nullptr)
		{
			if(mobile->velocity.x != 0.0f ||
				mobile->velocity.y != 0.0f)
			{
				gobj->grounded = false;
			}
			if(!gobj->grounded) // pull entity to ground if not grounded
			{
				mobile->velocity.y += frameTime*force;
				mobile->velocity.y = mobile->velocity.y > force ? force : mobile->velocity.y;
			}
		}
	}
}