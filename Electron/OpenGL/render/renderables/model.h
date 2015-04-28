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
#include "../shader/shader.h"

class ModelRenderMesh {
public:
    GLuint m_Buffers[7];
    
    float base_u;
    float base_v;
    
    // Rendering
    GLuint geometryVAO;
    int indexCount;
    int vertCount;
    
    // Arrays
    GLfloat* vertex_array;
    GLfloat* texture_uv;
    GLfloat* light_uv;
    GLfloat* normals;
    GLfloat* binormals;
    GLfloat* tangents;
    GLint* index_array;
    
    shader_object *shader;
    void setup();
};

class ModelManager;
class Model {
private:
    bool ready = false;
    const char *name;
    std::vector<shader_object*> shaders;
    std::vector<uint8_t> renderIndices;
public:
    ModelRenderMesh *** geom;
    uint16_t *geomCount;
    
    std::vector<std::vector<ModelRenderMesh*>> geometries;
    Model(ModelManager *manager, ProtonMap *map, HaloTagDependency tag);
    void render(ShaderType pass);
};

class ModelManager {
private:
    std::map<uint16_t, Model*> models;
public:
    ShaderManager *shaders;
    ModelManager(ShaderManager* manager);
    Model *create_model(ProtonMap *map, HaloTagDependency tag);
};

#endif /* defined(__BSP__) */
