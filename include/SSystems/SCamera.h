#ifndef _SCAaMERA_H
#define _SCAaMERA_H

#include "world\\EntityManager.h"
#include "systems\\SSystem.h"
#include "properties\\PCamera.h"

class SCamera : public SSystem
{
private:

public:
	SCamera() : SSystem(){};

	void startUp(EntityManager * entM){};

	void update(Map * world, float frameTime);

	static void moveCamera(Map * world, float dx, float dy, bool interpolate = true);

	void shutDown(){};	//null all

	void performZoom();
};

#endif