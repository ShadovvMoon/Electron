//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "defines.h"
#include "texture.h"

#ifndef ____EShader__
#define ____EShader__

class shader {
public:
    virtual void setup() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

class shader_object;
class ShaderManager {
private:
    shader *shaders[ShaderCount];
    TextureManager *textures = nullptr;
public:
    ShaderManager();
    TextureManager *texture_manager();
    shader *get_shader(ShaderType pass);
    shader_object *create_shader(ProtonMap *map, HaloTagDependency shader);
};

class shader_object {
public:
    virtual void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) = 0;
    virtual void render() = 0;
};

GLuint make_shader(GLenum type, const char *filename);
GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);

#endif
