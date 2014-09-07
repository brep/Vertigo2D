#ifndef _PPOSITgION_H
#define _PPOSITgION_H

#include "properties\\PProperty.h"

struct PPosition : public PProperty
{
	PPosition() : x(0.0f), y(0.0f), worldX(0.0f), worldY(0.0f), width(2), height(2), scale(1.0f), followCamera(false),
				  prevX(0.0f), prevY(0.0f), percentWidth(1.0f), percentHeight(1.0f) {};
	
	// position in screen space and world space
	float x, y, worldX, worldY, prevX, prevY;

	float scale;

	int width, height;

	float percentWidth, percentHeight;

	// followCamera
	// if true:
	//		for every (dx,dy) the camera moves, this entity also moves (dx,dy)
	// if false:
	//		for every (dx,dy) the camera moves, this entity moves (-dx,-dy)
	bool followCamera;

	float getCenterX() const { return x + (width*scale)/2; }
	float getCenterY() const { return y + (height*scale)/2; }

	float getScaledWidth() const { return width*scale; }
	float getScaledHeight() const { return height*scale; }

	// lua bindings

	// set both x,y and prevX,prevY in lua scripting set..() calls. 
	// The reason for "doing it this way" is that 99.9% of the time, 
	// when a scripter is setting an entity to a explicit location, the
	// desire is for the entity to instantly "teleport" to that location.
	void luabind_setX(float x);
	void luabind_setY(float y);

	float luabind_getX() const { return this->x; }
	float luabind_getY() const { return this->y; }
};

#endif