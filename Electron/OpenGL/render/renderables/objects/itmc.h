//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __OBJECT_ITMC__
#define __OBJECT_ITMC__
#include "object.h"

class ItmcInstance : public ObjectInstance {
public:
    ObjectInstance *duplicate();
    SelectionType type();
    void read(ObjectClass *manager, ProtonTag *scenario, uint8_t* offset, uint8_t size);
    void render(ShaderType pass);
};

class ItmcClass : public ObjectClass {
private:
    ObjectManager *manager;
    ProtonMap *map;
    void read_spawn(ProtonTag *scenario, HaloTagReflexive spawn, uint8_t size);
public:
    ObjectRef *create_object(HaloTagDependency tag);
    void read(ObjectManager *manager, ProtonMap *map, ProtonTag *scenario);
    void write(ProtonMap *map, ProtonTag *scenario);
};

#endif
