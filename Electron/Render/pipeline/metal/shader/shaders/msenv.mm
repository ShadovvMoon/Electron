//
//  senv.cpp
//  Electron
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#include "msenv.h"
#include "metal.hpp"
#include "SharedStructures.h"
#include <string>

// Max API memory buffer size.
static const size_t kMaxBytesPerFrame = sizeof(float) * 256;

// Global instance
void msenv::setup(id <MTLDevice> device, MTKView *view, id <MTLLibrary> library) {
    printf("msenv setup\n");
    
    // Load the fragment program into the library
    id <MTLFunction> fragmentProgram = [library newFunctionWithName:@"lighting_fragment"];
    
    // Load the vertex program into the library
    id <MTLFunction> vertexProgram = [library newFunctionWithName:@"lighting_vertex"];
    
    // Allocate one region of memory for the uniform buffer
    _dynamicConstantBuffer = [device newBufferWithLength:kMaxBytesPerFrame options:0];
    _dynamicConstantBuffer.label = @"SenvUniform";
    
    // Set up render pipeline
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    
    // Layout
    vertexDescriptor.layouts[0].stride = VBCount * 3 * sizeof(float);
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    // Attributes
    vertexDescriptor.attributes[VBPosition].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[VBPosition].bufferIndex = 0;
    vertexDescriptor.attributes[VBPosition].offset = VBPosition * 3 * sizeof(float);
    vertexDescriptor.attributes[VBTextureCoordinate].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[VBTextureCoordinate].bufferIndex = 0;
    vertexDescriptor.attributes[VBTextureCoordinate].offset = VBTextureCoordinate * 3 * sizeof(float);
    vertexDescriptor.attributes[VBLightCoordinate].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[VBLightCoordinate].bufferIndex = 0;
    vertexDescriptor.attributes[VBLightCoordinate].offset = VBLightCoordinate * 3 * sizeof(float);
    vertexDescriptor.attributes[VBNormal].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[VBNormal].bufferIndex = 0;
    vertexDescriptor.attributes[VBNormal].offset = VBNormal * 3 * sizeof(float);
    vertexDescriptor.attributes[VBBinormal].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[VBBinormal].bufferIndex = 0;
    vertexDescriptor.attributes[VBBinormal].offset = VBBinormal * 3 * sizeof(float);
    vertexDescriptor.attributes[VBTangent].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[VBTangent].bufferIndex = 0;
    vertexDescriptor.attributes[VBTangent].offset = VBTangent * 3 * sizeof(float);
    
    // Create a reusable pipeline state
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"MetalPipeline";
    pipelineStateDescriptor.sampleCount = view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;
    pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
    
    NSError *error = NULL;
    _pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState) {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
}


matrix_float4x4 float2mat(float *input) {
    matrix_float4x4 look = matrix_identity_float4x4;
    look.columns[0] = (vector_float4) { input[0], input[1], input[2], input[3] };
    look.columns[1] = (vector_float4) { input[4], input[5], input[6], input[7] };
    look.columns[2] = (vector_float4) { input[8], input[9], input[10], input[11] };
    look.columns[3] = (vector_float4) { input[12], input[13], input[14], input[15] };
    return look;
}

void printmat(matrix_float4x4 mat) {
    std::cerr << mat.columns[0][0] << "\t" << mat.columns[1][0] << "\t" << mat.columns[2][0] << "\t" << mat.columns[3][0] << "\t" << std::endl;
    std::cerr << mat.columns[0][1] << "\t" << mat.columns[1][1] << "\t" << mat.columns[2][1] << "\t" << mat.columns[3][1] << "\t" << std::endl;
    std::cerr << mat.columns[0][2] << "\t" << mat.columns[1][2] << "\t" << mat.columns[2][2] << "\t" << mat.columns[3][2] << "\t" << std::endl;
    std::cerr << mat.columns[0][3] << "\t" << mat.columns[1][3] << "\t" << mat.columns[2][3] << "\t" << mat.columns[3][3] << "\t" << std::endl;
};

void msenv::start(shader_options *options) {
    MetalPipeline *mpipe = (MetalPipeline *)options->pipeline;
    [mpipe->encoder setRenderPipelineState:_pipelineState];
    
    // Update modelview
    uint _constantDataBufferIndex = 0;
    matrix_float4x4 look = float2mat(options->modelview);
    matrix_float4x4 _projectionMatrix = float2mat(options->perspective);
    
    // Load constant buffer data into appropriate buffer at current index
    senv_uniform *uniforms = &((senv_uniform *)[_dynamicConstantBuffer contents])[_constantDataBufferIndex];
    uniforms->shared.normal_matrix = matrix_invert(matrix_transpose(look));
    uniforms->shared.modelview_projection_matrix = matrix_multiply(_projectionMatrix, look);
    [mpipe->encoder setVertexBuffer:_dynamicConstantBuffer offset:0 atIndex:1];
    [mpipe->encoder setFragmentBuffer:_dynamicConstantBuffer offset:0 atIndex:1];
}
void msenv::update(shader_options *options) {
}
void msenv::stop() {
}

void msenv_object::setBaseUV(float u, float v) {
    options.scaleU = u;
    options.scaleV = v;
}

void msenv_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
}

// Senv object
void msenv_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    printf("senv object setup\n");
    baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(shaderTag->Data() + 0x88));
    bitmask16 environmentFlags = *(bitmask16*)(shaderTag->Data() + 0x28);
    //useBlend = (environmentFlags & 15);
    
    HaloTagDependency primary = *(HaloTagDependency*)(shaderTag->Data() + 0xB8);
    if (primary.tag_id.tag_index != NULLED_TAG_ID) {
        options.primaryScale = *(float*)(shaderTag->Data() + 0xB4);
        primaryDetailMap = manager->texture_manager()->create_texture(map, primary);
        options.mixPrimary = 1.0;
    } else {
        options.mixPrimary = 0.0;
    }
    
    printf("secondary setup\n");
    HaloTagDependency secondary = *(HaloTagDependency*)(shaderTag->Data() + 0xCC);
    if (secondary.tag_id.tag_index != NULLED_TAG_ID) {
        options.secondaryScale = *(float*)(shaderTag->Data() + 0xC8);
        secondaryDetailMap = manager->texture_manager()->create_texture(map, secondary);
        options.mixSecondary = 1.0;
    } else {
        options.mixSecondary = 0.0;
    }
    
    printf("bump setup\n");
    HaloTagDependency bump = *(HaloTagDependency*)(shaderTag->Data() + 0x128);
    if (bump.tag_id.tag_index != NULLED_TAG_ID) {
        options.bumpScale = *(float*)(shaderTag->Data() + 0x124);
        bumpMap = manager->texture_manager()->create_texture(map, bump);
        options.mixBump = 1.0;
    } else {
        options.mixBump = 0.0;
    }
    
    printf("cube setup\n");
    HaloTagDependency cube = *(HaloTagDependency*)(shaderTag->Data() + 0x324);
    if (cube.tag_id.tag_index != NULLED_TAG_ID) {
        cubeMap = manager->texture_manager()->create_cubemap(map, cube);
        options.mixCube = 1.0;
    } else {
        options.mixCube = 0.0;
    }
    //reflectionPerpendicular = *(float*)(shaderTag->Data() + 0x2F4);
    //reflectionParallel = *(float*)(shaderTag->Data() + 0x2F8);
    
    printf("shader setup\n");
    shader = (msenv *)(manager->get_shader(shader_SENV));
};

bool msenv_object::is(ShaderType type) {
    return (type == shader_SENV || type == shader_SENV_REFLECT);
}

bool msenv_object::render(ShaderType type, Pipeline *pipeline) {
    MetalPipeline *mpipe = (MetalPipeline*)pipeline;
    mpipe->activeTexture = 0;
    baseMap->bind(0, pipeline);
    if (options.mixPrimary > 0.5) {
        mpipe->activeTexture = 1;
        primaryDetailMap->bind(0, pipeline);
    }
    if (options.mixSecondary > 0.5) {
        mpipe->activeTexture = 2;
        secondaryDetailMap->bind(0, pipeline);
    }
    if (type == shader_SENV) {
        if (options.mixCube > 0.5) {
            mpipe->activeTexture = 4;
            //cubeMap->bind(pipeline);
        }
        if (options.mixBump > 0.5) {
            mpipe->activeTexture = 5;
            bumpMap->bind(0, pipeline);
        }
    }
    senv_uniform *uniforms = &((senv_uniform *)[shader->_dynamicConstantBuffer contents])[0];
    uniforms->options = options;
    return true;
}
