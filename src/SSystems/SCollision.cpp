#include "systems\\SCollision.h"
#include "game\\game.h"
#include "game\\LuaAPI.h"

void SCollision::update(Map * world, float frameTime)
{
	EntityManager * entM = world->getEntityManager();

	// if entity manager has changed, ( happens when switching worlds ), cache new entity manager's entities who have hitboxes
	if( this->entM != entM )
		startUp( entM );

	auto deletedlist = entM->getDeletedEntities();
	auto newlist = entM->getNewEntities();

	PPosition * pos1, * pos2;
	PBoundingBox * hitbox1, * hitbox2;
	PImage * img1, * img2;

	// query entity manager for newly created entities and
	// update system's vectors with new entities if new entities exist
	acknowledgeInserts( newlist, entM );

	for(size_t cache_index = 0; cache_index < pboxes.size(); cache_index++)
	{	
		pos1 = pposes[cache_index]; 
		hitbox1 = pboxes[cache_index];
		img1 = images[cache_index];
		
		// clear this entity of the previous pass's colliding entities.
		if ( cache_index == 0 )
			hitbox1->collidingEntities.clear();

		// if this entity was marked for deletion...
		if( removeDelEnt( deletedlist, hitbox1->entity_id, cache_index ) )
		{
			// continue on to next entity
			cache_index--;
			continue;
		}

		for( size_t cache_index_inner = cache_index+1; cache_index_inner < pboxes.size(); cache_index_inner++ )
		{
			pos2 = pposes[cache_index_inner];
			hitbox2 = pboxes[cache_index_inner];			
			img2 = images[cache_index_inner];

			if ( cache_index == 0 )
				hitbox2->collidingEntities.clear();

			// if this entity was marked for deletion...
			if( removeDelEnt( deletedlist, hitbox2->entity_id, cache_index_inner ) )
			{
				// continue on to next entity
				cache_index_inner--;
				continue;
			}			
			
			if(hitbox1->active && hitbox2->active)
			{	
				if(collisionOccurring(entM, pos1, hitbox1, pos2, hitbox2))
				{
					hitbox1->collidingEntities.push_back(hitbox2->entity_id);
					hitbox2->collidingEntities.push_back(hitbox1->entity_id);

					// notify scripts of collision event
					LuaDoa::fireEventMulti(ON_COLLIDE, hitbox1->entity_id, hitbox2->entity_id);
					LuaDoa::fireEventMulti(ON_COLLIDE, hitbox2->entity_id, hitbox1->entity_id);
				}		
			}
		}
	}
	// acknowledge the deletions so that the entity manager may remove marked entities from store
	for(auto it = deletedlist.begin(); it != deletedlist.end(); it++)
	{
		if(!entM->acknowledgedDelete[this->sys_id][(*it)])
			entM->acknowledgeDelete(this->sys_id, (*it));
	}
}

// queries entity manager for newly created entities and
// updates system's vectors with new entities if new entities exist
void SCollision::acknowledgeInserts( std::list<unsigned int> &newlist, EntityManager * entM )
{
	for(auto it = newlist.begin(); it != newlist.end(); it++)
	{
		if(!entM->acknowledgedInsert[this->sys_id][(*it)])
		{
			PBoundingBox * insertBox = entM->getProperty<PBoundingBox>((*it));
			if(insertBox != nullptr)
			{
				pboxes.push_back(insertBox);
				images.push_back(entM->getProperty<PImage>((*it)));
				pposes.push_back(entM->getProperty<PPosition>((*it)));
			}
			
			entM->acknowledgeInsert(this->sys_id, (*it));
		}
	}
}

// remove pointer to entity if this entity is marked for deletion
bool SCollision::removeDelEnt( std::list<unsigned int> &deletedList, unsigned int id, size_t cache_index )
{
	for( auto it = deletedList.begin(); it != deletedList.end(); it++ )
	{
		if( id == (*it) )
		{
			auto temp = pboxes[pboxes.size()-1];
			pboxes[pboxes.size() - 1] = pboxes[cache_index];
			pboxes[cache_index] = temp;
			pboxes.erase(pboxes.end() - 1);

			auto temp2 = images[images.size()-1];
			images[images.size() - 1] = images[cache_index];
			images[cache_index] = temp2;
			images.erase(images.end() - 1);

			auto temp3 = pposes[pposes.size()-1];
			pposes[pposes.size() - 1] = pposes[cache_index];
			pposes[cache_index] = temp3;
			pposes.erase(pposes.end() - 1);

			return true;
		}
	}

	return false;
}

// TODO: split this beast up into sub functions!
// returns true if there is a collision occuring between the two entities.
bool SCollision::collisionOccurring(EntityManager * entM,
									PPosition* pos1,
									PBoundingBox* ent1,
									PPosition* pos2,
									PBoundingBox* ent2)
{
	VECTOR2 collisionVector;
	collisionVector.y = 0.0f;
	collisionVector.x = 0.0f;
	int count = 0;

	if(ent1->collisionType == PBoundingBoxNS::LINE && ent2->collisionType == PBoundingBoxNS::CIRCLE)
	{
		PGravity * gobj = entM->getProperty<PGravity>(ent2->entity_id);
		PMobile * mob = entM->getProperty<PMobile>(ent2->entity_id);

		// store the entity's current position in temporary variables. we will be checking collision with the entity's
		// upcoming position that has yet to be computed for this frame (the SMovement system computes new entity positions
		// after the SCollision system resolves collisions).
		float tempX = pos2->x, tempY = pos2->y;

		if ( mob != nullptr )
		{
			// determine the entity's upcoming position for this frame, and resolve collision
			// with this new position
			pos2->x = pos2->x + mob->direction.x*mob->velocity.x*_Game->getDeltaTime();
			pos2->y = pos2->y + mob->direction.y*mob->velocity.y*_Game->getDeltaTime();

			if( gobj != nullptr && !gobj->grounded && lineCircleColliding(pos1, ent1, pos2, ent2, collisionVector))
			{
				// reset y position
				pos2->y = tempY;

				/* 
				   if collisionVector.y != 0, then this line is representative of the "ground" or "floor".
				   collisionVector.y == 0 only if the line's rise is greater than its run, in which case
				   we treat the line as an impassable "wall" and restrict the colliding entity's movement in the x-direction
				   with which it is colliding with the wall.	
				   additionally, we only resolve collisions with the ground when falling (velocity.y >= 0)
				*/
				if ( collisionVector.y < 0.0f && mob->velocity.y >= 0 )
				{
					/* 
					   Determine how much of the radius is factored into shifting the entity upwards away from the line
					   that is being collided with. 
					   (In other words, find the y position of the circle tangent to the line being collided with)
					   
					   For one extreme case, when the slope is zero, the entirety of the
					   entity's radius is subtracted from the y point of collision on the line (a line with slope zero is tangent to
					   the bottom (or top) of the circle. So we can simply do y_collision_point - center of entity - radius to
					   determine the dy with which to move our entity as to position it so that its circle hitbox is tangent to the line.) 
					  
					   On the other hand, if the slope were infinite instead of zero (a vertical line), then none of the entity's 
					   radius is subtracted from the y point of collision, because in this case the line is tangent to the 
					   left or right of the circle (the horizontal radius is perpendicular to the line), and so we do not adjust
					   the entity's y position at all (as adjusting the y position does nothing to separate these colliding entities
					   if the line intersecting the circle is vertical).

					   For lines with intermediate slopes, we can multiply the radius by the cosine of the triangle whose
					   hypotenuse is the line being collided with to determine how far we can shift the entity so that its
					   comprised circle is tangent to the line.
					*/
					
					float slope = ent1->line.B.y / ent1->line.B.x;
					float magnitude = sqrt( ent1->line.B.x*ent1->line.B.x + ent1->line.B.y*ent1->line.B.y);

					// cosine of the triangle whose hypotenuse is the line 
					float cosine = ent1->line.B.x/magnitude;
					// sine of the triangle whose hypotenuse is the line
					float sine = -ent1->line.B.y/magnitude;

					// determine the y point of collision on the line
					float y = slope*( pos2->getCenterX() + ent2->radius*sine - pos1->x ) + pos1->y;

					// subtract the y point of collision from the entity's center, minus the y component of the vector 
					// perpendicular to the line ( whose magnitude is the circle's radius ).
					// to the line
					float dy = ( y - pos2->getCenterY() ) - ent2->radius*cosine;

					// reset the entity's x position ( we reset it here because we needed to use the upcoming x position
					// in the above computation of y )
					pos2->x = tempX;
					SMovement::moveEntity(pos2, 0, dy);
				
					// set the entity's gravity property's grounded field to true so that the SGravity system
					// knows not to "pull down" on the entity. 
					gobj->grounded = true;

					// set this entity's downward velocity to zero.
					// set this entity's downward velocity to zero.
					mob->velocity.y = 0.0f;
				}
				else if ( collisionVector.x != 0.0f )	// if colliding with a vertical line
				{
					// reset entity's x position after checking collision of future position
					pos2->x = tempX;

					// if entity is to the right of the vertical line, shift factor is the colliding entity's radius. 
					// otherwise, the shift factor is the negative of this radius.
					float shift = collisionVector.x == -1.0f ? ent2->radius*-1 - 2.0f : ent2->radius + 2.0f;
					float dx = pos1->x - pos2->getCenterX() + shift;

					SMovement::moveEntity(pos2, dx, 0);
					mob->velocity.x = 0;
				}

				return true;
			}
			
			pos2->x = tempX;
			pos2->y = tempY;
		}
		
		return false;
	}
	else if(ent1->collisionType == PBoundingBoxNS::CIRCLE && ent2->collisionType == PBoundingBoxNS::LINE)
	{
		PGravity * gobj = entM->getProperty<PGravity>(ent1->entity_id);
		PMobile * mob = entM->getProperty<PMobile>(ent1->entity_id);

		float tempX = pos1->x, tempY = pos1->y;

		if ( mob != nullptr )
		{
			// determine the entity's upcoming position for this frame, and resolve collision
			// with this new position
			pos1->x = pos1->x + mob->direction.x*mob->velocity.x*_Game->getDeltaTime();
			pos1->y = pos1->y + mob->direction.y*mob->velocity.y*_Game->getDeltaTime();

			if( gobj != nullptr && !gobj->grounded && lineCircleColliding(pos2, ent2, pos1, ent1, collisionVector))
			{
				// reset y position
				pos1->y = tempY;

				if ( collisionVector.y < 0.0f && mob->velocity.y >= 0 )
				{
					
					float slope = ent2->line.B.y / ent2->line.B.x;
					float magnitude = sqrt( ent2->line.B.x*ent2->line.B.x + ent2->line.B.y*ent2->line.B.y);

					// cosine of the triangle whose hypotenuse is the line 
					float cosine = ent2->line.B.x/magnitude;
					// sine of the triangle whose hypotenuse is the line
					float sine = -ent2->line.B.y/magnitude;

					// determine the y point of collision on the line
					float y = slope*( pos1->getCenterX() + ent1->radius*sine - pos2->x ) + pos2->y;

					float dy = y - pos1->getCenterY() - ent1->radius*cosine;

					pos1->x = tempX;
					SMovement::moveEntity(pos1, 0, dy);
				
					// set the entity's gravity property's grounded field to true so that the SGravity system
					// knows not to "pull down" on the entity. 
					gobj->grounded = true;

					// set this entity's downward velocity to zero.
					mob->velocity.y = 0.0f;
				}
				else if ( collisionVector.x != 0.0f )	// if colliding with a vertical line
				{
					pos1->x = tempX;

					float shift = collisionVector.x == -1.0f ? ent1->radius*-1 - 2.0f : ent1->radius + 2.0f;
					float dx = pos2->x - pos1->getCenterX() + shift;

					SMovement::moveEntity(pos1, dx, 0);
					mob->velocity.x = 0;
				}

				return true;
			}
			
			pos1->x = tempX;
			pos1->y = tempY;
		}
		
		return false;
	}
	else if(ent1->collisionType == PBoundingBoxNS::CIRCLE && ent2->collisionType == PBoundingBoxNS::BOX)
	{
		float x1 = 0.0f, x2= 0.0f, y1= 0.0f, y2= 0.0f;
		while((ent1->deflect || ent2->deflect) && count < 10 && boxCircleColliding(pos2, ent2, pos1, ent1, collisionVector))
		{
			if(ent2->deflect)
			{
				y1 += collisionVector.y;
				x1 += collisionVector.x;
			}
			if(ent1->deflect)
			{
				y2 -= collisionVector.y;
				x2 -= collisionVector.x;
			}
			count++;
		}

		SMovement::moveEntity(pos1, x1, y1);
		SMovement::moveEntity(pos2, x2, y2);

		if(count == 0)
		{
			return boxCircleColliding(pos2, ent2, pos1, ent1, collisionVector);
		}
		
		return true;
	}
	else if(ent1->collisionType == PBoundingBoxNS::BOX && ent2->collisionType == PBoundingBoxNS::CIRCLE)
	{
		float x1 = 0.0f, x2= 0.0f, y1= 0.0f, y2= 0.0f;
		while((ent1->deflect || ent2->deflect) && count < 10 && boxCircleColliding(pos1, ent1, pos2, ent2, collisionVector))
		{
			if(ent2->deflect)
			{
				y1 += collisionVector.y;
				x1 += collisionVector.x;
			}
			if(ent1->deflect)
			{
				y2 -= collisionVector.y;
				x2 -= collisionVector.x;
			}
			count++;
		}

		SMovement::moveEntity(pos1, x1, y1);
		SMovement::moveEntity(pos2, x2, y2);

		if(count == 0)
		{
			return boxCircleColliding(pos1, ent1, pos2, ent2, collisionVector);
		}
		
		return true;
	}
	else if(ent1->collisionType == PBoundingBoxNS::CIRCLE && ent2->collisionType == PBoundingBoxNS::CIRCLE)
	{
		float x1 = 0.0f, x2= 0.0f, y1= 0.0f, y2= 0.0f;
		while((ent1->deflect || ent2->deflect) && count < 10 && circlesColliding(pos1, ent1, pos2, ent2, collisionVector))
		{
			Graphics::Vector2Normalize(&collisionVector);
			if(ent2->deflect)
			{
				y1 += collisionVector.y;
				x1 += collisionVector.x;
			}
			if(ent1->deflect)
			{
				y2 -= collisionVector.y;
				x2 -= collisionVector.x;
			}

			count++;
		}

		SMovement::moveEntity(pos1, x1, y1);
		SMovement::moveEntity(pos2, x2, y2);

		if( count == 0 )
		{
			return circlesColliding(pos1, ent1, pos2, ent2, collisionVector);
		}
		
		return false;
	}
	else if(ent1->collisionType == PBoundingBoxNS::BOX)
	{
		while(count < 10 && (ent1->deflect || ent2->deflect) && boxesColliding(pos1, ent1, pos2, ent2, collisionVector))
		{
			if(ent2->deflect)
				SMovement::moveEntity(pos1, collisionVector.x, collisionVector.y);
			if(ent1->deflect)
				SMovement::moveEntity(pos2, -collisionVector.x, -collisionVector.y);

			count++;
		}

		if(count == 0)
			return false;

		return true;
	}

	return false;
}


// checks if two circles are colliding by computing whether the distances between
// the center of both circles is less than or equal to the sum of their radii.
bool SCollision::circlesColliding(PPosition * pos1, PBoundingBox * ent1, PPosition * pos2, PBoundingBox * ent2, VECTOR2& collisionVector)
{
	//use square distance so as to not compute the square root (which may be slow)
	float distanceSquared = (pos1->getCenterX() - pos2->getCenterX())*(pos1->getCenterX() - pos2->getCenterX()) + 
							(pos1->getCenterY() - pos2->getCenterY())*(pos1->getCenterY() - pos2->getCenterY());

	float collDistSquared = (ent1->radius + ent2->radius)*(ent1->radius + ent2->radius) + 
							(ent1->radius + ent2->radius)*(ent1->radius + ent2->radius);

	collisionVector.x = pos1->getCenterX()- pos2->getCenterX();
	collisionVector.y = pos1->getCenterY() - pos2->getCenterY();

	return (distanceSquared < collDistSquared);	
}

// line is ent1, circle is ent2
bool SCollision::lineCircleColliding(PPosition * pos1,
									 PBoundingBox * ent1,
									 PPosition * pos2,
									 PBoundingBox * ent2,
									 VECTOR2& collisionVector)
{
	Point A, B;	//compute ent1's points' world coordinates

	A.x = (pos1->x + ent1->line.A.x);
	A.y = (pos1->y + ent1->line.A.y);

	B.x = (pos1->x + ent1->line.B.x);
	B.y = (pos1->y + ent1->line.B.y);

	bool rise_greater = false, run_greater = false;

	if ( abs(ent1->line.B.x) > abs(ent1->line.B.y) )
		run_greater = true;
	else
		rise_greater = true;

	float startpt_x, startpt_y, endpt_x, endpt_y;

	if ( B.x >= A.x )
	{
		startpt_x = A.x;
		endpt_x = B.x;
	}
	else
	{
		startpt_x = B.x;
		endpt_x = A.x;
	}
	if ( B.y >= A.y )
	{
		startpt_y = A.y;
		endpt_y = B.y;
	}
	else
	{
		startpt_y = B.y;
		endpt_y = A.y;
	}

	if( ( run_greater && 
		( pos2->getCenterX() < endpt_x && pos2->getCenterX() >= startpt_x )   ) 
		||
		( rise_greater &&
		( pos2->getCenterY() < endpt_y && pos2->getCenterY() >= startpt_y ) ) )
	{
		//distance vector from center of circle to right-hand endpoint of line
		VECTOR2 distance;
		distance.x = B.x - pos2->getCenterX();
		distance.y = B.y - pos2->getCenterY();

		VECTOR2 line;
		line.x = B.x - A.x;
		line.y = B.y - A.y;

		float dotp = Graphics::Vector2Dot(&distance, &line);

		VECTOR2 projection;		//project the distance from center of circle to pointB onto the line
		projection.x = dotp/(line.x*line.x + line.y*line.y) * line.x;
		projection.y = dotp/(line.x*line.x + line.y*line.y) * line.y;

		float distanceSquared = (distance.x)*(distance.x) + (distance.y)*(distance.y);
		float projectionSquared = (projection.x*projection.x) + (projection.y*projection.y);

		// pythagoras to find the height of the triangle
		// projection^2 + height^2 = distance^2	==> height^2 = distance^2 - projection^2
		float heightSquared = distanceSquared - projectionSquared;

		//if the height of the derived triangle is less than the circle's radius, the entities are colliding
		if(heightSquared <= ent2->radius*ent2->radius)
		{		
			// if the run is greater than the rise, collision vector only has y component
			if ( run_greater )
			{
				if( pos2->getCenterY() <= endpt_y )	// less than endpt_y, must be colliding must from the top
				{
					collisionVector.x = 0.0f;
					collisionVector.y = -1.0f;	
				}
				else								// else, colliding from the bottom
				{
					collisionVector.x = 0.0f;
					collisionVector.y = 1.0f;
				}
			}
			else
			{
				if( pos2->getCenterX() <= B.x )				// less than B.x, must be colliding from left
				{
					collisionVector.x = -1.0f;
					collisionVector.y = 0.0f;	
				}
				else										// else, colliding from right
				{
					collisionVector.x = 1.0f;
					collisionVector.y = 0.0f;
				}
			}
			
			return true;
		}
	}
	return false;
}

// ent1 is box, ent2 is circle
bool SCollision::boxCircleColliding(PPosition* pos1, PBoundingBox* ent1, PPosition* pos2, PBoundingBox* ent2, VECTOR2& collisionVector)
{
	bool lessThanLeftX = true, greaterThanRightX = true, lessThanBotY = true, greaterThanTopY = true;

	// check centerX + radius < boxX + box width, centerY + radius < boxY + box height
	if ( pos2->getCenterX() + ent2->radius < pos1->x + ent1->corners[0].x )
	{
		return false;
	}
	else
	{
		lessThanLeftX = false;
	}

	if ( pos2->getCenterX() - ent2->radius > pos1->x + ent1->corners[1].x ) 
	{
		return false;
	}
	else
	{
		greaterThanRightX = false;
	}

	if ( pos2->getCenterY() + ent2->radius < pos1->y + ent1->corners[0].y )
	{
		return false;
	}
	else
	{
		lessThanBotY = false;
	}
	if ( pos2->getCenterY() - ent2->radius > pos1->y + ent1->corners[3].y )
	{
		return false;
	}
	else
	{
		greaterThanTopY = false;
	}

	float leftX = pos2->getCenterX() + ent2->radius -  pos1->x + ent1->corners[0].x;
	float rightX = pos1->x + ent1->corners[3].x - pos2->getCenterX() - ent2->radius;

	float topY = pos2->getCenterY() + ent2->radius - pos1->y + ent1->corners[0].y;
	float botY = pos1->y + ent1->corners[3].y - pos2->getCenterY() - ent2->radius;

	if ( !lessThanLeftX && leftX > rightX )
		collisionVector.x = -1.0f;
	else if ( !greaterThanRightX )
		collisionVector.x = 1.0f;

	if ( !lessThanBotY && topY > botY )
		collisionVector.y = -1.0f;
	else if ( !greaterThanTopY )
		collisionVector.y = 1.0f;

	return true;
}

//TODO: implement
bool SCollision::boxesColliding(PPosition * pos1, PBoundingBox * ent1, PPosition * pos2, PBoundingBox * ent2, VECTOR2& collisionVector)
{
	/*	
	if(pos1->getX() + pos1->getScaledWidth() >= pos2->getX() && pos1->getX() <= pos2->getX() + pos2->getScaledWidth() &&
		pos1->getY() + pos1->getScaledHeight() >= pos2->getY() && pos1->getY() <= pos2->getY() + pos2->getScaledHeight())
	{
		if(pos1->getY() + pos1->getScaledHeight() >= pos2->getY() && pos1->getY() <= pos2->getY() + pos2->getScaledHeight())
		{
			if(pos1->getY() > pos2->getY() + (pos2->getHeight()/2))
			{
				collisionVector.x = 0.0f;
				collisionVector.y = 1.0f;
			}
			else
			{
				collisionVector.x = 0.0f;
				collisionVector.y = -1.0f;
			}
			return true;
		}
		if(pos1->getX() + pos1->getScaledWidth() >= pos2->getX() && pos1->getX() <= pos2->getX() + pos2->getScaledWidth())
		{
			if(pos1->getX() > pos2->getX() + (pos2->getWidth())/2)
			{
				collisionVector.x = 1.0f;
				collisionVector.y = 0.0f;
			}
			else
			{
				collisionVector.x = -1.0f;
				collisionVector.y = 0.0f;
			}
			return true;
		}
	}*/
		return false;
}