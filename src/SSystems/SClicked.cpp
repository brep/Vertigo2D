#include "systems\\SClicked.h"
#include "game\\game.h"

void SClicked::update( Map * world, float frameTime )
{	
	EntityManager * entM = world->getEntityManager();

	auto list = entM->getGobjsByLayer();
	Input * input = _Game->getInput();

	Graphics * g = _Game->getGraphics();
	float zoom = world->getCameraZoom();

	// process clicks on entities by highest layer
	for( int k = list.size()-1; k >= 0; k-- )
	{
		// check if this entity has a pframe
		PFrame * frame = entM->getProperty<PFrame>( list[k]->entity_id );

		// process clicks on the frame's widgets
		if( frame != nullptr && frame->isActive() )
		{
			for( size_t i = 0; i < frame->parts.size(); i++ )
			{
				for(auto jt = frame->parts[i].widgets.begin(); jt != frame->parts[i].widgets.end(); jt++ )
				{
					PClickable * click = entM->getProperty<PClickable>( (*jt)->entity_id );
					PImage * img = entM->getProperty<PImage>( (*jt)->entity_id );

					if( click != nullptr && click->isActive() )
					{
						// if mouse click occurred on position
						if( checkMouseEvents((*jt)->getPosition(), click, list[k]->getLayer() ) )
							return;	// return; process 1 click max per frame
					}
				}
			}
			// check click on main frame
			if( checkMouseEvents( list[k]->getPosition(), nullptr, list[k]->getLayer() ) )
				return;

			// if this frame is the focus, do not process mouse events on other entities
			if( frame->isFocus() )
				return;
		}
		// process click if entity is visible
		else if( list[k]->getVisible() )
		{
			// check if this entity has a pclickable property
			PClickable * click = entM->getProperty<PClickable>(list[k]->entity_id);
		
			if( click != nullptr )
			{
				// if mouse click occurred on position
				if( checkMouseEvents(list[k]->getPosition(), click, list[k]->getLayer() ) )
					break;	// return; process 1 click max per frame
			}
		}
	}

	// fire general mouse events
	if( input->mouseLButtonReleased )
		LuaDoa::fireAllEvent(ON_LEFTMOUSE_UP, 0);
	if( input->mouseRButtonReleased )
		LuaDoa::fireAllEvent(ON_RIGHTMOUSE_UP, 1);
	if( input->mouseLDblClk )
		LuaDoa::fireAllEvent(ON_LEFTMOUSE_DBLCLICK, 2);
}

// returns true if mouse click occurred on pos rect
bool SClicked::checkMouseEvents(PPosition* pos, PClickable* click, int layer)
{
	int mouseX, mouseY;
	int width = (int)pos->x + pos->getScaledWidth();
	int height = (int)pos->y + pos->getScaledHeight();
	int x = (int)pos->x;
	int y = (int)pos->y;
	bool ret = false;

	if( layer >= SRenderNS::UI_LAYER )
	{
		// use gui-layer mouse coords
		mouseX = _Game->getInput()->getGMouseX();
		mouseY = _Game->getInput()->getGMouseY();
	}
	else
	{
		// use world mouse coords
		mouseX = _Game->getInput()->getWMouseX();
		mouseY = _Game->getInput()->getWMouseY();
	}

	//check if mouse entered clickable region ( any point on the image )
	if( mouseX >= x && mouseX <= width &&
		mouseY >= y && mouseY <= height )
	{
			//if left mouse click occurred on pos
			if(_Game->getInput()->mouseLButtonReleased)
			{
				LuaDoa::fireEvent(ON_CLICK, pos->entity_id);
				ret = true;
			}
			//if right mouse click occurred on pos
			if(_Game->getInput()->mouseRButtonReleased)
			{
				LuaDoa::fireEvent(ON_RIGHTCLICK, pos->entity_id);
				ret = true;
			}
			//if mouse entered button region
			if(click != nullptr && !click->getMouseIn())
			{
				click->setMouseIn(true);
				LuaDoa::fireEvent(ON_MOUSEENTER, pos->entity_id);
			}
	}
	else if(click != nullptr && click->getMouseIn())
	{
		click->setMouseIn(false);
		LuaDoa::fireEvent(ON_MOUSEEXIT, pos->entity_id);
	}

	return ret;
}