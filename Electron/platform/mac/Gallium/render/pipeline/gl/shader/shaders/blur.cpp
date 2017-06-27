//
//  senv.cpp
//
//
//  Created by Samuco on 19/04/2015.
//
//

#include "blur.hpp"

// Global instance
void blur::setup(std::string path) {
    printf("blur setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/blur.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/blur.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    tDiffuse        = glGetUniformLocation(program, "tDiffuse");
    tPosition       = glGetUniformLocation(program, "tPosition");
    tNormals        = glGetUniformLocation(program, "tNormals");
    tDepth          = glGetUniformLocation(program, "tDepth");
}

void blur::start(shader_options *options) {
    glUseProgram(program);
    glUniform1i(tDiffuse,0);
    glUniform1i(tPosition,1);
    glUniform1i(tNormals,2);
    glUniform1i(tDepth,3);
}
void blur::update(shader_options *options) {
}

void blur::stop() {
}

void blur_object::setBaseUV(float u, float v) {
}
void blur_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
}

// Senv object
void blur_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    printf("shader setup\n");
    blur *shader = (blur *)(manager->get_shader(shader_BLUR));
};

bool blur_object::is(ShaderType type) {
    return (type == shader_DEFF);
}
bool blur_object::render(ShaderType type) {
    return true;
}
