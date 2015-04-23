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

#define SELECTION_TYPES 8
typedef enum
{
    s_all = 0,
    s_scenery = 1,
    s_vehicle = 2,
    s_playerspawn = 3,
    s_encounter = 9,
    s_item = 4,
    s_netgame = 5,
    s_machine = 6,
    s_playerobject = 7
} SelectionType;

class ObjectManager;
class ObjectClass;

class ObjectRef {
private:
    Model *model;
    HaloTagDependency TagID;
public:
    ObjectRef(ObjectManager *manager, ProtonMap *map, HaloTagDependency tag);
    void updateTag();
    void render(ShaderType pass);
};

class ObjectInstance {
public:
    ObjectRef *reference;
    float x,y,z;
    float yaw, pitch, roll;
    void *data;
    bool selected;
    
    virtual SelectionType type() = 0;
    virtual ObjectInstance *duplicate() = 0;
    virtual void read(ObjectClass *manager, ProtonTag *scenario, uint8_t* offset, uint8_t size) = 0;
    virtual void render(ShaderType pass) = 0;
};

class ObjectClass {
public:
    std::vector<ObjectRef*> references;
    std::vector<ObjectInstance*> objects;
    
    virtual void read(ObjectManager *manager, ProtonMap *map, ProtonTag *scenario)=0;
    virtual void write(ProtonMap *map, ProtonTag *scenario) = 0;
};

class ObjectManager {
private:
    ObjectClass *scen;
    ObjectClass *vehi;
    ObjectClass *itmc;
    std::map<uint16_t, ObjectRef*> objects;
    ObjectClass *getClass(SelectionType type);
public:
    ModelManager *modelManager;
    
    // Setup
    void read(ShaderManager *shaders, ProtonMap *map, ProtonTag *scenario);
    void write(ProtonMap *map, ProtonTag *scenario);
    ObjectRef *create_object(ProtonMap *map, HaloTagDependency tag);
    
    // Rendering
    void render(GLuint *name, GLuint *lookup, ShaderType pass);
    
    // Selection
    std::vector<ObjectInstance*> selection;
    void clearSelection();
    void select(bool shift, float x, float y);
    
    // Modifiers
    ObjectInstance *duplicate(ObjectInstance *instance);
};

#endif /* defined(__BSP__) */
