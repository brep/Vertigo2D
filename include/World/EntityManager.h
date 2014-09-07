/*
This class is used as a data structure for managing and maintaining game entities.
The structure is inspired by the entity-component architecture.

Entities only exist implicitly in that each entity is made up of a collection
of properties, for example an Image, and/or a PBoundingBox (hitbox), and/or a PPosition,
etc. Each of these properties has an ID that identifies it as a particular entity.
For example, the aforementioned properties may all have an entity_id = 2. The entity
whose ID is 2 is nothing more than a collection of the aforementioned properties.

These properties gain meaning from the logic that the Systems perform on them.
For example, the SRender system queries the current world's entity manager for
all available image properties. The entity manager returns a list of all these images,
and the SRender system iterates through this list, rendering each image with a call
to graphics::drawSprite(...).

Some Systems maintain their own lists of properties instead of querying the entity manager
on every update() call. This is the case for systems that process a larger amount of
properties, or do so in a more time-consuming manner. For example, SCollision collision
check is quadratic - it checks collision of each entity with every other entity within
a particular region of the world. In addition, Many entities have a PBoundingBox property,
so the quadratic loop will be particularly slow if having to query the entity manager
for each PBoundingBox property (twice!).

For deletions/additions of entities, the entity manager keeps a reference count of all
entities to be deleted/added. when all systems that maintain their own list acknowledge the 
delete/insert, the entity is finally deleted (meaning, its properties are all deleted
with a call to delete) from this entity manager.
*/

#ifndef _ENTITY_MANAGER_H
#define _ENTITY_MANAGER_H

#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <typeindex>
#include <queue>
#include "game\\input.h"
#include "properties\\PDialog.h"
#include "properties\\PFrame.h"
#include "properties\\PGravity.h"
#include "properties\\PParticle.h"
#include "properties\\PAI.h"
#include "properties\\PImage.h"
#include "properties\\PProperty.h"
#include "properties\\PPosition.h"
#include "properties\\PBoundingBox.h"
#include "properties\\PMobile.h"
#include "properties\\PCamera.h"
#include "properties\\PTimer.h"
#include "properties\\PClickable.h"
#include "properties\\PText.h"
#include "properties\\PQuad.h"
#include "properties\\PComponent.h"
#include "properties\\PGraphicObject.h"

typedef std::map<std::string, std::map<int, void*> > propertiesDataStruct;

struct Message
{
	std::string propertyName;		//the name of the property
	bool create;					//whether or not an entity was deleted or created
	int id;							//the id of the entity that was created/deleted 
};

struct Notification
{
	Notification()									
		: timer() { timer.entity_id = UINT_MAX; }
	Notification(unsigned int id)					
		: timer() { timer.entity_id = id; }
	Notification(std::string id)					
		: timer() { timer.entity_id = UINT_MAX; this->id = id; }
	Notification(unsigned int id, float endTime)	
		: timer() { timer.entity_id = id; timer.setActive(true); timer.setEndTime(endTime); }
	Notification(std::string id, float endTime)		
		: timer() { timer.entity_id = UINT_MAX; timer.setActive(true); timer.setEndTime(endTime); this->id = id; }
	Notification(unsigned int id, float endTime, bool (*fnc)(void *), void * caller )	
		: timer() { timer.entity_id = id; timer.setActive(true); timer.setEndTime(endTime); timer.addTask(fnc, caller); }

	PTimer timer;
	std::string id;					// set when id is not an integer, but rather a string
};

class EntityManager
{
private:
	//================================================
	// data structures for entities
	//================================================

	//data is structured in the following manner:
	//1. hash table {key = type of class; value = another hash table}
	//2. hash table 2 {key = ID of entity; value = Property data (image, AI, physics, etc.)} 
	std::unordered_map<std::type_index, std::unordered_map<unsigned int, PProperty*> > data;

	std::vector<PGraphicObject*> gobjsByLayer;					//images sorted in ascending order by their layer
	std::queue<int> gobjsToBeDeleted;					//entities (their IDs) to be deleted.

	
	unsigned int registeredSystems;	// systems registered to reference counted deleted entities / inserted entities
	
	std::list<unsigned int> newEntities;
	std::list<unsigned int> deletedEntities;

	// key: entity ID, val: number of systems that have not acknowledged deletion/insertion
	std::map< unsigned int, unsigned int > deletedEntitiesRefCount;
	std::map< unsigned int, unsigned int > newEntitiesRefCount;

	// vector of entity ids registered to receive onDelete events.
	// the 0th index of each vector is the ID of the entity being deleted
	// every other element in the vector is an ID of entities registered to
	// be notified of when the entity with ID at index 0 is deleted.
	std::vector< std::vector< unsigned int > > registeredOnDelete;

	//===============================================
	// other
	//===============================================
	// list of Notifications - entities OR lua tables OR lua functions that wish to
	// be "called back" after a certain period of time.
	std::list< Notification > registeredOnNotify;

	unsigned int lowestAvailableId;		//updated on entity creation/deletion
	unsigned int lastCreatedEntityId;
	unsigned int lastDeletedEntityId;

public:
	unsigned int refCount;
	std::vector< std::vector<bool> > acknowledgedInsert;	//sys_id, ent_id
	std::vector< std::vector<bool> > acknowledgedDelete;	//sys_id, ent_id
	
	EntityManager()
	{
		this->lastCreatedEntityId = 0;
		this->lowestAvailableId = 0;
		this->lastDeletedEntityId = -1;
	}

	// rewrite acknowlededDelete/Insert to be vectors. initialize will take in the number of registered systems and
	// the number of entities to initialize with
	void initialize(short num_systems, unsigned int max_entities)
	{
		this->refCount = num_systems;
		acknowledgedInsert = std::vector< std::vector<bool> >(num_systems, std::vector<bool>(max_entities, false) );
		acknowledgedDelete = std::vector< std::vector<bool> >(num_systems, std::vector<bool>(max_entities, false) );
	}

	unsigned int createEntity()
	{
		lastCreatedEntityId = lowestAvailableId++;
		newEntities.push_front(lastCreatedEntityId);
		newEntitiesRefCount.insert(std::make_pair(lastCreatedEntityId, refCount));
		// create new entry in registeredOnDelete (registeredOnDelete's size is always == lastCreateEntityId
		// so that we may look up entities registered to receive onDelete events by entity ID index.)
		// so for now we push back an empty vector.
		registeredOnDelete.push_back(std::vector<unsigned int>());

		return lastCreatedEntityId;
	}

	template <class T>
	void addProperty(PProperty * prop, unsigned int Id)
	{
		auto key = std::type_index(typeid(T));
		prop->entity_id = Id;

		if (data.find(key) == data.end())
		{
			auto value = new std::unordered_map<unsigned int, PProperty*>();
			data.insert(std::make_pair(key, *value));
		}
			
		data[key].insert(std::make_pair(Id, prop));
	}

	template <class T>
	std::unordered_map<unsigned int, PProperty*> getGunitProperties()
	{
		auto ret = data[std::type_index(typeid(T))]; 
		return ret;
	}

	template <class T>
	std::unordered_map<unsigned int, T*> getProperties()
	{
		std::unordered_map<unsigned int, T*> ret;
		auto key = std::type_index(typeid(T));
		for(auto it = data[key].begin(); it != data[key].end(); it++)
		{
			ret.insert(std::make_pair(it->first, static_cast<T*>(it->second)));
		}
		return ret;
	}

	//broken
	template <class T>
	std::vector<T*> getPropertiesAsVector()
	{
		std::vector<T*> ret;
		auto key = std::type_index(typeid(T));
		for(auto it = data[key].begin(); it != data[key].end(); it++)
		{
			ret.push_back(static_cast<T*>(it->second));
		}
		return ret;
	}

	template <class T>
	std::vector<T*> getPropertiesByIds(std::vector<unsigned int> ids)
	{
		std::vector<T*> ret;
		std::type_index key = std::type_index(typeid(T));
		for(int i = 0; i < ids.size(); i++)
		{
			ret.push_back(static_cast<T*>(data[key][ids[i]]));
		}
		return ret;
	}

	template <class T>
	T* getProperty(unsigned int id)
	{
		std::type_index key = std::type_index(typeid(T));
		PProperty * ret = NULL;
		if(data.find(key) != data.end()){
			if(data[key].find(id) != data[key].end())
				ret = data[key][id];
		}
		
		return static_cast<T*>(ret);
	}

	//rewrite this to return the entire table with property T.
	//the client will then have access to both the key (entity IDs) and the property
	template <class T>
	std::vector<unsigned int> getEntitiesWithProperty()
	{
		std::vector<unsigned int> ret;
		auto key = std::type_index(typeid(T));

		if(data.find(key) == data.end())
			return ret;
		
		std::unordered_map<unsigned int, PProperty*> map = data[key];
		
		for(auto i = map.begin(); i != map.end(); i++)
		{
			ret.push_back(i->first);
		}

		return ret;
	}

	//used in system initialization
	std::vector<unsigned int> getCommonEntities(unsigned int numProps, std::vector<unsigned int> ents);

	std::vector<PGraphicObject*> getGobjsByLayer(){ return gobjsByLayer; }

	std::list<Notification> * getNotifyees() { return &registeredOnNotify; } 

	void registerOnNotify(unsigned int id, float time);			// register entity for notification callback

	void registerOnNotifyStr(std::string name, float time);		// register lua table/function for notification callback

	void registerTaskOnNotify(unsigned int id, float time, bool (*fnc)(void *), void * caller );

	void removeNotification(unsigned int id);

	void removeImage(int id);

	//returns image IDs to be deleted
	std::queue<int> const getGobjsToBeDeleted(){ return gobjsToBeDeleted; }

	void deleteEntity(unsigned int id);

	void deleteEntityFinal(unsigned int id);
	
	// registers notifyId to onDelete event when entity with deletedId is deleted
	void registerOnDelete(unsigned int deletedId, unsigned int notifyId){ registeredOnDelete[ deletedId ].push_back( notifyId ); }

	void unregisterOnDelete(unsigned int deletedId, unsigned int notifyId);

	// called before an entity is deleted; notifies all entities registered that entity with ID == id is 
	// to be deleted (aka calls their onDelete() function).
	void notifyDeletion(unsigned int id);

	void registerSystem(){ refCount++; }

	void clearDeletedEntity(unsigned int id);

	void clearNewEntity(unsigned int id);

	void clearDeletedEntities();

	void clearNewEntities();

	void acknowledgeDelete(unsigned int sys_id, unsigned int id);

	void acknowledgeInsert(unsigned int sys_id, unsigned int id);

	std::list<unsigned int> getDeletedEntities() { return deletedEntities; }

	std::list<unsigned int> getNewEntities() { return newEntities; }


//================================================================================================================
//												LUA	BINDING FUNCTIONS
//								 Necessary since cannot use templated functions from Lua
//================================================================================================================
	PImage* getImage(unsigned int ent_id){ return getProperty<PImage>(ent_id); }
	
	PBoundingBox* getHitbox(unsigned int ent_id) { return getProperty<PBoundingBox>(ent_id); }
	
	PMobile* getMobile(unsigned int ent_id) { return getProperty<PMobile>(ent_id); }

	PImage* attachImage(unsigned int id);
	PImage* attachImageWithLayer(unsigned int id, int layer);
	// don't insert image in graphic object list
	PImage* attachGuiImage(unsigned int id);
	PBoundingBox* attachBoundingBox(unsigned int id);
	PMobile* attachMobile(unsigned int id);
	PPosition* attachPosition(unsigned int id);
	PTimer* attachTimer(unsigned int id);
	PClickable* attachClickable(unsigned int id);
	PQuad* attachQuad(unsigned int id);
	PComponent* attachComponent(unsigned int id);
	PText* attachText(unsigned int id);
	PText* attachTextWithLayer(unsigned int id, int layer);
	// don't insert text in graphic object list
	PText* attachGuiText(unsigned int id);
	PGravity* attachGravity(unsigned int id);
	PCamera* attachCamera(unsigned int id);
	PParticleEmiter* attachParticleEmitter(unsigned int id);
	PAI* attachAI(unsigned int id);

	//////////////////	GUI-binding property functions //////////////////////
	PFrame* makeFrame(unsigned int id);
	PDialog* makeDialog(unsigned int id);

	void insertGobj(PGraphicObject *gobj);

	int getLastCreatedEntityId(){ return lastCreatedEntityId; }

	int getLastDeletedEntityId(){ return lastDeletedEntityId; }

	void sortGobjs();
};

#endif