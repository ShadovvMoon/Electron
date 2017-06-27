//
//  senv.cpp
//  Electron
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#include "msenv.h"

// Global instance
void msenv::setup(id <MTLLibrary> library) {
    printf("msenv setup\n");
    
    // Load the fragment program into the library
    id <MTLFunction> fragmentProgram = [library newFunctionWithName:@"lighting_fragment"];
    
    // Load the vertex program into the library
    id <MTLFunction> vertexProgram = [library newFunctionWithName:@"lighting_vertex"];

}

void msenv::start(shader_options *options) {
}
void msenv::update(shader_options *options) {
}
void msenv::stop() {
}

void msenv_object::setBaseUV(float u, float v) {
    uscale = u;
    vscale = v;
}

void msenv_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
}

// Senv object
void msenv_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    printf("senv object setup\n");
    baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(shaderTag->Data() + 0x88));
    bitmask16 environmentFlags = *(bitmask16*)(shaderTag->Data() + 0x28);
    useBlend = (environmentFlags & 15);
    
    HaloTagDependency primary = *(HaloTagDependency*)(shaderTag->Data() + 0xB8);
    if (primary.tag_id.tag_index != NULLED_TAG_ID) {
        primaryScale = *(float*)(shaderTag->Data() + 0xB4);
        primaryDetailMap = manager->texture_manager()->create_texture(map, primary);
        usePrimary = true;
    }
    
    printf("secondary setup\n");
    HaloTagDependency secondary = *(HaloTagDependency*)(shaderTag->Data() + 0xCC);
    if (secondary.tag_id.tag_index != NULLED_TAG_ID) {
        secondaryScale = *(float*)(shaderTag->Data() + 0xC8);
        secondaryDetailMap = manager->texture_manager()->create_texture(map, secondary);
        useSecondary = true;
    }
    
    printf("bump setup\n");
    HaloTagDependency bump = *(HaloTagDependency*)(shaderTag->Data() + 0x128);
    if (bump.tag_id.tag_index != NULLED_TAG_ID) {
        bumpScale = *(float*)(shaderTag->Data() + 0x124);
        bumpMap = manager->texture_manager()->create_texture(map, bump);
        useBump = true;
    }
    
    printf("cube setup\n");
    HaloTagDependency cube = *(HaloTagDependency*)(shaderTag->Data() + 0x324);
    if (cube.tag_id.tag_index != NULLED_TAG_ID) {
        cubeMap = manager->texture_manager()->create_cubemap(map, cube);
        useCube = true;
    }
    reflectionPerpendicular = *(float*)(shaderTag->Data() + 0x2F4);
    reflectionParallel = *(float*)(shaderTag->Data() + 0x2F8);
    
    printf("shader setup\n");
    msenv *shader = (msenv *)(manager->get_shader(shader_SENV));
};

bool msenv_object::is(ShaderType type) {
    return (type == shader_SENV || type == shader_SENV_REFLECT);
}

bool msenv_object::render(ShaderType type) {
    return true;
}
