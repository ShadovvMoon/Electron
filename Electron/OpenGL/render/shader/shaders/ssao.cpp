//
//  senv.cpp
//
//
//  Created by Samuco on 19/04/2015.
//
//

#include "ssao.hpp"

// Global instance
void ssao::setup(std::string path) {
    printf("deff setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/ssao.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/ssao.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    tDiffuse        = glGetUniformLocation(program, "tDiffuse");
    tPosition       = glGetUniformLocation(program, "tPosition");
    tNormals        = glGetUniformLocation(program, "tNormals");
    tDepth          = glGetUniformLocation(program, "tDepth");
}

void ssao::start(shader_options *options) {
    glUseProgram(program);
    glUniform1i(tDiffuse,0);
    glUniform1i(tPosition,1);
    glUniform1i(tNormals,2);
    glUniform1i(tDepth,3);
}
void ssao::update(shader_options *options) {
}

void ssao::stop() {
}

void ssao_object::setBaseUV(float u, float v) {
}
void ssao_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
}

// Senv object
void ssao_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    //printf("shader setup\n");
    //ssao *shader = (ssao *)(manager->get_shader(shader_SSAO));
};

bool ssao_object::is(ShaderType type) {
    return (type == shader_DEFF);
}
bool ssao_object::render(ShaderType type) {
    return true;
}
