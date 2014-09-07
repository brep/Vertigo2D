#include "world\\EntityManager.h"
#include "game\\game.h"

bool hasGreaterZValue(PGraphicObject* img1, PGraphicObject* img2)
{
	return img1->getLayer() < img2->getLayer();
}

void EntityManager::registerOnNotify(unsigned int id, float time)
{
	registeredOnNotify.push_back( Notification(id, time) );
}

void EntityManager::registerOnNotifyStr(std::string id, float time)
{
	registeredOnNotify.push_back( Notification(id, time) );
}

void EntityManager::registerTaskOnNotify(unsigned int id, float time, bool (*fnc)(void *), void * caller)
{
	registeredOnNotify.push_back( Notification(id, time, fnc, caller) );
}

void EntityManager::removeNotification(unsigned int id)
{
	for( auto it = registeredOnNotify.begin(); it != registeredOnNotify.end(); it++ )
	{
		if( it->timer.entity_id == id )
		{
			registeredOnNotify.erase( it );
			return;
		}
	}
}

void EntityManager::acknowledgeDelete(unsigned int sys_id, unsigned int id)
{
	if(  sys_id > acknowledgedDelete.size() || sys_id < 0 )
	{
		Log(consoleNS::ERR, "Invalid sys_id index: " + std::to_string(sys_id) + ".");
		return;
	}
	if( id > acknowledgedDelete[0].size() || id < 0 )
	{
		Log(consoleNS::ERR, "Invalid Entity ID.");
		return;
	}

	acknowledgedDelete[sys_id][id] = true;	
	unsigned int del = --deletedEntitiesRefCount.find(id)->second;
		
	if(del < 0)
		Log(consoleNS::ERR, "Deleted entity reference count fell below 0!");

	if(del == 0)
		clearDeletedEntity(id);
}

void EntityManager::acknowledgeInsert(unsigned int sys_id, unsigned int id)
{
	if(  sys_id > acknowledgedDelete.size() || sys_id < 0 )
	{
		Log(consoleNS::ERR, "Invalid sys_id index: " + std::to_string(sys_id) + ".");
		return;
	}
	if( id > acknowledgedDelete[0].size() || id < 0 )
	{
		Log(consoleNS::ERR, "Invalid Entity ID.");
		return;
	}

	acknowledgedInsert[sys_id][id] = true;
	unsigned int ins = --newEntitiesRefCount.find(id)->second;
		
	if(ins < 0)
		Log(consoleNS::ERR, "Deleted entity reference count fell below 0!");

	if(ins == 0)
		clearNewEntity(id);
}

void EntityManager::unregisterOnDelete(unsigned int deletedId, unsigned int notifyId)
{
	for( size_t i = 0; i < registeredOnDelete[ deletedId ].size(); i++ )
	{
		if( registeredOnDelete[ deletedId ][ i ] == notifyId )
		{
			registeredOnDelete[ deletedId ].erase( registeredOnDelete[ deletedId ].begin() + i );
			return;
		}
	}
}

void EntityManager::deleteEntity(unsigned int id)
{
	if(refCount == 0)
	{
		deleteEntityFinal(id);
		return;
	}

	lastDeletedEntityId = id;
	deletedEntities.push_front(id);
	deletedEntitiesRefCount.insert(std::make_pair(id, refCount));
}

void EntityManager::notifyDeletion(unsigned int id)
{
	// notify entities of id's deletion
	for( size_t i = 0; i < registeredOnDelete[id].size(); i++ )
	{
		unsigned int notifyId = registeredOnDelete[id][i];
		for(auto it = data.begin(); it != data.end(); it++)
		{
			auto jt = it->second.find(notifyId);

			if(jt != it->second.end())
				jt->second->onDelete(id);
		}
	}
	// remove deleted from registeredOnDelete
	if( !registeredOnDelete[id].empty() )
	{
		registeredOnDelete.erase( registeredOnDelete.begin() + id );
		registeredOnDelete.push_back( std::vector< unsigned int >() );
	}
}

void EntityManager::deleteEntityFinal(unsigned int id)
{
	// notify all entities registered of entity with ID == id deletion
	notifyDeletion(id);
	// remove any notifications associated with this id
	removeNotification(id);
	// remove PGraphicObject from imagesById vector (if this entity with ID == id has an image property)
	removeImage(id);

	// delete entity's properties from data
	for(auto it = data.begin(); it != data.end(); it++)
	{
		auto jt = it->second.find(id);

		if(jt != it->second.end())
		{
			delete jt->second;
			it->second.erase(jt);
		}
	}
}

std::vector<unsigned int> EntityManager::getCommonEntities(unsigned int numProps, std::vector<unsigned int> ents)
{
	sort(ents.begin(), ents.end());
	std::vector<unsigned int> ret;
	int count = 1;
	//check for all entity IDs that repeat numProps amount of times.
	//if the ID repeats numProps many times, then its representative entity has all the properties
	for(unsigned int i = 1; i < ents.size(); i++)
	{
		count++;
		if(ents[i] != ents[i-1])
			count = 1;
		if(count == numProps)
			ret.push_back(ents[i-1]);
	}

	return ret;
}

PImage* EntityManager::attachImage(unsigned int id)
{
	PImage* img = new PImage();
	addProperty<PImage>(img, id);
	insertGobj(img);
	return img;
}
PImage* EntityManager::attachImageWithLayer(unsigned int id, int layer)
{
	PImage* img = new PImage();
	img->setLayer(layer);
	addProperty<PImage>(img, id);
	insertGobj(img);
	return img;
}
// don't insert image in graphic object list
PImage* EntityManager::attachGuiImage(unsigned int id)
{
	PImage* img = new PImage();
	addProperty<PImage>(img, id);
	return img;
}
PBoundingBox* EntityManager::attachBoundingBox(unsigned int id)
{
	PBoundingBox* box = new PBoundingBox();
	addProperty<PBoundingBox>(box, id);
	return box;
}
PMobile* EntityManager::attachMobile(unsigned int id)
{
	PMobile* mobile = new PMobile();
	addProperty<PMobile>(mobile, id);
	return mobile;
}
PPosition* EntityManager::attachPosition(unsigned int id)
{
	PPosition* pos = new PPosition();
	addProperty<PPosition>(pos, id);
	return pos;
}
PTimer* EntityManager::attachTimer(unsigned int id)
{
	PTimer* pt = new PTimer();
	addProperty<PTimer>(pt, id);
	return pt;
}
PClickable* EntityManager::attachClickable(unsigned int id)
{
	PClickable* pc = new PClickable();
	addProperty<PClickable>(pc, id);
	return pc;
}
PQuad* EntityManager::attachQuad(unsigned int id)
{
	PQuad* pq = new PQuad();
	addProperty<PQuad>(pq, id);
	return pq;
}
PComponent* EntityManager::attachComponent(unsigned int id)
{
	PComponent* pc = new PComponent();
	addProperty<PComponent>(pc, id);
	return pc;
}
PText* EntityManager::attachText(unsigned int id)
{
	PText* pt = new PText();
	addProperty<PText>(pt, id);
	insertGobj(pt);
	return pt;
}
PText* EntityManager::attachTextWithLayer(unsigned int id, int layer)
{
	PText* pt = new PText();
	pt->setLayer(layer);
	addProperty<PText>(pt, id);
	insertGobj(pt);
	return pt;
}
// don't insert text in graphic object list
PText* EntityManager::attachGuiText(unsigned int id)
{
	PText* img = new PText();
	addProperty<PText>(img, id);
	return img;
}
PGravity* EntityManager::attachGravity(unsigned int id)
{
	PGravity* pg = new PGravity();
	addProperty<PGravity>(pg, id);
	return pg;
}
PCamera* EntityManager::attachCamera(unsigned int id)
{
	PCamera* pg = new PCamera();
	addProperty<PCamera>(pg, id);
	return pg;
}
PParticleEmiter* EntityManager::attachParticleEmitter(unsigned int id)
{
	PParticleEmiter* pe = new PParticleEmiter();
	addProperty<PParticleEmiter>(pe, id);
	return pe;
}
PAI* EntityManager::attachAI(unsigned int id)
{
	PAI* ai = new PAI();
	addProperty<PAI>(ai, id);
	return ai;
}

//////////////////	GUI-binding property functions //////////////////////

PFrame* EntityManager::makeFrame(unsigned int id)
{
	PFrame* pf = new PFrame();
	addProperty<PFrame>(pf, id);
	return pf;
}
PDialog* EntityManager::makeDialog(unsigned int id)
{
	PDialog* diag = new PDialog();
	addProperty<PDialog>(diag, id);
	return diag;
}

void EntityManager::removeImage(int id)
{
	for(auto it = gobjsByLayer.begin(); it != gobjsByLayer.end(); it++)
	{
		if((*it)->entity_id == id)
		{
			gobjsByLayer.erase(it);
			return;
		}
	}
}

void EntityManager::clearDeletedEntity(unsigned int id)
{
	deleteEntityFinal(id);

	auto it = deletedEntities.begin();
	while(it != deletedEntities.end())
	{
		if((*it) == id)
		{
			deletedEntities.erase(it);
			return;
		}
		else
		{
			++it;
		}
	}
}

void EntityManager::clearNewEntity(unsigned int id)
{
	auto it = newEntities.begin();
	while(it != newEntities.end())
	{
		if((*it) == id)
		{
			newEntities.erase(it);
			return;
		}
		else
		{
			++it;
		}
	}
}

void EntityManager::clearDeletedEntities()
{
	deletedEntities.clear();
}

void EntityManager::clearNewEntities()
{
	newEntities.clear();
}

void EntityManager::insertGobj(PGraphicObject * gobj)
{
	gobjsByLayer.push_back(gobj);
	std::_Insertion_sort(gobjsByLayer.begin(), gobjsByLayer.end(), hasGreaterZValue);
}

void EntityManager::sortGobjs()
{
	std::_Insertion_sort(gobjsByLayer.begin(), gobjsByLayer.end(), hasGreaterZValue);
}