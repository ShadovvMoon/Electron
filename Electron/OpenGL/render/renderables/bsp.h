//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __BSP__
#define __BSP__

#include "defines.h"
#include "../shader/shader.h"
#include "../camera/vector3d.h"

class BSPRenderBuffer {
public:
    GLuint m_Buffers[7];
    GLuint geometryVAO;
    GLfloat* vertex_array;
    GLfloat* texture_uv;
    GLfloat* light_uv;
    GLfloat* normals;
    GLfloat* binormals;
    GLfloat* tangents;
    GLint* index_array;
    void setup();
};

class BSPRenderSubmesh {
public:
    GLuint m_Buffers[7];
    
    // Rendering
    GLuint geometryVAO;
    int indexCount;
    int vertCount;
    int indexOffset;
    int vertexOffset;
    int lightmap;
    
    shader_object *shader;
    void setup();
};

class BSPRenderMesh {
public:
    texture *lightTexture;
    std::vector<BSPRenderSubmesh*> submeshes;
};

class BSP {
    BSPRenderBuffer *vao;
    ShaderManager *shaders;
    std::vector<BSPRenderMesh*> renderables;
public:
    BSP(ShaderManager* manager);
    void setup(ProtonMap *map, ProtonTag *scenario);
    void render(ShaderType pass);
    
    
    vector3d *intersect(vector3d *p, vector3d *q, ProtonMap *map, ProtonTag *scenario);
};

#endif /* defined(__BSP__) */
