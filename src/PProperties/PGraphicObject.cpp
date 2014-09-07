#include "properties\\PGraphicObject.h"
#include "game\\game.h"

// timer task func for fading a graphic object
bool _PGOBJ_doFadeIn(void * ptr)
{
	PGraphicObject * gobj = (PGraphicObject*)ptr;
	
	gobj->alpha += gobj->alinc;
	
	if( gobj->alpha >= gobj->endAlpha)
		gobj->alpha = gobj->endAlpha;

	gobj->setColorFilterRawLua(gobj->alpha, 255, 255, 255);

	if( gobj->alpha >= gobj->endAlpha )
	{
		return true;
	}
	return false;
}

// timer task func for fading a graphic objec
bool _PGOBJ_doFadeOut(void * ptr)
{
	PGraphicObject * gobj = (PGraphicObject*)ptr;
	
	gobj->alpha += gobj->alinc;
	
	if( gobj->alpha <= gobj->endAlpha)
		gobj->alpha = gobj->endAlpha;

	gobj->setColorFilterRawLua(gobj->alpha, 255, 255, 255);

	if( gobj->alpha <= gobj->endAlpha )
	{
		return true;
	}
	return false;
}

// timer task func for scaling a graphic object
bool _PGOBJ_doScaling(void * ptr)
{
	PGraphicObject * gobj = (PGraphicObject*)ptr;
	PPosition * pos = gobj->getPosition();

	pos->scale += gobj->scinc;

	if( gobj->scinc > 0 && pos->scale >= gobj->endScale )
	{
		pos->scale = gobj->endScale;
		return true;
	}
	else if( gobj->scinc < 0 && pos->scale <= gobj->endScale )
	{
		pos->scale = gobj->endScale;
		return true;
	}

	return false;
}

void PGraphicObject::fadeIn(int alpha, int endAlpha, int increment, float duration)
{	
	if( alpha < 0 || alpha > 255 )
	{
		Log( consoleNS::WARNING, "Image alpha out of range: " + std::to_string( alpha ) );
		return;
	}

	this->alpha = alpha;
	this->endAlpha = endAlpha;
	this->alinc = increment;

	_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerTaskOnNotify(entity_id, duration,&(_PGOBJ_doFadeIn), this);
}

void PGraphicObject::fadeOut(int alpha, int endAlpha, int increment, float duration)
{
	if( alpha < 0 || alpha > 255 )
	{
		Log( consoleNS::WARNING, "Image alpha out of range: " + std::to_string( alpha ) );
		return;
	}

	this->alpha = alpha;
	this->endAlpha = endAlpha;
	this->alinc = increment;
	
	_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerTaskOnNotify(entity_id, duration,&(_PGOBJ_doFadeOut), this);
}

void PGraphicObject::scaleOverTime(float scale, float endScale, float increment, float duration)
{
	if( position == nullptr )
	{
		Log( consoleNS::WARNING, "Attempted to scale an image with no PPosition." );
		return;
	}

	this->position->scale = scale;
	this->endScale = endScale;
	this->scinc = increment;
	
	_Game->getWorldManager()->getCurrentWorld()->getEntityManager()->registerTaskOnNotify(entity_id, duration,&(_PGOBJ_doScaling), this);
}

void PGraphicObject::setColorFilterLua(std::string color)
{
	if( color == "none" )
	{
		colorFilter = graphicsNS::WHITE;
	}
	else if(color == "red")
	{
		colorFilter = graphicsNS::RED;
	}
	else if(color == "green")
	{
		colorFilter = graphicsNS::GREEN;
	}
	else if( color == "blue")
	{
		colorFilter = graphicsNS::BLUE;
	}
	else if( color == "purple" )
	{
		colorFilter = graphicsNS::PURPLE;
	}
	else if( color == "yellow" )
	{
		colorFilter = graphicsNS::YELLOW;
	}
	else if( color == "neon" )
	{
		colorFilter = graphicsNS::LIME;
	}
	else if( color == "black" )
	{
		colorFilter = graphicsNS::BLACK;
	}
	else if( color == "white" )
	{
		colorFilter = graphicsNS::WHITE;
	}
	else if( color == "alpha50" )
	{
		colorFilter = graphicsNS::ALPHA50;
	}

}