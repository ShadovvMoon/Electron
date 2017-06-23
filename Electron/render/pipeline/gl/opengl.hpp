//
//  opengl.hpp
//  Electron
//
//  Created by Samuco on 6/22/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef opengl_hpp
#define opengl_hpp

// Include OpenGL
#include "defines.h"
#ifdef _WINDOWS
#define _USE_MATH_DEFINES
#include "glew/GL/glew.h"
#define RENDER_VAO_NORMAL
#elif __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

// Mesh pipeline
#include <stdio.h>
#include "pipeline.hpp"

class GLSubmeshTexture {
public:
    GLenum index;
    texture *texture;
    int mipmap;
};

class GLSubmesh: public Submesh {
public:
    shader_object *shader = nullptr;
    
    // Buffer
    int vertexOffset;
    int vertexCount;
    int indexOffset;
    int indexCount;
    
    // Texture
    std::vector<GLSubmeshTexture*> textures;
    void bindTexture(int index, texture *texture, int mipmap);
};

class GLMesh: public Mesh {
private:
    GLuint *m_Buffers;
    GLuint geometryVAO;
    std::vector<GLSubmesh*> submesh;
    
    void bind();
    void unbind();
public:
    GLMesh(int count);
    void render(ShaderType pass);
    
    Submesh* addSubmesh(shader_object *shader, int vertexOffset, int vertexCount, int indexOffset, int indexCount);
    void writeIndexData(int buffer, void *array, int vertices);
    void writeVertexData(int buffer, int uniform, int count, void *array, int vertices);
};

#endif /* opengl_hpp */
