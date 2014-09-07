#ifndef _PFRAME_H
#define _PFRAME_H

#include "properties\\PProperty.h"
#include "properties\\PGraphicObject.h"
#include "properties\\PImage.h"
#include <vector>

class Game;

namespace pframeNS
{

};

struct Partition
{
	float x;
	float y;
	float x_offset_left;
	float x_offset_right;
	float y_offset_top;
	float y_offset_bot;
	int width;
	int height;
	std::vector<PGraphicObject*> widgets;
};

class PFrame : public PProperty
{

protected:
	PGraphicObject * image;
	bool focus, active, initialized;
	std::vector<PGraphicObject*> widgets;

	std::string flag;		// flag for positioning elements
	unsigned int maxWidgets;

	float lastFrameX;		// x position last frame - used for repositioning widgets
	float lastFrameY;		// y position last frame - used for repositioning widgets

	float offset_x;
	float offset_y;

	RECT scrollRect;

	void spreadElements(std::string flag);
	void spreadWidgets(std::string flag, unsigned int part_id);
	Partition* getPartition(unsigned int part_id);

public:
	PFrame() : PProperty()
	{
		maxWidgets = UINT_MAX;
		offset_x = 0;
		offset_y = 0;
		initialized = false;
		focus = false;
		active = true;
		scrollable = false;
		image = NULL;
	}
	~PFrame(){};
	std::vector<Partition> parts;
	bool scrollable;

	void initialize_allOffsets(PGraphicObject * image, 
								float x_offset_left,
								float x_offset_right,
								float y_offset_top,
								float y_offset_bot);

	void initialize(PGraphicObject* image, float x_offset, float y_offset);

	void initializeNoPart(PGraphicObject* image);

	PGraphicObject * getImage() const {return image;}
	
	bool isFocus() const {return focus;}
	void setFocus(bool focus){this->focus = focus;}

	bool isActive() const {return active;}
	void setActive(bool active){this->active = active;}

	bool isInitialized() const {return initialized;}

	std::vector<PGraphicObject*> getWidgets() const { return widgets; }

	void setFlag(std::string flag){ this->flag = flag; }

	unsigned int getMaxWidgets() const { return this->maxWidgets; }

	void setMaxWidgets(unsigned int max){ this->maxWidgets = max; }

	void setScrollRect(LONG left, LONG top, LONG right, LONG bot);

	RECT getScrollRect() const { return scrollRect; }
	
	void addWidget(PGraphicObject * widget, std::string flag);

	void addWidgetIndex(PGraphicObject * widget, std::string flag, unsigned int part_id, unsigned int index);

	void addWidgetAbsolute(PGraphicObject * widget, float x, float y);

	void addWidgetPart(PGraphicObject * widget, std::string flag, unsigned int part_id);

	void addWidgetAbsolutePart(PGraphicObject * widget, float x, float y, unsigned int part_id);

	void addWidgetNoPosition(PGraphicObject * widget, unsigned int part_id);

	void removeWidget(unsigned int id, unsigned int part_id);

	// vertically/horizontally split partition whose id is part_id into two partitions
	void makePartition(float x,
					   float y,
					   int width,
					   int height);

	void reposWidgets();	// reposition widgets

	virtual void onDelete(unsigned int id);
};

#endif