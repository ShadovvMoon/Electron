//
//  senv.cpp
//
//
//  Created by Samuco on 19/04/2015.
//
//

#include "deff.hpp"

// Global instance
void deff::setup(std::string path) {
    printf("deff setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/deff.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/deff.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    tDiffuse            = glGetUniformLocation(program, "tDiffuse");
    tPosition           = glGetUniformLocation(program, "tPosition");
    tNormals            = glGetUniformLocation(program, "tNormals");
    tDepth              = glGetUniformLocation(program, "tDepth");
    tSSAO               = glGetUniformLocation(program, "tSSAO");
    tReflection         = glGetUniformLocation(program, "tReflection");
    ProjectionMatrix    = glGetUniformLocation(program, "ProjectionMatrix");
    UBOCamera           = glGetUniformLocation(program, "UBOCamera");
    fog                 = glGetUniformLocation(program, "fog");
    fogSettings         = glGetUniformLocation(program, "fogSettings");
}

void deff::start(shader_options *options) {
    glUseProgram(program);
    glUniform1i(tDiffuse,0);
    glUniform1i(tPosition,1);
    glUniform1i(tNormals,2);
    glUniform1i(tSSAO,3);
    glUniform1i(tDepth,4);
    glUniform1i(tReflection,5);
    glUniform4f(fog, options->fogr, options->fogg, options->fogb, 1.0);
    glUniform2f(fogSettings, options->fogdist, options->fogcut);
    glUniformMatrix4fv(ProjectionMatrix, 1, false, options->perspective);
    glUniform3f(UBOCamera, options->camera[0], options->camera[1], options->camera[2]);
}
void deff::update(shader_options *options) {
}

void deff::stop() {
}

void deff_object::setBaseUV(float u, float v) {
}
void deff_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
}

// Senv object
void deff_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    //printf("shader setup\n");
    //deff *shader = (deff *)(manager->get_shader(shader_DEFF));
};

bool deff_object::is(ShaderType type) {
    return (type == shader_DEFF);
}
bool deff_object::render(ShaderType type) {
    return true;
}
