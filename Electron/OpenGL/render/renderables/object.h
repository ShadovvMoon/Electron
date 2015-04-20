//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __OBJECT__
#define __OBJECT__

#include "defines.h"
#include "model.h"
#include <vector>
#include <map>

class ObjectManager;
class ObjectClass;

class ObjectRef {
private:
    Model *model;
    HaloTagDependency TagID;
public:
    ObjectRef(ObjectManager *manager, ProtonMap *map, HaloTagDependency tag);
    void updateTag();
};

class ObjectInstance {
private:
    ObjectRef *reference;
    float x,y,z;
    float yaw, pitch, roll;
    void *data;
public:
    virtual void read(ObjectClass *manager, ProtonTag *scenario, uint8_t* offset, uint8_t size) = 0;
};

class ObjectClass {
public:
    std::vector<ObjectRef*> references;
    std::vector<ObjectInstance*> objects;
    
    void read(ObjectManager *manager, ProtonMap *map, ProtonTag *scenario);
    void write(ProtonMap *map, ProtonTag *scenario);
    void render(ShaderType type);
};

class ObjectManager {
private:
    std::map<uint16_t, ObjectRef*> objects;
public:
    ModelManager *modelManager;
    void read(ProtonMap *map, ProtonTag *scenario);
    ObjectRef *create_object(ProtonMap *map, HaloTagDependency tag);
};

#endif /* defined(__BSP__) */
