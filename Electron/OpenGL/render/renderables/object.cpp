//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "object.h"
#include "objects/scen.h"

ObjectRef::ObjectRef(ObjectManager *manager, ProtonMap *map, HaloTagDependency tag) {
    printf("creating object ref\n");
    TagID = tag;
    
    if (tag.tag_id.tag_index != NULLED_TAG_ID) {
        ProtonTag *objectTag = map->tags.at(tag.tag_id.tag_index).get();
        HaloObjectTagData *objectData = (HaloObjectTagData *)objectTag->Data();
        
        printf("talking to models\n");
        model = manager->modelManager->create_model(map, objectData->model);
    } else {
        printf("null model\n");
        model = nullptr;
    }
}
void ObjectRef::render(ShaderType pass) {
    model->render(pass);
}

ObjectRef *ObjectManager::create_object(ProtonMap *map, HaloTagDependency tag) {
    printf("creating object %d\n", tag.tag_id.tag_index);
    
    // Has this bitmap been loaded before? Check the cache
    std::map<uint16_t, ObjectRef*>::iterator iter = this->objects.find(tag.tag_id.tag_index);
    if (iter != this->objects.end()) {
        return iter->second;
    }
    
    // Create a new texture
    ObjectRef *obj = new ObjectRef(this, map, tag);
    this->objects[tag.tag_id.tag_index] = obj;
    return obj;
}

void ObjectManager::read(ShaderManager *shaders, ProtonMap *map, ProtonTag *scenario) {
    printf("starting object manager\n");
    modelManager = new ModelManager(shaders);
    scen = new ScenClass;
    scen->read(this, map, scenario);
}

void ObjectManager::render(ShaderType pass) {
    scen->render(pass);
}

