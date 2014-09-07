#ifndef _Image_H             
#define _Image_H                
#define WIN32_LEAN_AND_MEAN

#include "properties\\PGraphicObject.h"
#include "graphics\\textureManager.h"
#include "game\\constants.h"
#include <map>

// animation information associated with the image object
struct animationInfo
{
	std::string name;
	int startFrame, endFrame;
	float delay, rotation;
	bool loop, flipHoriz, flipVert;
};

class PImage : public PGraphicObject
{
  // Image properties
  protected:
    TextureManager * textureManager; // pointer to texture manager
   
	// spriteData contains the data required to draw the Image by Graphics::drawSprite()
	SpriteData spriteData; 
	float baseScale;	
	int		margin;			// for sprites with borders
	int		spacing;		// space in between tiles
    int     cols;           // number of cols (1 to n) in multi-frame sprite with no associated texture data file
    int     startFrame;     // first frame of current animation
    int     endFrame;       // end frame of current animation
    int     currentFrame;   // current frame of animation
	std::string currentAnimation;
	std::string currentAnimationPostfix;
    float   frameDelay;     // how long between frames of a specific animation
    float   animTimer;      // animation timer
    HRESULT hr;             // standard return type
    bool    loop;           // true to loop frames
    bool    initialized;    // true when successfully initialized
    bool    animComplete;   // true when loop is false and endFrame has finished displaying
	bool	doAnimation;
	bool	tiled;

	std::map<std::string, animationInfo> animations;

  public:
    // Constructor
    PImage();	

    // Destructor
    virtual ~PImage();

	// Initialize PImage
    // Pre: *g = pointer to Graphics object
    //      width = width of Image in pixels  (0 = use full texture width)
    //      height = height of Image in pixels (0 = use full texture height)
    //      ncols = number of columns in texture (1 to n) (0 same as 1)
    //      *textureM = pointer to TextureManager object
    virtual bool initialize(PPosition * pos, int width, int height, 
                                    int ncols, TextureManager * textureM, bool tiled);

	void draw(Graphics * g);

	void animate(float frameTime);

	void updatePosition();

    ////////////////////////////////////////
    //           Get functions            //
    ////////////////////////////////////////

	TextureManager* getTextureManager(){return textureManager;}

    // Return reference to SpriteData structure.
    //virtual SpriteData& getSpriteInfo() {return spriteData;}

	float getAnimTimer() const {return animTimer;}
	void setAnimTimer(float anim){animTimer = anim;}

	bool getLoop() const {return loop;}

	bool isAnimationOn() const {return doAnimation;}
	void setAnimationOn(bool anim){this->doAnimation = anim;}

	bool isInitialized() const { return initialized; }

	// Return reference to SpriteData structure.
    virtual SpriteData& getSpriteInfo() {return spriteData;}

	//virtual PPosition * getPosition() { return position; }

	// Return X position.
    virtual float getSpriteX()  const      {return spriteData.x;}

    // Return Y position.
    virtual float getSpriteY()   const     {return spriteData.y;}

	// Return previous X position.
    virtual float getSpritePrevX()  const      {return spriteData.prevX;}

    // Return previous Y position.
    virtual float getSpritePrevY()   const     {return spriteData.prevY;}

    // Return width.
    virtual int   getSpriteWidth()  const  {return spriteData.width;}

    // Return height.
    virtual int   getSpriteHeight() const  {return spriteData.height;}

    // Return rotation angle in degrees.
    virtual float getDegrees()  const    {return spriteData.angle*(180.0f/(float)PI);}

	virtual float getBaseScale()	const {return baseScale;}

    // Return rotation angle in radians.
    virtual float getRadians()  const    {return spriteData.angle;}

	 // Return RECT structure of Image.
    virtual RECT  getSpriteDataRect() const {return spriteData.rect;}

	////////////////////////////////////////
    //           Setters                  //
    ////////////////////////////////////////

	virtual void setBaseScale(float scale)	{this->baseScale = scale;}

	// set sprite x position (no lerp)
    virtual void setSpriteX_NL(float newX)
	{
		spriteData.prevX = newX; 
		spriteData.x = newX;
	}

	// set sprite y position (no lerp)
    virtual void setSpriteY_NL(float newY)
	{
		spriteData.prevY = newY; 
		spriteData.y = newY;
	}

	// set sprite x position
    virtual void setSpriteX(float newX){ spriteData.x = newX; }

	// set sprite y position
    virtual void setSpriteY(float newY){ spriteData.y = newY; }

	virtual void setSpritePrevX(float prevX) { spriteData.prevX = prevX; }

	virtual void setSpritePrevY(float prevY) { spriteData.prevY = prevY; }

    virtual void setSpriteScale(float s) {spriteData.scale = s;}

    // set rotation angle in degrees.
    // 0 degrees is up. angles progress clockwise not cc!
    virtual void setDegrees(float deg) {spriteData.angle = deg*((float)PI/180.0f);}

    // set rotation angle in radians.
    // 0 radians is up. angles progress clockwise not cc!
    virtual void setRadians(float rad)  {spriteData.angle = rad;}
	
	// set spriteData.rect to r.
    virtual void setSpriteDataRect(RECT r)  {spriteData.rect = r;}

	virtual void setSpriteWidth(int width) {this->spriteData.width = width;}

	virtual void setSpriteHeight(int height) {this->spriteData.height = height;}

	void setSpriteRotPointX( int rotPointX ) { this->spriteData.rotPointX = rotPointX; }
	
	void setSpriteRotPointY( int rotPointY ) { this->spriteData.rotPointY = rotPointY; }

	virtual bool isTiled() const {return tiled;}

    // return delay between frames of animation.
    virtual float getFrameDelay() const  {return frameDelay;}

    // return number of starting frame.
    virtual int   getStartFrame() const  {return startFrame;}

    // return number of ending frame.
    virtual int   getEndFrame()   const  {return endFrame;}

    // return number of current frame.
    virtual int getCurrentFrame() const {return currentFrame;}

	virtual int getMargin() const {return margin;}

    virtual bool  getAnimationComplete() const {return animComplete;}

	virtual void setStartFrame(int s){startFrame = s;}

	virtual void setEndFrame(int e){endFrame = e;}

    // set delay between frames of animation.
    virtual void setFrameDelay(float d) {frameDelay = d;}

    // set starting and ending frames of animation.
    virtual void setFrames(int s, int e){startFrame = s; endFrame = e;}

    // set current frame of animation.
    virtual void setCurrentFrame(unsigned int c);

	virtual void setMargin(int margin) { this->margin = margin; }

    // set spriteData.rect to draw currentFrame
    virtual void setRect(); 

    // set animation loop. lp = true to loop.
    virtual void setLoop(bool lp) {loop = lp;}

	virtual void setSpacing(int spacing) {this->spacing = spacing;}
	
	virtual int getSpacing() { return spacing; }

    virtual void setAnimationComplete(bool a) {animComplete = a;};

    virtual void flipHorizontal(bool flip)  {spriteData.flipHorizontal = flip;}

    virtual void flipVertical(bool flip)    {spriteData.flipVertical = flip;}

	bool getFlipHorizontal() const { return spriteData.flipHorizontal; }

	bool getFlipVertical() const { return spriteData.flipVertical; }

    virtual void setTextureManager(TextureManager * textureM)
    { textureManager = textureM; }

	virtual void attachAnimation(std::string name, int startFrame, int endFrame, float delay, bool loop, float rotation, bool flipHorizontal, bool flipVertical);

	virtual bool hasAnimation(std::string name);

	virtual int getDefaultStartFrame();

	virtual int getDefaultEndFrame();

	virtual std::string getCurrentAnimation() const { return currentAnimation; }

	virtual std::string getCurrentAnimationPostfix();

	virtual void setCurrentAnimation(std::string key);
};

#endif