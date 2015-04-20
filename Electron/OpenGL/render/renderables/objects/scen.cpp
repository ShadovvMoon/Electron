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
    data  = offset;
};

void ScenClass::read_spawn(ProtonTag *scenario, HaloTagReflexive spawn, uint8_t size) {
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