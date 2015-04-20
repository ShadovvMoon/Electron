//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "soso.h"

// Global instance
void soso::setup(std::string path) {
    printf("soso setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/soso.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/soso.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    baseTexture         = glGetUniformLocation(program, "baseTexture");
    primaryDetailMap    = glGetUniformLocation(program, "primaryDetailMap");
    secondaryDetailMap  = glGetUniformLocation(program, "secondaryDetailMap");
    
    maps = glGetUniformLocation(program, "maps");
    glBindAttribLocation(program, 1, "texCoord_buffer");
}

void soso::start() {
    glUseProgram(program);
    glUniform1i(baseTexture, 0);
    glUniform1i(primaryDetailMap, 1);
    glUniform1i(secondaryDetailMap, 2);
    
}

void soso::stop() {
    
}

// Senv object
void soso_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    printf("soso object setup\n");
    baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(shaderTag->Data() + 0x88));
    
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
    
    printf("shader setup\n");
    soso *shader = (soso *)(manager->get_shader(shader_SOSO));
    mapsId = shader->maps;
};

bool soso_object::is(ShaderType type) {
    return (type == shader_SOSO);
}
void soso_object::render() {
    
    // Texturing
    glActiveTexture(GL_TEXTURE0);
    baseMap->bind();
    glActiveTexture(GL_TEXTURE1);
    if (usePrimary) {
        primaryDetailMap->bind();
    }
    glActiveTexture(GL_TEXTURE2);
    if (useSecondary) {
        secondaryDetailMap->bind();
    }
    
    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Scales
    glUniform4f(mapsId, b2f(usePrimary), primaryScale, b2f(useSecondary), secondaryScale);
}
