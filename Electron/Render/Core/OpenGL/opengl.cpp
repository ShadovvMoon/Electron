//
//  opengl.cpp
//  Electron
//
//  Created by Samuco on 6/22/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#include "opengl.hpp"

// Bind VAO
void GLMesh::bind() {
#ifdef RENDER_VAO_NORMAL
    glBindVertexArray(geometryVAO);
#else
    glBindVertexArrayAPPLE(geometryVAO);
#endif
}

// Unbind VAO
void GLMesh::unbind() {
#ifdef RENDER_VAO_NORMAL
    glBindVertexArray(0);
#else
    glBindVertexArrayAPPLE(0);
#endif
}

GLMesh::GLMesh(int count) {
#ifdef RENDER_VAO_NORMAL
    glGenVertexArrays(1, &geometryVAO);
#else
    glGenVertexArraysAPPLE(1, &geometryVAO);
#endif
    
    bind();
    m_Buffers = (GLuint*) malloc(sizeof(GLuint) * count);
    glGenBuffers(count, m_Buffers);
    unbind();
}

void GLMesh::writeIndexData(int buffer, void *array, int vertices) {
    bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[buffer]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices * sizeof(GLint), array, GL_STATIC_DRAW);
    unbind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::writeVertexData(int buffer, int uniform, int count, void *array, int vertices){
    bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[buffer]);
    glBufferData(GL_ARRAY_BUFFER, count * vertices * sizeof(GLfloat), array, GL_STATIC_DRAW);
    glEnableVertexAttribArray(uniform);
    glVertexAttribPointer(uniform, count, GL_FLOAT, GL_FALSE, 0, 0);
    unbind();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLSubmesh::bindTexture(int index, texture *texture, int mipmap) {
    GLenum tnum;
    switch (index) {
        case 0:
            tnum = GL_TEXTURE0;
            break;
        case 1:
            tnum = GL_TEXTURE1;
            break;
        case 2:
            tnum = GL_TEXTURE2;
            break;
        case 3:
            tnum = GL_TEXTURE3;
            break;
        case 4:
            tnum = GL_TEXTURE4;
            break;
        default:
            tnum = GL_TEXTURE5;
            break;
    }
    
    GLSubmeshTexture *tex = new GLSubmeshTexture;
    tex->index = tnum;
    tex->texture = texture;
    tex->mipmap = mipmap;
    textures.push_back(tex);
}

Submesh* GLMesh::addSubmesh(shader_object *shader, int vertexOffset, int vertexCount, int indexOffset, int indexCount) {
    GLSubmesh *mesh = new GLSubmesh;
    mesh->shader = shader;
    mesh->vertexOffset = vertexOffset;
    mesh->vertexCount = vertexCount;
    mesh->indexOffset = indexOffset;
    mesh->indexCount = indexCount;
    submesh.push_back(mesh);
    return mesh;
}

void GLMesh::render(ShaderType pass) {
    errorCheck();
    bind();
    shader_object *previous_shader = nullptr;
    for (int i = 0; i < submesh.size(); i++) {
        GLSubmesh *sub = submesh[i];
        if (sub->shader == nullptr && pass != shader_NULL) {
            continue;
        }
        if (sub->shader != nullptr && !sub->shader->is(pass)) {
            continue;
        }
        
        // Set up shader if necessary
        if (sub->shader && sub->shader != previous_shader) {
            sub->shader->setBaseUV(1.0, 1.0);
            if (!sub->shader->render(pass)) {
                continue;
            }
            previous_shader = sub->shader;
        }
            
        // Bind textures
        for (int t = 0; t < sub->textures.size(); t++) {
            GLSubmeshTexture *tex = sub->textures[t];
            if (!tex) {
                std::cerr << "null opengl submesh texture bind" << std::endl;
                continue;
            }
            if (!tex->texture) {
                std::cerr << "null opengl submesh texture data" << std::endl;
                continue;
            }
            glActiveTexture(tex->index);
            tex->texture->bind(tex->mipmap);
        }

        // Draw the mesh
        glDrawRangeElementsBaseVertex(GL_TRIANGLES, sub->vertexOffset, sub->vertexOffset + sub->vertexCount, sub->indexCount, GL_UNSIGNED_INT, (void*)(sub->indexOffset * sizeof(GLuint)), sub->vertexOffset);
    }
    unbind();
    errorCheck();
}
