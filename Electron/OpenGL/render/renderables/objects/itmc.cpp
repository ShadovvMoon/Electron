//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "itmc.h"

#define NETGAME_EQUIP_CHUNK 0x90
typedef struct netgame_equipment
{
    uint32_t bitmask32;
    
    short type1; // Enum16
    short type2; // Enum16
    short type3; // Enum16
    short type4; // Enum16
    
    short team_index;
    short spawn_time;
    
    uint32_t unknown[12];
    float coord[3];
    float yaw;
    HaloTagDependency item_ref;
} NetgameEquipment;

void ItmcInstance::read(ObjectClass *manager, ProtonTag *scenario, uint8_t* offset, uint8_t size) {
    NetgameEquipment *spawn = (NetgameEquipment*)offset;
    reference = ((ItmcClass*)manager)->create_object(spawn->item_ref);
    x = spawn->coord[0];
    y = spawn->coord[1];
    z = spawn->coord[2];
    yaw   = spawn->yaw;
    pitch = 0.0;
    roll  = 0.0;
    data  = malloc(size);
    memcpy(data, offset, size);
};
ObjectInstance *ItmcInstance::duplicate() {
    ItmcInstance *clone = new ItmcInstance();
    clone->reference = reference;
    clone->x = x;
    clone->y = y;
    clone->z = z;
    clone->yaw   = yaw;
    clone->pitch = pitch;
    clone->roll  = roll;
    clone->data  = malloc(NETGAME_EQUIP_CHUNK);
    memcpy(clone->data, data, NETGAME_EQUIP_CHUNK);
    return clone;
}
void ItmcInstance::render(ShaderType pass) {
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
SelectionType ItmcInstance::type() {
    return s_item;
}

void ItmcClass::read_spawn(ProtonTag *scenario, HaloTagReflexive spawn, uint8_t size) {
    printf("%d\n", spawn.count);
    int i;
    for (i=0; i < spawn.count; i++) {
        uint8_t *data = (uint8_t *)scenario->Data() + scenario->PointerToOffset(spawn.address) + size * i;
        ItmcInstance *object = new ItmcInstance;
        object->read(this, scenario, data, size);
        objects.push_back(object);
    }
}

ObjectRef *ItmcClass::create_object(HaloTagDependency tag) {
    return this->manager->create_object(this->map, tag);
}
void ItmcClass::read(ObjectManager *manager, ProtonMap *map, ProtonTag *scenario) {
    this->manager = manager;
    this->map = map;
    
    printf("creating itmc manager\n");
    HaloScenarioTag *tag = (HaloScenarioTag *)scenario->Data();
    printf("reading spawns\n");
    read_spawn(scenario, tag->mpEquip, NETGAME_EQUIP_CHUNK);
}

void ItmcClass::write(ProtonMap *map, ProtonTag *scenario) {
    HaloScenarioTag *tag = (HaloScenarioTag *)scenario->Data();

    // Remove the old scenery spawn reflexive data
    printf("writing equipment %d %d\n", tag->scen.count, objects.size());
    uint32_t offset = scenario->PointerToOffset(tag->mpEquip.address);
    scenario->DeleteData(offset, tag->mpEquip.count * NETGAME_EQUIP_CHUNK);
    
    // Assemble the new spawn data
    char *data = (char*)malloc(NETGAME_EQUIP_CHUNK * objects.size()); //cleaned
    int i;
    for (i=0; i < objects.size(); i++) {
        ItmcInstance *object = (ItmcInstance*)objects[i];
        NetgameEquipment *spawn = (NetgameEquipment*)(data + i * NETGAME_EQUIP_CHUNK);
        memcpy(spawn, object->data, NETGAME_EQUIP_CHUNK);
        
        spawn->coord[0] = object->x;
        spawn->coord[1] = object->y;
        spawn->coord[2] = object->z;
        spawn->yaw = object->yaw;
    }
    scenario->InsertData(offset, data, (uint32_t)(objects.size() * NETGAME_EQUIP_CHUNK));
    free(data);
    
    // Update the scen reflexive
    tag->mpEquip.count   = (uint16_t)objects.size();
    tag->mpEquip.address = scenario->OffsetToPointer(offset);
}

