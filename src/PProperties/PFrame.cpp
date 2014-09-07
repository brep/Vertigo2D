#include "properties\\PFrame.h"
#include "game\\game.h"

void PFrame::initialize(PGraphicObject* image, float x_offset, float y_offset)
{
	if( image->entity_id != this->entity_id )
		_Game->getConsole()->warning("PFrame::initialize() - passed a PGraphicObject whose ID is not equal to PFrame's ID.");

	this->image = image;

	parts.push_back(Partition());

	parts[0].x_offset_left = x_offset;
	parts[0].x_offset_right = x_offset;
	parts[0].y_offset_top = y_offset;
	parts[0].y_offset_bot = y_offset;

	parts[0].x = image->getX() + x_offset*image->getScale();
	parts[0].y = image->getY() + y_offset*image->getScale();
	parts[0].width = (int)( image->getScaledWidth() - (x_offset + x_offset)*image->getScale() );
	parts[0].height = (int)( image->getScaledHeight() - (y_offset + y_offset)*image->getScale() );

	scrollRect.left = (LONG)parts[0].x;
	scrollRect.right = (LONG)parts[0].width;
	scrollRect.top = (LONG)parts[0].y;
	scrollRect.bottom = (LONG)parts[0].height;

	lastFrameX = image->getX();
	lastFrameY = image->getY();

	this->initialized = true;
}

void PFrame::initialize_allOffsets(PGraphicObject * image,
								   float x_offset_left,
								   float x_offset_right,
								   float y_offset_top,
								   float y_offset_bot)
{
	this->image = image;

	parts.push_back(Partition());

	parts[0].x_offset_left = x_offset_left;
	parts[0].x_offset_right = x_offset_right;
	parts[0].y_offset_top = y_offset_top;
	parts[0].y_offset_bot = y_offset_bot;

	parts[0].x = image->getX() + x_offset_left*image->getScale();
	parts[0].y = image->getY() + y_offset_top*image->getScale();
	parts[0].width = (int)( image->getScaledWidth() - (x_offset_left + x_offset_right)*image->getScale() );
	parts[0].height = (int)( image->getScaledHeight() - (y_offset_top + y_offset_bot)*image->getScale() );

	lastFrameX = image->getX();
	lastFrameY = image->getY();

	this->initialized = true;
}

void PFrame::initializeNoPart(PGraphicObject * image)
{
	this->image = image;

	lastFrameX = image->getX();
	lastFrameY = image->getY();

	this->initialized = true;
}

void PFrame::addWidgetPart(PGraphicObject * widget, std::string flag, unsigned int part_id)
{
	// acquiring an alien entity's property... register with entityManager's onDelete event.
	_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerOnDelete(widget->entity_id, this->entity_id);

	if( parts[part_id].widgets.size() < maxWidgets )
	{
		parts[part_id].widgets.push_back(widget);
		spreadWidgets(flag, part_id);
	}
}

void PFrame::addWidget(PGraphicObject * widget, std::string flag)
{
	addWidgetPart( widget, flag, 0 );
}

void PFrame::addWidgetIndex(PGraphicObject * widget, std::string flag, unsigned int part_id, unsigned int index)
{
	// acquiring an alien entity's property... register with entityManager's onDelete event.
	_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerOnDelete(widget->entity_id, this->entity_id);

	Partition * part = &parts[part_id];
	part->widgets.push_back(widget);

	for( size_t i = part->widgets.size()-1; i > index; i-- )
	{
		PGraphicObject * temp = part->widgets[i];
		part->widgets[i] = part->widgets[i-1];
		part->widgets[i-1] = temp;
	}
	
	spreadWidgets(flag, part_id);
}

/*
* position widget relative to the frame
* translate frame coordinates to screen coordinates
*/
void PFrame::addWidgetAbsolutePart(PGraphicObject * widget, float x, float y, unsigned int part_id)
{
	// acquiring an alien entity's property... register with entityManager's onDelete event.
	_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerOnDelete(widget->entity_id, this->entity_id);

	x += parts[part_id].x;
	y += parts[part_id].y;

	widget->setX_NL(x);
	widget->setY_NL(y);		
		
	parts[part_id].widgets.push_back(widget);
}

void PFrame::addWidgetNoPosition(PGraphicObject * widget, unsigned int part_id)
{
	// acquiring an alien entity's property... register with entityManager's onDelete event.
	_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerOnDelete(widget->entity_id, this->entity_id);

	parts[part_id].widgets.push_back(widget);
}

void PFrame::addWidgetAbsolute(PGraphicObject * widget, float x, float y)
{	
	addWidgetAbsolutePart(widget, x, y, 0);
}

//assumes all elements are the same size.
void PFrame::spreadWidgets(std::string flag, unsigned int part_id)
{
	Partition * part = &parts[part_id];

	if(flag == "CENTER_X")
	{
		float dx = (float)part->width/(float)part->widgets.size();

		for(size_t i = 0; i < part->widgets.size(); i++)
		{
			part->widgets[i]->setX_NL( part->x + i*dx + (dx - part->widgets[i]->getScaledWidth())/2 );
			part->widgets[i]->setY_NL( part->y + (part->height - part->widgets[i]->getScaledHeight())/2 );
		}
	}
	if(flag == "CENTER_Y")
	{
		float dy = (float)part->height/(float)part->widgets.size();

		for(size_t i = 0; i < part->widgets.size(); i++)
		{
			part->widgets[i]->setX_NL( part->x + (part->width - part->widgets[i]->getScaledWidth())/2.0f );
			part->widgets[i]->setY_NL( part->y + i*dy + (dy - part->widgets[i]->getScaledHeight())/2.0f );
		}
	}
	if(flag == "LEFT_CENTER_Y")
	{
		float dy = (float)part->height/(float)part->widgets.size();

		for(size_t i = 0; i < part->widgets.size(); i++)
		{
			part->widgets[i]->setX_NL( part->x );
			part->widgets[i]->setY_NL( part->y + i*dy + (dy - part->widgets[i]->getScaledHeight())/2.0f );
		}
	}
	if(flag == "TOP_LEFT_NO_SPREAD")
	{
		part->widgets[ part->widgets.size()-1 ]->setX_NL( part->x );

		if( part->widgets.size() > 1 )
		{
			part->widgets[ part->widgets.size()-1 ]->setY_NL( part->widgets[ part->widgets.size()-2 ]->getY() + part->widgets[ part->widgets.size()-1 ]->getScaledHeight() );
		}
		else
		{
			part->widgets[ part->widgets.size()-1 ]->setY_NL( part->y );
		}
	}
	if(flag == "NO_SPREAD_Y")
	{
		part->widgets[ part->widgets.size()-1 ]->setX_NL( part->x + (part->width - part->widgets[ part->widgets.size()-1 ]->getScaledWidth())/2.0f );

		if( part->widgets.size() > 1 )
		{
			part->widgets[ part->widgets.size()-1 ]->setY_NL( part->widgets[ part->widgets.size()-2 ]->getY() + part->widgets[ part->widgets.size()-1 ]->getScaledHeight() );
		}
		else
		{
			part->widgets[ part->widgets.size()-1 ]->setY_NL( part->y );
		}
	}
}

void PFrame::removeWidget(unsigned int id, unsigned int part_id)
{
	for( auto it = parts[part_id].widgets.begin(); it != parts[part_id].widgets.end(); it++ )
	{
		if( id == 0 )
		{
			parts[part_id].widgets.erase(it);
			// unacquiring an alien entity's property... unregister with entityManager's onDelete event.
			_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerOnDelete((*it)->entity_id, this->entity_id);

			return;
		}
		id--;
	}
}

// vertically/horizontally split partition whose id is part_id into two partitions
// the offsets are applied to the rightmost / bottommost partition
void PFrame::makePartition(float x,
						   float y,
						   int width,
						   int height)
{
	size_t i = parts.size();
	parts.push_back(Partition());

	parts[i].x = x;
	parts[i].y = y;
	parts[i].width = width;
	parts[i].height = height;
}

void PFrame::setScrollRect(LONG left, LONG top, LONG right, LONG bot)
{
	scrollRect.left = left;
	scrollRect.right = right;
	scrollRect.top = top;
	scrollRect.bottom = bot;
}

void PFrame::reposWidgets()
{
	float dx = image->getX() - lastFrameX;
	float dy = image->getY() - lastFrameY;

	if( dx != 0 || dy != 0 )
	{
		for( size_t i = 0; i < parts.size(); i++ )
		{
			parts[i].x += dx;
			parts[i].y += dy;

			for( auto it = parts[i].widgets.begin(); it != parts[i].widgets.end(); it++ )
			{
				(*it)->setX( (*it)->getX() + dx );
				(*it)->setY( (*it)->getY() + dy );
			}
		}
	}

	lastFrameX = image->getX();
	lastFrameY = image->getY();
}

void PFrame::onDelete(unsigned int id)
{
	// look through all partitions for widget with entity_id == id, and then erase said widget
	for( size_t i = 0; i < parts.size(); i++ )
	{
		for( size_t j = 0; j < parts[i].widgets.size(); j++ )
		{
			if( parts[i].widgets[j]->entity_id == id )
			{
				parts[i].widgets.erase( parts[i].widgets.begin() + j );
				return;
			}
		}
	}
}