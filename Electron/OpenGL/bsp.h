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
#include "shader.h"

class BSPRenderMesh {
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


class BSP {
    ShaderManager *shaders;
    std::vector<BSPRenderMesh*> renderables;
public:
    BSP(ShaderManager* manager);
    void setup(ProtonMap *map, ProtonTag *scenario);
    void render(ProtonMap *map, ProtonTag *scenario);
};

#endif /* defined(__BSP__) */
