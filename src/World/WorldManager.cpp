#include "world\\WorldManager.h"
#include "game\\game.h"
#include "game\\LuaAPI.h"

WorldManager::WorldManager(){
	currentWorld = NULL;
}

WorldManager::~WorldManager(){
	onLostDevice();
}

bool WorldManager::buildWorld(std::string id, Game* g, const char mapDataFileName[], const char tilesetFileName[], bool isTiled){
		Log(consoleNS::INFO, "Building world " + id + ".");	
		Map* world = new Map();

		if(worlds.find(id) != worlds.end())
		{
			Log(consoleNS::INFO, "World " + id + " already exists; clearing previous invocation.");
			worlds.erase(worlds.find(id));
		}

		LuaDoa::pushWorld( world );

		Log(consoleNS::INFO, "Initializing Entity Manager.");	
		world->getEntityManager()->initialize(g->registeredSystems, 5000);

		Log(consoleNS::INFO, "Initializing Map.");
		if( !world->initialize(g, id, mapDataFileName, tilesetFileName, isTiled) )
		{
			Log(consoleNS::ERR, "Failed to initialize Map.");
			return false;
		}
		/*
		//if cameras exist, position the cameras in the world
		float x = -1.0f, y = -1.0f, screenFocusX = -1.0f, screenFocusY = -1.0f;
		auto cams = world->getEntityManager()->getProperties<PCamera>();
		for(auto it = cams.begin(); it != cams.end(); it++)
		{
			x = it->second->prevX;
			y = it->second->prevY;
			screenFocusX = it->second->offsetX;
			screenFocusY = it->second->offsetY;
		}
		
		if(x != -1.0f && y != -1.0f)
		{
			world->moveWorld(-(x - screenFocusX), -(y - screenFocusY));
		//	world->update(g, true);	//ignore entities followCamera field when updating the world to adjust camera's initial pos
		}
		*/
		worlds.insert(std::pair<std::string, Map*>(id, world));

		return true;
}

Map * WorldManager::loadWorld(std::string id){
	Log(consoleNS::INFO, "Loading world " + id + ".");	

	auto iter = worlds.find(id);
	if(iter == worlds.end())
	{
		Log(consoleNS::ERR, "Failed to load world " + id + ".");
		return nullptr;
	}

	currentWorld = iter->second;
	currentWorld->getEntityManager()->sortGobjs();
	return currentWorld;
}

bool WorldManager::hasWorld(std::string id){
	if(worlds.find(id) == worlds.end())
		return false;

	return true;
}

Map* const WorldManager::getCurrentWorld(){
	return this->currentWorld;
}

void WorldManager::onLostDevice(){
	//release all vertex buffers
	for(auto it = worlds.begin(); it != worlds.end(); it++){
		auto list = it->second->getEntityManager()->getEntitiesWithProperty<PQuad>();
		std::sort(list.begin(), list.end());
		for(int i = list.size()-1; i >= 0; i--){
			PQuad* pq = it->second->getEntityManager()->getProperty<PQuad>(list[i]);
			safeOnLostDevice(pq);
		}
	}

	//release all text
	for(auto it = worlds.begin(); it != worlds.end(); it++){
		auto list = it->second->getEntityManager()->getEntitiesWithProperty<PText>();
		std::sort(list.begin(), list.end());
		for(int i = list.size()-1; i >= 0; i--){
			PText* pq = it->second->getEntityManager()->getProperty<PText>(list[i]);
			safeOnLostDevice(pq);
		}
	}
}

void WorldManager::onResetDevice(){
	//reset all vertex buffers
	for(auto it = worlds.begin(); it != worlds.end(); it++){
		auto list = it->second->getEntityManager()->getEntitiesWithProperty<PQuad>();
		std::sort(list.begin(), list.end());
		for(auto jt = list.begin(); jt != list.end(); jt++){
			it->second->getEntityManager()->getProperty<PQuad>(*jt)->onResetDevice();
		}
	}

	//reset DX text
	for(auto it = worlds.begin(); it != worlds.end(); it++){
		auto list = it->second->getEntityManager()->getEntitiesWithProperty<PText>();
		std::sort(list.begin(), list.end());
		for(auto jt = list.begin(); jt != list.end(); jt++){
			it->second->getEntityManager()->getProperty<PText>(*jt)->onResetDevice();
		}
	}
}