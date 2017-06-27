//
//  mshader.cpp
//  Metallic
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#include "mshader.hpp"
#include "texture.h"
#include "shaders/msenv.h"

MetalShaderManager::MetalShaderManager(id <MTLLibrary> library) {
    printf("shader manager setup\n");
    
    // Create the reflection
    reflections.resize(Reflections);
    int i;
    for (i=0; i < Reflections; i++) {
        glDeleteTextures(1, &reflections[i]);
        glGenTextures(1, &reflections[i]);
        glBindTexture(GL_TEXTURE_2D, reflections[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Create a texture manager
    //textures = new MetalTextureManager;
    
    // Setup our shader types
    msenv *senv_shader = new msenv;
    senv_shader->setup(library);
    shaders[shader_SENV] = (shader*)senv_shader;
}

shader_object * MetalShaderManager::create_shader(ProtonMap *map, HaloTagDependency shader) {
    // Do we already have this shader?
    std::map<uint16_t, shader_object*>::iterator iter = shader_objects.find(shader.tag_id.tag_index);
    if (iter != shader_objects.end()) {
        return iter->second;
    }
    
    ProtonTag *shaderTag = map->tags.at((uint16_t)shader.tag_id.tag_index).get();
    shader_object *shaderObj = nullptr;
    if(strncmp(shaderTag->tag_classes, "vnes", 4) == 0) { // senv shader
        shaderObj = new msenv_object;
    }
    
    if (shaderObj != nullptr) {
        shaderObj->setup(this, map, shaderTag);
    }
    shader_objects[shader.tag_id.tag_index] = shaderObj;
    return shaderObj;
}

TextureManager *MetalShaderManager::texture_manager() {
    return textures;
}

shader * MetalShaderManager::get_shader(ShaderType pass) {
    return shaders[pass];
}

GLuint MetalShaderManager::get_reflection(int index) {
    return reflections[index];
}

bool MetalShaderManager::needs_reflection() {
    return reflecting;
}

void MetalShaderManager::set_needs_reflection(bool reflect) {
    reflecting = reflect;
}

float MetalShaderManager::reflection_height() {
    return reflect_height;
}
void MetalShaderManager::set_reflection_height(float height) {
    reflect_height = height;
}
