//
//  metal.hpp
//  Electron
//
//  Created by Samuco on 6/22/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef metal_hpp
#define metal_hpp
#include "defines.h"
#include "pipeline.hpp"
#include <stdio.h>
#include <MetalKit/MetalKit.h>

class MetalSubmeshTexture {
public:
    GLenum index;
    texture *texture;
    int mipmap;
};

class MetalSubmesh: public Submesh {
public:
    shader_object *shader = nullptr;
    
    // Buffer
    int vertexOffset;
    int vertexCount;
    int indexOffset;
    int indexCount;
    
    // Texture
    std::vector<MetalSubmeshTexture*> textures;
    void bindTexture(int index, texture *texture, int mipmap);
};

class MetalMesh: public Mesh  {
    id <MTLDevice> device;
    MTKMeshBufferAllocator *allocator;
    
    MTLVertexDescriptor *mtlVertexDescriptor;
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> indexData;
    bool setup;
    
    std::vector<MetalSubmesh*> submesh;
    float *data;
    int dataLength = 0;
    int stride;
public:
    MetalMesh(id <MTLDevice> device, MTKMeshBufferAllocator *allocator, int count, int verts);
    void render(ShaderType pass, Pipeline *pipeline);
    Submesh* addSubmesh(shader_object *shader, int vertexOffset, int vertexCount, int indexOffset, int indexCount);
    void writeIndexData(int buffer, void *array, int vertices);
    void writeVertexData(VertexBuffer buffer, int uniform, int count, void *array, int vertices);
};

class MetalPipeline: public Pipeline {
private:
    id <MTLDevice> _device;
    MTKMeshBufferAllocator *allocator;
public:
    uint activeTexture;
    id <MTLRenderCommandEncoder> encoder;
    
    MetalPipeline(MTKView *view, id <MTLDevice> device);
    Mesh *createMesh(int count, int verts);
};

#endif /* metal_hpp */
