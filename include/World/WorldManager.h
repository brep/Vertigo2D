/*
	This class is used for maintaing all the game's worlds (levels) - this includes
	"building" the world ( making a call to world.initialize(), where world is a Map
	object ) and loading the world by its ID (a string), which sets the current world
	to the world with this ID. every frame, the Game passes the current world
	in its WorldManager to each active system for processing.
*/

#ifndef _WORLDMANAGER_H
#define _WORLDMANAGER_H
#define WIN32_LEAN_AND_MEAN

#include <map>
#include "map.h"
#include "game\\gameError.h"

class Game;

class WorldManager
{

private:
	std::map<std::string, Map*> worlds;
	Map* currentWorld; 

public:
	WorldManager();
	virtual ~WorldManager();

	Map* const getCurrentWorld();

	bool buildWorld(std::string id, Game * g, const char mapDataFileName[], const char tilesetFileName[], bool isTiled);

	Map * loadWorld(std::string id);

	bool hasWorld(std::string id);

	void onLostDevice();

	void onResetDevice();
};

#endif