#ifndef _PMOBILE_H
#define _PMOBILE_H

#include "graphics\\graphics.h"
#include "properties\\pproperty.h"

class PMobile : public PProperty
{

public:

	PMobile() : destination(VECTOR2(0,0)),
				velocity(VECTOR2(0,0)),
				direction(VECTOR2(1,1)),
				acceleration(VECTOR2(0,0)),
				deceleration(VECTOR2(0,0)), 
				limitVelocityX(FLT_MAX),
				limitVelocityY(FLT_MAX),
				greaterThanX(FLT_MAX),									
				greaterThanY(FLT_MAX),
				lessThanX(-999999.0f),
				lessThanY(-999999.0f),
				greaterThanWorldX(FLT_MAX),								
				greaterThanWorldY(FLT_MAX),
				lessThanWorldX(-999999.0f),
				lessThanWorldY(-999999.0f),
				angle(0.0f),
				radius(0.0f),
				onAngle(FLT_MAX),
				ignorePause(false){}

	VECTOR2 destination;
	VECTOR2 velocity;
	VECTOR2 direction;
	VECTOR2 acceleration;
	VECTOR2 deceleration;

	float mass;

	/* 
	   angle - for parabolic/circular motion. 
	   angle is the angle that the direction vector makes with the x axis.
	   angle is measured in radians.
	*/
	float angle;
	/**/
	float onAngle;
	
	/* radius for centripetal acceleration */
	float radius;

	/* limits for velocity by dimension */
	float limitVelocityX;
	float limitVelocityY;

	/* position flags */
	float greaterThanX;									
	float greaterThanY;
	float lessThanX;
	float lessThanY;
	float greaterThanWorldX;								
	float greaterThanWorldY;
	float lessThanWorldX;
	float lessThanWorldY;

	/* when true, SMovement ignores pause for this pmobile (i.e. moves it along its velocity vector) */
	bool ignorePause;

	//lua accessibility functions
	float getVelocityX() const {return velocity.x;}
	float getVelocityY() const {return velocity.y;}

	void setVelocityX(float x){velocity.x = x;}
	void setVelocityY(float y){velocity.y = y;}

	// setSpeed() is equivalent to setting the magnitude of the direction vector
	void setSpeed(float speed){ velocity.x = speed; velocity.y = speed; }
	void setAccel(float accel){ acceleration.x = accel; acceleration.y = accel; }
	void setLimVel(float lim){ limitVelocityX = lim; limitVelocityY = lim; }

	float getDirectionX() const {return direction.x;}
	float getDirectionY() const {return direction.y;}

	void setDirectionX(float x){direction.x = x;}
	void setDirectionY(float y){direction.y = y;}

	float getAccelerationX() const {return acceleration.x;}
	float getAccelerationY() const {return acceleration.y;}
	float getDecelerationY() const {return deceleration.y;}

	void setAccelerationX(float x){acceleration.x = x;}
	void setAccelerationY(float y){acceleration.y = y;}
	void setDecelerationY(float y){deceleration.y = y;}
};

#endif