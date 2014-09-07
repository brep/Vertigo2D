#ifndef _SRENDER_H
#define _SRENDER_H

#include "graphics\\Graphics.h"
#include "properties\\PImage.h"
#include "world\\EntityManager.h"
#include "systems\\SSystem.h"

class Game;

namespace SRenderNS
{
	const int LIGHT_LAYER = 100;
	const int UI_LAYER = 110;
}

class SRender : public SSystem
{

private:
	bool lightsExist;
	unsigned int lightBeginIndex;
	int UI_Index;

public:
	SRender() : SSystem()
	{
		UI_Index = -1;
		lightBeginIndex = 0;
		lightsExist = false;	
	}

	void startUp(EntityManager * entM);

	void update(Map * world, float frameTime);

	inline void draw( PGraphicObject * gobj, Graphics * g, float frameTime )
	{
		gobj->updatePosition();
		gobj->draw( g );
		gobj->animate( frameTime );
	}

	size_t renderImages(Graphics * g, EntityManager * entM, std::vector<PGraphicObject*> list, int stopLayer, size_t beginIndex, float frameTime);

	void renderLights(Graphics * g, EntityManager * entM, std::vector<PGraphicObject*> list, float frameTime);

	void renderGui(Graphics * g, EntityManager * entM, std::vector<PGraphicObject*> list, float frameTime);

	void renderTiles(Graphics * g, Map * world);

	void renderParallaxLayers(Graphics * g, Map * world);

	void shutDown(){};
};

#endif