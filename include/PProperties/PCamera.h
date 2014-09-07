#ifndef _PCAMERA_H
#define _PCAMERA_H

#include "properties\\PProperty.h"

struct PPosition;

struct PCamera : public PProperty
{
	PCamera() : initialized(false), prevX(0.0f), prevY(0.0f), dx(0.0f), dy(0.0f), offsetX(0.0f), offsetY(0.0f), focusTargetId(999999), 
	zoom(1.0f), priority(0), newzoom(0.0f), timeInterval(0.0f), elapsedTime(0.0f), focus(nullptr){};

	bool initialized;

	PPosition * focus;

	unsigned int priority;

	unsigned int focusTargetId;

	float offsetX;		//the screen offset from the origin (top left corner) to the x position of the focus target
	float offsetY;		//the screen offset from the origin (top left corner) to the y position of the focus target

	float prevX;		//used to determine dx. set every frame
	float prevY;		//used to determine dy. set every frame

	float dx;			//how far to move the world in the x direction. computed by taking the focus target's change in X in 1 frame
	float dy;			//how far to move the world in the y direction. computed by taking the focus target's change in Y in 1 frame

	float followSpeed;	//the speed at which the camera follows its focus target

	float delay;		//the delay in seconds with which the camera follows its focus target

	float zoom;			//the camera's zoom factor

	float newzoom;

	float timeInterval;	

	float elapsedTime;

	void initialize(PPosition * focus, float screenX, float screenY);

	void moveTo(float x, float y);

	// switches camera focus from oldFocus to newFocus
	void switchFocus(PPosition * newFocus);

	virtual void onDelete(unsigned int id);

};

#endif