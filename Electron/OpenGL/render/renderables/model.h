//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __MODEL__
#define __MODEL__

#include "defines.h"
#include "shader.h"

class GeometryRenderMesh {
public:
    GLuint m_Buffers[5];
    GLuint geometryVAO;
    int indexCount;
    int vertCount;
    
    GLfloat* vertex_array;
    GLfloat* texture_uv;
    GLfloat* light_uv;
    GLfloat* normals;
    GLint* index_array;
    
    shader_object *shader;
    void setup();
};

class Geometry {
public:
    std::vector<GeometryRenderMesh*> renderables;
    Geometry(uint8_t *offset);
};

class Model {
public:
    std::vector<Geometry*> geometries;
    Model(ProtonMap *map, HaloTagDependency tag);
};

class ModelManager {
private:
    std::map<uint16_t, Model*> models;
public:
    Model *create_model(ProtonMap *map, HaloTagDependency tag);
};

#endif /* defined(__BSP__) */
