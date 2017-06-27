//
//  metal.cpp
//  Electron
//
//  Created by Samuco on 6/22/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#include "metal.hpp"

/// Indices for buffer bind points.
enum AAPLBufferIndex  {
    AAPLMeshVertexBuffer      = 0,
    AAPLFrameUniformBuffer    = 1,
    AAPLMaterialUniformBuffer = 2,
};
enum AAPLVertexAttributes {
    AAPLVertexAttributePosition = 0,
    AAPLVertexAttributeNormal   = 1,
    AAPLVertexAttributeBinormal = 1,
    AAPLVertexAttributeTangent  = 1,
    AAPLVertexAttributeTexcoord = 2,
};

MetalMesh::MetalMesh(id <MTLDevice> device, MTKMeshBufferAllocator *allocator, int count, int verts) {
    this->device = device;
    this->allocator = allocator;
    stride = VBCount;
    mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];
    
    // Layout
    mtlVertexDescriptor.layouts[AAPLMeshVertexBuffer].stride = VBCount * 3 * sizeof(GLfloat);
    mtlVertexDescriptor.layouts[AAPLMeshVertexBuffer].stepRate = 1;
    mtlVertexDescriptor.layouts[AAPLMeshVertexBuffer].stepFunction = MTLVertexStepFunctionPerVertex;
    
    // Data
    dataLength = VBCount * 3 * verts * sizeof(float);
    data = (float*)malloc(dataLength);
}

void MetalMesh::render(ShaderType pass, Pipeline *pipeline) {
    MetalPipeline *mpipe = (MetalPipeline*)pipeline;
    id <MTLRenderCommandEncoder> renderEncoder = mpipe->encoder;
    if (!setup || !vertexBuffer) {
        vertexBuffer = [device newBufferWithBytes:data length:dataLength options:MTLResourceOptionCPUCacheModeDefault];
        setup = true;
    }
    /*
    if (!vertexBuffer) {
        std::cerr << "missing vertex buffer" << std::endl;
        return;
    }
     */
    
    [renderEncoder pushDebugGroup:@"MetalMesh"];
    
    shader_object *previous_shader = nullptr;
    for (int i = 0; i < submesh.size(); i++) {
        MetalSubmesh *sub = submesh[i];
        if (sub->shader == nullptr && pass != shader_NULL) {
            continue;
        }
        if (sub->shader != nullptr && !sub->shader->is(pass)) {
            continue;
        }
        
        // Set up shader if necessary
        if (sub->shader && sub->shader != previous_shader) {
            sub->shader->setBaseUV(1.0, 1.0);
            if (!sub->shader->render(pass, pipeline)) {
                continue;
            }
            previous_shader = sub->shader;
        }
        
        // Bind textures
        for (int t = 0; t < sub->textures.size(); t++) {
            MetalSubmeshTexture *tex = sub->textures[t];
            if (!tex) {
                std::cerr << "null metal submesh texture bind" << std::endl;
                continue;
            }
            if (!tex->texture) {
                std::cerr << "null metal submesh texture data" << std::endl;
                continue;
            }
            
            mpipe->activeTexture = tex->index;
            tex->texture->bind(tex->mipmap, pipeline);
        }
        
        // Draw the mesh
        [renderEncoder setVertexBuffer:vertexBuffer offset:sub->vertexOffset * VBCount * 3 * sizeof(float) atIndex:0];
        [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:sub->indexCount indexType:MTLIndexTypeUInt32 indexBuffer:indexData indexBufferOffset:sub->indexOffset * sizeof(uint32_t)];
        }
    
    // Pop the debug group
    [renderEncoder popDebugGroup];
}

void MetalMesh::writeIndexData(int buffer, void *array, int vertices) {
    indexData = [device newBufferWithBytes:array length:vertices * sizeof(uint32_t) options:MTLResourceOptionCPUCacheModeDefault];
}

void MetalMesh::writeVertexData(VertexBuffer buffer, int uniform, int count, void *array, int vertices) {
    
    // Write
    switch (count) {
        case 2:
            mtlVertexDescriptor.attributes[uniform].format = MTLVertexFormatFloat2;
            break;
        case 3:
            mtlVertexDescriptor.attributes[uniform].format = MTLVertexFormatFloat3;
            break;
        default:
            std::cerr << "invalid vertex count for metal mesh" << std::endl;
            break;
    }
    mtlVertexDescriptor.attributes[uniform].offset = buffer * 3 * sizeof(float);
    mtlVertexDescriptor.attributes[uniform].bufferIndex = AAPLMeshVertexBuffer;
    
    // Copy the data into the correct stride
    for (int i = 0; i < vertices; i++) {
        for (int v = 0; v < count; v++) {
            data[(i * stride + buffer) * 3 + v] = ((float*)array)[i * count + v];
        }
    }
}

void MetalSubmesh::bindTexture(int index, texture *texture, int mipmap) {
    MetalSubmeshTexture *tex = new MetalSubmeshTexture;
    tex->index = index;
    tex->texture = texture;
    tex->mipmap = mipmap;
    textures.push_back(tex);
}

Submesh* MetalMesh::addSubmesh(shader_object *shader, int vertexOffset, int vertexCount, int indexOffset, int indexCount) {
    MetalSubmesh *mesh = new MetalSubmesh;
    mesh->shader = shader;
    mesh->vertexOffset = vertexOffset;
    mesh->vertexCount = vertexCount;
    mesh->indexOffset = indexOffset;
    mesh->indexCount = indexCount;
    submesh.push_back(mesh);
    return mesh;
}

MetalPipeline::MetalPipeline(MTKView *view, id <MTLDevice> device) {
    this->_device = device;
    allocator = [[MTKMeshBufferAllocator alloc] initWithDevice:device];
}

Mesh *MetalPipeline::createMesh(int count, int verts) {
    return new MetalMesh(_device, allocator, count, verts);
}
