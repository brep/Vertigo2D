#include "systems\\SRender.h"
#include "game\\game.h"
#include <vector>
#include <algorithm>

void SRender::startUp( EntityManager * entM ){}

void SRender::update( Map * world, float frameTime )
{
	EntityManager * entM = world->getEntityManager();
	Graphics * g = _Game->getGraphics();

	auto list = entM->getGobjsByLayer();
	size_t i = 0;

	renderParallaxLayers( g, world );

	g->setupCamera();

	g->spriteBegin();
		i = renderImages( g, entM, list, -1, i, frameTime );
		renderTiles( g, world );
		renderImages( g, entM, list, SRenderNS::UI_LAYER, i, frameTime );
	g->spriteEnd();

	renderLights( g, entM, list, frameTime );
	renderGui( g, entM, list, frameTime );
}

size_t SRender::renderImages( Graphics * g, EntityManager * entM, std::vector<PGraphicObject*> list, int stopLayer, size_t i, float frameTime )
{
	while ( i < list.size() )
	{
		PGraphicObject* img = list[i];

		if( img != nullptr )
		{
			if( img->getLayer() > stopLayer )	
			{
				if(stopLayer >= SRenderNS::UI_LAYER && UI_Index == -1)
					UI_Index = i;

				break;
			}
			if( img->getLayer() == SRenderNS::LIGHT_LAYER )	
			{	
				if(!lightsExist)
				{
					lightsExist = true;
					lightBeginIndex = i;	// i is the index of the first light
				}
				i++;
				continue;	
			}
			if( img->getLayer() < SRenderNS::UI_LAYER )
			{	
				draw( img, g, frameTime );
			}
		}
		i++;
	}

	return i;
}

void SRender::renderLights(Graphics * g, EntityManager * entM, std::vector<PGraphicObject*> list, float frameTime)
{
	if(lightsExist)
	{
		std::vector<SpriteData> lights;
		for(unsigned int i = lightBeginIndex; i < list.size() && list[i]->getLayer() == 100; i++)
		{
			PImage * img = (PImage*)list[i];
			img->updatePosition();
			//set spriteData texture to this image's textureManager's texture
			img->getSpriteInfo().texture = img->getTextureManager()->getTexture();	
			lights.push_back(img->getSpriteInfo());	
		}

		lightBeginIndex = -1;

		auto cams = entM->getProperties<PCamera>();
		PCamera * cam = cams.empty() ? nullptr : cams[0];
		g->doLightRendering(lights);
		lightsExist = false;
	}
}

void SRender::renderGui(Graphics * g, EntityManager * entM, std::vector<PGraphicObject*> list, float frameTime)
{
	// setup camera with no zoom
	g->setupCamera(true);

	g->spriteBegin();

	if( UI_Index > 0 )
	{
		for( ; (size_t)UI_Index < list.size(); UI_Index++ )
		{
			PGraphicObject* img = list[UI_Index];
			
			if( img != nullptr )
			{
				draw( img, g, frameTime );

				// check if this entity has a frame
				PFrame * frame = entM->getProperty<PFrame>(img->entity_id);
				if( frame != nullptr )
				{
					// if scrollable frame, set scissor rect
					if( frame->scrollable )
					{
						// get scroll rect region, translate from world to screen coords
						RECT srct = frame->getScrollRect();
						D3DXVECTOR3 coords, wh_coords;

						coords.x = (float)srct.left;
						coords.y = (float)srct.top;

						g->worldToScreen(coords);

						wh_coords.x = (float)(srct.left + srct.right);
						wh_coords.y = (float)(srct.top + srct.bottom);

						g->worldToScreen(wh_coords);

						g->spriteEnd();
						g->setScissorRect(  (LONG)coords.x,
											(LONG)coords.y, 
											(LONG)wh_coords.x, 
											(LONG)wh_coords.y );
						g->spriteBegin();
					}

					// draw this frame's widgets
					for( auto jt = frame->parts.begin(); jt != frame->parts.end(); jt++ )
					{
						for( size_t k = 0; k < jt->widgets.size(); k++ )
						{
							// draw the widgets
							draw( jt->widgets[k], g, frameTime );

							PComponent * comp = entM->getProperty<PComponent>(jt->widgets[k]->entity_id);
							if( comp != nullptr )
							{
								PText * text = entM->getProperty<PText>(comp->getComponentId());
								if( text != nullptr )
								{
									// draw widgets' text
									draw( text, g, frameTime );
								}
							}
						}
					}
					if( frame->scrollable )
					{
						g->spriteEnd();
						g->endScissor();
						g->spriteBegin();
					}
				}
			}
		}
	}
	g->spriteEnd();
	UI_Index = -1;	
}

void SRender::renderParallaxLayers( Graphics * g, Map * world )
{
	g->spriteBegin();

	for( auto it = world->parallaxLayers.begin(); it != world->parallaxLayers.end(); it++ )
	{
		float bg_px = it->background.getSpritePrevX();
		float bg_py = it->background.getSpritePrevY();
		float bg_x = it->background.getSpriteX();
		float bg_y = it->background.getSpriteY();
		float old_x = bg_x;
		float old_y = bg_y;

		float left = g->getWorldLeft();
		float right = g->getWorldRight();
		float top = g->getWorldTop();
		float bottom = g->getWorldBottom();

		// gotta do some manual lerping
		Graphics::lerp( bg_px, bg_py, bg_x, bg_y, _Game->getLerpFraction() );
		// set x and y to new interpolated values
		it->background.setSpriteX_NL( bg_x );
		it->background.setSpriteY_NL( bg_y );

		// draw image at original (interpolated) location first
		it->background.draw(g);

		// while top of image is visible...
		while( it->background.getSpriteY() > top )
		{
			// move image up by its height, and draw
			it->background.setSpriteY_NL( it->background.getSpriteY() - it->background.getSpriteHeight() );
			it->background.draw(g);
		}
		// restore y position
		it->background.setSpriteY_NL( bg_y );

		// while bottom of image is visible...
		while( it->background.getSpriteY() < bottom )
		{
			// move image down by its height, and draw
			it->background.setSpriteY_NL( it->background.getSpriteY() + it->background.getSpriteHeight() );
			it->background.draw(g);					
		}
		// restore y position
		it->background.setSpriteY_NL( bg_y );

		// while left side of image is visible...
		while( it->background.getSpriteX() > left )
		{
			// move image to the left by its width
			it->background.setSpriteX_NL( it->background.getSpriteX() - it->background.getSpriteWidth() );
			it->background.draw(g);

			// while top of image is visible...
			while( it->background.getSpriteY() > top )
			{
				// move image up by its height, and draw
				it->background.setSpriteY_NL( it->background.getSpriteY() - it->background.getSpriteHeight() );
				it->background.draw(g);
			}

			// restore y position
			it->background.setSpriteY_NL( bg_y );

			// while bottom of image is visible...
			while( it->background.getSpriteY() < bottom )
			{
				// move image down by its height, and draw
				it->background.setSpriteY_NL( it->background.getSpriteY() + it->background.getSpriteHeight() );
				it->background.draw(g);					
			}

			// restore y position
			it->background.setSpriteY_NL( bg_y );
		}

		// restore x position
		it->background.setSpriteX_NL( bg_x );

		// while right side of image is visible...
		while( it->background.getSpriteX() < right )
		{
			// move image to the right by its width
			it->background.setSpriteX_NL( it->background.getSpriteX() + it->background.getSpriteWidth() );
			it->background.draw(g);

			// while top of image is visible...
			while( it->background.getSpriteY() > top )
			{
				// move image up and draw
				it->background.setSpriteY_NL( it->background.getSpriteY() - it->background.getSpriteHeight() );
				it->background.draw(g);
			}
			// reset y position
			it->background.setSpriteY_NL( bg_y );

			// while bottom of image is visible...
			while( it->background.getSpriteY() < bottom )
			{
				// move image down and draw
				it->background.setSpriteY_NL( it->background.getSpriteY() + it->background.getSpriteHeight() );
				it->background.draw(g);					
			}
			// reset y position
			it->background.setSpriteY_NL( bg_y );
		}

		it->background.setSpritePrevX( bg_px );
		it->background.setSpritePrevY( bg_py );
		it->background.setSpriteX( old_x );
		it->background.setSpriteY( old_y );
	}

	g->spriteEnd();
}

void SRender::renderTiles( Graphics * g, Map * world )
{
	float x = world->getX(), y = world->getY();
	// linearly interpolate change in world position
	Graphics::lerp( world->getPrevX(), world->getPrevY(), x, y, _Game->getLerpFraction() );

	// get world data needed for rendering tiles
	int minX = world->getMinX();
	int minY = world->getMinY();
	int tileWidth = world->getTileWidth();
	int tileHeight = world->getTileHeight();
	int width = world->getWidth();
	int height = world->getHeight();
	bool showGrid = world->getDrawGrid();

	WorldGraph * graph = world->getGraph();
	PCamera * primaryCamera = world->getPrimaryCamera();

	// prevent going off edge of the map
	// and prevent rendering loop to go out of tile buffer range
	if(y > 0.0f)
		y = 0.0f;
	if(x > 0.0f)
		x = 0.0f;
	if(x <= (float) minX )
		y = (float) minX;
	if(y <= (float) minX )
		y = (float) minY;

	// loop through the layers vector, rendering each tile
	// that falls within screen region y + tile height, and
	// screen region x + tile width
	int differenceX = 0, differenceY = 0;

	int renderRangeX = (int)(g->getWidth())/(tileWidth);
	int renderRangeY = (int)(g->getHeight())/(tileHeight);

	float sr_width = 1600.0f/(float)g->getWidth();
	float sr_height = 900.0f/(float)g->getHeight();

	if( primaryCamera != nullptr )
	{
		float zoom = primaryCamera->zoom < 2.0f ? 2.0f : primaryCamera->zoom;
		differenceX = (int)(sr_width*zoom*g->getWidth() - g->getWidth() + 25)/2;
		differenceY = (int)(sr_height*zoom*g->getHeight() - g->getHeight() + 25)/2;
		renderRangeX = (int)(sr_width*g->getWidth()*zoom)/(tileWidth);
		renderRangeY = (int)(sr_height*g->getHeight()*zoom)/(tileHeight);
	}

	for(auto it = world->layers.begin(); it != world->layers.end(); it++)
	{
		int rowcount = 0;
		
		for(int row = ((int)-y)/tileHeight - differenceY; rowcount < (renderRangeY + differenceY + 1)*2; row++)
		{
			rowcount++;

			if(row >= 0 && row < height)
			{
				int colcount = 0;
				for(int col = ((int)-x)/tileWidth - differenceX; colcount < (renderRangeX + differenceX + 1)*2; col++)
				{
					if(col >= 0 && col < width)
					{
						int tile_id = it->tileFrames[ col*height + row ];	//tile_id is the frame number of the spritesheet to render
						
						if(tile_id >= 0)
						{
							/* TODO: Tiled logic here */
						}
						Tile * t = world->getTile(row, col);
	
						if( t->image != nullptr && ( t->imgFlag == WorldGraphNS::NODEIMG_RENDER_ALWAYS || showGrid ) )
						{
							t->image->setSpriteScale( t->imgScale );
							t->image->setSpriteX_NL( (float)( t->imgX + x ) );
							t->image->setSpriteY_NL( (float)( t->imgY + y ) );
							t->image->setColorFilter( t->color );

							t->image->draw(g);
						}
					}
					colcount++;
				}
			}
		}
	}
}