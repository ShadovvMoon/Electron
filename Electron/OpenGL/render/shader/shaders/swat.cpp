//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "swat.h"

// Global instance
void swat::setup(std::string path) {
    printf("swat setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/swat.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/swat.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    printf("done\n");
}

void swat::start() {
    glUseProgram(program);
}

void swat::stop() {
    
}

void swat_object::setBaseUV(float u, float v) {
}

// Senv object
void swat_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
};

bool swat_object::is(ShaderType type) {
    return (type == shader_SWAT);
}
void swat_object::render() {
}
