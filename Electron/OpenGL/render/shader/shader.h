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

typedef struct {
    float fogr, fogg, fogb;
    float fogdist, fogcut;
} shader_options;

class shader {
public:
    virtual void setup(std::string path) = 0;
    virtual void start(shader_options *options) = 0;
    virtual void stop() = 0;
};

class ShaderManager;
class shader_object {
public:
    virtual void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) = 0;
    virtual void setBaseUV(float u, float v) = 0;
    virtual void setFogSettings(float r, float g, float b, float distance, float cutoff) = 0;
    virtual bool render() = 0;
    virtual bool is(ShaderType type) = 0;
};

class ShaderManager {
private:
    std::map<uint16_t, shader_object*> shader_objects;
    std::vector<GLuint> reflections;
    shader *shaders[ShaderCount];
    TextureManager *textures = nullptr;
    bool reflecting = false;
    float reflect_height = 0.0;
public:
    ShaderManager(const char *resources);
    TextureManager *texture_manager();
    shader *get_shader(ShaderType pass);

    bool needs_reflection();
    float reflection_height();
    void set_needs_reflection(bool reflect);
    void set_reflection_height(float height);
    GLuint get_reflection(int index);
    shader_object *create_shader(ProtonMap *map, HaloTagDependency shader);
};

float b2f(bool b);
GLuint make_shader(GLenum type, const char *filename);
GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);

#endif
