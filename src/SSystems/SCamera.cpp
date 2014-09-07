#include "systems\\SCamera.h"
#include "game\\game.h"

void SCamera::update( Map * world, float frameTime )
{
	PCamera * cam = world->getPrimaryCamera();

	if( cam == nullptr )
		return;

	// apply zoom to camera over specified time
	if( cam->timeInterval != 0 && cam->elapsedTime < cam->timeInterval )
	{
		float dzoom = cam->newzoom - cam->zoom;
		cam->zoom += ( dzoom*frameTime )/cam->timeInterval;
		cam->elapsedTime += frameTime;
	}

	// move the world by the camera's change in position
	moveCamera( world, -cam->dx, -cam->dy );

	// reset dx and dy
	cam->dx = 0.0f;
	cam->dy = 0.0f;
}

// simulate camera movement by adding deltaX and deltaY to all entities' PPosition properties
//
// IMPORTANT NOTE:
//		this is an "old-school" way of doing Camera movement. We could have instead taken advantage of
//		DirectX 3d capabilities and moved the camera by transforming the View matrix, and as a result get
//		camera movement for free instead of having to move each individual entity in the world. I originally
//		did it the way it's implemented now out of ignorance to the fact, but the important thing is that
//		it works fine, so I won't change it YET. If the worlds end up containing so many entities that
//		as a result this function causes a bottleneck in the engine, then this code will be modified.

void SCamera::moveCamera( Map * world, float deltaX, float deltaY, bool interpolate )
{
	Graphics * graphics = _Game->getGraphics();
	EntityManager * entM = world->getEntityManager();

	// add deltaX and deltaY to world.x and world.y
	world->setX( world->getX() + deltaX );
	world->setY( world->getY() + deltaY );

	// bound checks
	if(world->getX() > 0.0f)
	{
		deltaX = 0.0f;
		world->setX( 0.0f );
	}
	if(world->getY() > 0.0f)
	{
		deltaY = 0.0f;
		world->setY( 0.0f );
	}
	if(world->getX() < world->getMinX())
	{
		deltaX = 0.0f;
		world->setX( (float)world->getMinX() );
	}
	if(world->getY() < world->getMinY())
	{
		deltaY = 0.0f;
		world->setY( (float)world->getMinY() );
	}

	if( deltaX == 0.0f && deltaY == 0.0f )
		return;								// no updating of entities' positions needs to occur ; world has not moved

	float x = world->getX();
	float y = world->getY();

	// if not interpolating world movement, set previous coordinates to current coordinates
	if( !interpolate )
	{
		world->setPrevX( x );
		world->setPrevY( y );
	}

	auto list = entM->getProperties<PPosition>();

	for(auto it = list.begin(); it != list.end(); it++ )
	{
		PPosition * pos = it->second;

		// do not shift position of entity if followCamera field is true
		if(!pos->followCamera)	
		{
			// adjust position of entity to account for world moving
			pos->x += deltaX;
			if( !interpolate )
				pos->prevX += deltaX;

			pos->y += deltaY;
			if( !interpolate )
				pos->prevY += deltaY;
		}

		pos->worldX = pos->x - x;
		pos->worldY = pos->y - y;
	}

	// update parallax layers' positions
	for(auto it = world->parallaxLayers.begin(); it != world->parallaxLayers.end(); it++)
	{
		float left = graphics->getWorldLeft();
		float top = graphics->getWorldTop();
		float right = graphics->getWorldRight();
		float bot = graphics->getWorldBottom();
		float speedFactor = it->speedFactor;

		float dx = deltaX*speedFactor, dy = deltaY*speedFactor;

		// mod deltaX by image width to keep background within the range (-background.width, left) and (-background.height, top)
		if( abs(dx) > it->background.getSpriteWidth() )
			dx = (int)dx % (int)it->background.getSpriteWidth() + (dx - (int)dx);
		if( abs(dy) > it->background.getSpriteHeight() )
			dy = (int)dy % (int)it->background.getSpriteHeight() + (dy - (int)dy);

		it->background.setSpriteX( it->background.getSpriteX() + dx );
		it->background.setSpriteY( it->background.getSpriteY() + dy );

		float bx = it->background.getSpriteX();
		float by = it->background.getSpriteY();

		// if left side of image is visible..
		if ( bx > left )   
		{
			// move image to the left
			it->background.setSpriteX( bx - (float)it->background.getSpriteWidth() );
			it->background.setSpritePrevX( it->background.getSpriteX() - dx );
		}
		// if image is too far left...
		if ( bx + (float)it->background.getSpriteWidth() < left )
		{
			// move image to the right
			it->background.setSpriteX( bx + (float)it->background.getSpriteWidth() );
			it->background.setSpritePrevX( it->background.getSpriteX() - dx );
		}
		// if top of image visible...
		if ( by > top )    
		{
			// move image up
			it->background.setSpriteY( by - (float)it->background.getSpriteHeight() );
			it->background.setSpritePrevY( it->background.getSpriteY() - dy );
		}
		// if image too far up...
		if ( by + (float)it->background.getSpriteHeight() < top )
		{
			// move image down
			it->background.setSpriteY( by + (float)it->background.getSpriteHeight() );
			it->background.setSpritePrevY( it->background.getSpriteY() - dy );
		}
	}
}

void performZoom()
{

}