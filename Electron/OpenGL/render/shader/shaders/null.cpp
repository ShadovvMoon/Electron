//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "null.h"

// Global instance
void null::setup(std::string path) {
    printf("null setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/null.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/null.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    printf("done\n");
}

void null::start(shader_options *options) {
    glUseProgram(program);
}
void null::update(shader_options *options) {
    
}

void null::stop() {
    
}

void null_object::setBaseUV(float u, float v) {
}
void null_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
}

// Senv object
void null_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
};

bool null_object::is(ShaderType type) {
    return (type == shader_NULL);
}
bool null_object::render() {
    return true;
}
