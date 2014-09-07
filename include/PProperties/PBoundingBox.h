#ifndef _PBOUNDBOX_H
#define _PBOUNDBOX_H

#include "properties\\PProperty.h"
#include "graphics\\graphics.h"

namespace PBoundingBoxNS
{
	enum COLLISION_TYPE {NONE, LINE, CIRCLE, BOX, ROTATED_BOX};
	const int INDESTRUCTIBLE = 2;
	const int DESTRUCTIBLE = 4;
	const int PLAYER = 8;
};

struct Point
{
	float x;
	float y;
};

struct Line
{
	Point A;
	Point B;
	float magnitude;
};

struct PBoundingBox : public PProperty
{
	PBoundingBox() : active(false), onCollide(false), deflect(true), isGround(false), roundness(0.0f) {};
	//========================================
	// triggers
	//========================================
    bool onCollide;

	//========================================
	// data
	//========================================
	bool active;
	bool deflect;			//deflects off colliding entities; defaults to true
	
	bool isGround;			//true if this bounding box represents a ground/floor entity

	float roundness;

    PBoundingBoxNS::COLLISION_TYPE collisionType;
	VECTOR2 center;         // center of entity
	float radius;			// for CIRCLE collision det
    RECT    edge;           // for BOX and ROTATED_BOX collision detection
    VECTOR2 corners[4];     // for ROTATED_BOX collision detection
    VECTOR2 edge01,edge03;  // edges used for projection
	Line line;

	std::vector<int> collidingEntities;

    // min and max projections for this entity
    float   edge01Min, edge01Max, edge03Min, edge03Max;	

	//Lua wrapper function
	void setCollisionType(std::string type)
	{
		if(type == "CIRCLE")
			collisionType = PBoundingBoxNS::CIRCLE;

		if(type == "BOX")
			collisionType = PBoundingBoxNS::BOX;

		if(type == "LINE")
			collisionType = PBoundingBoxNS::LINE;

		active = true;
	}
	
	void setCorners(float topleftX, float topleftY, float botrightX, float botrightY)
	{
		corners[0].x = topleftX;
		corners[0].y = topleftY;

		corners[1].x = botrightX;
		corners[1].y = topleftY;		

		corners[2].x = botrightX;
		corners[2].y = botrightY;

		corners[3].x = topleftX;
		corners[3].y = botrightY;
	}

	void setLinePoints(float x1, float y1, float x2, float y2)
	{
		line.A.x = x1;
		line.A.y = y1;
	
		line.B.x = x2;
		line.B.y = y2;

		line.magnitude = sqrt( line.B.x*line.B.x + line.B.y*line.B.y);
	}

	// search this entity's collidingEntities vector to see if it is currently colliding
	// this physics-frame with entity with id id
	bool checkCollision(int id)
	{
		for (unsigned int i = 0; i < collidingEntities.size(); i++ )
		{
			if ( collidingEntities[i] == id )
				return true;
		}

		return false;
	}
};

#endif