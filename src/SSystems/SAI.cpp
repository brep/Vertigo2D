#include "systems\\SAI.h"
#include "game\\game.h"

void SAI::update(Map * world, float frameTime)
{
	EntityManager * entM = world->getEntityManager();
	auto list = entM->getGunitProperties<PAI>();
	
	for(auto it = list.begin(); it != list.end(); it++)
	{
		PAI * pai = static_cast<PAI*>(it->second);
		
		if ( pai != nullptr )
		{
			// finished pathing, fire event to scripting system
			if ( pai->finish )
			{
				// fire Lua event to notify script that entity has reach end of path
				LuaDoa::fireEvent( ON_PATHINGCOMPLETE, pai->entity_id );

				pai->finish = false;
				// reset curr and next nodes
				pai->curr = VECTOR2( -1, -1 );
				pai->next = VECTOR2( -1, -1 );
			}
		}
	}
}