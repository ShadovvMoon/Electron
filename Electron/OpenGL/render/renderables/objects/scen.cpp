//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "scen.h"

#define SCENERY_SPAWN_CHUNK 0x48
#define SCENERY_REF_CHUNK   0x30

typedef struct
{
    short numid;
    short flag;
    short not_placed;
    short desired_permutation;
    float coord[3];
    float rotation[3];
    float unknown[10];
} ScenerySpawn;

typedef struct
{
    HaloTagDependency scen_ref;
    uint32_t zero[8];
} SceneryReference;

void ScenInstance::read(ObjectClass *manager, ProtonTag *scenario, uint8_t* offset, uint8_t size) {
    ScenerySpawn *spawn = (ScenerySpawn*)offset;
    reference = manager->references[spawn->numid];
    x = spawn->coord[0];
    y = spawn->coord[1];
    z = spawn->coord[2];
    yaw   = spawn->rotation[0];
    pitch = spawn->rotation[1];
    roll  = spawn->rotation[2];
    data  = malloc(size);
    memcpy(data, offset, size);
};
ObjectInstance *ScenInstance::duplicate() {
    ScenInstance *clone = new ScenInstance();
    clone->reference = reference;
    clone->x = x;
    clone->y = y;
    clone->z = z;
    clone->yaw   = yaw;
    clone->pitch = pitch;
    clone->roll  = roll;
    clone->data  = malloc(SCENERY_SPAWN_CHUNK);
    memcpy(clone->data, data, SCENERY_SPAWN_CHUNK);
    return clone;
}
void ScenInstance::render(ShaderType pass) {
    if (selected) {
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_BLEND);
        glColor4f(1.0, 1.0, 0.0, 0.2);
    }
    
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(roll   * (57.29577951), 1, 0, 0);
    glRotatef(-pitch * (57.29577951), 0, 1, 0);
    glRotatef(yaw    * (57.29577951), 0, 0, 1);
    reference->render(pass);
    glPopMatrix();
    
    if (selected) {
        glColor4f(1.0, 1.0, 1.0, 1.0);
    }
}
SelectionType ScenInstance::type() {
    return s_scenery;
}

void ScenClass::read_spawn(ProtonTag *scenario, HaloTagReflexive spawn, uint8_t size) {
    printf("%d\n", spawn.count);
    int i;
    for (i=0; i < spawn.count; i++) {
        uint8_t *data = (uint8_t *)scenario->Data() + scenario->PointerToOffset(spawn.address) + size * i;
        ScenInstance *object = new ScenInstance;
        object->read(this, scenario, data, size);
        objects.push_back(object);
    }
}

void ScenClass::read(ObjectManager *manager, ProtonMap *map, ProtonTag *scenario) {
    printf("creating scen manager\n");
    HaloScenarioTag *tag = (HaloScenarioTag *)scenario->Data();
    HaloTagReflexive scenRef = tag->scenRef;
    int i;
    for (i=0; i < scenRef.count; i++) {
        printf("reading scen ref %d\n", i);
        SceneryReference *data = (SceneryReference *)(scenario->Data() + scenario->PointerToOffset(scenRef.address) + SCENERY_REF_CHUNK * i);
        ObjectRef *ref = manager->create_object(map, data->scen_ref);
        references.push_back(ref);
        printf("%lu\n", references.size());
    }
    printf("reading spawns\n");
    read_spawn(scenario, tag->scen, SCENERY_SPAWN_CHUNK);
}

void ScenClass::write(ProtonMap *map, ProtonTag *scenario) {
    HaloScenarioTag *tag = (HaloScenarioTag *)scenario->Data();

    // Remove the old scenery spawn reflexive data
    printf("writing scenery %d %d\n", tag->scen.count, objects.size());
    uint32_t offset = scenario->PointerToOffset(tag->scen.address);
    scenario->DeleteData(offset, tag->scen.count * SCENERY_SPAWN_CHUNK);
    
    // Assemble the new spawn data
    char *data = (char*)malloc(SCENERY_SPAWN_CHUNK * objects.size()); //cleaned
    int i;
    for (i=0; i < objects.size(); i++) {
        ScenInstance *object = (ScenInstance*)objects[i];
        ScenerySpawn *spawn = (ScenerySpawn*)(data + i * SCENERY_SPAWN_CHUNK);
        memcpy(spawn, object->data, SCENERY_SPAWN_CHUNK);
        
        spawn->coord[0] = object->x;
        spawn->coord[1] = object->y;
        spawn->coord[2] = object->z;
        spawn->rotation[0] = object->yaw;
        spawn->rotation[1] = object->pitch;
        spawn->rotation[2] = object->roll;
    }
    scenario->InsertData(offset, data, (uint32_t)(objects.size() * SCENERY_SPAWN_CHUNK));
    free(data);
    
    // Update the scen reflexive
    tag->scen.count   = (uint16_t)objects.size();
    tag->scen.address = scenario->OffsetToPointer(offset);
}

