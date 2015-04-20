//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "senv.h"

// Global instance
void senv::setup() {
    printf("senv setup\n");
    GLuint vertex_shader, fragment_shader;
    const char *vert_path = "/Users/samuco/GitHub/Electron/Electron/OpenGL/shaders/senv.vert";
    const char *frag_path = "/Users/samuco/GitHub/Electron/Electron/OpenGL/shaders/senv.frag";
    
    // Load the shader
    vertex_shader   = make_shader(GL_VERTEX_SHADER, vert_path);
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, frag_path);
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    baseTexture = glGetUniformLocation(program, "baseTexture");
    glBindAttribLocation(program, 1, "texCoord_buffer");

    /*
    const char* attribute_name = "coord2d";
    attribute_coord2d = glGetAttribLocation(program, attribute_name);
    if (attribute_coord2d == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return;
    }
    */
}

void senv::start() {
    glUseProgram(program);
    glUniform1i(baseTexture, 0);
}

void senv::stop() {
    
}

// Senv object
void senv_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    printf("senv object setup\n");
    baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(shaderTag->Data() + 0x88));
};

void senv_object::render() {
    glActiveTexture(GL_TEXTURE0);
    baseMap->bind();
}
