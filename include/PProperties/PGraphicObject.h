#ifndef _PGRAPHICOBJ_H
#define _PGRAPHICOBJ_H

#include "properties\\pproperty.h"
#include "properties\\PPosition.h"
#include "properties\\\PTimer.h"
#include "graphics\\graphics.h"

class Graphics;

class PGraphicObject : public PProperty
{
protected:
	bool visible;				// Self explanatory
	int layer;					// Graphic objects are drawn by layer (lower layers drawn first)
	COLOR_ARGB colorFilter;		// 
	PPosition * position;		// Every graphic object has a position in screen and world space

public:
	PGraphicObject(){visible = true; layer = 0; position = nullptr; colorFilter = graphicsNS::FILTER;}
	~PGraphicObject(){ position = nullptr; }

	int alpha;
	int endAlpha;
	int alinc;				// alpha increment per task call
	float endScale;
	float scinc;			// scale increment per task call
	
	virtual void setX(float x){ position->x = x; }
	virtual void setX_NL(float x){ position->x= x; position->prevX = x; }
	virtual void setY(float y){ position->y = y; }
	virtual void setY_NL(float y){ position->y = y; position->prevY = y; }

	virtual float getX(){ return position->x; }
	virtual float getY(){ return position->y; }
	
	virtual float getScale(){ return position->scale; }

	virtual float getScaledWidth(){ return position->getScaledWidth(); }
	virtual float getScaledHeight(){ return position->getScaledHeight(); }

	virtual PPosition * getPosition(){ return position; }

	int getLayer() const {return layer;}
	void setLayer(int layer){this->layer = layer;}

	virtual COLOR_ARGB getColorFilter() const {return colorFilter;}

    // set color filter. (use WHITE/FILTER for no change)
    virtual void setColorFilter(COLOR_ARGB color) {colorFilter = color;}
	
	virtual void setColorFilterLua(std::string color);

	virtual void setColorFilterRawLua( int a, int r, int g, int b ){ colorFilter = D3DCOLOR_ARGB( a, r, g, b ); }

	bool getVisible() const {return visible;}
	void setVisible(bool vis){ this->visible = vis; }

	// inheriting class must implement draw() -> to be used in SRender:update()
	virtual void draw( Graphics * g ){}

	virtual void animate( float frameTime ){}

	// inheriting class must describe how its position is to be updated
	virtual void updatePosition(){}

	void fadeIn(int alpha, int endAlpha, int increment, float duration);

	void fadeOut(int alpha, int endAlpha, int increment, float duration);

	void scaleOverTime(float scale, float endScale, float increment, float duration);

};

#endif