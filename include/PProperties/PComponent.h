#ifndef _PCOMPONENT_H
#define _PCOMPONENT_H

#include "properties\\PImage.h"
#include "properties\\PPosition.h"
#include "properties\\PText.h"
#include "graphics\\graphics.h"

namespace pcomponentNS
{

};

class PComponent : public PProperty
{
private:
	unsigned int hostID;
	unsigned int componentID;

	bool shareColorFilter;
	bool shareScale;
	bool shareX;
	bool shareY;
	bool shareVisible;

	float offsetX;
	float offsetY;

	std::string flags;

public:
	PComponent() : shareScale(false), shareColorFilter(false), shareX(true), shareY(true), offsetX(0), offsetY(0), shareVisible(true){};

	void initialize(unsigned int host_id, unsigned int component_id, std::string flag, float offset_x = 0.0f, float offset_y = 0.0f);

	void updatePosition(PPosition * host_pos, PPosition * comp_pos);

	void updatePositionX(PPosition * host_pos, PPosition * comp_pos);

	void updatePositionY(PPosition * host_pos, PPosition * comp_pos);

	unsigned int getHostId() const { return hostID; }

	unsigned int getComponentId() const { return componentID; }

	bool getShareX() const { return shareX; }

	bool getShareY() const { return shareY; }
	
	bool getShareColor() const { return shareColorFilter; }
	
	bool getShareScale() const { return shareScale; }
	
	float getOffsetX() const { return offsetX; }

	float getOffsetY() const { return offsetY; }

	bool getShareVisible() const { return shareVisible; }

	void setShareVisible(bool shareVis) { shareVisible = shareVis; }

	void setHostId(unsigned int host_id) { this->hostID = host_id; }

	void setComponentId(unsigned int component_id) { this->componentID = component_id; }

	void setShareX(bool shareX) { this->shareX = shareX; }

	void setShareY(bool shareY) { this->shareY = shareY; }

	void setOffsetX(float offset_x) { this->offsetX = offset_x; }

	void setOffsetY(float offset_y) { this->offsetY = offset_y; }

	void setShareColor(bool shareColor) { this->shareColorFilter = shareColor; }

	void setShareScale(bool shareScale) { this->shareScale = shareScale; }

	std::string getFlags() const { return flags; }

	void setFlags(std::string val) { this->flags = val;}
};

#endif