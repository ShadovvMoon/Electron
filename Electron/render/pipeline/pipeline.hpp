//
//  pipeline.hpp
//  Electron
//
//  Created by Samuco on 6/22/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef pipeline_hpp
#define pipeline_hpp

#include <stdio.h>
#include "texture.h"

// Pipeline


// Shaders
class shader {
public:
    int program;
    virtual void start(shader_options *options) = 0;
    virtual void update(shader_options *options) = 0;
    virtual void stop() = 0;
};

class ShaderManager;
class shader_object {
public:
    virtual void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) = 0;
    virtual void setBaseUV(float u, float v) = 0;
    virtual void setFogSettings(float r, float g, float b, float distance, float cutoff) = 0;
    virtual bool render(ShaderType type) = 0;
    virtual bool is(ShaderType type) = 0;
};

class ShaderManager {
public:
    //virtual ShaderManager(const char *resources) = 0;
    virtual TextureManager *texture_manager() = 0;
    virtual shader *get_shader(ShaderType pass) = 0;
    virtual bool needs_reflection() = 0;
    virtual float reflection_height() = 0;
    virtual void set_needs_reflection(bool reflect) = 0;
    virtual void set_reflection_height(float height) = 0;
    virtual uint get_reflection(int index) = 0;
    virtual shader_object *create_shader(ProtonMap *map, HaloTagDependency shader) = 0;
};

// Mesh
class Submesh {
public:
    virtual void bindTexture(int index, texture *texture, int mipmap) = 0;
};

class Mesh {
public:
    virtual void render(ShaderType pass) = 0;
    virtual void writeIndexData(int i, void *array, int vertices) = 0;
    virtual void writeVertexData(int buffer, int uniform, int count, void *array, int vertices) = 0;
    virtual Submesh* addSubmesh(shader_object *shader, int vertexOffset, int vertexCount, int indexOffset, int indexCount) = 0;
};

class Pipeline {
public:
    virtual Mesh *createMesh(int count) = 0;
};

#endif /* pipeline_hpp */
