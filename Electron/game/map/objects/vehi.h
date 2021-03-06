//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __OBJECT_VEHI__
#define __OBJECT_VEHI__
#include "object.h"

class VehiInstance : public ObjectInstance {
public:
    ObjectInstance *duplicate();
    SelectionType type();
    void read(ObjectClass *manager, ProtonTag *scenario, uint8_t* offset, uint8_t size);
    void render(ShaderType pass);
};

class VehiClass : public ObjectClass {
private:
    void read_spawn(ProtonTag *scenario, HaloTagReflexive spawn, uint8_t size);
public:
    void read(ObjectManager *manager, ProtonMap *map, ProtonTag *scenario);
    void write(ProtonMap *map, ProtonTag *scenario);
    void render(GLuint *name, GLuint *lookup, ShaderType pass);
    void select(GLuint index);
};

#endif
