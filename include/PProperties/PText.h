#ifndef _PTEXT_H
#define _PTEXT_H

#include "graphics\\textDX.h"
#include "properties\\PGraphicObject.h"

class PText : public PGraphicObject
{
private:
	bool initialized;

	TextDX textDX;
	std::string text;

	Graphics * g;                                                 

	std::string fontType;

	bool bold;
	bool italic;

	int fontWidth;
	int fontHeight;

public:
	PText() : PGraphicObject() {bold = false; italic = false; initialized = false;}
	~PText() { g = nullptr; position = nullptr; textDX.onLostDevice(); }

	bool initialize(Graphics * graphics, PPosition * pos, int font_width, std::string fontType, bool bold, bool italic, std::string text);
	
	void draw( Graphics * g );

	void updatePosition();

	std::string getText() const { return text; }
 	int getFontWidth() const { return fontWidth; }
	int getFontHeight() const { return fontHeight; }

	void setFontWidth(int fontWidth)
	{
		this->fontWidth = fontWidth;
		this->fontHeight = fontWidth*2;
	}

	void wrapText(int width);

	void setText(std::string text);

	void setWidth(int width);

	void setHeight(int height);

	void setFontColor(std::string color);

	void print() { textDX.print(text, position, (float)fontHeight, colorFilter); }
	
	void remakeText(Graphics * g, std::string fontType, int font_width, bool bold, bool italic);

	void onLostDevice();

	void onResetDevice();
};

#endif