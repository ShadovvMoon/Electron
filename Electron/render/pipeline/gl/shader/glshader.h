//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "defines.h"
#include "texture.h"
#include <map>
#include <chrono>
#include "pipeline.hpp"

#ifndef ____EShader__
#define ____EShader__

class GLShaderManager: public ShaderManager {
private:
    std::map<uint16_t, shader_object*> shader_objects;
    std::map<uint16_t, shader_object*> reflect_objects;
    std::vector<uint> reflections;
    shader *shaders[ShaderCount];
    TextureManager *textures = nullptr;
    bool reflecting = false;
    float reflect_height = 0.0;
    
public:
    GLShaderManager(const char *resources);
    TextureManager *texture_manager();
    shader *get_shader(ShaderType pass);

    bool needs_reflection();
    float reflection_height();
    void set_needs_reflection(bool reflect);
    void set_reflection_height(float height);
    uint get_reflection(int index);
    shader_object *create_shader(ProtonMap *map, HaloTagDependency shader);
};

float b2f(bool b);
uint make_shader(uint type, const char *filename);
uint make_program(uint vertex_shader, uint fragment_shader);

#endif
