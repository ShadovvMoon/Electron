//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "defines.h"

#ifndef ____EShader__
#define ____EShader__

class shader {
public:
    void setup();
    void start();
    void stop();
};

class shader_object {
public:
    void setup(ProtonMap *map, ProtonTag *shaderTag);
    void render();
};

void load_shaders();
GLuint make_shader(GLenum type, const char *filename);
GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);
shader *get_shader(ShaderType pass);
GLuint load_bitm(HaloTagDependency bitm);
shader_object *load_shader(ProtonMap *map, HaloTagDependency shader);

#endif
