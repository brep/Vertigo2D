#ifndef _PPROPERTY_H
#define _PPROPERTY_H

// The base property from which all other properties inherit
struct PProperty
{
	PProperty(){};
	virtual ~PProperty(){};

	// the unique entity identifier
	unsigned int entity_id;

	// called if entity is registered to onDelete event - overwritten by subclass
	virtual void onDelete(unsigned int id){}
};

#endif