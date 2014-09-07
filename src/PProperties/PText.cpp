#include "properties\\PText.h"
#include "game\\game.h"

void PText::onLostDevice() 
{
	textDX.onLostDevice(); 
}

void PText::onResetDevice()
{
	textDX.onResetDevice();
	//initialize(g, position, fontWidth, fontType, bold, italic, text);
}

bool PText::initialize(Graphics * graphics, PPosition * pos, int font_width, std::string fontType, bool bold, bool italic, std::string text)
{
	if( graphics == nullptr )
	{
		Log(consoleNS::WARNING, "Attempted to initialize PText with NULL Graphics.");
		return false;
	}
	if( pos == nullptr )
	{
		Log(consoleNS::WARNING, "Attempted to initialize PText with NULL PPosition.");
		return false;
	}

	this->g = graphics;
	this->text = text;
	this->position = pos;

	this->fontType = fontType;
	this->fontWidth = font_width;
	this->fontHeight = fontWidth*2;
	this->bold = bold;
	this->italic = italic;

	if( !textDX.initialize(graphics, (int)(fontHeight*pos->scale), bold, italic, fontType, (int)(fontWidth*pos->scale)) )
	{
		Log(consoleNS::ERR, "Failed to initialize DX text.");
		return false;
	}
	textDX.setFontColor(graphicsNS::WHITE);

	RECT rect = textDX.getTextRect( this->text );

	this->position->width = rect.right;
	this->position->height = rect.bottom;
	
	initialized = true;
	return true;
}

void PText::draw( Graphics * g )
{
	if( initialized && visible )
		 textDX.print(text, position, (float)fontHeight, colorFilter); 
}

void PText::updatePosition()
{
	if( initialized )
	{
		textDX.setFontRect(  (LONG)(position->y + position->height + 2),
							 (LONG)position->x, 
							 (LONG)(position->x + position->width + 2), 
							 (LONG)position->y );
	}
}

void PText::wrapText( int width )
{	
	if( initialized )
	{
		if( fontWidth*(int)text.size() < width )
			return;	 // no need to wrap; text string is shorter than width
	
		size_t begin = 0;
		size_t last = 0;
		int numNewlineChars = 0;
		std::string line = "";

		int totalLines = (int)(text.size()*fontWidth/width);

		while( ( (line.size() - numNewlineChars) + width/fontWidth) < text.size() )
		{
			begin = last;
			last = ( last + (size_t)(width/fontWidth) > text.size() ) ? text.size() : last + (size_t)(width/fontWidth);

			while( last > 0 && text.at( last-1 ) != ' ' )
			{
				last--;
			}

			size_t numchars = last - begin;

			line.append( text.substr( begin, numchars ) );
			line.push_back('\n');
			numNewlineChars++;
		}	

		if( line.size() < text.size() )
			line.append( this->text.substr( line.size() - numNewlineChars, text.size() ) );

		RECT rect = textDX.getTextRect( this->text );

		this->position->height = fontHeight*(numNewlineChars) + rect.bottom;
		this->position->width = width;				
		text = line;
	}
}

void PText::setText(std::string text) 
{
	if( initialized )
	{
		this->text = text; 

		// if '\n' char not found, text width has not been calculated.
		if( text.find('\n') == std::string::npos )
		{
			RECT rect = textDX.getTextRect( this->text );

			this->position->width = rect.right;
			this->position->height = rect.bottom;
		}
	}
}

void PText::setWidth(int width)
{
	if( initialized )
		this->position->width = width;
}

void PText::setHeight(int height)
{
	if( initialized )
		this->position->height = height;
}

void PText::setFontColor(std::string color)
{
	if( color == "black" )
	{
		textDX.setFontColor( graphicsNS::BLACK );
	}
	else if( color == "red" )
	{
		textDX.setFontColor( graphicsNS::RED );
	}
	else if( color == "green" )
	{
		textDX.setFontColor( graphicsNS::GREEN );
	}
	else if( color == "blue" )
	{
		textDX.setFontColor( graphicsNS::BLUE );
	}
	else if( color == "white" )
	{
		textDX.setFontColor( graphicsNS::WHITE );
	}
	else if( color == "neon" )
	{
		textDX.setFontColor( graphicsNS::LIME );
	}
}

void PText::remakeText(Graphics * g, std::string fontType, int font_width, bool bold, bool italic)
{
	
	if( !textDX.initialize(g, (int)(font_width*2*position->scale), bold, italic, fontType, (int)(font_width*position->scale)) )
	{
		if( !FAILED(_Game->getGraphics()->getDeviceState()) )
			Log(consoleNS::ERR, "Failed to remake text.");
		initialized = false;
		return;
	}
	textDX.setFontColor(graphicsNS::WHITE);

	/*
	RECT rect = textDX.getTextRect( this->text );

	this->position->width = rect.right;
	this->position->height = rect.bottom;
	*/
}