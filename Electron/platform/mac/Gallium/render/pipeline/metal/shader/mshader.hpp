//
//  mshader.hpp
//  Metallic
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef mshader_hpp
#define mshader_hpp

// Cocoa
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

// Electron
#include "defines.h"
#include <stdio.h>
#include <string>
#include <map>
#include <chrono>
#include "pipeline.hpp"

class MetalShaderManager: public ShaderManager {
private:
    std::map<uint16_t, shader_object*> shader_objects;
    std::map<uint16_t, shader_object*> reflect_objects;
    std::vector<uint> reflections;
    shader *shaders[ShaderCount];
    TextureManager *textures = nullptr;
    bool reflecting = false;
    float reflect_height = 0.0;
public:
    MetalShaderManager(id <MTLLibrary> library);
    TextureManager *texture_manager();
    shader *get_shader(ShaderType pass);
    
    bool needs_reflection();
    float reflection_height();
    void set_needs_reflection(bool reflect);
    void set_reflection_height(float height);
    uint get_reflection(int index);
    shader_object *create_shader(ProtonMap *map, HaloTagDependency shader);
};

#endif /* mshader_hpp */
