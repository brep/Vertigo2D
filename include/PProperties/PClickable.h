#ifndef _PCLICKABLE_H
#define _PCLICKABLE_H

#include "properties\\PProperty.h"

class PClickable : public PProperty
{
private:
	bool active;
	bool clicked;
	bool mouseIn;

public:
	PClickable(){ active = true; clicked = false; mouseIn = false; }
	~PClickable(){}

	
	bool isActive() const { return active; }

	void setActive(bool active) { this->active = active; }

	bool getClicked() const { return clicked; }

	bool getMouseIn() const { return mouseIn; }

	void setMouseIn(bool v) { mouseIn = v; }

	void setClicked(bool clicked){ this->clicked = clicked; }
};

#endif