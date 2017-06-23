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
#include "shader.h"

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
