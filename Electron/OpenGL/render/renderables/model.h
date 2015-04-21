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

class ModelRenderMesh {
public:
    GLuint m_Buffers[5];
    
    // Rendering
    GLuint geometryVAO;
    int indexCount;
    int vertCount;
    
    // Arrays
    GLfloat* vertex_array;
    GLfloat* texture_uv;
    GLfloat* light_uv;
    GLfloat* normals;
    GLint* index_array;
    
    shader_object *shader;
    void setup();
};

class ModelManager;
class Model {
public:
    std::vector<ModelRenderMesh*> renderables;
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
