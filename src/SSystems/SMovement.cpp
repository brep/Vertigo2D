#include "game\\LuaAPI.h"
#include "game\\game.h"
#include "properties\\PAI.h"
#include "systems\\SMovement.h"
#include "world\\map.h"

void SMovement::update(Map * world, float frameTime)
{
	EntityManager * entM = world->getEntityManager();
	auto pmobiles = entM->getGunitProperties<PMobile>();

	for(auto it = pmobiles.begin(); it != pmobiles.end(); it++)
	{
		PMobile* mobile = static_cast<PMobile*>(it->second);
		PPosition* pos = entM->getProperty<PPosition>(mobile->entity_id);
		PAI* pai = entM->getProperty<PAI>(mobile->entity_id);

		if( pos != nullptr )
		{
			PBoundingBox * hitbox = entM->getProperty<PBoundingBox>(pos->entity_id);
			float dx = 0.0f, dy = 0.0f;
			float dt = frameTime;

			// if ignore pause flag set, use the fixed delta time step
			if( mobile->ignorePause )
				dt = gameNS::FIXEDUPDATE_TIMESTEP;

			// fire movement related events to scripting environment
			firePosEvents( pos, mobile );

			// sets mobile.direction vector
			computeDirection( mobile, dt );

			// sets dx and dy
			computeDeltas( mobile, dt, dx, dy );

			// logic for moving the unit according to its AI
			moveAI( pos, mobile, pai, dt, dx, dy );

			// physically move the entity in the world
			moveEntity(pos, dx, dy);
		}
	}
}

void SMovement::firePosEvents( PPosition * pos, PMobile * mobile )
{
	// onPosition trigger - notify scripts of object position
	if( pos->x <= mobile->lessThanX )
	{
		mobile->lessThanX = -FLT_MAX;		// reset flag
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
	if( pos->x >= mobile->greaterThanX )
	{
		mobile->greaterThanX = FLT_MAX;		
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
	if( pos->y <= mobile->lessThanY )
	{
		mobile->lessThanY = -FLT_MAX;		
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
	if( pos->y >= mobile->greaterThanY )
	{
		mobile->greaterThanY = FLT_MAX;	
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
	if( pos->worldX <= mobile->lessThanWorldX )
	{
		mobile->lessThanWorldX = -FLT_MAX;	
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
	if( pos->worldX >= mobile->greaterThanWorldX )
	{
		mobile->greaterThanWorldX = FLT_MAX;	
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
	if( pos->worldY <= mobile->lessThanWorldY )
	{
		mobile->lessThanWorldY = -FLT_MAX;		
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
	if( pos->worldY >= mobile->greaterThanWorldY )
	{
		mobile->greaterThanWorldY = FLT_MAX;		
		LuaDoa::fireEvent( ON_POSITION, mobile->entity_id );
	}
}

// if mobile has radius, compute direction of motion
void SMovement::computeDirection( PMobile * mobile, float dt )
{
	if( mobile->radius == 0.0f )
		return;

	// angular velocity = rad/s = (linear velocity * theta)/r
	float increment = mobile->velocity.x * dt / mobile->radius;

	mobile->direction.x = cos( mobile->angle );
	mobile->direction.y = sin( mobile->angle );			

	if( mobile->angle + increment >= mobile->onAngle )
	{
		mobile->onAngle = -1.0f;
		LuaDoa::fireEvent( "onAngle", mobile->entity_id );
	}
				
	mobile->angle += mobile->velocity.x * dt / mobile->radius;

	if( mobile->angle >= 2*PI )
		mobile->angle -= 2*PI;
}

void SMovement::computeDeltas( PMobile * mobile, float dt, float &dx, float &dy )
{
	mobile->velocity.x += mobile->acceleration.x*dt;
	mobile->velocity.y += mobile->acceleration.y*dt;

	dx = mobile->velocity.x*mobile->direction.x*dt;
	dy = mobile->velocity.y*mobile->direction.y*dt;

	// if max velocity.x is set
	if( mobile->limitVelocityX != FLT_MAX )
	{
		// check if velocity exceeds limit
		if( (mobile->acceleration.x > 0 && mobile->velocity.x > mobile->limitVelocityX) ||
			(mobile->acceleration.x < 0 && mobile->velocity.x < mobile->limitVelocityX) )
		{
			// if velocity exceeds limit, set to limit
			mobile->velocity.x = mobile->limitVelocityX;
			dx = mobile->velocity.x*mobile->direction.x*dt;	
		}
	}
	// same logic as above for velocity.y
	if( mobile->limitVelocityY != FLT_MAX )
	{
		if( (mobile->acceleration.y > 0 && mobile->velocity.y > mobile->limitVelocityY) ||
			(mobile->acceleration.y < 0 && mobile->velocity.y < mobile->limitVelocityY) )
		{
			mobile->velocity.y = mobile->limitVelocityY;
			dy = mobile->velocity.y*mobile->direction.y*dt;	
		}
	}
}

void SMovement::moveAI( PPosition * pos, PMobile * mobile, PAI * pai, float dt, float &dx, float &dy )
{
	if( pai == nullptr )
		return;

	// remainder dx and dy (as in, remainder in the current path distance)
	float rem_dx = 0.0f, rem_dy = 0.0f;

	// subtract deltas from distance to next node
	pai->distanceX -= abs( dx );
	pai->distanceY -= abs( dy );

	// if overshot distance to next node in path..
	if( pai->distanceX < 0.0f )
	{
		// readjust deltas to not overshoot distance
		rem_dx = ( dx > 0.0f ? -pai->distanceX : pai->distanceX );
		dx -= rem_dx;
		pai->distanceX = 0.0f;
	}
	if( pai->distanceY < 0.0f )
	{
		rem_dy = ( dy > 0.0f ? -pai->distanceY : pai->distanceY );
		dy -= rem_dy;
		pai->distanceY = 0.0f;
	}

	if( rem_dx != 0.0f || rem_dy != 0.0f )
		moveEntity( pos, rem_dx, rem_dy );

	doPathFinding(pos, pai, mobile, dt);
}

void SMovement::moveEntity(PPosition * pos, float dx, float dy)
{		
	Map * map = _Game->getWorldManager()->getCurrentWorld();

	if (dx == 0 && dy == 0)
		return;

	if(pos->followCamera)
	{
		PCamera * cam = map->getPrimaryCamera();

		// if this entity is the camera's focus... 
		if(cam != nullptr && pos->entity_id == cam->focusTargetId )
		{
			if( map->getX() - dx > 0.0f || map->getX() - dx < map->getMinX() )
				dx = 0.0f;
			if( map->getY() - dy > 0.0f || map->getY() - dy < map->getMinY() )
				dy = 0.0f;

			//if( (int)pos->x >= (int)cam->offsetX || map->getX() <= 0.0f ) 
			cam->dx += dx;
			pos->x = cam->offsetX;
			pos->prevX = cam->offsetX;

			cam->dy += dy;
			pos->y = cam->offsetY;
			pos->prevY = cam->offsetY;

			return;
		}
	}

	pos->x += dx;
	pos->y += dy;
}

void SMovement::doPathFinding(PPosition * pos, PAI * pai, PMobile * mobile, float frameTime)
{
	Map * map = _Game->getWorldManager()->getCurrentWorld();

	// if begin pathing flag set
	if( pai->start )
	{
		Node * n = map->getGraph()->shortestPath(map, pai->y1, pai->x1, pai->y2, pai->x2, pai->neighborType, pai->tileOccupation);

		if( n == nullptr || n->prev == nullptr )
		{
			Log( consoleNS::WARNING, "ShortestPath returned NULL node." );
			Log( consoleNS::INFO_NO_TSTAMP, "x1: " + std::to_string( pai->x1 ) + ", y1: " + std::to_string( pai->y1 ) ); 
			Log( consoleNS::INFO_NO_TSTAMP, "x2: " + std::to_string( pai->x2 ) + ", y2: " + std::to_string( pai->y2 ) ); 
			pai->resetPathing();
			LuaDoa::fireEvent( ON_PATHINGCOMPLETE, pai->entity_id );
			return;
		}

		// fire Lua event to notify script that pathing will begin,
		// passing in the returned node ptr so that scripters
		// may do things like set each node's image
		LuaDoa::fireEvent( ON_PATHINGBEGIN, pai->entity_id, n );

		// check if script set a new end node in the path
		if( pai->endNode != nullptr )
			n = pai->endNode;

		// check if script reset path
		if( pai->reset )
			return;	// return if path reset

		for(; n->prev != nullptr; n = n->prev)
		{
			pai->path.push( VECTOR2( n->x + pai->offsetX, n->y + pai->offsetY ) );
		}
		// reset endNode
		pai->endNode = nullptr;

		pai->start = false;
		pai->distanceX = 0.0f;
		pai->distanceY = 0.0f;
	}

	if( pai->distanceX <= 0.0f && pai->distanceY <= 0.0f )
	{
		if ( !pai->path.empty() )
		{
			pai->curr = pai->path.top();
			pai->path.pop();
			LuaDoa::fireEvent( ON_NEXTNODE, pai->entity_id );

			mobile->direction.x = pai->curr.x + map->getX() - pos->x;		
			mobile->direction.y = pai->curr.y + map->getY() - pos->y;
			Graphics::Vector2Normalize(&mobile->direction);
			
			pai->distanceX = abs( pai->curr.x + map->getX() - pos->x );
			pai->distanceY = abs( pai->curr.y + map->getY() - pos->y );
		}
		else
		{
			pai->resetPathing();
			pai->finish = true;
		}
	}
}