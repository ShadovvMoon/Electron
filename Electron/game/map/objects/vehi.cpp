//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "vehi.h"

#define VEHICLE_SPAWN_CHUNK 0x78
#define VEHICLE_REF_CHUNK   0x30

typedef struct
{
    short numid;
    short flag;
    short not_placed;
    short desired_permutation;
    float coord[3];
    float rotation[3];
    uint32_t unknown2[22];
} VehicleSpawn;

typedef struct
{
    HaloTagDependency vehi_ref;
    uint32_t zero[8];
} VehicleReference;

void VehiInstance::read(ObjectClass *manager, ProtonTag *scenario, uint8_t* offset, uint8_t size) {
    VehicleSpawn *spawn = (VehicleSpawn*)offset;
    reference = manager->references[spawn->numid];
    x = spawn->coord[0];
    y = spawn->coord[1];
    z = spawn->coord[2];
    yaw   =  spawn->rotation[0] * CONVERSION;
    pitch = -spawn->rotation[1] * CONVERSION;
    roll  =  spawn->rotation[2] * CONVERSION;
    data  = malloc(size);
    memcpy(data, offset, size);
};
void VehiInstance::render(ShaderType pass) {
    /*
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(roll   * (57.29577951), 1, 0, 0);
    glRotatef(-pitch * (57.29577951), 0, 1, 0);
    glRotatef(yaw    * (57.29577951), 0, 0, 1);
    reference->render(pass);
    glPopMatrix();
    */
}
//not finished
ObjectInstance *VehiInstance::duplicate() {
    VehiInstance *clone = new VehiInstance;
    clone->reference = reference;
    clone->x = x;
    clone->y = y;
    clone->z = z;
    clone->yaw   = yaw;
    clone->pitch = pitch;
    clone->roll  = roll;
    clone->data  = malloc(VEHICLE_SPAWN_CHUNK);
    memcpy(clone->data, data, VEHICLE_SPAWN_CHUNK);
    return clone;
}
SelectionType VehiInstance::type() {
    return s_vehicle;
}

void VehiClass::read_spawn(ProtonTag *scenario, HaloTagReflexive spawn, uint8_t size) {
    int i;
    for (i=0; i < spawn.count; i++) {
        uint8_t *data = (uint8_t *)scenario->Data() + scenario->PointerToOffset(spawn.address) + size * i;
        VehiInstance *object = new VehiInstance;
        object->read(this, scenario, data, size);
        objects.push_back(object);
    }
}

void VehiClass::read(ObjectManager *manager, ProtonMap *map, ProtonTag *scenario) {
    printf("creating vehi manager\n");
    HaloScenarioTag *tag = (HaloScenarioTag *)scenario->Data();
    HaloTagReflexive vehiRef = tag->vehiRef;
    int i;
    for (i=0; i < vehiRef.count; i++) {
        printf("reading scen ref %d\n", i);
        VehicleReference *data = (VehicleReference *)(scenario->Data() + scenario->PointerToOffset(vehiRef.address) + VEHICLE_REF_CHUNK * i);
        ObjectRef *ref = manager->create_object(map, data->vehi_ref);
        references.push_back(ref);
        printf("%lu\n", references.size());
    }
    printf("reading spawns\n");
    read_spawn(scenario, tag->vehi, VEHICLE_SPAWN_CHUNK);
}

void VehiClass::write(ProtonMap *map, ProtonTag *scenario) {
    HaloScenarioTag *tag = (HaloScenarioTag *)scenario->Data();
    
    // Remove the old scenery spawn reflexive data
    printf("writing vehicle %d %d\n", tag->vehi.count, objects.size());
    uint32_t offset = scenario->PointerToOffset(tag->vehi.address);
    scenario->DeleteData(offset, tag->vehi.count * VEHICLE_SPAWN_CHUNK);
    
    // Assemble the new spawn data
    char *data = (char*)malloc(VEHICLE_SPAWN_CHUNK * objects.size()); //cleaned
    int i;
    for (i=0; i < objects.size(); i++) {
        VehiInstance *object = (VehiInstance*)objects[i];
        VehicleSpawn *spawn = (VehicleSpawn*)(data + i * VEHICLE_SPAWN_CHUNK);
        memcpy(spawn, object->data, VEHICLE_SPAWN_CHUNK);
        
        spawn->coord[0] = object->x;
        spawn->coord[1] = object->y;
        spawn->coord[2] = object->z;
        spawn->rotation[0] = object->yaw / CONVERSION;
        spawn->rotation[1] = -object->pitch / CONVERSION;
        spawn->rotation[2] = object->roll / CONVERSION;
    }
    scenario->InsertData(offset, data, (uint32_t)(objects.size() * VEHICLE_SPAWN_CHUNK));
    free(data);
    
    // Update the vehi reflexive
    tag->vehi.count   = (uint16_t)objects.size();
    tag->vehi.address = scenario->OffsetToPointer(offset);
}
