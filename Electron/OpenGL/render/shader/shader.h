//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "defines.h"
#include "../texture/texture.h"
#include <map>

#ifndef ____EShader__
#define ____EShader__

class shader {
public:
    virtual void setup(std::string path) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

class ShaderManager;
class shader_object {
public:
    virtual void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) = 0;
    virtual void setBaseUV(float u, float v) = 0;
    virtual void render() = 0;
    virtual bool is(ShaderType type) = 0;
};

class ShaderManager {
private:
    std::map<uint16_t, shader_object*> shader_objects;
    shader *shaders[ShaderCount];
    TextureManager *textures = nullptr;
public:
    ShaderManager(const char *resources);
    TextureManager *texture_manager();
    shader *get_shader(ShaderType pass);
    shader_object *create_shader(ProtonMap *map, HaloTagDependency shader);
};

float b2f(bool b);
GLuint make_shader(GLenum type, const char *filename);
GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);

#endif
