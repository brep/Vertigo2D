#include "properties\\PCamera.h"
#include "game\\game.h"

void PCamera::initialize(PPosition * focus, float screenX, float screenY)
{
	if( focus == nullptr )
	{
		Log(consoleNS::WARNING, "Attempted to initialize PCamera with NULL focus.");
		return;
	}
	if( focus->entity_id != this->entity_id )
	{
		// acquiring another PProperty ; must register to entity manager's onDelete event to be 
		// notified of focus's deletion (so that we may set the pointer to null and not attempt to
		// access a deleted property).
		_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerOnDelete(focus->entity_id, this->entity_id);
	}

	this->focus = focus;
	this->focusTargetId = focus->entity_id;
	this->offsetX = screenX;
	this->offsetY = screenY;

	focus->followCamera = true;
	this->initialized = true;

	// move camera to focus target location.
	moveTo( focus->x, focus->y );
}

// instantly moves camera to location x, y
void PCamera::moveTo(float x, float y)
{
	if( focus == nullptr )
	{
		Log(consoleNS::WARNING, "Attempted to move a camera with no focus target.");
		return;
	}
	if( !initialized )
	{
		Log(consoleNS::WARNING, "Attempted to move an unitialized PCamera.");
		return;
	}

	Map * world = _Game->getWorldManager()->getCurrentWorld();

	this->dx = offsetX - x;
	this->dy = offsetY - y;

	this->prevX = offsetX;
	this->prevY = offsetY;

	focus->x = offsetX;
	focus->y = offsetY;
	focus->prevX = offsetX;
	focus->prevY = offsetY;

	SCamera::moveCamera(world, dx, dy, false);	// do not wish to interpolate camera movement, hence pass in false 4th param
	this->dx = 0.0f;
	this->dy = 0.0f;
}

// switches camera focus from oldFocus to newFocus
void PCamera::switchFocus(PPosition * newFocus)
{
	if( focus->entity_id != this->entity_id )	// unregister onDelete event if the current focus's id != camera's id
		_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->unregisterOnDelete(focus->entity_id, this->entity_id);

	// set old focus followCamera property to false
	if( focus != nullptr )
		focus->followCamera = false;
	
	initialize(newFocus, offsetX, offsetY);
}

void PCamera::onDelete(unsigned int id)
{
	// if entity being deleted is this camera's focus, set focus pointer to null.
	if( focus != nullptr && id == focus->entity_id )
		focus = nullptr;
}