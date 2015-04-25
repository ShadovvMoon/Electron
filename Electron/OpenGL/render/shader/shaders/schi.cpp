//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "schi.h"

// Global instance
void schi::setup(std::string path) {
    printf("schi setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/schi.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/schi.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    baseTexture     = glGetUniformLocation(program, "baseTexture");
    maps = glGetUniformLocation(program, "maps");
    //glBindAttribLocation(program, 1, "texCoord_buffer");
}

void schi::start() {
    glUseProgram(program);
    glUniform1i(baseTexture, 0);
    errorCheck();
}

void schi::stop() {
    
}

void schi_object::setBaseUV(float u, float v) {
}

// Senv object
void schi_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    
    // SCHI has lots of keys
    HaloTagReflexive maps = *(HaloTagReflexive*)(shaderTag->Data() + 0x54);
    int i;
    for (i=0; i < maps.count; i++) {
        uint8_t *offset = (uint8_t *)shaderTag->Data() + shaderTag->PointerToOffset(maps.address);
        baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(offset + 0x6C));
        break;
    }
    
    printf("schi object setup\n");
    
    printf("shader setup\n");
    schi *shader = (schi *)(manager->get_shader(shader_SCHI));
    mapsId = shader->maps;
};

bool schi_object::is(ShaderType type) {
    return (type == shader_SCHI);
}

void schi_object::render() {
    
    // Texturing
    glActiveTexture(GL_TEXTURE0);
    baseMap->bind();

    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Scales
    glUniform4f(mapsId, 1.0, 1.0, 1.0, 1.0);
}
