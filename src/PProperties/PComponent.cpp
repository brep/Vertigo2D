#include "properties\\PComponent.h"

void PComponent::initialize(unsigned int host_id, unsigned int component_id, std::string flag, float offset_x, float offset_y)
{
	this->flags = flag;
	this->offsetX = offset_x;
	this->offsetY = offset_y;

	this->hostID = host_id;
	this->componentID = component_id;
}

void PComponent::updatePosition(PPosition * host_pos, PPosition * comp_pos)
{
	updatePositionX( host_pos, comp_pos );
	updatePositionY( host_pos, comp_pos );
}

void PComponent::updatePositionX(PPosition * host_pos, PPosition * comp_pos)
{
	if(this->flags == "ABSOLUTE")
	{
		comp_pos->x = host_pos->getCenterX() - this->offsetX;
	}
	if(this->flags == "CENTER_CENTER")
	{
		comp_pos->x = host_pos->getCenterX() - comp_pos->getScaledWidth()/2 - this->offsetX;
	}
	if(this->flags == "CENTER_TOP")
	{
		comp_pos->x = host_pos->getCenterX() - comp_pos->getScaledWidth()/2 - this->offsetX;
	}
}

void PComponent::updatePositionY(PPosition * host_pos, PPosition * comp_pos)
{
	if(this->flags == "ABSOLUTE")
	{
		comp_pos->y = host_pos->getCenterY() - this->offsetY;	
	}
	if(this->flags == "CENTER_CENTER")
	{
		comp_pos->y = host_pos->getCenterY() - comp_pos->getScaledHeight()/2 - this->offsetY;
	}
	if(this->flags == "CENTER_TOP")
	{
		comp_pos->y = host_pos->y - 25 - this->offsetY;
	}
}