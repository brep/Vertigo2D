#include "properties\\PImage.h"
#include "graphics\\textureManager.h"
#include "game\\game.h"
#include "game\\constants.h"

//=============================================================================
// default constructor
//=============================================================================
PImage::PImage() : PGraphicObject()
{
	spriteData.width = 2;
	spriteData.height = 2;
	spriteData.x = 0.0;
	spriteData.y = 0.0;
	spriteData.prevX = 0.0;
	spriteData.prevY = 0.0;
	spriteData.scale = 1.0;
	spriteData.angle = 0.0;
	spriteData.rect.left = 0;							// used to select one frame from multi-frame PImage
	spriteData.rect.top = 0;
	spriteData.rect.right = spriteData.width;
	spriteData.rect.bottom = spriteData.height;
	position = NULL;
	spriteData.texture = NULL;							// the sprite texture
	spriteData.flipHorizontal = false;
	spriteData.flipVertical = false;
	baseScale = 1.0f;
    initialized = false;								// set true when successfully initialized
    spriteData.flipHorizontal = false;
    spriteData.flipVertical = false;
	textureManager = nullptr;
    cols = 1;
	margin = 0;
	spacing = 0;
    startFrame = 0;
    endFrame = 0;
    currentFrame = 0;
    frameDelay = 1.0;									// arbitrarily default to 1 second per frame of animation
    animTimer = 0.0;
    loop = false;										// flag to repeat animation
    animComplete = true;								
    colorFilter = graphicsNS::WHITE;					// WHITE for no change
	doAnimation = false;
	tiled = false;
}

//=============================================================================
// destructor
//=============================================================================
PImage::~PImage()
{}

//=============================================================================
// Initialize the PImage.
// Post: returns true if successful, false if failed
// pointer to Graphics
// width of PImage in pixels  (0 = use full texture width)
// height of PImage in pixels (0 = use full texture height)
// number of columns in texture (1 to n) (0 same as 1)
// pointer to TextureManager
//=============================================================================
bool PImage::initialize(PPosition * pos, int width, int height, int ncols, TextureManager * textureM, bool tiled)
{
	this->position = pos;

	if( textureM == nullptr )
	{
		_Game->getConsole()->error("Attempted to initialize PImage object with a NULL texture manager.");
		initialized = false;
		return false;
	}

	this->tiled = tiled;
    try{
        textureManager = textureM;
        spriteData.texture = textureManager->getTexture();
        
		if(width == 0)
            width = textureManager->getWidth();     // u5 full width of texture
        
		spriteData.width = width;		

        if(height == 0)
            height = textureManager->getHeight();   // use full height of texture
        
		spriteData.height = height;

		cols = ncols;

		if(tiled){
			if (cols == 0)
				cols = 1;                               // if 0 cols use 1
		
			// configure spriteData.rect to draw currentFrame
			spriteData.rect.left = (currentFrame % cols) * (spriteData.width);
			// right edge + 1
			spriteData.rect.right = spriteData.rect.left + spriteData.width;
			spriteData.rect.top = (currentFrame / cols) * spriteData.height;
			// bottom edge + 1
			spriteData.rect.bottom = spriteData.rect.top + (spriteData.height);

			initialized = true;
		}
		else
		{
			cols = 0;
			initialized = true;
			setRect();
		}
    }
    catch(...) 
	{ 
		_Game->getConsole()->error("Failed to initialize PImage.");
		initialized = false;
	}
   
	if(position != nullptr)
	{
		if( pos->entity_id != this->entity_id )
			_Game->getConsole()->warning("PImage::initialize() was passed a PPosition with a different ID.");

		spriteData.x = position->x;
		spriteData.y = position->y;
		spriteData.prevX = spriteData.x;
		spriteData.prevY = spriteData.y;
		spriteData.scale = position->scale;
		position->height = spriteData.height;
		position->width = spriteData.width;
	}

	return initialized;
}

void PImage::draw(Graphics * g)
{
    if( initialized && visible )
	{
		// set texture to draw
		if( textureManager->isInitialized() )
			spriteData.texture = textureManager->getTexture();
		else
			spriteData.texture = NULL;
		// draw heemeth
		g->drawSprite(spriteData, colorFilter);        
	}
}

//=============================================================================
// frameTime is used to regulate the speed of movement and animation
//=============================================================================
void PImage::animate(float frameTime)
{
	if( animComplete )
		return;

	if( endFrame - startFrame > 0 )          // if animated sprite
	{
		animTimer += frameTime;
		if( animTimer > frameDelay )
		{
			animTimer = 0.0f;
			setCurrentFrame(currentFrame + 1);

			if( 
				( tiled && (currentFrame < startFrame || currentFrame >= endFrame) )
				|| 
				( !tiled && (currentFrame > endFrame) ) 
			  )
			{
				if( loop )					 // if looping animation
					setCurrentFrame( startFrame );
				else                         // not looping animation
				{
					std::string name = getCurrentAnimation();
					auto i = name.find('_');
					auto ext = name.substr(i, name.length());
					setCurrentAnimation("default" + ext);
					animComplete = true;    // animation complete
				}
				// notify scripts of animation complete event
				LuaDoa::fireEvent( ON_ANIM_COMPLETE, this->entity_id );
			}

			setRect();
		}
	}
}

void PImage::updatePosition()
{
	if( initialized && position != nullptr )
	{
		spriteData.prevX = position->prevX;
		spriteData.prevY = position->prevY;
		spriteData.x = position->x;
		spriteData.y = position->y;
		spriteData.width = position->width;
		spriteData.height = position->height;
		spriteData.scale = position->scale;
	}
}

//=============================================================================
// Set the current frame of the PImage
//=============================================================================
void PImage::setCurrentFrame(unsigned int c) 
{
    currentFrame = c;
    setRect();                          // set spriteData.rect
}

//=============================================================================
//  Set spriteData.rect to draw currentFrame
//  sprite refers to individual sprite within a sprite sheet
//  if the width of each sprite is equal, then width
//  of each sprite is (textureManager->width)%cols
//=============================================================================
void PImage::setRect() 
{
	if( initialized )
	{
		if( tiled )
		{
				// configure spriteData.rect to draw currentFrame
			spriteData.rect.left = (currentFrame % cols) * (spriteData.width + spacing) + margin;
			// right edge + 1
			spriteData.rect.right = spriteData.rect.left + spriteData.width - margin - spacing;
			spriteData.rect.top = (currentFrame / cols) * (spriteData.height + spacing) + margin;
			// bottom edge + 1
			spriteData.rect.bottom = spriteData.rect.top + spriteData.height - margin - spacing;
		}
		else if( currentFrame >= startFrame && currentFrame <= this->getEndFrame() )
		{
			spriteInfo spt = textureManager->getSpritesheetInfo(currentFrame);
			spriteData.rect.left = spt.x;
			spriteData.rect.right = spt.x + spt.width;
			spriteData.rect.top = spt.y;
			spriteData.rect.bottom = spt.y + spt.height;
			spriteData.width = spt.width;
			spriteData.height = spt.height;
		}

		if( position != nullptr )
		{
			position->height = spriteData.height;
			position->width = spriteData.width;
		}
	}
}

void PImage::attachAnimation(std::string name, 
							int startFrame, 
							int endFrame, 
							float delay, 
							bool loop, 
							float rotation, 
							bool flipHoriz, 
							bool flipVert)
{
	animationInfo info;
	info.name = name;
	info.startFrame = startFrame;
	info.endFrame = endFrame;
	info.loop = loop;
	info.delay = delay;
	info.flipHoriz = flipHoriz;
	info.flipVert = flipVert;
	info.rotation = rotation;
	animations.insert(std::pair<std::string, animationInfo>(name, info));

	auto i = name.find('_');
	auto pre = name.substr(0, i);

	if(pre == "default")
		setCurrentAnimation( name );
}

bool PImage::hasAnimation(std::string name)
{
	if(animations.find(name) == animations.end())
		return false;
	
	return true;
}

int PImage::getDefaultStartFrame()
{
	auto it = animations.find("default");

	if(it != animations.end())
		return it->second.startFrame;

	return startFrame;
}

int PImage::getDefaultEndFrame()
{
	auto it = animations.find("default");

	if(it != animations.end())
		return it->second.endFrame;

	return endFrame;
}

std::string PImage::getCurrentAnimationPostfix()
{
	if(currentAnimation.empty())
		return "";
	
	auto i = currentAnimation.find("_");
	if(i != currentAnimation.size())
		return currentAnimation.substr(i+1, currentAnimation.length());

	return "";
}

void PImage::setCurrentAnimation(std::string key)
{
	if( currentAnimation == key )
	{
		// this animation is already set; reset animComplete and currentFrame to replay current animation
		animComplete = false;
		currentFrame = startFrame;
		return;
	}

	auto i = key.find("_");
	auto it = animations.find(key);

	// copy data from ca
	startFrame = it->second.startFrame;
	currentFrame = startFrame;
	endFrame = it->second.endFrame;
	currentAnimation = it->second.name;
	loop = it->second.loop;
	frameDelay = it->second.delay;
	flipHorizontal(it->second.flipHoriz);
	flipVertical(it->second.flipVert);

	animComplete = false;

	// set image rect
	setRect();
}